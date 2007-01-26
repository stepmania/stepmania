/*
 * Handle loading and decoding of sounds.
 *
 * For small files, pre-decode the entire file into a regular buffer.  We
 * might want to play many samples at once, and we don't want to have to decode
 * 5-10 mp3s simultaneously during play.
 *
 * For larger files, decode them on the fly.  These are usually music, and there's
 * usually only one of those playing at a time.  When we get updates, decode data
 * at the same rate we're playing it.  If we don't do this, and we're being read
 * in large chunks, we're forced to decode in larger chunks as well, which can
 * cause framerate problems.
 *
 * Error handling:
 * Decoding errors (eg. CRC failures) will be recovered from when possible.
 *
 * When they can't be recovered, the sound will stop (unless loop or !autostop)
 * and the error will be available in GetError().
 *
 * Seeking past the end of the file will throw a warning and rewind.
 */

#include "global.h"
#include "RageSound.h"
#include "RageSoundManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "RageSoundUtil.h"

#include "RageSoundReader_Extend.h"
#include "RageSoundReader_Pan.h"
#include "RageSoundReader_PitchChange.h"
#include "RageSoundReader_PostBuffering.h"
#include "RageSoundReader_Preload.h"
#include "RageSoundReader_Resample_Good.h"
#include "RageSoundReader_FileReader.h"
#include "RageSoundReader_ThreadedBuffer.h"

#define samplerate() m_pSource->GetSampleRate()

RageSoundParams::RageSoundParams():
	m_StartTime( RageZeroTimer )
{
	m_StartSecond = 0;
	m_LengthSeconds = -1;
	m_FadeLength = 0;
	m_Volume = 1.0f;
	m_fPitch = 1.0f;
	m_fSpeed = 1.0f;
	StopMode = M_AUTO;
	m_bIsCriticalSound = false;
}

RageSoundLoadParams::RageSoundLoadParams()
{
	m_bSupportRateChanging = false;
	m_bSupportPan = false;
}

RageSound::RageSound():
	m_Mutex( "RageSound" )
{
	ASSERT( SOUNDMAN );

	m_pSource = NULL;
	m_iStreamFrame = 0;
	m_iStoppedSourceFrame = 0;
	m_bPlaying = false;
	m_bDeleteWhenFinished = false;
}

RageSound::~RageSound()
{
	Unload();
}

RageSound::RageSound( const RageSound &cpy ):
	RageSoundBase( cpy ),
	m_Mutex( "RageSound" )
{
	ASSERT(SOUNDMAN);

	m_pSource = NULL;

	*this = cpy;
}

RageSound &RageSound::operator=( const RageSound &cpy )
{
	LockMut(cpy.m_Mutex);

	/* If m_bDeleteWhenFinished, then nobody that has a reference to the sound should
	 * be making copies. */
	ASSERT( !cpy.m_bDeleteWhenFinished );

	m_Param = cpy.m_Param;
	m_iStreamFrame = cpy.m_iStreamFrame;
	m_iStoppedSourceFrame = cpy.m_iStoppedSourceFrame;
	m_bPlaying = false;
	m_bDeleteWhenFinished = false;

	delete m_pSource;
	if( cpy.m_pSource )
		m_pSource = cpy.m_pSource->Copy();
	else
		m_pSource = NULL;

	m_sFilePath = cpy.m_sFilePath;

	return *this;
}

void RageSound::Unload()
{
	if( IsPlaying() )
		StopPlaying();

	LockMut(m_Mutex);

	delete m_pSource;
	m_pSource = NULL;
	
	m_sFilePath = "";
}

/* The sound will self-delete itself when it stops playing.  If the sound is not
 * playing, the sound will be deleted immediately.  The caller loses ownership
 * of the sound. */
void RageSound::DeleteSelfWhenFinishedPlaying()
{
	m_Mutex.Lock();

	if( !m_bPlaying )
	{
		m_Mutex.Unlock();
		delete this;
		return;
	}

	m_bDeleteWhenFinished = true;
	m_Mutex.Unlock();
}

bool RageSound::IsLoaded() const
{
	return m_pSource != NULL;
}

