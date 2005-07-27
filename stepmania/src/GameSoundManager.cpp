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
#include "NoteData.h"
#include "song.h"
#include "Steps.h"
#include "LightsManager.h"

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
 * on the same fractional beat when we loop.
 */

/* Lock this before touching g_UpdatingTimer or g_Playing. */
static RageEvent *g_Mutex;
static bool g_UpdatingTimer;
static bool g_Shutdown;
static bool g_bFlushing = false;

enum FadeState { FADE_NONE, FADE_OUT, FADE_WAIT, FADE_IN };
static FadeState g_FadeState = FADE_NONE;
static float g_fDimVolume = 1.0f;
static float g_fOriginalVolume = 1.0f;
static float g_fDimDurationRemaining = 0.0f;

struct MusicPlaying
{
	bool m_bTimingDelayed;
	bool m_bHasTiming;
	/* The timing data that we're currently using. */
	TimingData m_Timing;
	NoteData m_Lights;

	/* If m_bTimingDelayed is true, this will be the timing data for the song that's starting.
	 * We'll copy it to m_Timing once sound is heard. */
	TimingData m_NewTiming;
	RageSound *m_Music;
	MusicPlaying( RageSound *Music )
	{
		m_Timing.AddBPMSegment( BPMSegment(0,120) );
		m_NewTiming.AddBPMSegment( BPMSegment(0,120) );
		m_bHasTiming = false;
		m_bTimingDelayed = false;
		m_Music = Music;
	}

	~MusicPlaying()
	{
		SOUNDMAN->DeleteSound( m_Music );
	}
};

static MusicPlaying *g_Playing;

static RageThread MusicThread;

vector<CString> g_SoundsToPlayOnce;
vector<CString> g_SoundsToPlayOnceFromDir;
vector<CString> g_SoundsToPlayOnceFromAnnouncer;

struct MusicToPlay
{
	CString file, timing_file;
	bool HasTiming;
	TimingData timing_data;
	NoteData lights_data;
	bool force_loop;
	float start_sec, length_sec, fade_len;
	bool align_beat;
	MusicToPlay()
	{
		HasTiming = false;
	}
};
vector<MusicToPlay> g_MusicsToPlay;

