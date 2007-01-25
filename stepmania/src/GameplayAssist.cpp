#include "global.h"
#include "GameplayAssist.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "RageSoundManager.h"
#include "CommonMetrics.h"
#include "song.h"
#include "NoteData.h"

void GameplayAssist::Init()
{
	m_soundAssistClap.Load(			THEME->GetPathS("GameplayAssist","clap"), true );
	m_soundAssistMetronomeMeasure.Load(	THEME->GetPathS("GameplayAssist","metronome measure"), true );
	m_soundAssistMetronomeBeat.Load(	THEME->GetPathS("GameplayAssist","metronome beat"), true );
}

void GameplayAssist::PlayTicks( const NoteData &nd )
{
	bool bClap = GAMESTATE->m_SongOptions.GetCurrent().m_bAssistClap;
	bool bMetronome = GAMESTATE->m_SongOptions.GetCurrent().m_bAssistMetronome;
	if( !bClap  &&  !bMetronome )
		return;

	/* Sound cards have a latency between when a sample is Play()ed and when the sound
	 * will start coming out the speaker.  Compensate for this by boosting fPositionSeconds
	 * ahead.  This is just to make sure that we request the sound early enough for it to
	 * come out on time; the actual precise timing is handled by SetStartTime. */
	float fPositionSeconds = GAMESTATE->m_fMusicSeconds;
	fPositionSeconds += SOUNDMAN->GetPlayLatency() + (float)CommonMetrics::TICK_EARLY_SECONDS + 0.250f;
	const float fSongBeat = GAMESTATE->m_pCurSong->m_Timing.GetBeatFromElapsedTimeNoOffset( fPositionSeconds );

	const int iSongRow = max( 0, BeatToNoteRowNotRounded( fSongBeat ) );
	static int iRowLastCrossed = -1;
	if( iSongRow < iRowLastCrossed )
		iRowLastCrossed = iSongRow;

	if( bClap )
	{
		int iClapRow = -1;
		// for each index we crossed since the last update:
		FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( nd, r, iRowLastCrossed+1, iSongRow+1 )
			if( nd.IsThereATapOrHoldHeadAtRow( r ) )
				iClapRow = r;

		if( iClapRow != -1 )
		{
			const float fTickBeat = NoteRowToBeat( iClapRow );
			const float fTickSecond = GAMESTATE->m_pCurSong->m_Timing.GetElapsedTimeFromBeatNoOffset( fTickBeat );
			float fSecondsUntil = fTickSecond - GAMESTATE->m_fMusicSeconds;
			fSecondsUntil /= GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate; /* 2x music rate means the time until the tick is halved */

			RageSoundParams p;
			p.m_StartTime = GAMESTATE->m_LastBeatUpdate  + (fSecondsUntil - (float)CommonMetrics::TICK_EARLY_SECONDS);
			m_soundAssistClap.Play( &p );
		}
	}

	if( bMetronome )
	{
		//iRowLastCrossed+1, iSongRow+1

		int iLastCrossedBeat = iRowLastCrossed / ROWS_PER_BEAT;
		int iCurrentBeat = iSongRow / ROWS_PER_BEAT;
		
		int iMetronomeRow = -1;
		bool bIsMeasure = false;

		if( iLastCrossedBeat != iCurrentBeat )
		{
			iMetronomeRow = iCurrentBeat * ROWS_PER_BEAT;
			bIsMeasure = (iCurrentBeat % BEATS_PER_MEASURE) == 0;
		}

		if( iMetronomeRow != -1 )
		{
			const float fTickBeat = NoteRowToBeat( iMetronomeRow );
			const float fTickSecond = GAMESTATE->m_pCurSong->m_Timing.GetElapsedTimeFromBeatNoOffset( fTickBeat );
			float fSecondsUntil = fTickSecond - GAMESTATE->m_fMusicSeconds;
			fSecondsUntil /= GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate; /* 2x music rate means the time until the tick is halved */

			RageSoundParams p;
			p.m_StartTime = GAMESTATE->m_LastBeatUpdate  + (fSecondsUntil - (float)CommonMetrics::TICK_EARLY_SECONDS);
			if( bIsMeasure )
				m_soundAssistMetronomeMeasure.Play( &p );
			else
				m_soundAssistMetronomeBeat.Play( &p );
		}
	}

	iRowLastCrossed = iSongRow;
}

void GameplayAssist::StopPlaying()
{
	m_soundAssistClap.StopPlaying();
	m_soundAssistMetronomeMeasure.StopPlaying();
	m_soundAssistMetronomeBeat.StopPlaying();
}

/*
 * (c) 2003-2006 Chris Danford
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
