#include "global.h"
#include "RageSoundManager.h"
#include "GameSoundManager.h"
#include "RageSound.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "GameState.h"
#include "TimingData.h"
#include "NotesLoaderSSC.h"
#include "NotesLoaderSM.h"
#include "PrefsManager.h"
#include "RageDisplay.h"
#include "AnnouncerManager.h"
#include "NoteData.h"
#include "Song.h"
#include "Steps.h"
#include "LightsManager.h"
#include "SongUtil.h"
#include "LuaManager.h"

#include "arch/Sound/RageSoundDriver.h"

GameSoundManager *SOUND = nullptr;

/*
 * When playing music, automatically search for an SM file for timing data.  If one is
 * found, automatically handle GAMESTATE->m_fSongBeat, etc.
 *
 * modf(GAMESTATE->m_fSongBeat) should always be continuously moving from 0 to 1.  To do
 * this, wait before starting a sound until the fractional portion of the beat will be
 * the same.
 *
 * If PlayMusic(fLengthSeconds) is set, peek at the beat, and extend the length so we'll be
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
static bool g_bWasPlayingOnLastUpdate = false;

struct MusicPlaying
{
	bool m_bTimingDelayed;
	bool m_bHasTiming;
	bool m_bApplyMusicRate;
	// The timing data that we're currently using.
	TimingData m_Timing;
	NoteData m_Lights;

	/* If m_bTimingDelayed is true, this will be the timing data for the
	 * song that's starting. We'll copy it to m_Timing once sound is heard. */
	TimingData m_NewTiming;
	RageSound *m_Music;
	MusicPlaying( RageSound *Music )
	{
		m_Timing.AddSegment( BPMSegment(0,120) );
		m_NewTiming.AddSegment( BPMSegment(0,120) );
		m_bHasTiming = false;
		m_bTimingDelayed = false;
		m_bApplyMusicRate = false;
		m_Music = Music;
	}

	~MusicPlaying()
	{
		delete m_Music;
	}
};

static MusicPlaying *g_Playing;

static RageThread MusicThread;

vector<RString> g_SoundsToPlayOnce;
vector<RString> g_SoundsToPlayOnceFromDir;
vector<RString> g_SoundsToPlayOnceFromAnnouncer;
// This should get updated to unordered_map when once C++11 is supported
std::map<RString, vector<int>> g_DirSoundOrder;

struct MusicToPlay
{
	RString m_sFile, m_sTimingFile;
	bool HasTiming;
	TimingData m_TimingData;
	NoteData m_LightsData;
	bool bForceLoop;
	float fStartSecond, fLengthSeconds, fFadeInLengthSeconds, fFadeOutLengthSeconds;
	bool bAlignBeat, bApplyMusicRate;
	MusicToPlay()
	{
		HasTiming = false;
	}
};
vector<MusicToPlay> g_MusicsToPlay;
static GameSoundManager::PlayMusicParams g_FallbackMusicParams;