class RageSoundReader_Silence: public RageSoundReader
{
public:
	int GetLength() const { return 0; }
	int GetLength_Fast() const { return 0; }
	int SetPosition( int iFrame )  { return 1; }
	int Read( int16_t *pBuf, int iFrames ) { return RageSoundReader::END_OF_FILE; }
	RageSoundReader *Copy() const { return new RageSoundReader_Silence; }
	int GetSampleRate() const { return 44100; }
	unsigned GetNumChannels() const { return 1; }
	int GetNextSourceFrame() const { return 0; }
	float GetStreamToSourceRatio() const { return 1.0f; }
	RString GetError() const { return ""; }
};


bool RageSound::Load( RString sSoundFilePath )
{
	/* Automatically determine whether to precache */
	/* TODO: Hook this up to a pref? */
	return Load( sSoundFilePath, false );
}

bool RageSound::Load( RString sSoundFilePath, bool bPrecache, const RageSoundLoadParams *pParams )
{
	LOG->Trace( "RageSound::LoadSound( '%s', %d )", sSoundFilePath.c_str(), bPrecache );

	if( pParams == NULL )
	{
		static const RageSoundLoadParams Defaults;
		pParams = &Defaults;
	}

	/* If this sound is already preloaded and held by SOUNDMAN, just make a copy
	 * of that.  Since RageSoundReader_Preload is refcounted, this is cheap. */
	RageSoundReader *pSound = SOUNDMAN->GetLoadedSound( sSoundFilePath );
	if( pSound == NULL )
	{
		RString error;
		pSound = RageSoundReader_FileReader::OpenFile( sSoundFilePath, error );
		if( pSound == NULL )
		{
			LOG->Warn( "RageSound::Load: error opening sound \"%s\": %s",
				sSoundFilePath.c_str(), error.c_str() );

			pSound = new RageSoundReader_Silence;
		}
	}
	else
	{
		/* The sound we were given from SOUNDMAN is already preloaded. */
		bPrecache = false;
	}

	LoadSoundReader( pSound );

	/* Try to precache.  Do this after calling LoadSoundReader() to put the
	 * sound in this->m_pSource, so we preload after resampling. */
	if( bPrecache )
	{
		if( RageSoundReader_Preload::PreloadSound(m_pSource) )
		{
			/* We've preloaded the sound.  Pass it to SOUNDMAN, for reuse. */
			SOUNDMAN->AddLoadedSound( sSoundFilePath, (RageSoundReader_Preload *) m_pSource );
		}
		else
		{
			bPrecache = false;
		}
	}

	m_pSource = new RageSoundReader_Extend( m_pSource );
	if( !bPrecache )
		m_pSource = new RageSoundReader_ThreadedBuffer( m_pSource );
	m_pSource = new RageSoundReader_PostBuffering( m_pSource );

	if( pParams->m_bSupportRateChanging )
	{
		RageSoundReader_PitchChange *pRate = new RageSoundReader_PitchChange( m_pSource );
		m_pSource = pRate;
	}

	if( pParams->m_bSupportPan )
		m_pSource = new RageSoundReader_Pan( m_pSource );

	m_sFilePath = sSoundFilePath;

	m_Mutex.SetName( ssprintf("RageSound (%s)", Basename(sSoundFilePath).c_str() ) );

	return true;
}

void RageSound::LoadSoundReader( RageSoundReader *pSound )
{
	Unload();

	m_iStreamFrame = m_iStoppedSourceFrame = 0;

	const int iNeededRate = SOUNDMAN->GetDriverSampleRate();
	bool bSupportRateChange = false;
	if( iNeededRate != pSound->GetSampleRate() || bSupportRateChange )
	{
		RageSoundReader_Resample_Good *Resample = new RageSoundReader_Resample_Good( pSound, iNeededRate );
		pSound = Resample;
	}

	m_pSource = pSound;
}

/*
 * Retrieve audio data, for mixing.  At the time of this call, the frameno at which the
 * sound will be played doesn't have to be known.  Once committed, and the frameno
 * is known, call CommitPCMData.
 *
 * RageSound::GetDataToPlay and RageSound::FillBuf are the main threaded API.  These
 * need to execute without blocking other threads from calling eg. GetPositionSeconds,
 * since they may take some time to run.
 *
 * On underrun, if no data was read, returns WOULD_BLOCK.  On end of file, if no
 * data was read, returns END_OF_FILE.  If any data is read, it is returned; these
 * conditions are masked and will be seen on the next call.  Otherwise, the requested 
 * number of frames will always be returned.
 */
