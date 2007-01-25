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
	m_soundAssistTick.Load(		THEME->GetPathS("ScreenEdit","assist tick"), true );
}

void GameplayAssist::PlayTicks( const NoteData &nd )
{
	if( !GAMESTATE->m_SongOptions.GetStage().m_bAssistTick )
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

	int iTickRow = -1;
	// for each index we crossed since the last update:
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( nd, r, iRowLastCrossed+1, iSongRow+1 )
		if( nd.IsThereATapOrHoldHeadAtRow( r ) )
			iTickRow = r;

	iRowLastCrossed = iSongRow;

	if( iTickRow != -1 )
	{
		const float fTickBeat = NoteRowToBeat( iTickRow );
		const float fTickSecond = GAMESTATE->m_pCurSong->m_Timing.GetElapsedTimeFromBeatNoOffset( fTickBeat );
		float fSecondsUntil = fTickSecond - GAMESTATE->m_fMusicSeconds;
		fSecondsUntil /= GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate; /* 2x music rate means the time until the tick is halved */

		RageSoundParams p;
		p.m_StartTime = GAMESTATE->m_LastBeatUpdate  + (fSecondsUntil - (float)CommonMetrics::TICK_EARLY_SECONDS);
		m_soundAssistTick.Play( &p );
	}
}

void GameplayAssist::StopPlaying()
{
	m_soundAssistTick.StopPlaying();
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
