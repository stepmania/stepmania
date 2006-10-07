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
#include "RageUtil.h"
#include "RageLog.h"
#include "NoteData.h"
#include "GameManager.h"
#include "NoteDataUtil.h"
#include "NotesLoaderSM.h"

#include <algorithm>

Steps::Steps()
{
	m_bSavedToDisk = false;
	m_StepsType = StepsType_Invalid;
	m_LoadedFromProfile = ProfileSlot_Invalid;
	m_iHash = 0;
	m_Difficulty = Difficulty_Invalid;
	m_iMeter = 0;

	m_pNoteData = new NoteData;
	m_bNoteDataIsFilled = false;
	m_sNoteDataCompressed = "";
	parent = NULL;
}

Steps::~Steps()
{
}

unsigned Steps::GetHash() const
{
	if( parent )
		return parent->GetHash();
	if( m_iHash )
		return m_iHash;
	if( m_sNoteDataCompressed.empty() )
	{
		if( !m_bNoteDataIsFilled )
			return 0; // No data, no hash.
		NoteDataUtil::GetSMNoteDataString( *m_pNoteData, m_sNoteDataCompressed );
	}
	m_iHash = GetHashForString( m_sNoteDataCompressed );
	return m_iHash;
}		

void Steps::SetNoteData( const NoteData& noteDataNew )
{
	ASSERT( noteDataNew.GetNumTracks() == GameManager::StepsTypeToNumTracks(m_StepsType) );

	if( parent )
	{
		copy( parent->m_CachedRadarValues, parent->m_CachedRadarValues + NUM_PLAYERS, m_CachedRadarValues );
		m_sDescription	= parent->m_sDescription;
		m_Difficulty	= parent->m_Difficulty;
		m_iMeter	= parent->m_iMeter;
		parent		= NULL;
	}

	*m_pNoteData = noteDataNew;
	m_bNoteDataIsFilled = true;
	
	m_sNoteDataCompressed = RString();
	m_iHash = 0;
	m_sFilename = RString(); // We can no longer read from the file because it has changed in memory.
}

void Steps::GetNoteData( NoteData& noteDataOut ) const
{
	Decompress();

	if( m_bNoteDataIsFilled )
	{
		noteDataOut = *m_pNoteData;
	}
	else
	{
		noteDataOut.ClearAll();
		noteDataOut.SetNumTracks( GameManager::StepsTypeToNumTracks(m_StepsType) );
	}
}

void Steps::SetSMNoteData( const RString &notes_comp_ )
{
	m_pNoteData->Init();
	m_bNoteDataIsFilled = false;

	m_sNoteDataCompressed = notes_comp_;
	m_iHash = 0;
	m_sFilename = RString(); // We can no longer read from the file because it has changed in memory.
}

/* XXX: this function should pull data from m_sFilename, like Decompress() */
void Steps::GetSMNoteData( RString &notes_comp_out ) const
{
	if( m_sNoteDataCompressed.empty() )
	{
		if( !m_bNoteDataIsFilled ) 
		{
			/* no data is no data */
			notes_comp_out = "";
			return;
		}

		NoteDataUtil::GetSMNoteDataString( *m_pNoteData, m_sNoteDataCompressed );
	}

	notes_comp_out = m_sNoteDataCompressed;
}

float Steps::PredictMeter() const
{
	float pMeter = 0.775f;
	
	const float RadarCoeffs[NUM_RadarCategory] =
	{
		10.1f, 5.27f,-0.905f, -1.10f, 2.86f,
		0,0,0,0,0,0
	};
	const RadarValues &rv = GetRadarValues( PLAYER_1 );
	for( int r = 0; r < NUM_RadarCategory; ++r )
		pMeter += rv[r] * RadarCoeffs[r];
	
	const float DifficultyCoeffs[NUM_Difficulty] =
	{
		-0.877f, -0.877f, 0, 0.722f, 0.722f, 0
	};
	pMeter += DifficultyCoeffs[this->GetDifficulty()];
	
	// Init non-radar values
	const float SV = rv[RadarCategory_Stream] * rv[RadarCategory_Voltage];
	const float ChaosSquare = rv[RadarCategory_Chaos] * rv[RadarCategory_Chaos];
	pMeter += -6.35f * SV;
	pMeter += -2.58f * ChaosSquare;
	if (pMeter < 1) pMeter = 1;	
	return pMeter;
}

