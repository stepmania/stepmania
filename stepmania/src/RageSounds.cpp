#include "global.h"
#include "RageSoundManager.h"
#include "RageSounds.h"
#include "RageSound.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "GameState.h"
#include "TimingData.h"
#include "MsdFile.h"
#include "NotesLoaderSM.h"

RageSounds *SOUND = NULL;

/*
 * When playing music, automatically search for an SM file for timing data.  If one is
 * found, automatically handle GAMESTATE->m_fSongBeat, etc.
 *
 * modf(GAMESTATE->m_fSongBeat) should always be continuously moving from 0 to 1.  To do
 * this, wait before starting a sound until the fractional portion of the beat will be
 * the same.
 *
 * If PlayMusic(length_sec) is set, peek at the beat, and extend the length so we'll be
 * on an integral beat 0 when we loop.  (XXX: should we increase fade_len, too?
 * That would cause the extra pad time to be silence.)
 */
static RageSound *g_Music;
static TimingData g_Timing;
static bool g_HasTiming;
static bool g_UpdatingTimer;

RageSounds::RageSounds()
{
	/* Init RageSoundMan first: */
	ASSERT( SOUNDMAN );

	g_Music = new RageSound;
	g_Timing = TimingData();
	g_HasTiming = false;
	g_UpdatingTimer = false;
}

RageSounds::~RageSounds()
{
	delete g_Music;
}

struct MusicToPlay
{
	CString file;
	TimingData timing;
	bool HasTiming;
	CString timing_file;
	bool force_loop;
	float start_sec, length_sec, fade_len;
};
static MusicToPlay g_MusicToPlay;


void StartQueuedMusic()
{
	if( g_MusicToPlay.file.size() == 0 )
		return; /* nothing to do */

	/* Go. */
	LOG->Trace("Start sound \"%s\"", g_MusicToPlay.file.c_str() );

	g_HasTiming = g_MusicToPlay.HasTiming;
	g_Timing = g_MusicToPlay.timing;

	g_Music->Load( g_MusicToPlay.file, false );

	if( g_MusicToPlay.force_loop )
		g_Music->SetStopMode( RageSound::M_LOOP );

	if( g_MusicToPlay.start_sec == -1 )
		g_Music->SetStartSeconds();
	else
		g_Music->SetStartSeconds( g_MusicToPlay.start_sec );

	if( g_MusicToPlay.length_sec == -1 )
		g_Music->SetLengthSeconds();
	else
		g_Music->SetLengthSeconds( g_MusicToPlay.length_sec );

	g_Music->SetFadeLength( g_MusicToPlay.fade_len );
	g_Music->SetPositionSeconds();
	g_Music->StartPlaying();

	g_MusicToPlay.file = ""; /* done */
}


/* If we have a music file queued, try to start it.  If we're doing a beat update,
 * fOldBeat is the beat before the update and fNewBeat is the beat after the update;
 * we'll only start the sound if the fractional part of the beat we'll be moving to
 * lies between them. */
void TryToStartQueuedMusic( float fOldBeat, float fNewBeat )
{
	if( g_MusicToPlay.file.size() == 0 )
		return; /* nothing to do */

	if( g_UpdatingTimer )
	{
		float fOldBeatFraction = fmodfp( fOldBeat,1 );
		float fNewBeatFraction = fmodfp( fNewBeat,1 );

		/* The beat that the new sound will start on.  See if this lies within the beat
		 * range of this update. */
		float fStartBeatFraction = fmodfp( g_MusicToPlay.timing.GetBeatFromElapsedTime(g_MusicToPlay.start_sec), 1 );
		if( fNewBeatFraction < fOldBeatFraction )
			fNewBeatFraction += 1.0f; /* unwrap */
		if( fStartBeatFraction < fOldBeatFraction )
			fStartBeatFraction += 1.0f; /* unwrap */

		if( !( fOldBeatFraction <= fStartBeatFraction && fStartBeatFraction <= fNewBeatFraction ) )
			return;
	}

	StartQueuedMusic();
}

void RageSounds::Update( float fDeltaTime )
{
	const float fOldBeat = GAMESTATE->m_fSongBeat;
	if( g_UpdatingTimer )
	{
		if( g_Music->IsPlaying() )
			GAMESTATE->UpdateSongPosition( g_Music->GetPositionSeconds(), g_Timing );
		else
		{
			/* There's no song playing.  Fake it. */
			GAMESTATE->UpdateSongPosition( GAMESTATE->m_fMusicSeconds + fDeltaTime, g_Timing );
		}
	}

	/* If we have a sound queued to play, try to start it. */
	TryToStartQueuedMusic( fOldBeat, GAMESTATE->m_fSongBeat );
}