int RageSound::GetDataToPlay( int16_t *pBuffer, int iFrames, int64_t &iStreamFrame, int &iFramesStored )
{
	/* We only update m_iStreamFrame; only take a shared lock, so we don't block the main thread. */
//	LockMut(m_Mutex);

	ASSERT_M( m_bPlaying, ssprintf("%p", this) );
	ASSERT( m_pSource );

	iFramesStored = 0;
	iStreamFrame = m_iStreamFrame;

	while( iFrames > 0 )
	{
		float fRate = 1.0f;
		int iSourceFrame;

		/* Read data from our source. */
		int iGotFrames = m_pSource->RetriedRead( pBuffer + (iFramesStored * m_pSource->GetNumChannels()), iFrames, &iSourceFrame, &fRate );

		if( iGotFrames == RageSoundReader::ERROR )
		{
			m_sError = m_pSource->GetError();
			LOG->Warn( "Decoding %s failed: %s", GetLoadedFilePath().c_str(), m_sError.c_str() );
		}

		if( iGotFrames < 0 )
		{
			if( !iFramesStored )
				return iGotFrames;
			else
				break;
		}

		m_Mutex.Lock();
		m_StreamToSourceMap.Insert( m_iStreamFrame, iGotFrames, iSourceFrame, fRate );
		m_Mutex.Unlock();

		m_iStreamFrame += iGotFrames;

		iFramesStored += iGotFrames;
		iFrames -= iGotFrames;
	}
	if( m_pSource->GetNumChannels() == 1 )
		RageSoundUtil::ConvertMonoToStereoInPlace( pBuffer, iFramesStored );

	return iFramesStored;
}

/* Indicate that a block of audio data has been written to the device. */
void RageSound::CommitPlayingPosition( int64_t iHardwareFrame, int64_t iStreamFrame, int iGotFrames )
{
	m_Mutex.Lock();
	m_HardwareToStreamMap.Insert( iHardwareFrame, iGotFrames, iStreamFrame );
	m_Mutex.Unlock();
}

/* Start playing from the current position. */
void RageSound::StartPlaying()
{
	ASSERT( !m_bPlaying );

	// Move to the start position.
	SetPositionFrames( lrintf(m_Param.m_StartSecond * samplerate()) );

	/* If m_StartTime is in the past, then we probably set a start time but took too
	 * long loading.  We don't want that; log it, since it can be unobvious. */
	if( !m_Param.m_StartTime.IsZero() && m_Param.m_StartTime.Ago() > 0 )
		LOG->Trace("Sound \"%s\" has a start time %f seconds in the past",
			GetLoadedFilePath().c_str(), m_Param.m_StartTime.Ago() );

	/* Tell the sound manager to start mixing us. */
//	LOG->Trace("set playing true for %p (StartPlaying) (%s)", this, this->GetLoadedFilePath().c_str());

	m_bPlaying = true;

	if( !m_Param.m_bIsCriticalSound && SOUNDMAN->GetPlayOnlyCriticalSounds() )
		m_Param.m_Volume = 0;
	ApplyParams();

	/* Don't lock while calling SOUNDMAN driver calls. */
	ASSERT( !m_Mutex.IsLockedByThisThread() );

	SOUNDMAN->StartMixing( this );

//	LOG->Trace("StartPlaying %p finished (%s)", this, this->GetLoadedFilePath().c_str());
}

void RageSound::StopPlaying()
{
	/* Don't lock while calling SOUNDMAN driver calls. */
	ASSERT( !m_Mutex.IsLockedByThisThread() );

	/* Tell the sound driver to stop mixing this sound. */
	SOUNDMAN->StopMixing(this);
}