void Steps::TidyUpData()
{
	if( GetDifficulty() == Difficulty_Invalid )
		SetDifficulty( StringToDifficulty(GetDescription()) );
	
	if( GetDifficulty() == Difficulty_Invalid )
	{
		if(	 GetMeter() == 1 )	SetDifficulty( DIFFICULTY_BEGINNER );
		else if( GetMeter() <= 3 )	SetDifficulty( DIFFICULTY_EASY );
		else if( GetMeter() <= 6 )	SetDifficulty( DIFFICULTY_MEDIUM );
		else				SetDifficulty( DIFFICULTY_HARD );
	}

	if( GetMeter() < 1) // meter is invalid
		SetMeter( int(PredictMeter()) );
}

void Steps::CalculateRadarValues( float fMusicLengthSeconds )
{
	// If we're autogen, don't calculate values.  GetRadarValues will take from our parent.
	if( parent != NULL )
		return;
	
	// Do write radar values, and leave it up to the reading app whether they want to trust
	// the cached values without recalculating them.
	/*
	// If we're an edit, leave the RadarValues invalid.
	if( IsAnEdit() )
		return;
	*/

	NoteData tempNoteData;
	this->GetNoteData( tempNoteData );
	
	FOREACH_PlayerNumber( pn )
		m_CachedRadarValues[pn].Zero();
	
	if( tempNoteData.IsComposite() )
	{
		vector<NoteData> vParts;

		NoteDataUtil::SplitCompositeNoteData( tempNoteData, vParts );
		for( size_t pn = 0; pn < min(vParts.size(), size_t(NUM_PLAYERS)); ++pn )
			NoteDataUtil::CalculateRadarValues( vParts[pn], fMusicLengthSeconds, m_CachedRadarValues[pn] );
	}
	else
	{
		NoteDataUtil::CalculateRadarValues( tempNoteData, fMusicLengthSeconds, m_CachedRadarValues[0] );
		fill_n( m_CachedRadarValues + 1, NUM_PLAYERS-1, m_CachedRadarValues[0] );
	}
}

