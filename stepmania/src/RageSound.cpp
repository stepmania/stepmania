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
#include "arch/ArchHooks/ArchHooks.h"
#include "RageSoundUtil.h"

#include "RageSoundReader_Preload.h"
#include "RageSoundReader_Resample_Good.h"
#include "RageSoundReader_FileReader.h"

static const int channels = 2;
static const int framesize = 2 * channels; /* 16-bit */
#define samplerate() m_pSource->GetSampleRate()

RageSoundParams::RageSoundParams():
	m_StartTime( RageZeroTimer )
{
	m_StartSecond = 0;
	m_LengthSeconds = -1;
	m_FadeLength = 0;
	m_Volume = 1.0f;
	m_Balance = 0; // center
	m_fRate = 1.0f;
	m_bAccurateSync = false;
	StopMode = M_AUTO;
	m_bIsCriticalSound = false;
}

RageSound::RageSound():
	m_Mutex( "RageSound" )
{
	ASSERT( SOUNDMAN );

	m_pSource = NULL;
	m_iStreamFrame = 0;
	m_iSourceFrame = 0;
	m_iStoppedSourceFrame = 0;
	m_iMaxDriverFrame = 0;
	m_bPlaying = false;

	m_iID = SOUNDMAN->GetUniqueID();

	/* Register ourself last, once everything is initialized. */
	SOUNDMAN->RegisterSound( this );
}

RageSound::~RageSound()
{
	Unload();

	/* Unregister ourself. */
	SOUNDMAN->UnregisterSound( this );
}

RageSound::RageSound( const RageSound &cpy ):
	RageSoundBase( cpy ),
	m_Mutex( "RageSound" )
{
	ASSERT(SOUNDMAN);

	m_pSource = NULL;

	*this = cpy;

	/* We have a different ID than our parent. */
	m_iID = SOUNDMAN->GetUniqueID();

	/* Register ourself. */
	SOUNDMAN->RegisterSound( this );
}

RageSound &RageSound::operator=( const RageSound &cpy )
{
	LockMut(cpy.m_Mutex);

	m_Param = cpy.m_Param;
	m_iStreamFrame = cpy.m_iStreamFrame;
	m_iSourceFrame = cpy.m_iSourceFrame;
	m_iStoppedSourceFrame = cpy.m_iStoppedSourceFrame;
	m_iMaxDriverFrame = 0;
	m_bPlaying = false;

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
	LockMut(m_Mutex);

	if(IsPlaying())
		StopPlaying();

	delete m_pSource;
	m_pSource = NULL;
	
	m_sFilePath = "";
}

bool RageSound::IsLoaded() const
{
	return m_pSource != NULL;
}

void RageSound::Fail( RString sReason )
{
	LOG->Warn( "Decoding %s failed: %s", GetLoadedFilePath().c_str(), sReason.c_str() );

	m_sError = sReason;
}

class RageSoundReader_Silence: public RageSoundReader
{
public:
	int GetLength() const { return 0; }
	int GetLength_Fast() const { return 0; }
	int SetPosition_Accurate(int ms)  { return 0; }
	int SetPosition_Fast(int ms) { return 0; }
	int Read(char *buf, unsigned len) { return 0; }
	RageSoundReader *Copy() const { return new RageSoundReader_Silence; }
	int GetSampleRate() const { return 44100; }
	bool IsStreamingFromDisk() const { return false; }
	int GetNextSourceFrame() const { return 0; }
	float GetStreamToSourceRatio() const { return 1.0f; }
};


bool RageSound::Load( RString sSoundFilePath )
{
	/* Automatically determine whether to precache */
	/* TODO: Hook this up to a pref? */
	return Load( sSoundFilePath, false );
}

bool RageSound::Load( RString sSoundFilePath, bool bPrecache )
{
	LOG->Trace( "RageSound::LoadSound( '%s', %d )", sSoundFilePath.c_str(), bPrecache );

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

	m_sFilePath = sSoundFilePath;

	m_Mutex.SetName( ssprintf("RageSound (%s)", Basename(sSoundFilePath).c_str() ) );

	return true;
}