/* This is called by sound drivers when we're done playing. */
void RageSound::SoundIsFinishedPlaying()
{
	if( !m_bPlaying )
		return;

	/* Get our current hardware position. */
	int64_t iCurrentHardwareFrame = SOUNDMAN->GetPosition( NULL );

	m_Mutex.Lock();

	if( m_bDeleteWhenFinished )
	{
		m_bDeleteWhenFinished = false;
		m_Mutex.Unlock();
		delete this;
		return;
	}

	/* Lock the mutex after calling SOUNDMAN->GetPosition().  We must not make driver
	 * calls with our mutex locked (driver mutex < sound mutex). */
	if( !m_HardwareToStreamMap.IsEmpty() && !m_StreamToSourceMap.IsEmpty() )
		m_iStoppedSourceFrame = (int) GetSourceFrameFromHardwareFrame( iCurrentHardwareFrame );

//	LOG->Trace("set playing false for %p (SoundIsFinishedPlaying) (%s)", this, this->GetLoadedFilePath().c_str());
	m_bPlaying = false;

	m_HardwareToStreamMap.Clear();
	m_StreamToSourceMap.Clear();

//	LOG->Trace("SoundIsFinishedPlaying %p finished (%s)", this, this->GetLoadedFilePath().c_str());

	m_Mutex.Unlock();
}

void RageSound::Play( const RageSoundParams *pParams )
{
	if( m_pSource == NULL )
	{
		LOG->Warn( "RageSound::Play: sound not loaded" );
		return;
	}

	if( IsPlaying() )
	{
		PlayCopy( pParams );
		return;
	}

	if( pParams )
		SetParams( *pParams );

	StartPlaying();
}

void RageSound::PlayCopy( const RageSoundParams *pParams ) const
{
	RageSound *pSound = new RageSound( *this );

	if( pParams )
		pSound->SetParams( *pParams );

	pSound->StartPlaying();
	pSound->DeleteSelfWhenFinishedPlaying();
}

void RageSound::Stop()
{
	StopPlaying();
}

bool RageSound::Pause( bool bPause )
{
	if( m_pSource == NULL )
	{
		LOG->Warn( "RageSound::Pause: sound not loaded" );
		return false;
	}

	return SOUNDMAN->Pause( this, bPause );
}
	

float RageSound::GetLengthSeconds()
{
	if( m_pSource == NULL )
	{
		LOG->Warn( "RageSound::GetLengthSeconds: sound not loaded" );
		return -1;
	}

	int iLength = m_pSource->GetLength();

	if( iLength < 0 )
	{
		LOG->Warn( "GetLengthSeconds failed on %s: %s", GetLoadedFilePath().c_str(), m_pSource->GetError().c_str() );
		return -1;
	}

	return iLength / 1000.f; /* ms -> secs */
}

int RageSound::GetSourceFrameFromHardwareFrame( int64_t iHardwareFrame, bool *bApproximate ) const
{
	if( m_HardwareToStreamMap.IsEmpty() || m_StreamToSourceMap.IsEmpty() )
		return 0;

	bool bApprox;
	int64_t iStreamFrame = m_HardwareToStreamMap.Search( iHardwareFrame, &bApprox );
	if( bApproximate && bApprox )
		*bApproximate = true;
	int64_t iSourceFrame = m_StreamToSourceMap.Search( iStreamFrame, &bApprox );
	if( bApproximate && bApprox )
		*bApproximate = true;
	return (int) iSourceFrame;
}

/*
 * If non-NULL, approximate is set to true if the returned time is approximated because of
 * underrun, the sound not having started (after Play()) or finished (after EOF) yet.
 *
 * If non-NULL, Timestamp is set to the real clock time associated with the returned sound
 * position.  We might take a variable amount of time before grabbing the timestamp (to
 * lock SOUNDMAN); we might lose the scheduler after grabbing it, when releasing SOUNDMAN.
 */
float RageSound::GetPositionSeconds( bool *bApproximate, RageTimer *pTimestamp ) const
{
	/* Get our current hardware position. */
	int64_t iCurrentHardwareFrame = SOUNDMAN->GetPosition( pTimestamp );

	/* Lock the mutex after calling SOUNDMAN->GetPosition().  We must not make driver
	 * calls with our mutex locked (driver mutex < sound mutex). */
	LockMut( m_Mutex );

	if( bApproximate )
		*bApproximate = false;

	/* If we're not playing, just report the static position. */
	if( !IsPlaying() )
		return m_iStoppedSourceFrame / float(samplerate());

	/* If we don't yet have any position data, CommitPlayingPosition hasn't yet been called at all,
	 * so guess what we think the real time is. */
	if( m_HardwareToStreamMap.IsEmpty() || m_StreamToSourceMap.IsEmpty() )
	{
		// LOG->Trace( "no data yet; %i", m_iStoppedSourceFrame );
		if( bApproximate )
			*bApproximate = true;
		return m_iStoppedSourceFrame / float(samplerate());
	}

	int iSourceFrame = GetSourceFrameFromHardwareFrame( iCurrentHardwareFrame, bApproximate );
	return iSourceFrame / float(samplerate());
}