static void StartMusic( MusicToPlay &ToPlay )
{
	LockMutex L( *g_Mutex );
	if( g_Playing->m_Music->IsPlaying() && g_Playing->m_Music->GetLoadedFilePath().EqualsNoCase(ToPlay.m_sFile) )
		return;

	/* We're changing or stopping the music.  If we were dimming, reset. */
	g_FadeState = FADE_NONE;

	if( ToPlay.m_sFile.empty() )
	{
		/* StopPlaying() can take a while, so don't hold the lock while we stop the sound.
		 * Be sure to leave the rest of g_Playing in place. */
		RageSound *pOldSound = g_Playing->m_Music;
		g_Playing->m_Music = new RageSound;
		L.Unlock();

		delete pOldSound;
		return;
	}

	/* Unlock, load the sound here, and relock.  Loading may take a while if we're
	 * reading from CD and we have to seek far, which can throw off the timing below. */
	MusicPlaying *NewMusic;
	{
		g_Mutex->Unlock();
		RageSound *pSound = new RageSound;
		RageSoundLoadParams params;
		params.m_bSupportRateChanging = ToPlay.bApplyMusicRate;
		pSound->Load( ToPlay.m_sFile, false, &params );
		g_Mutex->Lock();

		NewMusic = new MusicPlaying( pSound );
	}

	NewMusic->m_Timing = g_Playing->m_Timing;
	NewMusic->m_Lights = g_Playing->m_Lights;

	/* See if we can find timing data, if it's not already loaded. */
	if( !ToPlay.HasTiming && IsAFile(ToPlay.m_sTimingFile) )
	{
		LOG->Trace( "Found '%s'", ToPlay.m_sTimingFile.c_str() );
		Song song;
		SSCLoader loaderSSC;
		SMLoader loaderSM;
		if(GetExtension(ToPlay.m_sTimingFile) == ".ssc" &&
		   loaderSSC.LoadFromSimfile(ToPlay.m_sTimingFile, song) )
		{
			ToPlay.HasTiming = true;
			ToPlay.m_TimingData = song.m_SongTiming;
			// get cabinet lights if any
			Steps *pStepsCabinetLights = SongUtil::GetOneSteps( &song, StepsType_lights_cabinet );
			if( pStepsCabinetLights )
				pStepsCabinetLights->GetNoteData( ToPlay.m_LightsData );
		}
		else if(GetExtension(ToPlay.m_sTimingFile) == ".sm" &&
			loaderSM.LoadFromSimfile(ToPlay.m_sTimingFile, song) )
		{
			ToPlay.HasTiming = true;
			ToPlay.m_TimingData = song.m_SongTiming;
			// get cabinet lights if any
			Steps *pStepsCabinetLights = SongUtil::GetOneSteps( &song, StepsType_lights_cabinet );
			if( pStepsCabinetLights )
				pStepsCabinetLights->GetNoteData( ToPlay.m_LightsData );
		}
	}

	if( ToPlay.HasTiming )
	{
		NewMusic->m_NewTiming = ToPlay.m_TimingData;
		NewMusic->m_Lights = ToPlay.m_LightsData;
	}

	if( ToPlay.bAlignBeat && ToPlay.HasTiming && ToPlay.bForceLoop && ToPlay.fLengthSeconds != -1 )
	{
		/* Extend the loop period so it always starts and ends on the same fractional
		 * beat.  That is, if it starts on beat 1.5, and ends on beat 10.2, extend it
		 * to end on beat 10.5.  This way, effects always loop cleanly. */
		float fStartBeat = NewMusic->m_NewTiming.GetBeatFromElapsedTimeNoOffset( ToPlay.fStartSecond );
		float fEndSec = ToPlay.fStartSecond + ToPlay.fLengthSeconds;
		float fEndBeat = NewMusic->m_NewTiming.GetBeatFromElapsedTimeNoOffset( fEndSec );
		
		const float fStartBeatFraction = fmodfp( fStartBeat, 1 );
		const float fEndBeatFraction = fmodfp( fEndBeat, 1 );

		float fBeatDifference = fStartBeatFraction - fEndBeatFraction;
		if( fBeatDifference < 0 )
			fBeatDifference += 1.0f; /* unwrap */

		fEndBeat += fBeatDifference;

		const float fRealEndSec = NewMusic->m_NewTiming.GetElapsedTimeFromBeatNoOffset( fEndBeat );
		const float fNewLengthSec = fRealEndSec - ToPlay.fStartSecond;

		/* Extend fFadeOutLengthSeconds, so the added time is faded out. */
		ToPlay.fFadeOutLengthSeconds += fNewLengthSec - ToPlay.fLengthSeconds;
		ToPlay.fLengthSeconds = fNewLengthSec;
	}

	bool StartImmediately = false;
	if( !ToPlay.HasTiming )
	{
		/* This song has no real timing data.  The offset is arbitrary.  Change it so
		 * the beat will line up to where we are now, so we don't have to delay. */
		float fDestBeat = fmodfp( GAMESTATE->m_Position.m_fSongBeatNoOffset, 1 );
		float fTime = NewMusic->m_NewTiming.GetElapsedTimeFromBeatNoOffset( fDestBeat );

		NewMusic->m_NewTiming.m_fBeat0OffsetInSeconds = fTime;

		StartImmediately = true;
	}

	/* If we have an active timer, try to start on the next update.  Otherwise,
	 * start now. */
	if( !g_Playing->m_bHasTiming && !g_UpdatingTimer )
		StartImmediately = true;
	if( !ToPlay.bAlignBeat )
		StartImmediately = true;

	RageTimer when; /* zero */
	if( !StartImmediately )
	{
		/* GetPlayLatency returns the minimum time until a sound starts.  That's
		 * common when starting a precached sound, but our sound isn't, so it'll
		 * probably take a little longer.  Nudge the latency up. */
		const float fPresumedLatency = SOUNDMAN->GetPlayLatency() + 0.040f;
		const float fCurSecond = GAMESTATE->m_Position.m_fMusicSeconds + fPresumedLatency;
		const float fCurBeat = g_Playing->m_Timing.GetBeatFromElapsedTimeNoOffset( fCurSecond );

		/* The beat that the new sound will start on. */
		const float fStartBeat = NewMusic->m_NewTiming.GetBeatFromElapsedTimeNoOffset( ToPlay.fStartSecond );
		const float fStartBeatFraction = fmodfp( fStartBeat, 1 );

		float fCurBeatToStartOn = truncf(fCurBeat) + fStartBeatFraction;
		if( fCurBeatToStartOn < fCurBeat )
			fCurBeatToStartOn += 1.0f;

		const float fSecondToStartOn = g_Playing->m_Timing.GetElapsedTimeFromBeatNoOffset( fCurBeatToStartOn );
		const float fMaximumDistance = 2;
		const float fDistance = min( fSecondToStartOn - GAMESTATE->m_Position.m_fMusicSeconds, fMaximumDistance );

		when = GAMESTATE->m_Position.m_LastBeatUpdate + fDistance;
	}

	/* Important: don't hold the mutex while we load and seek the actual sound. */
	L.Unlock();
	{
		NewMusic->m_bHasTiming = ToPlay.HasTiming;
		if( ToPlay.HasTiming )
			NewMusic->m_NewTiming = ToPlay.m_TimingData;
		NewMusic->m_bTimingDelayed = true;
		NewMusic->m_bApplyMusicRate = ToPlay.bApplyMusicRate;
//		NewMusic->m_Music->Load( ToPlay.m_sFile, false );

		RageSoundParams p;
		p.m_StartSecond = ToPlay.fStartSecond;
		p.m_LengthSeconds = ToPlay.fLengthSeconds;
		p.m_fFadeInSeconds = ToPlay.fFadeInLengthSeconds;
		p.m_fFadeOutSeconds = ToPlay.fFadeOutLengthSeconds;
		p.m_StartTime = when;
		if( ToPlay.bForceLoop )
			p.StopMode = RageSoundParams::M_LOOP;
		NewMusic->m_Music->SetParams( p );
		NewMusic->m_Music->StartPlaying();
	}

	LockMut( *g_Mutex );
	delete g_Playing;
	g_Playing = NewMusic;
}

