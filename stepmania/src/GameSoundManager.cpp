#include "global.h"
#include "RageSoundManager.h"
#include "GameSoundManager.h"
#include "RageSound.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "GameState.h"
#include "TimingData.h"
#include "NotesLoaderSM.h"
#include "PrefsManager.h"
#include "RageDisplay.h"
#include "AnnouncerManager.h"

GameSoundManager *SOUND = NULL;

/*
 * When playing music, automatically search for an SM file for timing data.  If one is
 * found, automatically handle GAMESTATE->m_fSongBeat, etc.
 *
 * modf(GAMESTATE->m_fSongBeat) should always be continuously moving from 0 to 1.  To do
 * this, wait before starting a sound until the fractional portion of the beat will be
 * the same.
 *
 * If PlayMusic(length_sec) is set, peek at the beat, and extend the length so we'll be
 * on the same fractional beat when we loop.  (XXX: should we increase fade_len, too?
 * That would cause the extra pad time to be silence.)
 */
/* Lock this before touching g_UpdatingTimer or g_Playing. */
static RageMutex *g_Mutex;
static bool g_UpdatingTimer;
static bool g_ThreadedMusicStart = true;
static bool g_Shutdown;

struct MusicPlaying
{
	bool m_TimingDelayed;
	bool m_HasTiming;
	/* The timing data that we're currently using. */
	TimingData m_Timing;

	/* If m_TimingDelayed is true, this will be the timing data for the song that's starting.
	 * We'll copy it to m_Timing once sound is heard. */
	TimingData m_NewTiming;
	RageSound *m_Music;
	MusicPlaying( RageSound *Music )
	{
		m_Timing.AddBPMSegment( BPMSegment(0,120) );
		m_NewTiming.AddBPMSegment( BPMSegment(0,120) );
		m_HasTiming = false;
		m_TimingDelayed = false;
		m_Music = Music;
	}

	~MusicPlaying()
	{
		SOUNDMAN->DeleteSound( m_Music );
	}
};

static MusicPlaying *g_Playing;

static RageThread MusicThread;

/* These buffers can be accessed without locking. */
#include "RageUtil_CircularBuffer.h"
CircBuf<CString *> g_SoundsToPlayOnce;
CircBuf<CString *> g_SoundsToPlayOnceFromDir;
CircBuf<CString *> g_SoundsToPlayOnceFromAnnouncer;

struct MusicToPlay
{
	CString file, timing_file;
	bool HasTiming;
	TimingData timing_data;
	bool force_loop;
	float start_sec, length_sec, fade_len;
	bool align_beat;
	MusicToPlay()
	{
		HasTiming = false;
	}
};
CircBuf<MusicToPlay *> g_MusicsToPlay;