void RageSound::LoadSoundReader( RageSoundReader *pSound )
{
	Unload();

	m_iStreamFrame = m_iSourceFrame = m_iStoppedSourceFrame = 0;

	const int iNeededRate = SOUNDMAN->GetDriverSampleRate( pSound->GetSampleRate() );
	bool bSupportRateChange = false;
	if( iNeededRate != pSound->GetSampleRate() || bSupportRateChange )
	{
		RageSoundReader_Resample_Good *Resample = new RageSoundReader_Resample_Good( pSound, iNeededRate );
		pSound = Resample;
	}

	pSound->SetProperty( "Speed", 1.5f );

	m_pSource = pSound;
}

/* Get a block of data from the input.  If buffer is NULL, just return the amount
 * that would be read. */
int RageSound::GetData( char *pBuffer, int iFrames )
{
	if( m_Param.m_LengthSeconds != -1 )
	{
		/* We have a length; only read up to the end. */
		const float fLastSecond = m_Param.m_StartSecond + m_Param.m_LengthSeconds;
		int iFramesToRead = int(fLastSecond*samplerate()) - m_iSourceFrame;

		/* If it's negative, we're past the end, so cap it at 0. Don't read
		 * more than size. */
		iFrames = clamp( iFramesToRead, 0, iFrames );
	}

	int iGotFrames = 0;
	float fRate = 1.0f;
	int iSourceFrame = m_iSourceFrame;
	if( m_iSourceFrame < 0 )
	{
		/* We havn't *really* started playing yet, so just feed silence.  How
		 * many more bytes of silence do we need? */
		iSourceFrame =  -m_iSourceFrame;
		iGotFrames = -m_iSourceFrame;
		iGotFrames = min( iGotFrames, iFrames );
		memset( pBuffer, 0, iGotFrames*framesize );
	}

	if( iGotFrames == 0 && iFrames )
	{
		/* Read data from our source. */
		ASSERT( m_pSource );

		fRate = m_pSource->GetStreamToSourceRatio();
		int iNewSourceFrame = m_pSource->GetNextSourceFrame();
		int iReadSize = iFrames * sizeof(int16_t) * m_pSource->GetNumChannels();
		iGotFrames = m_pSource->Read( pBuffer, iReadSize );
		if( iGotFrames == -1 )
		{
			Fail( m_pSource->GetError() );

			/* Pretend we got EOF. */
			return 0;
		}
		iGotFrames /= sizeof(int16_t) * m_pSource->GetNumChannels();

		/* If we didn't get any data, don't update iSourceFrame, so we just keep
		 * extrapolating if we're in M_CONTINUE. */
		if( iGotFrames != 0 )
			iSourceFrame = iNewSourceFrame;

		if( m_pSource->GetNumChannels() == 1 )
			RageSoundUtil::ConvertMonoToStereoInPlace( (int16_t *) pBuffer, iGotFrames );
	}
//		LOG->Trace( "add %i, %f (%i)", iSourceFrame, fRate, iGotFrames );

	/* If we didn't get any data, see if we need to pad the end of the file with
	 * silence for m_LengthSeconds. */
	if( iGotFrames == 0 && m_Param.m_LengthSeconds != -1 )
	{
		const float fLastSecond = m_Param.m_StartSecond + m_Param.m_LengthSeconds;
		int iLastFrame = int(fLastSecond*samplerate());
		int iFramesOfSilence = iLastFrame - m_iSourceFrame;
		iFramesOfSilence = clamp( iFramesOfSilence, 0, iFrames );
		if( iFramesOfSilence > 0 )
		{
			memset( pBuffer, 0, iFramesOfSilence * framesize );
			iGotFrames = iFramesOfSilence;
		}
	}

	if( iGotFrames == 0 && GetStopMode() == RageSoundParams::M_CONTINUE )
	{
		/* Keep playing silence past EOF. */
		memset( pBuffer, 0, iFrames*framesize );
		iGotFrames = iFrames;
	}

	/* We want to fade when there's m_FadeLength seconds left, but if
	 * m_LengthFrames is -1, we don't know the length we're playing.
	 * (m_LengthFrames is the length to play, not the length of the
	 * source.)  If we don't know the length, don't fade. */
	if( m_Param.m_FadeLength != 0 && m_Param.m_LengthSeconds != -1 )
	{
		const float fFinishFadingOutAt = m_Param.m_StartSecond + m_Param.m_LengthSeconds;
		const float fStartFadingOutAt = fFinishFadingOutAt - m_Param.m_FadeLength;
		const float fStartSecond = float(iSourceFrame) / samplerate();
		const float fEndSecond = float(iSourceFrame+lrintf(iGotFrames * fRate)) / samplerate();
		const float fStartVolume = SCALE( fStartSecond, fStartFadingOutAt, fFinishFadingOutAt, 1.0f, 0.0f );
		const float fEndVolume = SCALE( fEndSecond, fStartFadingOutAt, fFinishFadingOutAt, 1.0f, 0.0f );
		RageSoundUtil::Fade( (int16_t *) pBuffer, iGotFrames, fStartVolume, fEndVolume );
	}

	m_Mutex.Lock();
	m_StreamToSourceMap.Insert( m_iStreamFrame, iGotFrames, iSourceFrame, fRate );
	m_Mutex.Unlock();

	m_iStreamFrame += iGotFrames;
	m_iSourceFrame = iSourceFrame + lrintf(iGotFrames * fRate);

	return iGotFrames;
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
 * If no data is returned (we're at the end of the stream), return false.
 */
bool RageSound::GetDataToPlay( int16_t *pBuffer, int iFrames, int64_t &iStreamFrame, int &iFramesStored )
{
	int iNumRewindsThisCall = 0;

	/* We only update m_iStreamFrame; only take a shared lock, so we don't block the main thread. */
//	LockMut(m_Mutex);

	ASSERT_M( m_bPlaying, ssprintf("%p", this) );

	iFramesStored = 0;
	iStreamFrame = m_iStreamFrame;

	while( 1 )
	{
		/* Get a block of data. */
		int iGotFrames = GetData( (char *) pBuffer, iFrames );

		if( !iGotFrames )
		{
			/* EOF. */
			switch( GetStopMode() )
			{
			case RageSoundParams::M_STOP:
				/* Not looping.  Normally, we'll just stop here. */
				return false;

			case RageSoundParams::M_LOOP:
				/* Rewind and restart. */
				iNumRewindsThisCall++;
				if( iNumRewindsThisCall > 3 )
				{
					/* We're rewinding a bunch of times in one call.  This probably means
					 * that the length is too short.  It might also mean that the start
					 * position is very close to the end of the file, so we're looping
					 * over the remainder.  If we keep doing this, we'll chew CPU rewinding,
					 * so stop. */
					LOG->Warn( "Sound %s is busy looping.  Sound stopped (start = %f, length = %f)",
						GetLoadedFilePath().c_str(), m_Param.m_StartSecond, m_Param.m_LengthSeconds );

					return false;
				}

				/* Rewind and start over.  XXX: this will take an exclusive lock */
				SetPositionSeconds( m_Param.m_StartSecond );
				continue;

			case RageSoundParams::M_CONTINUE:
				FAIL_M("M_CONTINUE");
				break;

			default:
				ASSERT(0);
			}
		}

		/* This block goes from iStreamFrame to iStreamFrame+iGotFrames. */
		RageSoundUtil::Pan( pBuffer, iGotFrames, m_Param.m_Balance );

		iFramesStored = iGotFrames;
		return true;
	}
}

/* Indicate that a block of audio data has been written to the device. */
void RageSound::CommitPlayingPosition( int64_t frameno, int64_t pos, int iGotFrames )
{
	m_Mutex.Lock();
	m_HardwareToStreamMap.Insert( frameno, iGotFrames, pos );
	m_Mutex.Unlock();
}

/* Called by the mixer: return a block of sound data. 
 * Be careful; this is called in a separate thread. */
int RageSound::GetPCM( char *pBuffer, int iSize, int64_t iFrameno )
{
	ASSERT( m_bPlaying );

	/* Now actually put data from the correct buffer into the output. */
	int iBytesStored = 0;
	while( iBytesStored < iSize )
	{
		int64_t iPosition;
		int iGotFrames;
		bool bEof = !GetDataToPlay( (int16_t *)(pBuffer+iBytesStored), (iSize-iBytesStored)/framesize, iPosition, iGotFrames );

		/* Save this frameno/position map. */
		SOUNDMAN->CommitPlayingPosition( GetID(), iFrameno, iPosition, iGotFrames );

		iBytesStored += iGotFrames * framesize;
		iFrameno += iGotFrames;

		if( bEof )
			break;
	}

	return iBytesStored;
}

/* Start playing from the current position. */
void RageSound::StartPlaying()
{
	ASSERT( !m_bPlaying );

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

	SOUNDMAN->StartMixing( this );

//	LOG->Trace("StartPlaying %p finished (%s)", this, this->GetLoadedFilePath().c_str());
}

void RageSound::StopPlaying()
{
	if( !m_bPlaying )
		return;

	m_iStoppedSourceFrame = (int) GetPositionSecondsInternal();

	/* Tell the sound driver to stop mixing this sound. */
	SOUNDMAN->StopMixing(this);

	/* Lock the mutex after calling UnregisterPlayingSound.  We must not make driver
	 * calls with our mutex locked (driver mutex < sound mutex).  Nobody else will
	 * see our sound as not playing until we set playing = false. */
	m_Mutex.Lock();

//	LOG->Trace("set playing false for %p (StopPlaying) (%s)", this, this->GetLoadedFilePath().c_str());
	m_bPlaying = false;
	
	m_iMaxDriverFrame = 0;
	m_HardwareToStreamMap.Clear();
	m_StreamToSourceMap.Clear();

	/* We may still have positions queued up in RageSoundManager.  We need to make sure
	 * that we don't accept those; otherwise, if we start playing again quickly, they'll
	 * confuse GetPositionSeconds().  Do this by changing our ID. */
	SOUNDMAN->UnregisterSound( this );
	m_iID = SOUNDMAN->GetUniqueID();
	SOUNDMAN->RegisterSound( this );

//	LOG->Trace("StopPlaying %p finished (%s)", this, this->GetLoadedFilePath().c_str());

	m_Mutex.Unlock();
}

/* This is similar to StopPlaying, except it's called by sound drivers when we're done
 * playing, rather than by users to as us to stop.  (The only difference is that this
 * doesn't call SOUNDMAN->StopMixing; there's no reason to tell the sound driver to
 * stop mixing, since they're the one telling us we're done.)
 *
 * This is only called from the main thread. */
void RageSound::SoundIsFinishedPlaying()
{
	if( !m_bPlaying )
		return;
	m_Mutex.Lock();

	m_iStoppedSourceFrame = (int) GetPositionSecondsInternal();

//	LOG->Trace("set playing false for %p (SoundIsFinishedPlaying) (%s)", this, this->GetLoadedFilePath().c_str());
	m_bPlaying = false;

	m_HardwareToStreamMap.Clear();
	m_StreamToSourceMap.Clear();

//	LOG->Trace("SoundIsFinishedPlaying %p finished (%s)", this, this->GetLoadedFilePath().c_str());

	m_Mutex.Unlock();
}

RageSound *RageSound::Play( const RageSoundParams *pParams )
{
	if( m_pSource == NULL )
	{
		LOG->Warn( "RageSound::Play: sound not loaded" );
		return NULL;
	}

	return SOUNDMAN->PlaySound( *this, pParams );
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

/* Get the position in frames. */
int64_t RageSound::GetPositionSecondsInternal( bool *bApproximate ) const
{
	LockMut( m_Mutex );

	if( bApproximate )
		*bApproximate = false;

	/* If we're not playing, just report the static position. */
	if( !IsPlaying() )
		return m_iStoppedSourceFrame;

	/* If we don't yet have any position data, GetPCM hasn't yet been called at all,
	 * so guess what we think the real time is. */
	if( m_HardwareToStreamMap.IsEmpty() || m_StreamToSourceMap.IsEmpty() )
	{
		// LOG->Trace( "no data yet; %i", m_iStoppedSourceFrame );
		if( bApproximate )
			*bApproximate = true;
		return m_iStoppedSourceFrame;
	}

	/* Get our current hardware position. */
	int64_t iCurrentHardwareFrame = SOUNDMAN->GetPosition(this);

	/* It's sometimes possible for the hardware position to move backwards, usually
	 * on underrun.  We can try to prevent this in each driver, but it's an obscure
	 * error, so let's clamp the result here instead.  Be sure to reset this on stop,
	 * since the position may reset. */
	if( iCurrentHardwareFrame < m_iMaxDriverFrame )
	{
		/* Clamp the output to one per second, so one underruns don't cascade due to
		 * output spam. */
		static RageTimer last(RageZeroTimer);
		if( last.IsZero() || last.Ago() > 1.0f )
		{
			LOG->Trace( "Sound %s: driver returned a lesser position (%i < %i)",
				this->GetLoadedFilePath().c_str(), (int) iCurrentHardwareFrame, (int) m_iMaxDriverFrame );
			last.Touch();
		}
	}
	m_iMaxDriverFrame = iCurrentHardwareFrame = max( iCurrentHardwareFrame, m_iMaxDriverFrame );

	bool bApprox;
	int64_t iStreamFrame = m_HardwareToStreamMap.Search( iCurrentHardwareFrame, &bApprox );
	if( bApproximate && bApprox )
		*bApproximate = true;
	int64_t iSourceFrame = m_StreamToSourceMap.Search( iStreamFrame, &bApprox );
	if( bApproximate && bApprox )
		*bApproximate = true;

	return iSourceFrame;
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
	LockMut( m_Mutex );

	if( pTimestamp )
	{
		HOOKS->EnterTimeCriticalSection();
		pTimestamp->Touch();
	}

	const int64_t iPositionFrames = GetPositionSecondsInternal( bApproximate );
	const float fPosition = iPositionFrames / float(samplerate());
	if( pTimestamp )
		HOOKS->ExitTimeCriticalSection();

	return fPosition;
}


bool RageSound::SetPositionSeconds( float fSeconds )
{
	if( m_pSource == NULL )
	{
		LOG->Warn( "RageSound::SetPositionSeconds(%f): sound not loaded", fSeconds );
		return false;
	}

	return SetPositionFrames( int(fSeconds * samplerate()) );
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

bool RageSound::IsStreamingFromDisk() const
{
	if( m_pSource == NULL )
	{
		LOG->Warn( "RageSound::IsStreamingFromDisk: sound not loaded" );
		return false;
	}

	return m_pSource->IsStreamingFromDisk();
}

bool RageSound::SetPositionFrames( int iFrames )
{
	LockMut( m_Mutex );

	if( m_pSource == NULL )
	{
		LOG->Warn( "RageSound::SetPositionFrames(%d): sound not loaded", iFrames );
		return false;
	}

	/* The position we're going to seek the input stream to.  We have
	 * to do this in floating point to avoid overflow. */
	int ms = int( float(iFrames) * 1000.f / samplerate() );
	ms = max( ms, 0 );

	int iRet;
	if( m_Param.m_bAccurateSync )
		iRet = m_pSource->SetPosition_Accurate(ms);
	else
		iRet = m_pSource->SetPosition_Fast(ms);

	if( iRet == -1 )
	{
		Fail( m_pSource->GetError() );
		return false; /* failed */
	}

	{
		/* If we're already there, don't do anything. */
		if( m_iSourceFrame == iFrames )
			return true;

		m_iSourceFrame = iFrames;
		m_iStoppedSourceFrame = iFrames;
	}

	if( iRet == 0 && ms != 0 )
	{
		/* We were told to seek somewhere, and we got 0 instead, which means
		 * we passed EOF.  This could be a truncated file or invalid data. */
		LOG->Warn( "SetPositionFrames: %i ms is beyond EOF in %s",
			ms, GetLoadedFilePath().c_str() );

		return false; /* failed */
	}

	return true;
}

float RageSound::GetAbsoluteVolume() const
{
	float f = m_Param.m_Volume;
	f *= SOUNDMAN->GetMixVolume();
	return f;
}

float RageSound::GetPlaybackRate() const
{
	return m_Param.m_fRate;
}

RageTimer RageSound::GetStartTime() const
{
	return m_Param.m_StartTime;
}

void RageSound::SetParams( const RageSoundParams &p )
{
	m_Param = p;
}

void RageSound::ApplyParams()
{
	if( m_pSource == NULL )
		return;

	m_pSource->SetProperty( "Speed", m_Param.m_fRate );
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

