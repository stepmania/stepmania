/*
 * This stores a single note pattern for a song.
 *
 * We can have too much data to keep everything decompressed as NoteData, so most
 * songs are kept in memory compressed as SMData until requested.  NoteData is normally
 * not requested casually during gameplay; we can move through screens, the music
 * wheel, etc. without touching any NoteData.
 *
 * To save more memory, if data is cached on disk, read it from disk on demand.  Not
 * all Steps will have an associated file for this purpose.  (Profile edits don't do
 * this yet.)
 *
 * Data can be on disk (always compressed), compressed in memory, and uncompressed in
 * memory.
 */
#include "global.h"
#include "Steps.h"
#include "StepsUtil.h"
#include "song.h"
#include "Steps.h"
#include "IniFile.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "NoteData.h"
#include "GameInput.h"
#include "RageException.h"
#include "MsdFile.h"
#include "GameManager.h"
#include "NoteDataUtil.h"
#include "ProfileManager.h"
#include "PrefsManager.h"
#include "NotesLoaderSM.h"

const int MAX_DESCRIPTION_LENGTH = 20;

Steps::Steps()
{
	m_StepsType = STEPS_TYPE_INVALID;
	m_LoadedFromProfile = PROFILE_SLOT_INVALID;
	m_uHash = 0;
	m_Difficulty = DIFFICULTY_INVALID;
	m_iMeter = 0;

	notes = NULL;
	notes_comp = NULL;
	parent = NULL;
}

Steps::~Steps()
{
	delete notes;
	delete notes_comp;
}

void Steps::SetNoteData( const NoteData* pNewNoteData )
{
	ASSERT( pNewNoteData->GetNumTracks() == GameManager::StepsTypeToNumTracks(m_StepsType) );

	DeAutogen();

	delete notes;
	notes = new NoteData(*pNewNoteData);
	
	delete notes_comp;
	notes_comp = new CompressedNoteData;
	NoteDataUtil::GetSMNoteDataString( *notes, notes_comp->notes, notes_comp->attacks );
	m_uHash = GetHashForString( notes_comp->notes );
}

void Steps::GetNoteData( NoteData* pNoteDataOut ) const
{
	ASSERT(this);
	ASSERT(pNoteDataOut);

	Decompress();

	if( notes != NULL )
		*pNoteDataOut = *notes;
	else
	{
		pNoteDataOut->ClearAll();
		pNoteDataOut->SetNumTracks( GameManager::StepsTypeToNumTracks(m_StepsType) );
	}
}

void Steps::SetSMNoteData( const CString &notes_comp_, const CString &attacks_comp_ )
{
	delete notes;
	notes = NULL;

	if(!notes_comp)
		notes_comp = new CompressedNoteData;

	notes_comp->notes = notes_comp_;
	notes_comp->attacks = attacks_comp_;
	m_uHash = GetHashForString( notes_comp->notes );
}

void Steps::GetSMNoteData( CString &notes_comp_out, CString &attacks_comp_out ) const
{
	if(!notes_comp)
	{
		if(!notes) 
		{
			/* no data is no data */
			notes_comp_out = attacks_comp_out = "";
			return;
		}

		notes_comp = new CompressedNoteData;
		NoteDataUtil::GetSMNoteDataString( *notes, notes_comp->notes, notes_comp->attacks );
	}

	notes_comp_out = notes_comp->notes;
	attacks_comp_out = notes_comp->attacks;
}

float Steps::PredictMeter() const
{
	float pMeter = 0.775f;
	
	const float RadarCoeffs[NUM_RADAR_CATEGORIES] =
	{
		10.1f, 5.27f,-0.905f, -1.10f, 2.86f,
		0,0,0,0,0
	};
	for( int r = 0; r < NUM_RADAR_CATEGORIES; ++r )
		pMeter += this->GetRadarValues()[r] * RadarCoeffs[r];
	
	const float DifficultyCoeffs[NUM_DIFFICULTIES] =
	{
		-0.877f, -0.877f, 0, 0.722f, 0.722f, 0
	};
	pMeter += DifficultyCoeffs[this->GetDifficulty()];
	
	// Init non-radar values
	const float SV = this->GetRadarValues()[RADAR_STREAM] * this->GetRadarValues()[RADAR_VOLTAGE];
	const float ChaosSquare = this->GetRadarValues()[RADAR_CHAOS] * this->GetRadarValues()[RADAR_CHAOS];
	pMeter += -6.35f * SV;
	pMeter += -2.58f * ChaosSquare;
	if (pMeter < 1) pMeter = 1;	
	return pMeter;
}

void Steps::TidyUpData()
{
	if( GetDifficulty() == DIFFICULTY_INVALID )
		SetDifficulty( StringToDifficulty(GetDescription()) );
	
	if( GetDifficulty() == DIFFICULTY_INVALID )
	{
		if(		 GetMeter() == 1 )	SetDifficulty( DIFFICULTY_BEGINNER );
		else if( GetMeter() <= 3 )	SetDifficulty( DIFFICULTY_EASY );
		else if( GetMeter() <= 6 )	SetDifficulty( DIFFICULTY_MEDIUM );
		else						SetDifficulty( DIFFICULTY_HARD );
	}

	if( GetMeter() < 1) // meter is invalid
		SetMeter( int(PredictMeter()) );

	if( int(m_sDescription.size()) > MAX_DESCRIPTION_LENGTH )
		m_sDescription = m_sDescription.Left( MAX_DESCRIPTION_LENGTH );
}

