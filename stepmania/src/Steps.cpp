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
#include "RageUtil.h"
#include "RageLog.h"
#include "NoteData.h"
#include "RageException.h"
#include "GameManager.h"
#include "NoteDataUtil.h"
#include "NotesLoaderSM.h"

Steps::Steps()
{
	m_bSavedToDisk = false;
	m_StepsType = STEPS_TYPE_INVALID;
	m_LoadedFromProfile = PROFILE_SLOT_INVALID;
	m_uHash = 0;
	m_Difficulty = DIFFICULTY_INVALID;
	m_iMeter = 0;

	m_NoteData.Init();
	m_bNoteDataIsFilled = false;
	m_sNoteDataCompressed = "";
	parent = NULL;
}

Steps::~Steps()
{
}

void Steps::SetNoteData( const NoteData& noteDataNew )
{
	ASSERT( noteDataNew.GetNumTracks() == GameManager::StepsTypeToNumTracks(m_StepsType) );

	DeAutogen();

	m_NoteData = noteDataNew;
	m_bNoteDataIsFilled = true;
	
	NoteDataUtil::GetSMNoteDataString( m_NoteData, m_sNoteDataCompressed );
	m_uHash = GetHashForString( m_sNoteDataCompressed );
}

void Steps::GetNoteData( NoteData& noteDataOut ) const
{
	ASSERT(this);

	Decompress();

	if( m_bNoteDataIsFilled )
	{
		noteDataOut = m_NoteData;
	}
	else
	{
		noteDataOut.ClearAll();
		noteDataOut.SetNumTracks( GameManager::StepsTypeToNumTracks(m_StepsType) );
	}
}

void Steps::SetSMNoteData( const CString &notes_comp_ )
{
	m_NoteData.Init();
	m_bNoteDataIsFilled = false;

	m_sNoteDataCompressed = notes_comp_;
	m_uHash = GetHashForString( m_sNoteDataCompressed );
}

/* XXX: this function should pull data from m_sFilename, like Decompress() */
void Steps::GetSMNoteData( CString &notes_comp_out ) const
{
	if( m_sNoteDataCompressed.empty() )
	{
		if( !m_bNoteDataIsFilled ) 
		{
			/* no data is no data */
			notes_comp_out = "";
			return;
		}

		NoteDataUtil::GetSMNoteDataString( m_NoteData, m_sNoteDataCompressed );
	}

	notes_comp_out = m_sNoteDataCompressed;
}

float Steps::PredictMeter() const
{
	float pMeter = 0.775f;
	
	const float RadarCoeffs[NUM_RADAR_CATEGORIES] =
	{
		10.1f, 5.27f,-0.905f, -1.10f, 2.86f,
		0,0,0,0,0,0
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
}

void Steps::CalculateRadarValues( float fMusicLengthSeconds )
{
	m_CachedRadarValues = RadarValues();
	
	// If we're autogen, don't calculate values.  GetRadarValues will take from our parent.
	if( parent != NULL )
		return;

	// If we're an edit, leave the RadarValues invalid.
	if( IsAnEdit() )
		return;

	NoteData tempNoteData;
	this->GetNoteData( tempNoteData );
	NoteDataUtil::CalculateRadarValues( tempNoteData, fMusicLengthSeconds, m_CachedRadarValues );
}

void Steps::Decompress() const
{
	if( m_bNoteDataIsFilled )
		return;	// already decompressed

	if(parent)
	{
		// get autogen m_NoteData
		NoteData notedata;
		parent->GetNoteData( notedata );

		m_bNoteDataIsFilled = true;

		int iNewTracks = GameManager::StepsTypeToNumTracks(m_StepsType);

		if( this->m_StepsType == STEPS_TYPE_LIGHTS_CABINET )
		{
			NoteDataUtil::LoadTransformedLights( notedata, m_NoteData, iNewTracks );
		}
		else
		{
			NoteDataUtil::LoadTransformedSlidingWindow( notedata, m_NoteData, iNewTracks );

			NoteDataUtil::RemoveStretch( m_NoteData, m_StepsType );
		}
		return;
	}

	if( !m_sFilename.empty() && m_sNoteDataCompressed.empty() )
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

		pSteps->GetSMNoteData( m_sNoteDataCompressed );
	}

	if( m_sNoteDataCompressed.empty() )
	{
		/* there is no data, do nothing */
	}
	else
	{
		// load from compressed
		m_bNoteDataIsFilled = true;
		m_NoteData.SetNumTracks( GameManager::StepsTypeToNumTracks(m_StepsType) );

		NoteDataUtil::LoadFromSMNoteDataString( m_NoteData, m_sNoteDataCompressed );
	}
}