CString RageSounds::GetMusicPath() const
{
	return g_Music->GetLoadedFilePath();
}

void RageSounds::PlayMusic( const CString &file, const CString &timing_file, bool force_loop, float start_sec, float length_sec, float fade_len )
{
//	LOG->Trace("play '%s' (current '%s')", file.c_str(), g_Music->GetLoadedFilePath().c_str());
	if( g_Music->IsPlaying() )
	{
		if( !g_Music->GetLoadedFilePath().CompareNoCase(file) )
			return;		// do nothing

		g_Music->StopPlaying();
	}

	g_Music->Unload();

	g_MusicToPlay.file = file;
	g_MusicToPlay.timing_file = timing_file;
	g_MusicToPlay.force_loop = force_loop;
	g_MusicToPlay.start_sec = start_sec;
	g_MusicToPlay.length_sec = length_sec;
	g_MusicToPlay.fade_len = fade_len;

	/* If file is blank, just stop. */
	if( file.empty() )
		return;

	/* If no timing file was specified, look for one in the same place as the music file. */
	CString real_timing_file = timing_file;
	if( real_timing_file == "" )
		real_timing_file = SetExtension( file, "sm" );

	/* See if we can find timing data. */
	g_MusicToPlay.HasTiming = false;
	g_MusicToPlay.timing = TimingData();

	if( IsAFile(real_timing_file) )
	{
		LOG->Trace("Found '%s'", real_timing_file.c_str());
		MsdFile msd;
		bool bResult = msd.ReadFile( real_timing_file );
		if( !bResult )
			LOG->Warn( "Couldn't load %s", real_timing_file.c_str(), msd.GetError().c_str() );
		else
		{
			SMLoader::LoadTimingFromSMFile( msd, g_MusicToPlay.timing );
			g_MusicToPlay.HasTiming = true;
		}
	}

	if( g_MusicToPlay.HasTiming && force_loop )
	{
		float fStartBeat = g_MusicToPlay.timing.GetBeatFromElapsedTime( g_MusicToPlay.start_sec );
		float fEndSec = start_sec + length_sec;
		float fEndBeat = g_MusicToPlay.timing.GetBeatFromElapsedTime( fEndSec );
		
		const float fStartBeatFraction = fmodfp( fStartBeat, 1 );
		const float fEndBeatFraction = fmodfp( fEndBeat, 1 );

		float fBeatDifference = fStartBeatFraction - fEndBeatFraction;
		if( fBeatDifference < 0 )
			fBeatDifference += 1.0f; /* unwrap */

		fEndBeat += fBeatDifference;

		float fRealEndSec = g_MusicToPlay.timing.GetElapsedTimeFromBeat( fEndBeat );
		g_MusicToPlay.length_sec = fRealEndSec - g_MusicToPlay.start_sec;
	}

	bool StartImmediately = false;
	if( !g_MusicToPlay.HasTiming )
	{
		/* This song has no real timing data.  Fake m_fSongBeat as a 120 BPM song,
		 * and don't do any forced-waiting for future songs. */
		g_MusicToPlay.timing.AddBPMSegment( BPMSegment(0,120) );
		
		/* The offset is arbitrary. Change it so the beat will line up to where we are
		 * now, so we don't have to delay. */
		float fDestBeat = fmodfp( GAMESTATE->m_fSongBeat, 1 );
		float fTime = g_MusicToPlay.timing.GetElapsedTimeFromBeat( fDestBeat );

		g_MusicToPlay.timing.m_fBeat0OffsetInSeconds = fTime;

		StartImmediately = true;
	}

	/* If we have an active timer, try to start on the next update.  Otherwise,
	 * start now. */
	if( !g_HasTiming && !g_UpdatingTimer )
		StartImmediately = true;

	if( StartImmediately )
		StartQueuedMusic();
	if( g_MusicToPlay.file != "" )
		LOG->Trace("Queued sound \"%s\"", g_MusicToPlay.file.c_str() );
}

void RageSounds::HandleSongTimer( bool on )
{
	g_UpdatingTimer = on;
}

void RageSounds::PlayOnce( CString sPath )
{
	SOUNDMAN->PlayOnce( sPath );
}

void RageSounds::PlayOnceFromDir( CString PlayOnceFromDir )
{
	SOUNDMAN->PlayOnceFromDir( PlayOnceFromDir );
}

float RageSounds::GetPlayLatency() const
{
	return SOUNDMAN->GetPlayLatency();
}

/*
-----------------------------------------------------------------------------
 Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
        Glenn Maynard
-----------------------------------------------------------------------------
*/