static void StartMusic( MusicToPlay &ToPlay )
{
	LockMutex L( *g_Mutex );
	if( g_Playing->m_Music->IsPlaying() && !g_Playing->m_Music->GetLoadedFilePath().CompareNoCase(ToPlay.file) )
		return;

	/* We're changing or stopping the music.  If we were dimming, reset. */
	g_FadeState = FADE_NONE;

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
	NewMusic->m_Lights = g_Playing->m_Lights;

	/* See if we can find timing data, if it's not already loaded. */
	if( !ToPlay.HasTiming && IsAFile(ToPlay.timing_file) )
	{
		LOG->Trace("Found '%s'", ToPlay.timing_file.c_str());
		Song song;
		SMLoader sml;
		if( sml.LoadFromSMFile( ToPlay.timing_file, song ) )
		{
			ToPlay.HasTiming = true;
			ToPlay.timing_data = song.m_Timing;
			// get cabinet lights if any
			Steps *pStepsCabinetLights = song.GetOneSteps( STEPS_TYPE_LIGHTS_CABINET );
			if( pStepsCabinetLights )
				pStepsCabinetLights->GetNoteData( ToPlay.lights_data );
		}
	}

	if( ToPlay.HasTiming )
	{
		NewMusic->m_NewTiming = ToPlay.timing_data;
		NewMusic->m_Lights = ToPlay.lights_data;
	}

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

	bool StartImmediately = false;
	if( !ToPlay.HasTiming )
	{
		/* This song has no real timing data.  The offset is arbitrary.  Change it so
		 * the beat will line up to where we are now, so we don't have to delay. */
		float fDestBeat = fmodfp( GAMESTATE->m_fSongBeat, 1 );
		float fTime = NewMusic->m_NewTiming.GetElapsedTimeFromBeat( fDestBeat );

		NewMusic->m_NewTiming.m_fBeat0OffsetInSeconds = fTime;

		StartImmediately = true;
	}

	/* If we have an active timer, try to start on the next update.  Otherwise,
	 * start now. */
	if( !g_Playing->m_bHasTiming && !g_UpdatingTimer )
		StartImmediately = true;
	if( !ToPlay.align_beat )
		StartImmediately = true;

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

	/* Important: don't hold the mutex while we load and seek the actual sound. */
	L.Unlock();
	{
		NewMusic->m_bHasTiming = ToPlay.HasTiming;
		if( ToPlay.HasTiming )
			NewMusic->m_NewTiming = ToPlay.timing_data;
		NewMusic->m_bTimingDelayed = true;
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

	LockMut( *g_Mutex );
	delete g_Playing;
	g_Playing = NewMusic;
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

static bool SoundWaiting()
{
	return !g_SoundsToPlayOnce.empty() ||
		!g_SoundsToPlayOnceFromDir.empty() ||
		!g_SoundsToPlayOnceFromAnnouncer.empty() ||
		!g_MusicsToPlay.empty();
}


static void StartQueuedSounds()
{
	g_Mutex->Lock();
	vector<CString> aSoundsToPlayOnce = g_SoundsToPlayOnce;
	g_SoundsToPlayOnce.clear();
	vector<CString> aSoundsToPlayOnceFromDir = g_SoundsToPlayOnceFromDir;
	g_SoundsToPlayOnceFromDir.clear();
	vector<CString> aSoundsToPlayOnceFromAnnouncer = g_SoundsToPlayOnceFromAnnouncer;
	g_SoundsToPlayOnceFromAnnouncer.clear();
	vector<MusicToPlay> aMusicsToPlay = g_MusicsToPlay;
	g_MusicsToPlay.clear();
	g_Mutex->Unlock();

	for( unsigned i = 0; i < aSoundsToPlayOnce.size(); ++i )
		if( aSoundsToPlayOnce[i] != "" )
			SOUNDMAN->PlayOnce( aSoundsToPlayOnce[i] );

	for( unsigned i = 0; i < aSoundsToPlayOnceFromDir.size(); ++i )
		DoPlayOnceFromDir( aSoundsToPlayOnceFromDir[i] );

	for( unsigned i = 0; i < aSoundsToPlayOnceFromAnnouncer.size(); ++i )
	{
		CString sPath = aSoundsToPlayOnceFromAnnouncer[i];
		if( sPath != "" )
		{
			sPath = ANNOUNCER->GetPathTo( sPath );
			DoPlayOnceFromDir( sPath );
		}
	}

	for( unsigned i = 0; i < aMusicsToPlay.size(); ++i )
	{
		/* Don't bother starting this music if there's another one in the queue after it. */
		/* Actually, it's a little trickier: the editor gives us a stop and then a sound in
		 * quick succession; if we ignore the stop, we won't rewind the sound if it was
		 * already playing.  We don't want to waste time loading a sound if it's going
		 * to be replaced immediately, though.  So, if we have more music in the queue,
		 * then forcibly stop the current sound. */
		if( i+1 == aMusicsToPlay.size() )
			StartMusic( aMusicsToPlay[i] );
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
	}
}

void GameSoundManager::Flush()
{
	g_Mutex->Lock();
	g_bFlushing = true;

	g_Mutex->Broadcast();

	while( g_bFlushing )
		g_Mutex->Wait();
	g_Mutex->Unlock();

	/* The thread won't actually delete the sound, waiting for SOUNDMAN to do it in
	 * the main thread.  Update it now, to make sure that the sound is actually deleted
	 * before returning. */
	SOUNDMAN->Update(0);
}

int MusicThread_start( void *p )
{
	while( !g_Shutdown )
	{
		g_Mutex->Lock();
		while( !SoundWaiting() && !g_Shutdown && !g_bFlushing )
			g_Mutex->Wait();
		g_Mutex->Unlock();

		/* This is a little hack: we want to make sure that we go through
		 * StartQueuedSounds after Flush() is called, to be sure we're flushed,
		 * so check g_bFlushing before calling.  This won't work if more than
		 * one thread might call Flush(), but only the main thread is allowed
		 * to make SOUND calls. */
		bool bFlushing = g_bFlushing;

		StartQueuedSounds();

		if( bFlushing )
		{
			g_Mutex->Lock();
			g_Mutex->Signal();
			g_bFlushing = false;
			g_Mutex->Unlock();
		}
	}

	return 0;
}

GameSoundManager::GameSoundManager()
{
	/* Init RageSoundMan first: */
	ASSERT( SOUNDMAN );

	g_Mutex = new RageEvent("GameSoundManager");
	g_Playing = new MusicPlaying( new RageSound );

	g_UpdatingTimer = true;

	g_Shutdown = false;
	MusicThread.SetName( "MusicThread" );
	MusicThread.Create( MusicThread_start, this );
}

GameSoundManager::~GameSoundManager()
{
	/* Signal the mixing thread to quit. */
	LOG->Trace("Shutting down music start thread ...");
	g_Mutex->Lock();
	g_Shutdown = true;
	g_Mutex->Broadcast();
	g_Mutex->Unlock();
	MusicThread.Wait();
	LOG->Trace("Music start thread shut down.");

	SAFE_DELETE( g_Playing );
	SAFE_DELETE( g_Mutex );
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

	{
		/* Duration of the fade-in and fade-out: */
		static float fFadeInSpeed = 1.5f;
		static float fFadeOutSpeed = 0.3f;
		float fVolume = g_Playing->m_Music->GetVolume();
		switch( g_FadeState )
		{
		case FADE_NONE: break;
		case FADE_OUT:
			fapproach( fVolume, g_fDimVolume, fDeltaTime/fFadeOutSpeed );
			if( fabsf(fVolume-g_fDimVolume) < 0.001f )
				g_FadeState = FADE_WAIT;
			break;
		case FADE_WAIT:
			g_fDimDurationRemaining -= fDeltaTime;
			if( g_fDimDurationRemaining <= 0 )
				g_FadeState = FADE_IN;
			break;
		case FADE_IN:
			fapproach( fVolume, g_fOriginalVolume, fDeltaTime/fFadeInSpeed );
			if( fabsf(fVolume-g_fOriginalVolume) < 0.001f )
				g_FadeState = FADE_NONE;
			break;
		}
		
		RageSoundParams p = g_Playing->m_Music->GetParams();
		p.m_Volume = fVolume;
		g_Playing->m_Music->SetParams( p );
	}

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
	 * During this time, m_bApproximate will be true.  Keep using the previous timing
	 * data until we get a non-approximate time, indicating that the sound has actually
	 * started playing. */
	bool m_bApproximate;
	RageTimer tm;
	const float fSeconds = g_Playing->m_Music->GetPositionSeconds( &m_bApproximate, &tm );

	//
	// Check for song timing skips.
	//
	if( PREFSMAN->m_bLogSkips && !g_Playing->m_bTimingDelayed )
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

	//
	// If g_Playing->m_bTimingDelayed, we're waiting for the new music to actually start
	// playing.
	//
	if( g_Playing->m_bTimingDelayed && !m_bApproximate )
	{
		/* Load up the new timing data. */
		g_Playing->m_Timing = g_Playing->m_NewTiming;
		g_Playing->m_bTimingDelayed = false;
	}

	if( g_Playing->m_bTimingDelayed )
	{
		/* We're still waiting for the new sound to start playing, so keep using the
		 * old timing data and fake the time. */
		GAMESTATE->UpdateSongPosition( GAMESTATE->m_fMusicSeconds + fDeltaTime, g_Playing->m_Timing );
	}
	else
	{
		GAMESTATE->UpdateSongPosition( fSeconds + fAdjust, g_Playing->m_Timing, tm + fAdjust );
	}


	//
	// Send crossed messages
	//
	if( GAMESTATE->m_pCurSong )
	{
		static int iRowLastCrossed = 0;

		float fPositionSeconds = GAMESTATE->m_fMusicSeconds;
		float fSongBeat = GAMESTATE->m_pCurSong->GetBeatFromElapsedTime( fPositionSeconds );

		int iRowNow = BeatToNoteRowNotRounded( fSongBeat );
		iRowNow = max( 0, iRowNow );

		for( int r=iRowLastCrossed+1; r<=iRowNow; r++ )
		{
			if( GetNoteType( r ) == NOTE_TYPE_4TH )
			{
				int iBeat = (int) NoteRowToBeat( r );
				MESSAGEMAN->Broadcast( ssprintf("CrossedBeat%d", iBeat) );
			}
		}

		iRowLastCrossed = iRowNow;
	}


	//
	// Update lights
	//
	NoteData &lights = g_Playing->m_Lights;
	if( lights.GetNumTracks() > 0 )	// lights data was loaded
	{
		bool bCrossedABeat = false;
		
		const float fSongBeat = GAMESTATE->m_fLightSongBeat;
		const int iSongRow = BeatToNoteRowNotRounded( fSongBeat );

		static int iRowLastCrossed = 0;

		float fBeatLast = roundf(NoteRowToBeat(iRowLastCrossed));
		float fBeatNow = roundf(NoteRowToBeat(iSongRow));

		bCrossedABeat = fBeatLast != fBeatNow;

		FOREACH_CabinetLight( cl )
		{	
			// for each index we crossed since the last update:
			FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( lights, cl, r, iRowLastCrossed+1, iSongRow+1 )
			{
				if( lights.GetTapNote( cl, r ).type != TapNote::empty )
				{
					LIGHTSMAN->BlinkCabinetLight( cl );
					goto done_with_cabinet_light;
				}
			}

			if( lights.IsHoldNoteAtRow( cl, iSongRow ) )
			{
				LIGHTSMAN->BlinkCabinetLight( cl );
				goto done_with_cabinet_light;
			}
done_with_cabinet_light:
			;
		}

		iRowLastCrossed = iSongRow;
	}
}


CString GameSoundManager::GetMusicPath() const
{
	LockMut( *g_Mutex );
	return g_Playing->m_Music->GetLoadedFilePath();
}

void GameSoundManager::PlayMusic( 
	const CString &file, 
	const TimingData *pTiming, 
	bool force_loop, 
	float start_sec, 
	float length_sec, 
	float fade_len, 
	bool align_beat )
{
	//	LOG->Trace("play '%s' (current '%s')", file.c_str(), g_Playing->m_Music->GetLoadedFilePath().c_str());

	MusicToPlay ToPlay;

	ToPlay.file = file;
	if( pTiming )
	{
		ToPlay.HasTiming = true;
		ToPlay.timing_data = *pTiming;
	}
	else
	{
		/* If no timing file was specified, look for one in the same place as the music file. */
		ToPlay.timing_file = SetExtension( file, "sm" );
	}

	ToPlay.force_loop = force_loop;
	ToPlay.start_sec = start_sec;
	ToPlay.length_sec = length_sec;
	ToPlay.fade_len = fade_len;
	ToPlay.align_beat = align_beat;

	/* Add the MusicToPlay to the g_MusicsToPlay queue. */
	g_Mutex->Lock();
	g_MusicsToPlay.push_back( ToPlay );
	g_Mutex->Broadcast();
	g_Mutex->Unlock();
}

void GameSoundManager::DimMusic( float fVolume, float fDurationSeconds )
{
	LockMut( *g_Mutex );

	if( g_FadeState == FADE_NONE )
		g_fOriginalVolume = g_Playing->m_Music->GetVolume();
	// otherwise, g_fOriginalVolume is already set and GetVolume will return the
	// current state, not the original state

	g_fDimDurationRemaining = fDurationSeconds;
	g_fDimVolume = fVolume;
	g_FadeState = FADE_OUT;
}

void GameSoundManager::HandleSongTimer( bool on )
{
	LockMut( *g_Mutex );
	g_UpdatingTimer = on;
}

void GameSoundManager::PlayOnce( CString sPath )
{
	/* Add the sound to the g_SoundsToPlayOnce queue. */
	g_Mutex->Lock();
	g_SoundsToPlayOnce.push_back( sPath );
	g_Mutex->Broadcast();
	g_Mutex->Unlock();
}

void GameSoundManager::PlayOnceFromDir( CString sPath )
{
	/* Add the path to the g_SoundsToPlayOnceFromDir queue. */
	g_Mutex->Lock();
	g_SoundsToPlayOnceFromDir.push_back( sPath );
	g_Mutex->Broadcast();
	g_Mutex->Unlock();
}

void GameSoundManager::PlayOnceFromAnnouncer( CString sPath )
{
	/* Add the path to the g_SoundsToPlayOnceFromDir queue. */
	g_Mutex->Lock();
	g_SoundsToPlayOnceFromDir.push_back( sPath );
	g_Mutex->Broadcast();
	g_Mutex->Unlock();
}

float GameSoundManager::GetPlayLatency() const
{
	return SOUNDMAN->GetPlayLatency();
}

void GameSoundManager::SetPlayerBalance( PlayerNumber pn, RageSoundParams &params )
{
	/* If two players are active, play sounds on each players' side. */
	if( GAMESTATE->GetNumPlayersEnabled() == 2 )
		params.m_Balance = (pn == PLAYER_1)? -1.0f:1.0f;
	else
		params.m_Balance = 0;
}


#include "LuaBinding.h"

class LunaGameSoundManager: public Luna<GameSoundManager>
{
public:
	LunaGameSoundManager() { LUA->Register( Register ); }

	static int DimMusic( T* p, lua_State *L )
	{
		float fVolume = FArg(1);
		float fDurationSeconds = FArg(2);
		p->DimMusic( fVolume, fDurationSeconds );
		return 0;
	}
	static int PlayOnce( T* p, lua_State *L ) { CString sPath = SArg(1); p->PlayOnce( sPath ); return 0; }

	static void Register(lua_State *L)
	{
		ADD_METHOD( DimMusic )
		ADD_METHOD( PlayOnce )
		Luna<T>::Register( L );

		// Add global singleton if constructed already.  If it's not constructed yet,
		// then we'll register it later when we reinit Lua just before 
		// initializing the display.
		if( SOUND )
		{
			lua_pushstring(L, "SOUND");
			SOUND->PushSelf( L );
			lua_settable(L, LUA_GLOBALSINDEX);
		}
	}
};

LUA_REGISTER_CLASS( GameSoundManager )


/*
 * Copyright (c) 2003-2005 Glenn Maynard
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