void Steps::Compress() const
{
	/* Always leave lights data uncompressed. */
	if( this->m_StepsType == STEPS_TYPE_LIGHTS_CABINET && m_bNoteDataIsFilled )
	{
		m_sNoteDataCompressed = CString("");
		return;
	}

	if( !m_sFilename.empty() )
	{
		/* We have a file on disk; clear all data in memory. */
		m_NoteData.Init();
		m_bNoteDataIsFilled = false;

		/* Be careful; 'x = ""', m_sNoteDataCompressed.clear() and m_sNoteDataCompressed.reserve(0)
		 * don't always free the alocated memory. */
		m_sNoteDataCompressed = CString("");
		return;
	}

	/* We have no file on disk.  XXX: check whether there's any memory benefit to storing
	 * data in SMData form.  We don't want to decompress everything on load (much too slow),
	 * but there may be litle benefit to recompressing here. */
	if( m_sNoteDataCompressed.empty() )
	{
		if( !m_bNoteDataIsFilled )
			return; /* no data is no data */
		NoteDataUtil::GetSMNoteDataString( m_NoteData, m_sNoteDataCompressed );
	}

	m_NoteData.Init();
	m_bNoteDataIsFilled = false;
}

/* Copy our parent's data.  This is done when we're being changed from autogen
 * to normal. (needed?) */
void Steps::DeAutogen()
{
	if(!parent)
		return; /* OK */

	Decompress();	// fills in m_NoteData with sliding window transform

	m_sDescription	= Real()->m_sDescription;
	m_Difficulty	= Real()->m_Difficulty;
	m_iMeter		= Real()->m_iMeter;
	m_CachedRadarValues   = Real()->m_CachedRadarValues;

	parent = NULL;

	Compress();
}

void Steps::AutogenFrom( const Steps *parent_, StepsType ntTo )
{
	parent = parent_;
	m_StepsType = ntTo;
}

void Steps::CopyFrom( Steps* pSource, StepsType ntTo, float fMusicLengthSeconds )	// pSource does not have to be of the same StepsType
{
	m_StepsType = ntTo;
	NoteData noteData;
	pSource->GetNoteData( noteData );
	noteData.SetNumTracks( GameManager::StepsTypeToNumTracks(ntTo) ); 
	this->SetNoteData( noteData );
	this->SetDescription( pSource->GetDescription() );
	this->SetDifficulty( pSource->GetDifficulty() );
	this->SetMeter( pSource->GetMeter() );
	this->CalculateRadarValues( fMusicLengthSeconds );
}

void Steps::CreateBlank( StepsType ntTo )
{
	m_StepsType = ntTo;
	NoteData noteData;
	noteData.SetNumTracks( GameManager::StepsTypeToNumTracks(ntTo) );
	this->SetNoteData( noteData );
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

void Steps::SetDifficultyAndDescription( Difficulty dc, CString sDescription )
{
	DeAutogen();
	m_Difficulty = dc;
	m_sDescription = sDescription;
	if( GetDifficulty() == DIFFICULTY_EDIT )
		MakeValidEditDescription( m_sDescription );
}

bool Steps::MakeValidEditDescription( CString &sPreferredDescription )
{
	if( int(sPreferredDescription.size()) > MAX_EDIT_DESCRIPTION_LENGTH )
	{
		sPreferredDescription = sPreferredDescription.Left( MAX_EDIT_DESCRIPTION_LENGTH );
		return true;
	}
	return false;
}

void Steps::SetMeter(int meter)
{
	DeAutogen();
	m_iMeter = meter;
}

void Steps::SetCachedRadarValues( const RadarValues& v )
{
	DeAutogen();
	m_CachedRadarValues = v;
}


// lua start
#include "LuaBinding.h"

class LunaSteps: public Luna<Steps>
{
public:
	LunaSteps() { LUA->Register( Register ); }

	static int GetStepsType( T* p, lua_State *L )	{ lua_pushnumber(L, p->m_StepsType ); return 1; }
	static int GetDifficulty( T* p, lua_State *L )	{ lua_pushnumber(L, p->GetDifficulty() ); return 1; }
	static int GetDescription( T* p, lua_State *L )	{ lua_pushstring(L, p->GetDescription() ); return 1; }
	static int GetMeter( T* p, lua_State *L )		{ lua_pushnumber(L, p->GetMeter() ); return 1; }

	static void Register(lua_State *L)
	{
		ADD_METHOD( GetStepsType )
		ADD_METHOD( GetDifficulty )
		ADD_METHOD( GetDescription )
		ADD_METHOD( GetMeter )
		Luna<T>::Register( L );
	}
};

LUA_REGISTER_CLASS( Steps )
// lua end


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