static void StartMusic( MusicToPlay &ToPlay )
{
CHECKPOINT;
	LockMutex L( *g_Mutex );
CHECKPOINT;
	if( g_Playing->m_Music->IsPlaying() && !g_Playing->m_Music->GetLoadedFilePath().CompareNoCase(ToPlay.file) )
		return;

CHECKPOINT;
	if( ToPlay.file.empty() )
	{
		/* StopPlaying() can take a while, so don't hold the lock while we stop the sound.
		 * Be sure to leave the rest of g_Playing in place. */
		RageSound *pOldSound = g_Playing->m_Music;
		g_Playing->m_Music = new RageSound;
		L.Unlock();

		/* We're not allowed to delete the sound in a separate thread, because
		 * RageSoundManager::FlushPosMapQueue might be running.  Stop the sound,
		 * and give it to RageSoundManager to delete. */
		SOUNDMAN->DeleteSound( pOldSound );
		return;
	}
CHECKPOINT;
	/* Unlock, load the sound here, and relock.  Loading may take a while if we're
	 * reading from CD and we have to seek far, which can throw off the timing below. */
	MusicPlaying *NewMusic;
	{
		g_Mutex->Unlock();
		RageSound *pSound = new RageSound;
		pSound->Load( ToPlay.file, false );
		g_Mutex->Lock();

		NewMusic = new MusicPlaying( pSound );
	}

	NewMusic->m_Timing = g_Playing->m_Timing;
CHECKPOINT;

	/* See if we can find timing data, if it's not already loaded. */
	if( !ToPlay.HasTiming && IsAFile(ToPlay.timing_file) )
	{
		LOG->Trace("Found '%s'", ToPlay.timing_file.c_str());
		if( SMLoader::LoadTimingFromFile( ToPlay.timing_file, ToPlay.timing_data ) )
			ToPlay.HasTiming = true;
	}
CHECKPOINT;
	if( ToPlay.HasTiming )
		NewMusic->m_NewTiming = ToPlay.timing_data;

	if( ToPlay.align_beat && ToPlay.HasTiming && ToPlay.force_loop && ToPlay.length_sec != -1 )
	{
		/* Extend the loop period so it always starts and ends on the same fractional
		 * beat.  That is, if it starts on beat 1.5, and ends on beat 10.2, extend it
		 * to end on beat 10.5.  This way, effects always loop cleanly. */
		float fStartBeat = NewMusic->m_NewTiming.GetBeatFromElapsedTime( ToPlay.start_sec );
		float fEndSec = ToPlay.start_sec + ToPlay.length_sec;
		float fEndBeat = NewMusic->m_NewTiming.GetBeatFromElapsedTime( fEndSec );
		
		const float fStartBeatFraction = fmodfp( fStartBeat, 1 );
		const float fEndBeatFraction = fmodfp( fEndBeat, 1 );

		float fBeatDifference = fStartBeatFraction - fEndBeatFraction;
		if( fBeatDifference < 0 )
			fBeatDifference += 1.0f; /* unwrap */

		fEndBeat += fBeatDifference;

		const float fRealEndSec = NewMusic->m_NewTiming.GetElapsedTimeFromBeat( fEndBeat );
		const float fNewLengthSec = fRealEndSec - ToPlay.start_sec;

		/* Extend the fade_len, so the added time is faded out. */
		ToPlay.fade_len += fNewLengthSec - ToPlay.length_sec;
		ToPlay.length_sec = fNewLengthSec;
	}

CHECKPOINT;
	bool StartImmediately = false;
	if( !ToPlay.HasTiming )
	{
		/* This song has no real timing data.  The offset is arbitrary.  Change it so
		 * the beat will line up to where we are now, so we don't have to delay. */
		float fDestBeat = fmodfp( GAMESTATE->m_fSongBeat, 1 );
CHECKPOINT_M(ssprintf("%f",GAMESTATE->m_fSongBeat));
CHECKPOINT_M(ssprintf("%p",NewMusic));
		float fTime = NewMusic->m_NewTiming.GetElapsedTimeFromBeat( fDestBeat );

		NewMusic->m_NewTiming.m_fBeat0OffsetInSeconds = fTime;

		StartImmediately = true;
	}

CHECKPOINT;
	/* If we have an active timer, try to start on the next update.  Otherwise,
	 * start now. */
	if( !g_Playing->m_HasTiming && !g_UpdatingTimer )
		StartImmediately = true;
	if( !ToPlay.align_beat )
		StartImmediately = true;

CHECKPOINT;
	RageTimer when; /* zero */
	if( !StartImmediately )
	{
		/* GetPlayLatency returns the minimum time until a sound starts.  That's
		 * common when starting a precached sound, but our sound isn't, so it'll
		 * probably take a little longer.  Nudge the latency up. */
		const float PresumedLatency = SOUND->GetPlayLatency() + 0.040f;
		const float fCurSecond = GAMESTATE->m_fMusicSeconds + PresumedLatency;
		const float fCurBeat = g_Playing->m_Timing.GetBeatFromElapsedTime( fCurSecond );
		const float fCurBeatFraction = fmodfp( fCurBeat,1 );

		/* The beat that the new sound will start on. */
		const float fStartBeat = NewMusic->m_NewTiming.GetBeatFromElapsedTime( ToPlay.start_sec );
		float fStartBeatFraction = fmodfp( fStartBeat, 1 );
		if( fStartBeatFraction < fCurBeatFraction )
			fStartBeatFraction += 1.0f; /* unwrap */

		const float fCurBeatToStartOn = truncf(fCurBeat) + fStartBeatFraction;
		const float fSecondToStartOn = g_Playing->m_Timing.GetElapsedTimeFromBeat( fCurBeatToStartOn );
		const float fMaximumDistance = 2;
		const float fDistance = min( fSecondToStartOn - fCurSecond, fMaximumDistance );

		when = GAMESTATE->m_LastBeatUpdate + PresumedLatency + fDistance;
	}
CHECKPOINT;

	/* Important: don't hold the mutex while we load and seek the actual sound. */
	L.Unlock();
	{
		NewMusic->m_HasTiming = ToPlay.HasTiming;
		if( ToPlay.HasTiming )
			NewMusic->m_NewTiming = ToPlay.timing_data;
		NewMusic->m_TimingDelayed = true;
//		NewMusic->m_Music->Load( ToPlay.file, false );

		RageSoundParams p;
		p.m_StartSecond = ToPlay.start_sec;
		p.m_LengthSeconds = ToPlay.length_sec;
		p.m_FadeLength = ToPlay.fade_len;
		p.StartTime = when;
		if( ToPlay.force_loop )
			p.StopMode = RageSoundParams::M_LOOP;
		NewMusic->m_Music->SetParams( p );

		NewMusic->m_Music->SetPositionSeconds( p.m_StartSecond );
		NewMusic->m_Music->StartPlaying();
	}

CHECKPOINT;
	LockMut( *g_Mutex );
	delete g_Playing;
	g_Playing = NewMusic;
CHECKPOINT;
}

