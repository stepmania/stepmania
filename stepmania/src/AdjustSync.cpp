#include "global.h"
#include "AdjustSync.h"
#include "GameState.h"
#include "song.h"
#include "PrefsManager.h"
#include "LocalizedString.h"
#include "ScreenManager.h"

TimingData *AdjustSync::s_pTimingDataOriginal = NULL;
float AdjustSync::s_fGlobalOffsetSecondsOriginal = 0;
int AdjustSync::s_iAutosyncOffsetSample = 0;
float AdjustSync::s_fAutosyncOffset[SAMPLE_COUNT];

void AdjustSync::ResetOriginalSyncData()
{
	if( s_pTimingDataOriginal == NULL )
		s_pTimingDataOriginal = new TimingData;

	if( GAMESTATE->m_pCurSong )
		*s_pTimingDataOriginal = GAMESTATE->m_pCurSong->m_Timing;
	else
		*s_pTimingDataOriginal = TimingData();
	s_fGlobalOffsetSecondsOriginal = PREFSMAN->m_fGlobalOffsetSeconds;

	s_iAutosyncOffsetSample = 0;
}

bool AdjustSync::IsSyncDataChanged()
{
	// Can't sync in course modes
	if( GAMESTATE->IsCourseMode() )
		return false;

	if( GAMESTATE->m_pCurSong  &&  *s_pTimingDataOriginal != GAMESTATE->m_pCurSong->m_Timing )
		return true;
	if( s_fGlobalOffsetSecondsOriginal != PREFSMAN->m_fGlobalOffsetSeconds )
		return true;

	return false;
}

void AdjustSync::SaveSyncChanges()
{
	if( GAMESTATE->IsCourseMode() )
		return;
	if( GAMESTATE->m_pCurSong  &&  *s_pTimingDataOriginal != GAMESTATE->m_pCurSong->m_Timing )
		GAMESTATE->m_pCurSong->Save();
	if( s_fGlobalOffsetSecondsOriginal != PREFSMAN->m_fGlobalOffsetSeconds )
		PREFSMAN->SavePrefsToDisk();
	ResetOriginalSyncData();
}

void AdjustSync::RevertSyncChanges()
{
	if( GAMESTATE->IsCourseMode() )
		return;
	PREFSMAN->m_fGlobalOffsetSeconds.Set( s_fGlobalOffsetSecondsOriginal );
	GAMESTATE->m_pCurSong->m_Timing = *s_pTimingDataOriginal;
}

static LocalizedString AUTOSYNC_CORRECTION_APPLIED	( "AdjustSync", "Autosync: Correction applied." );
static LocalizedString AUTOSYNC_CORRECTION_NOT_APPLIED	( "AdjustSync", "Autosync: Correction NOT applied. Deviation too high." );
static LocalizedString AUTOSYNC_SONG			( "AdjustSync", "Autosync Song" );
static LocalizedString AUTOSYNC_MACHINE			( "AdjustSync", "Autosync Machine" );
void AdjustSync::HandleAutosync( float fNoteOffBySeconds )
{
	if( GAMESTATE->m_SongOptions.m_AutosyncType == SongOptions::AUTOSYNC_OFF )
		return;

	s_fAutosyncOffset[s_iAutosyncOffsetSample] = fNoteOffBySeconds;
	s_iAutosyncOffsetSample++;

	if( s_iAutosyncOffsetSample < SAMPLE_COUNT ) 
		return; /* need more */

	const float mean = calc_mean( s_fAutosyncOffset, s_fAutosyncOffset+SAMPLE_COUNT );
	const float stddev = calc_stddev( s_fAutosyncOffset, s_fAutosyncOffset+SAMPLE_COUNT );

	RString sAutosyncType;
	switch( GAMESTATE->m_SongOptions.m_AutosyncType )
	{
	case SongOptions::AUTOSYNC_SONG:
		sAutosyncType = AUTOSYNC_SONG;
		break;
	case SongOptions::AUTOSYNC_MACHINE:
		sAutosyncType = AUTOSYNC_MACHINE;
		break;
	default:
		ASSERT(0);
	}

	if( stddev < .03 && stddev < fabsf(mean) )  // If they stepped with less than .03 error
	{
		switch( GAMESTATE->m_SongOptions.m_AutosyncType )
		{
		case SongOptions::AUTOSYNC_SONG:
			GAMESTATE->m_pCurSong->m_Timing.m_fBeat0OffsetInSeconds += mean;
			break;
		case SongOptions::AUTOSYNC_MACHINE:
			PREFSMAN->m_fGlobalOffsetSeconds.Set( PREFSMAN->m_fGlobalOffsetSeconds + mean );
			break;
		default:
			ASSERT(0);
		}

		SCREENMAN->SystemMessage( AUTOSYNC_CORRECTION_APPLIED.GetValue() );
	}
	else
	{
		SCREENMAN->SystemMessage( AUTOSYNC_CORRECTION_NOT_APPLIED.GetValue() );
	}

	s_iAutosyncOffsetSample = 0;
}


/*
 * (c) 2003-2004 Chris Danford
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