static void DoPlayOnce( RString sPath )
{
	/* We want this to start quickly, so don't try to prebuffer it. */
	RageSound *pSound = new RageSound;
	pSound->Load( sPath, false );

	pSound->Play(false);
	pSound->DeleteSelfWhenFinishedPlaying();
}

static void DoPlayOnceFromDir( RString sPath )
{
	if( sPath == "" )
		return;

	// make sure there's a slash at the end of this path
	if( sPath.Right(1) != "/" )
		sPath += "/";

	vector<RString> arraySoundFiles;
	GetDirListing( sPath + "*.mp3", arraySoundFiles );
	GetDirListing( sPath + "*.wav", arraySoundFiles );
	GetDirListing( sPath + "*.ogg", arraySoundFiles );
	GetDirListing( sPath + "*.oga", arraySoundFiles );

	if( arraySoundFiles.empty() )
	{
		return;
	}
	else if (arraySoundFiles.size() == 1)
	{
		DoPlayOnce(  sPath + arraySoundFiles[0]  );
		return;
	}

	g_Mutex->Lock();
	vector<int> &order = g_DirSoundOrder.insert({sPath, vector<int>()}).first->second;
	// If order is exhausted, repopulate and reshuffle
	if (order.size() == 0)
	{
		for (int i = 0; i < (int)arraySoundFiles.size(); ++i)
		{
			order.push_back(i);
		}
		std::random_shuffle(order.begin(), order.end());
	}

	int index = order.back();
	order.pop_back();
	g_Mutex->Unlock();

	DoPlayOnce(  sPath + arraySoundFiles[index]  );
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
	vector<RString> aSoundsToPlayOnce = g_SoundsToPlayOnce;
	g_SoundsToPlayOnce.clear();
	vector<RString> aSoundsToPlayOnceFromDir = g_SoundsToPlayOnceFromDir;
	g_SoundsToPlayOnceFromDir.clear();
	vector<RString> aSoundsToPlayOnceFromAnnouncer = g_SoundsToPlayOnceFromAnnouncer;
	g_SoundsToPlayOnceFromAnnouncer.clear();
	vector<MusicToPlay> aMusicsToPlay = g_MusicsToPlay;
	g_MusicsToPlay.clear();
	g_Mutex->Unlock();

	for( unsigned i = 0; i < aSoundsToPlayOnce.size(); ++i )
		if( aSoundsToPlayOnce[i] != "" )
			DoPlayOnce( aSoundsToPlayOnce[i] );

	for( unsigned i = 0; i < aSoundsToPlayOnceFromDir.size(); ++i )
		DoPlayOnceFromDir( aSoundsToPlayOnceFromDir[i] );

	for( unsigned i = 0; i < aSoundsToPlayOnceFromAnnouncer.size(); ++i )
	{
		RString sPath = aSoundsToPlayOnceFromAnnouncer[i];
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
			CHECKPOINT_M( ssprintf("Removing old sound at index %d", i));
			/* StopPlaying() can take a while, so don't hold the lock while we stop the sound. */
			g_Mutex->Lock();
			RageSound *pOldSound = g_Playing->m_Music;
			g_Playing->m_Music = new RageSound;
			g_Mutex->Unlock();

			delete pOldSound;
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
	ASSERT( SOUNDMAN != nullptr );

	g_Mutex = new RageEvent("GameSoundManager");
	g_Playing = new MusicPlaying( new RageSound );

	g_UpdatingTimer = true;

	g_Shutdown = false;
	MusicThread.SetName( "Music thread" );
	MusicThread.Create( MusicThread_start, this );

	// Register with Lua.
	{
		Lua *L = LUA->Get();
		lua_pushstring( L, "SOUND" );
		this->PushSelf( L );
		lua_settable( L, LUA_GLOBALSINDEX );
		LUA->Release( L );
	}
}

GameSoundManager::~GameSoundManager()
{
	// Unregister with Lua.
	LUA->UnsetGlobal( "SOUND" );

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

	if( iThisFPS != DISPLAY->GetActualVideoModeParams().rate || iThisFPS != iLastFPS )
	{
		iLastFPS = iThisFPS;
		return 0;
	}

	const float fExpectedDelay = 1.0f / iThisFPS;
	const float fExtraDelay = fDeltaTime - fExpectedDelay;
	if( fabsf(fExtraDelay) >= fExpectedDelay/2 )
		return 0;

	/* Subtract the extra delay. */
	return min( -fExtraDelay, 0 );
}

void GameSoundManager::Update( float fDeltaTime )
{
	{
		g_Mutex->Lock();
		if( g_Playing->m_bApplyMusicRate )
		{
			RageSoundParams p = g_Playing->m_Music->GetParams();
			float fRate = GAMESTATE->m_SongOptions.GetPreferred().m_fMusicRate;
			if( p.m_fSpeed != fRate )
			{
				p.m_fSpeed = fRate;
				g_Playing->m_Music->SetParams( p );
			}
		}

		bool bIsPlaying = g_Playing->m_Music->IsPlaying();
		g_Mutex->Unlock();
		if( !bIsPlaying && g_bWasPlayingOnLastUpdate && !g_FallbackMusicParams.sFile.empty() )
		{
			PlayMusic( g_FallbackMusicParams );

			g_FallbackMusicParams.sFile = "";
		}
		g_bWasPlayingOnLastUpdate = bIsPlaying;
	}

	LockMut( *g_Mutex );

	{
		/* Duration of the fade-in and fade-out: */
		//const float fFadeInSpeed = 1.5f;
		//const float fFadeOutSpeed = 0.3f;
		float fFadeInSpeed = g_Playing->m_Music->GetParams().m_fFadeInSeconds;
		float fFadeOutSpeed = g_Playing->m_Music->GetParams().m_fFadeOutSeconds;
		float fVolume = g_Playing->m_Music->GetParams().m_Volume;
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
		if( p.m_Volume != fVolume )
		{
			p.m_Volume = fVolume;
			g_Playing->m_Music->SetParams( p );
		}
	}

	if( !g_UpdatingTimer )
		return;

	const float fAdjust = GetFrameTimingAdjustment( fDeltaTime );
	if( !g_Playing->m_Music->IsPlaying() )
	{
		/* There's no song playing.  Fake it. */
		CHECKPOINT_M( ssprintf("%f, delta %f", GAMESTATE->m_Position.m_fMusicSeconds, fDeltaTime) );
		GAMESTATE->UpdateSongPosition( GAMESTATE->m_Position.m_fMusicSeconds + fDeltaTime * g_Playing->m_Music->GetPlaybackRate() , g_Playing->m_Timing );
		return;
	}

	/* There's a delay between us calling Play() and the sound actually playing.
	 * During this time, m_bApproximate will be true.  Keep using the previous timing
	 * data until we get a non-approximate time, indicating that the sound has actually
	 * started playing. */
	bool m_bApproximate;
	RageTimer tm;
	const float fSeconds = g_Playing->m_Music->GetPositionSeconds( &m_bApproximate, &tm );

	// Check for song timing skips.
	if( PREFSMAN->m_bLogSkips && !g_Playing->m_bTimingDelayed )
	{
		const float fExpectedTimePassed = (tm - GAMESTATE->m_Position.m_LastBeatUpdate) * g_Playing->m_Music->GetPlaybackRate();
		const float fSoundTimePassed = fSeconds - GAMESTATE->m_Position.m_fMusicSeconds;
		const float fDiff = fExpectedTimePassed - fSoundTimePassed;

		static RString sLastFile = "";
		const RString ThisFile = g_Playing->m_Music->GetLoadedFilePath();

		/* If fSoundTimePassed < 0, the sound has probably looped. */
		if( sLastFile == ThisFile && fSoundTimePassed >= 0 && fabsf(fDiff) > 0.003f )
			LOG->Trace("Song position skip in %s: expected %.3f, got %.3f (cur %f, prev %f) (%.3f difference)",
				Basename(ThisFile).c_str(), fExpectedTimePassed, fSoundTimePassed, fSeconds, GAMESTATE->m_Position.m_fMusicSeconds, fDiff );
		sLastFile = ThisFile;
	}

	// If g_Playing->m_bTimingDelayed, we're waiting for the new music to actually start
	// playing.
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
		GAMESTATE->UpdateSongPosition( GAMESTATE->m_Position.m_fMusicSeconds + fDeltaTime, g_Playing->m_Timing );
	}
	else
	{
		GAMESTATE->UpdateSongPosition( fSeconds + fAdjust, g_Playing->m_Timing, tm + fAdjust );
	}

	// Send crossed messages
	if( GAMESTATE->m_pCurSong )
	{
		static int iBeatLastCrossed = 0;

		float fSongBeat = GAMESTATE->m_Position.m_fSongBeat;

		int iRowNow = BeatToNoteRowNotRounded( fSongBeat );
		iRowNow = max( 0, iRowNow );

		int iBeatNow = iRowNow / ROWS_PER_BEAT;

		for( int iBeat = iBeatLastCrossed+1; iBeat<=iBeatNow; ++iBeat )
		{
			Message msg("CrossedBeat");
			msg.SetParam( "Beat", iBeat );
			MESSAGEMAN->Broadcast( msg );
		}

		iBeatLastCrossed = iBeatNow;
	}

	// Update lights
	NoteData &lights = g_Playing->m_Lights;
	if( lights.GetNumTracks() > 0 )	// lights data was loaded
	{
		const float fSongBeat = GAMESTATE->m_Position.m_fLightSongBeat;
		const int iSongRow = BeatToNoteRowNotRounded( fSongBeat );

		static int iRowLastCrossed = 0;

		FOREACH_CabinetLight( cl )
		{	
			// Are we "holding" the light?
			if( lights.IsHoldNoteAtRow( cl, iSongRow ) )
			{
				LIGHTSMAN->BlinkCabinetLight( cl );
				continue;
			}

			// Otherwise, for each index we crossed since the last update:
			FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( lights, cl, r, iRowLastCrossed+1, iSongRow+1 )
			{
				if( lights.GetTapNote( cl, r ).type != TapNoteType_Empty )
				{
					LIGHTSMAN->BlinkCabinetLight( cl );
					break;
				}
			}
		}

		iRowLastCrossed = iSongRow;
	}
}