static void DoPlayOnceFromDir( CString sPath )
{
	if( sPath == "" )
		return;

	// make sure there's a slash at the end of this path
	if( sPath.Right(1) != "/" )
		sPath += "/";

	CStringArray arraySoundFiles;
	GetDirListing( sPath + "*.mp3", arraySoundFiles );
	GetDirListing( sPath + "*.wav", arraySoundFiles );
	GetDirListing( sPath + "*.ogg", arraySoundFiles );

	if( arraySoundFiles.empty() )
		return;

	int index = rand() % arraySoundFiles.size();
	SOUNDMAN->PlayOnce( sPath + arraySoundFiles[index] );
}

static void StartQueuedSounds()
{
	CString *p;
	while( g_SoundsToPlayOnce.read( &p, 1 ) )
	{
		if( *p != "" )
			SOUNDMAN->PlayOnce( *p );
		delete p;
	}

	while( g_SoundsToPlayOnceFromDir.read( &p, 1 ) )
	{
		CString sPath( *p );
		DoPlayOnceFromDir( sPath );
		delete p;
	}

	while( g_SoundsToPlayOnceFromAnnouncer.read( &p, 1 ) )
	{
		CString sPath( *p );
		if( sPath != "" )
			sPath = ANNOUNCER->GetPathTo( sPath );
		DoPlayOnceFromDir( sPath );
		delete p;
	}

	MusicToPlay *pMusic;
	while( g_MusicsToPlay.read( &pMusic, 1 ) )
	{
		/* Don't bother starting this music if there's another one in the queue after it. */
		/* Actually, it's a little trickier: the editor gives us a stop and then a sound in
		 * quick succession; if we ignore the stop, we won't rewind the sound if it was
		 * already playing.  We don't want to waste time loading a sound if it's going
		 * to be replaced immediately, though.  So, if we have more music in the queue,
		 * then forcibly stop the current sound. */
		if( !g_MusicsToPlay.num_readable() )
			StartMusic( *pMusic );
		else
		{
			CHECKPOINT;
			/* StopPlaying() can take a while, so don't hold the lock while we stop the sound. */
			g_Mutex->Lock();
			RageSound *pOldSound = g_Playing->m_Music;
			g_Playing->m_Music = new RageSound;
			g_Mutex->Unlock();

			SOUNDMAN->DeleteSound( pOldSound );
		}
		delete pMusic;
	}
}

int MusicThread_start( void *p )
{
	while( !g_Shutdown )
	{
		usleep( 10000 );

		StartQueuedSounds();
	}

	SOUNDMAN->StopPlayingSoundsForThisThread();

	return 0;
}

GameSoundManager::GameSoundManager()
{
	/* Init RageSoundMan first: */
	ASSERT( SOUNDMAN );

	g_SoundsToPlayOnce.reserve( 16 );
	g_SoundsToPlayOnceFromDir.reserve( 16 );
	g_SoundsToPlayOnceFromAnnouncer.reserve( 16 );
	g_MusicsToPlay.reserve( 16 );

	g_Mutex = new RageMutex("GameSoundManager");
	g_Playing = new MusicPlaying( new RageSound );

	g_UpdatingTimer = false;

	if( g_ThreadedMusicStart )
	{
		g_Shutdown = false;
		MusicThread.SetName( "MusicThread" );
		MusicThread.Create( MusicThread_start, this );
	}
}