/* This is always the desired sample rate of the current driver. */
int RageSound::GetSampleRate() const
{
	if( m_pSource == NULL )
	{
		LOG->Warn( "RageSound::GetSampleRate(): sound not loaded" );
		return 44100;
	}

	return m_pSource->GetSampleRate();
}

bool RageSound::SetPositionFrames( int iFrames )
{
	LockMut( m_Mutex );

	if( m_pSource == NULL )
	{
		LOG->Warn( "RageSound::SetPositionFrames(%d): sound not loaded", iFrames );
		return false;
	}

	int iRet = m_pSource->SetPosition( iFrames );
	if( iRet == -1 )
	{
		m_sError = m_pSource->GetError();
		LOG->Warn( "SetPositionFrames: seek %s failed: %s", GetLoadedFilePath().c_str(), m_sError.c_str() );
	}
	else if( iRet == 0 )
	{
		/* Seeked past EOF. */
		LOG->Warn( "SetPositionFrames: %i samples is beyond EOF in %s",
			iFrames, GetLoadedFilePath().c_str() );
	}
	else
	{
		m_iStoppedSourceFrame = iFrames;
	}

	return iRet == 1;
}

float RageSound::GetAbsoluteVolume() const
{
	float f = m_Param.m_Volume;
	f *= SOUNDMAN->GetMixVolume();
	return f;
}

float RageSound::GetPlaybackRate() const
{
	return m_Param.m_fSpeed;
}

RageTimer RageSound::GetStartTime() const
{
	return m_Param.m_StartTime;
}

void RageSound::SetParams( const RageSoundParams &p )
{
	m_Param = p;
	ApplyParams();
}

void RageSound::ApplyParams()
{
	if( m_pSource == NULL )
		return;

	m_pSource->SetProperty( "Pitch", m_Param.m_fPitch );
	m_pSource->SetProperty( "Speed", m_Param.m_fSpeed );
	m_pSource->SetProperty( "StartSecond", m_Param.m_StartSecond );
	m_pSource->SetProperty( "LengthSeconds", m_Param.m_LengthSeconds );
	m_pSource->SetProperty( "FadeSeconds", m_Param.m_FadeLength );
	switch( GetStopMode() )
	{
	case RageSoundParams::M_LOOP:
		m_pSource->SetProperty( "Loop", 1.0f );
		break;
	case RageSoundParams::M_STOP:
		m_pSource->SetProperty( "Stop", 1.0f );
		break;
	case RageSoundParams::M_CONTINUE:
		m_pSource->SetProperty( "Continue", 1.0f );
		break;
	}
}

bool RageSound::SetProperty( const RString &sProperty, float fValue )
{
	return m_pSource->SetProperty( sProperty, fValue );
}

RageSoundParams::StopMode_t RageSound::GetStopMode() const
{
	if( m_Param.StopMode != RageSoundParams::M_AUTO )
		return m_Param.StopMode;

	if( m_sFilePath.find("loop") != string::npos )
		return RageSoundParams::M_LOOP;
	else
		return RageSoundParams::M_STOP;
}


// lua start
#include "LuaBinding.h"

class LunaRageSound: public Luna<RageSound>
{
public:
	static int pitch( T* p, lua_State *L )
	{
		RageSoundParams params( p->GetParams() );
		params.m_fPitch = FArg(1);
		p->SetParams( params );
		return 0;
	}

	static int speed( T* p, lua_State *L )
	{
		RageSoundParams params( p->GetParams() );
		params.m_fSpeed = FArg(1);
		p->SetParams( params );
		return 0;
	}

	static int SetProperty( T* p, lua_State *L )
	{
		LuaHelpers::Push( L, p->SetProperty(SArg(1), FArg(2)) );
		return 1;
	}

	LunaRageSound()
	{
		ADD_METHOD( pitch );
		ADD_METHOD( speed );
		ADD_METHOD( SetProperty );
	}
};

LUA_REGISTER_CLASS( RageSound )
// lua end

/*
 * Copyright (c) 2002-2004 Glenn Maynard
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