void Steps::Decompress() const
{
	if( m_bNoteDataIsFilled )
		return;	// already decompressed

	if( parent )
	{
		// get autogen m_pNoteData
		NoteData notedata;
		parent->GetNoteData( notedata );

		m_bNoteDataIsFilled = true;

		int iNewTracks = GameManager::StepsTypeToNumTracks( m_StepsType );

		if( this->m_StepsType == STEPS_TYPE_LIGHTS_CABINET )
		{
			NoteDataUtil::LoadTransformedLights( notedata, *m_pNoteData, iNewTracks );
		}
		else
		{
			NoteDataUtil::LoadTransformedSlidingWindow( notedata, *m_pNoteData, iNewTracks );

			NoteDataUtil::RemoveStretch( *m_pNoteData, m_StepsType );
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
		bool bComposite = m_StepsType == STEPS_TYPE_DANCE_ROUTINE;
		m_bNoteDataIsFilled = true;
		m_pNoteData->SetNumTracks( GameManager::StepsTypeToNumTracks(m_StepsType) );

		NoteDataUtil::LoadFromSMNoteDataString( *m_pNoteData, m_sNoteDataCompressed, bComposite );
	}
}

void Steps::Compress() const
{
	/* Always leave lights data uncompressed. */
	if( this->m_StepsType == STEPS_TYPE_LIGHTS_CABINET && m_bNoteDataIsFilled )
	{
		m_sNoteDataCompressed = RString();
		return;
	}

	if( !m_sFilename.empty() && m_LoadedFromProfile == ProfileSlot_Invalid )
	{
		/*
		 * We have a file on disk; clear all data in memory.
		 *
		 * Data on profiles can't be accessed normally (need to mount and time-out the
		 * device), and when we start a game and load edits, we want to be sure that
		 * it'll be available if the user picks it and pulls the device.  Also,
		 * Decompress() doesn't know how to load .edits.
		 */
		m_pNoteData->Init();
		m_bNoteDataIsFilled = false;

		/* Be careful; 'x = ""', m_sNoteDataCompressed.clear() and m_sNoteDataCompressed.reserve(0)
		 * don't always free the allocated memory. */
		m_sNoteDataCompressed = RString();
		return;
	}

	/* We have no file on disk.  Compress the data, if necessary. */
	if( m_sNoteDataCompressed.empty() )
	{
		if( !m_bNoteDataIsFilled )
			return; /* no data is no data */
		NoteDataUtil::GetSMNoteDataString( *m_pNoteData, m_sNoteDataCompressed );
	}

	m_pNoteData->Init();
	m_bNoteDataIsFilled = false;
}

/* Copy our parent's data.  This is done when we're being changed from autogen
 * to normal. (needed?) */
void Steps::DeAutogen()
{
	if( !parent )
		return; /* OK */

	Decompress();	// fills in m_pNoteData with sliding window transform

	m_sDescription		= Real()->m_sDescription;
	m_Difficulty		= Real()->m_Difficulty;
	m_iMeter		= Real()->m_iMeter;
	copy( Real()->m_CachedRadarValues, Real()->m_CachedRadarValues + NUM_PLAYERS, m_CachedRadarValues );

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
	parent = NULL;
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

void Steps::SetDifficultyAndDescription( Difficulty dc, RString sDescription )
{
	DeAutogen();
	m_Difficulty = dc;
	m_sDescription = sDescription;
	if( GetDifficulty() == DIFFICULTY_EDIT )
		MakeValidEditDescription( m_sDescription );
}

bool Steps::MakeValidEditDescription( RString &sPreferredDescription )
{
	if( int(sPreferredDescription.size()) > MAX_EDIT_STEPS_DESCRIPTION_LENGTH )
	{
		sPreferredDescription = sPreferredDescription.Left( MAX_EDIT_STEPS_DESCRIPTION_LENGTH );
		return true;
	}
	return false;
}

void Steps::SetMeter( int meter )
{
	DeAutogen();
	m_iMeter = meter;
}

void Steps::SetCachedRadarValues( const RadarValues v[NUM_PLAYERS] )
{
	DeAutogen();
	copy( v, v + NUM_PLAYERS, m_CachedRadarValues );
}


// lua start
#include "LuaBinding.h"

class LunaSteps: public Luna<Steps>
{
public:
	DEFINE_METHOD( GetStepsType,	m_StepsType )
	DEFINE_METHOD( GetDifficulty,	GetDifficulty() )
	DEFINE_METHOD( GetDescription,	GetDescription() )
	DEFINE_METHOD( GetMeter,	GetMeter() )
	DEFINE_METHOD( GetFilename,	GetFilename() )
	DEFINE_METHOD( IsAutogen,	IsAutogen() )
	
	static int GetRadarValues( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		RadarValues &rv = const_cast<RadarValues &>(p->GetRadarValues(pn));
		rv.PushSelf(L);
		return 1;
	}

	LunaSteps()
	{
		ADD_METHOD( GetStepsType );
		ADD_METHOD( GetDifficulty );
		ADD_METHOD( GetDescription );
		ADD_METHOD( GetMeter );
		ADD_METHOD( GetFilename );
		ADD_METHOD( GetRadarValues );
		ADD_METHOD( IsAutogen );
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