GameSoundManager::~GameSoundManager()
{
	if( g_ThreadedMusicStart )
	{
		/* Signal the mixing thread to quit. */
		g_Shutdown = true;
		LOG->Trace("Shutting down music start thread ...");
		MusicThread.Wait();
		LOG->Trace("Music start thread shut down.");
	}

	delete g_Playing;
	delete g_Mutex;

	CString *p;
	while( g_SoundsToPlayOnce.read( &p, 1 ) )
		delete p;

	while( g_SoundsToPlayOnceFromDir.read( &p, 1 ) )
		delete p;

	while( g_SoundsToPlayOnceFromAnnouncer.read( &p, 1 ) )
		delete p;

	MusicToPlay *pMusic;
	while( g_MusicsToPlay.read( &pMusic, 1 ) )
		delete pMusic;
}

float GameSoundManager::GetFrameTimingAdjustment( float fDeltaTime )
{
	/*
	 * We get one update per frame, and we're updated early, almost immediately after vsync,
	 * near the beginning of the game loop.  However, it's very likely that we'll lose the
	 * scheduler while waiting for vsync, and some other thread will be working.  Especially
	 * with a low-resolution scheduler (Linux 2.4, Win9x), we may not get the scheduler back
	 * immediately after the vsync; there may be up to a ~10ms delay.  This can cause jitter
	 * in the rendered arrows.
	 *
	 * Compensate.  If vsync is enabled, and we're maintaining the refresh rate consistently,
	 * we should have a very precise game loop interval.  If we have that, but we're off by
	 * a small amount (less than the interval), adjust the time to line it up.  As long as we
	 * adjust both the sound time and the timestamp, this won't adversely affect input timing.
	 * If we're off by more than that, we probably had a frame skip, in which case we have
	 * bigger skip problems, so don't adjust.
	 */
	static int iLastFPS = 0;
	int iThisFPS = DISPLAY->GetFPS();

	if( iThisFPS != DISPLAY->GetVideoModeParams().rate || iThisFPS != iLastFPS )
	{
		iLastFPS = iThisFPS;
		return 0;
	}

	const float fExpectedDelay = 1.0f / iThisFPS;
	const float fExtraDelay = fDeltaTime - fExpectedDelay;
	if( fabsf(fExtraDelay) >= fExpectedDelay )
		return 0;

	/* Subtract the extra delay. */
	return min( -fExtraDelay, 0 );
}

void GameSoundManager::Update( float fDeltaTime )
{
	LockMut( *g_Mutex );

	if( !g_UpdatingTimer )
		return;

	const float fAdjust = GetFrameTimingAdjustment( fDeltaTime );

	if( !g_Playing->m_Music->IsPlaying() )
	{
		/* There's no song playing.  Fake it. */
		CHECKPOINT_M( ssprintf("%f, delta %f", GAMESTATE->m_fMusicSeconds, fDeltaTime) );
		GAMESTATE->UpdateSongPosition( GAMESTATE->m_fMusicSeconds + fDeltaTime, g_Playing->m_Timing );
		return;
	}

	/* There's a delay between us calling Play() and the sound actually playing.
	 * During this time, approximate will be true.  Keep using the previous timing
	 * data until we get a non-approximate time, indicating that the sound has actually
	 * started playing. */
	bool approximate;
	RageTimer tm;
	const float fSeconds = g_Playing->m_Music->GetPositionSeconds( &approximate, &tm );

	if( g_Playing->m_TimingDelayed )
	{
		if( approximate )
		{
			/* We're still waiting for the new sound to start playing, so keep using the
			 * old timing data and fake the time. */
			CHECKPOINT_M( ssprintf("%f, delta %f", GAMESTATE->m_fMusicSeconds, fDeltaTime) );
			GAMESTATE->UpdateSongPosition( GAMESTATE->m_fMusicSeconds + fDeltaTime, g_Playing->m_Timing );
			return;
		}
		else
		{
			/* We've passed the start position of the new sound, so we should be OK.
			 * Load up the new timing data. */
			g_Playing->m_Timing = g_Playing->m_NewTiming;
			g_Playing->m_TimingDelayed = false;
		}
	}
	else if( PREFSMAN->m_bLogSkips )
	{
		const float fExpectedTimePassed = (tm - GAMESTATE->m_LastBeatUpdate) * g_Playing->m_Music->GetPlaybackRate();
		const float fSoundTimePassed = fSeconds - GAMESTATE->m_fMusicSeconds;
		const float fDiff = fExpectedTimePassed - fSoundTimePassed;

		static CString sLastFile = "";
		const CString ThisFile = g_Playing->m_Music->GetLoadedFilePath();

		/* If fSoundTimePassed < 0, the sound has probably looped. */
		if( sLastFile == ThisFile && fSoundTimePassed >= 0 && fabsf(fDiff) > 0.003f )
			LOG->Trace("Song position skip in %s: expected %.3f, got %.3f (cur %f, prev %f) (%.3f difference)",
				Basename(ThisFile).c_str(), fExpectedTimePassed, fSoundTimePassed, fSeconds, GAMESTATE->m_fMusicSeconds, fDiff );
		sLastFile = ThisFile;
	}

	CHECKPOINT_M( ssprintf("%f", fSeconds) );
	GAMESTATE->UpdateSongPosition( fSeconds+fAdjust, g_Playing->m_Timing, tm+fAdjust );
}