RString GameSoundManager::GetMusicPath() const
{
	LockMut( *g_Mutex );
	return g_Playing->m_Music->GetLoadedFilePath();
}

void GameSoundManager::PlayMusic( 
	RString sFile, 
	const TimingData *pTiming, 
	bool bForceLoop,
	float fStartSecond, 
	float fLengthSeconds, 
	float fFadeInLengthSeconds, 
	float fFadeOutLengthSeconds, 
	bool bAlignBeat,
	bool bApplyMusicRate
	)
{
	PlayMusicParams params;
	params.sFile = sFile;
	params.pTiming = pTiming;
	params.bForceLoop = bForceLoop;
	params.fStartSecond = fStartSecond;
	params.fLengthSeconds = fLengthSeconds;
	params.fFadeInLengthSeconds = fFadeInLengthSeconds;
	params.fFadeOutLengthSeconds = fFadeOutLengthSeconds;
	params.bAlignBeat = bAlignBeat;
	params.bApplyMusicRate = bApplyMusicRate;
	PlayMusic( params );
}

void GameSoundManager::PlayMusic( PlayMusicParams params, PlayMusicParams FallbackMusicParams )
{
	g_FallbackMusicParams = FallbackMusicParams;

	//	LOG->Trace("play '%s' (current '%s')", file.c_str(), g_Playing->m_Music->GetLoadedFilePath().c_str());

	MusicToPlay ToPlay;

	ToPlay.m_sFile = params.sFile;
	if( params.pTiming )
	{
		ToPlay.HasTiming = true;
		ToPlay.m_TimingData = *params.pTiming;
	}
	else
	{
		/* If no timing data was provided, look for it in the same place as the music file. */
		// todo: allow loading .ssc files as well -aj
		ToPlay.m_sTimingFile = SetExtension( params.sFile, "sm" );
	}

	ToPlay.bForceLoop = params.bForceLoop;
	ToPlay.fStartSecond = params.fStartSecond;
	ToPlay.fLengthSeconds = params.fLengthSeconds;
	ToPlay.fFadeInLengthSeconds = params.fFadeInLengthSeconds;
	ToPlay.fFadeOutLengthSeconds = params.fFadeOutLengthSeconds;
	ToPlay.bAlignBeat = params.bAlignBeat;
	ToPlay.bApplyMusicRate = params.bApplyMusicRate;

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
		g_fOriginalVolume = g_Playing->m_Music->GetParams().m_Volume;
	// otherwise, g_fOriginalVolume is already set and m_Volume will be the
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

void GameSoundManager::PlayOnce( RString sPath )
{
	/* Add the sound to the g_SoundsToPlayOnce queue. */
	g_Mutex->Lock();
	g_SoundsToPlayOnce.push_back( sPath );
	g_Mutex->Broadcast();
	g_Mutex->Unlock();
}

void GameSoundManager::PlayOnceFromDir( RString sPath )
{
	/* Add the path to the g_SoundsToPlayOnceFromDir queue. */
	g_Mutex->Lock();
	g_SoundsToPlayOnceFromDir.push_back( sPath );
	g_Mutex->Broadcast();
	g_Mutex->Unlock();
}

void GameSoundManager::PlayOnceFromAnnouncer( RString sPath )
{
	/* Add the path to the g_SoundsToPlayOnceFromAnnouncer queue. */
	g_Mutex->Lock();
	g_SoundsToPlayOnceFromAnnouncer.push_back( sPath );
	g_Mutex->Broadcast();
	g_Mutex->Unlock();
}

float GameSoundManager::GetPlayerBalance( PlayerNumber pn )
{
	/* If two players are active, play sounds on each players' side. */
	if( GAMESTATE->GetNumPlayersEnabled() == 2 )
		return (pn == PLAYER_1)? -1.0f:1.0f;
	else
		return 0;
}


#include "LuaBinding.h"

/** @brief Allow Lua to have access to the GameSoundManager. */ 
class LunaGameSoundManager: public Luna<GameSoundManager>
{
public:
	static int DimMusic( T* p, lua_State *L )
	{
		float fVolume = FArg(1);
		float fDurationSeconds = FArg(2);
		p->DimMusic( fVolume, fDurationSeconds );
		COMMON_RETURN_SELF;
	}
	static int PlayOnce( T* p, lua_State *L )
	{
		RString sPath = SArg(1);
		if(lua_toboolean(L, 2) && PREFSMAN->m_MuteActions)
		{
			COMMON_RETURN_SELF;
		}
		p->PlayOnce( sPath );
		COMMON_RETURN_SELF;
	}
	static int PlayAnnouncer( T* p, lua_State *L )
	{
		RString sPath = SArg(1);
		p->PlayOnceFromAnnouncer( sPath );
		COMMON_RETURN_SELF;
	}
	static int GetPlayerBalance( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		lua_pushnumber( L, p->GetPlayerBalance(pn) );
		return 1;
	}
	static int PlayMusicPart( T* p, lua_State *L )
	{
		RString musicPath = SArg(1);
		float musicStart = FArg(2);
		float musicLength = FArg(3);
		float fadeIn = 0;
		float fadeOut = 0;
		bool loop= false;
		bool applyRate= false;
		bool alignBeat= true;
		if(!lua_isnoneornil(L, 4))
		{
			fadeIn = FArg(4);
		}
		if(!lua_isnoneornil(L, 5))
		{
			fadeOut = FArg(5);
		}
		if(!lua_isnoneornil(L, 6))
		{
			loop = BArg(6);
		}
		if(!lua_isnoneornil(L, 7))
		{
			applyRate = BArg(7);
		}
		if(!lua_isnoneornil(L, 8))
		{
			alignBeat = BArg(8);
		}
		p->PlayMusic(musicPath, nullptr, loop, musicStart, musicLength,
			fadeIn, fadeOut, alignBeat, applyRate);
		COMMON_RETURN_SELF;
	}

	static int StopMusic( T* p, lua_State *L )			{ p->StopMusic(); COMMON_RETURN_SELF; }
	static int IsTimingDelayed( T* p, lua_State *L )	{ lua_pushboolean( L, g_Playing->m_bTimingDelayed ); return 1; }
	
	LunaGameSoundManager()
	{
		ADD_METHOD( DimMusic );
		ADD_METHOD( PlayOnce );
		ADD_METHOD( PlayAnnouncer );
		ADD_METHOD( GetPlayerBalance );
		ADD_METHOD( PlayMusicPart );
		ADD_METHOD( StopMusic );
		ADD_METHOD( IsTimingDelayed );
	}
};

LUA_REGISTER_CLASS(GameSoundManager);

int LuaFunc_get_sound_driver_list(lua_State* L);
int LuaFunc_get_sound_driver_list(lua_State* L)
{
	vector<RString> driver_names;
	split(RageSoundDriver::GetDefaultSoundDriverList(), ",", driver_names, true);
	lua_createtable(L, driver_names.size(), 0);
	for(size_t n= 0; n < driver_names.size(); ++n)
	{
		lua_pushstring(L, driver_names[n].c_str());
		lua_rawseti(L, -2, n+1);
	}
	return 1;
}
LUAFUNC_REGISTER_COMMON(get_sound_driver_list);


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