void Steps::Decompress() const
{
	if(notes)
		return;	// already decompressed

	if(parent)
	{
		// get autogen notes
		NoteData pdata;
		parent->GetNoteData(&pdata);

		notes = new NoteData;

		int iNewTracks = GameManager::StepsTypeToNumTracks(m_StepsType);

		if( this->m_StepsType == STEPS_TYPE_LIGHTS_CABINET )
		{
			NoteDataUtil::LoadTransformedLights( pdata, *notes, iNewTracks );
		} else {
			NoteDataUtil::LoadTransformedSlidingWindow( pdata, *notes, iNewTracks );

			NoteDataUtil::FixImpossibleRows( *notes, m_StepsType );
		}
		return;
	}

	if( !m_sFilename.empty() && notes_comp == NULL )
	{
		/* We have data on disk and not in memory.  Load it. */
		Song s;
		SMLoader ld;
		if( !ld.LoadFromSMFile( m_sFilename, s, true ) )
		{
			LOG->Warn( "Couldn't load \"%s\"", m_sFilename.c_str() );
			return;
		}

		/* Find the steps. */
		StepsID ID;
		ID.FromSteps( this );

		Steps *pSteps = ID.ToSteps( &s, true, false );	// don't use cache
		if( pSteps == NULL )
		{
			LOG->Warn( "Couldn't find %s in \"%s\"", ID.ToString().c_str(), m_sFilename.c_str() );
			return;
		}

		notes_comp = new CompressedNoteData;
		pSteps->GetSMNoteData( notes_comp->notes, notes_comp->attacks );
	}

	if( notes_comp == NULL )
	{
		/* there is no data, do nothing */
	}
	else
	{
		// load from compressed
		notes = new NoteData;
		notes->SetNumTracks( GameManager::StepsTypeToNumTracks(m_StepsType) );

		NoteDataUtil::LoadFromSMNoteDataString(*notes, notes_comp->notes, notes_comp->attacks );
	}
}

void Steps::Compress() const
{
	if( !m_sFilename.empty() )
	{
		/* We have a file on disk; clear all data in memory. */
		delete notes;
		notes = NULL;
		delete notes_comp;
		notes_comp = NULL;
		return;
	}

	if(!notes_comp)
	{
		if(!notes) return; /* no data is no data */
		notes_comp = new CompressedNoteData;
		NoteDataUtil::GetSMNoteDataString( *notes, notes_comp->notes, notes_comp->attacks );
	}

	delete notes;
	notes = NULL;
}

/* Copy our parent's data.  This is done when we're being changed from autogen
 * to normal. (needed?) */
void Steps::DeAutogen()
{
	if(!parent)
		return; /* OK */

	Decompress();	// fills in notes with sliding window transform

	m_sDescription	= Real()->m_sDescription;
	m_Difficulty	= Real()->m_Difficulty;
	m_iMeter		= Real()->m_iMeter;
	m_RadarValues   = Real()->m_RadarValues;

	parent = NULL;

	Compress();
}

void Steps::AutogenFrom( const Steps *parent_, StepsType ntTo )
{
	parent = parent_;
	m_StepsType = ntTo;
}

void Steps::CopyFrom( Steps* pSource, StepsType ntTo )	// pSource does not have to be of the same StepsType!
{
	m_StepsType = ntTo;
	NoteData noteData;
	pSource->GetNoteData( &noteData );
	noteData.SetNumTracks( GameManager::StepsTypeToNumTracks(ntTo) ); 
	this->SetNoteData( &noteData );
	this->SetDescription( "Copied from "+pSource->GetDescription() );
	this->SetDifficulty( pSource->GetDifficulty() );
	this->SetMeter( pSource->GetMeter() );
	this->SetRadarValues( pSource->GetRadarValues() );
}

void Steps::CreateBlank( StepsType ntTo )
{
	m_StepsType = ntTo;
	NoteData noteData;
	noteData.SetNumTracks( GameManager::StepsTypeToNumTracks(ntTo) );
	this->SetNoteData( &noteData );
}


const Steps *Steps::Real() const
{
	ASSERT( this );
	if(parent) return parent;
	return this;
}

bool Steps::IsAutogen() const
{
	return parent != NULL;
}

void Steps::SetFile( CString fn )
{
	m_sFilename = fn;
}

void Steps::SetDescription(CString desc)
{
	DeAutogen();
	m_sDescription = desc;
}

void Steps::SetDifficulty(Difficulty d)
{
	DeAutogen();
	m_Difficulty = d;
}

void Steps::SetMeter(int meter)
{
	DeAutogen();
	m_iMeter = meter;
}

void Steps::SetRadarValues( const RadarValues& v )
{
	DeAutogen();
	m_RadarValues = v;
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard, David Wilson
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