CString GameSoundManager::GetMusicPath() const
{
	LockMut( *g_Mutex );
	return g_Playing->m_Music->GetLoadedFilePath();
}

/* If g_ThreadedMusicStart, this function should not touch the disk at all. */
void GameSoundManager::PlayMusic( const CString &file, const CString &timing_file, bool force_loop, float start_sec, float length_sec, float fade_len, bool align_beat )
{
//	LOG->Trace("play '%s' (current '%s')", file.c_str(), g_Playing->m_Music->GetLoadedFilePath().c_str());

	MusicToPlay *ToPlay = new MusicToPlay;

	ToPlay->file = file;
	ToPlay->force_loop = force_loop;
	ToPlay->start_sec = start_sec;
	ToPlay->length_sec = length_sec;
	ToPlay->fade_len = fade_len;
	ToPlay->timing_file = timing_file;
	ToPlay->align_beat = align_beat;

	/* If no timing file was specified, look for one in the same place as the music file. */
	if( ToPlay->timing_file == "" )
		ToPlay->timing_file = SetExtension( file, "sm" );

	/* Add the MusicToPlay to the g_MusicsToPlay queue. */
	if( !g_MusicsToPlay.write( &ToPlay, 1 ) )
		delete ToPlay;

	if( !g_ThreadedMusicStart )
		StartQueuedSounds();
}

void GameSoundManager::PlayMusic( const CString &file, TimingData *pTiming, bool force_loop, float start_sec, float length_sec, float fade_len, bool align_beat )
{
	MusicToPlay *ToPlay = new MusicToPlay;
	

	ToPlay->file = file;
	if( pTiming )
	{
		ToPlay->HasTiming = true;
		ToPlay->timing_data = *pTiming;
	}
	else
		ToPlay->timing_file = SetExtension( file, "sm" );

	ToPlay->force_loop = force_loop;
	ToPlay->start_sec = start_sec;
	ToPlay->length_sec = length_sec;
	ToPlay->fade_len = fade_len;
	ToPlay->align_beat = align_beat;

	/* Add the MusicToPlay to the g_MusicsToPlay queue. */
	if( !g_MusicsToPlay.write( &ToPlay, 1 ) )
		delete ToPlay;

	if( !g_ThreadedMusicStart )
		StartQueuedSounds();
}

void GameSoundManager::HandleSongTimer( bool on )
{
	LockMut( *g_Mutex );
	g_UpdatingTimer = on;
}

void GameSoundManager::PlayOnce( CString sPath )
{
	/* Add the sound to the g_SoundsToPlayOnce queue. */
	CString *p = new CString( sPath );
	if( !g_SoundsToPlayOnce.write( &p, 1 ) )
		delete p;

	if( !g_ThreadedMusicStart )
		StartQueuedSounds();
}

void GameSoundManager::PlayOnceFromDir( CString PlayOnceFromDir )
{
	/* Add the path to the g_SoundsToPlayOnceFromDir queue. */
	CString *p = new CString( PlayOnceFromDir );
	if( !g_SoundsToPlayOnceFromDir.write( &p, 1 ) )
		delete p;

	if( !g_ThreadedMusicStart )
		StartQueuedSounds();
}

void GameSoundManager::PlayOnceFromAnnouncer( CString sFolderName )
{
	/* Add the path to the g_SoundsToPlayOnceFromDir queue. */
	CString *p = new CString( sFolderName );
	if( !g_SoundsToPlayOnceFromAnnouncer.write( &p, 1 ) )
		delete p;

	if( !g_ThreadedMusicStart )
		StartQueuedSounds();
}

float GameSoundManager::GetPlayLatency() const
{
	return SOUNDMAN->GetPlayLatency();
}

/*
 * Copyright (c) 2003-2004 Glenn Maynard
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

