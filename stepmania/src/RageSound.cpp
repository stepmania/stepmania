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
#include "RageException.h"
#include "PrefsManager.h"
#include "arch/ArchHooks/ArchHooks.h"
#include "RageSoundUtil.h"

#include "RageSoundReader_Preload.h"
#include "RageSoundReader_Resample.h"
#include "RageSoundReader_FileReader.h"

const int channels = 2;
const int framesize = 2 * channels; /* 16-bit */
#define samplerate() m_pSource->GetSampleRate()

/* The most data to buffer when streaming. */
const int internal_buffer_size = 1024*1;

/* The amount of data to read at once. */
const unsigned read_block_size = 1024;

RageSoundParams::RageSoundParams():
	m_StartTime( RageZeroTimer )
{
	m_StartSecond = 0;
	m_LengthSeconds = -1;
	m_FadeLength = 0;
	m_Volume = 1.0f;
	m_Balance = 0; // center
	speed_input_samples = speed_output_samples = 1;
	m_bAccurateSync = false;
	StopMode = M_AUTO;

	m_bIsCriticalSound = false;
}

RageSound::RageSound():
	m_Mutex( "RageSound" )
{
	ASSERT( SOUNDMAN );

	m_pSource = NULL;
	m_iDecodePosition = 0;
	m_iStoppedPosition = 0;
	m_iMaxDriverFrame = 0;
	m_bPlaying = false;
	m_DataBuffer.reserve( internal_buffer_size );

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
	m_iDecodePosition = cpy.m_iDecodePosition;
	m_iStoppedPosition = cpy.m_iStoppedPosition;
	m_iMaxDriverFrame = 0;
	m_bPlaying = false;

	m_DataBuffer.reserve( internal_buffer_size );
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
	m_DataBuffer.clear();
}

bool RageSound::IsLoaded() const
{
	return m_pSource != NULL;
}

void RageSound::Fail( CString sReason )
{
	LOG->Warn( "Decoding %s failed: %s", GetLoadedFilePath().c_str(), sReason.c_str() );

	m_sError = sReason;
}

class RageSoundReader_Silence: public SoundReader
{
public:
	int GetLength() const { return 0; }
	int GetLength_Fast() const { return 0; }
	int SetPosition_Accurate(int ms)  { return 0; }
	int SetPosition_Fast(int ms) { return 0; }
	int Read(char *buf, unsigned len) { return 0; }
	SoundReader *Copy() const { return new RageSoundReader_Silence; }
	int GetSampleRate() const { return 44100; }
	bool IsStreamingFromDisk() const { return false; }
};


bool RageSound::Load( CString sSoundFilePath )
{
	/* Automatically determine whether to precache */
	/* TODO: Hook this up to a pref? */
	return Load( sSoundFilePath, false );
}

bool RageSound::Load( CString sSoundFilePath, bool bPrecache )
{
	LOG->Trace( "RageSound::LoadSound( '%s', %d )", sSoundFilePath.c_str(), bPrecache );

	/* If this sound is already preloaded and held by SOUNDMAN, just make a copy
	 * of that.  Since RageSoundReader_Preload is refcounted, this is cheap. */
	SoundReader *pSound = SOUNDMAN->GetLoadedSound( sSoundFilePath );
	if( pSound == NULL )
	{
		CString error;
		pSound = SoundReader_FileReader::OpenFile( sSoundFilePath, error );
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
	}

	m_sFilePath = sSoundFilePath;

	m_Mutex.SetName( ssprintf("RageSound (%s)", Basename(sSoundFilePath).c_str() ) );

	return true;
}

void RageSound::LoadSoundReader( SoundReader *pSound )
{
	Unload();

	m_iDecodePosition = m_iStoppedPosition = 0;

	const int iNeededRate = SOUNDMAN->GetDriverSampleRate( pSound->GetSampleRate() );
	if( iNeededRate != pSound->GetSampleRate() )
	{
		RageSoundReader_Resample *Resample = RageSoundReader_Resample::MakeResampler();
		Resample->Open( pSound );
		Resample->SetSampleRate( iNeededRate );
		pSound = Resample;
	}

	m_pSource = pSound;
}

/* Return the number of bytes available in the input buffer. */
int RageSound::Bytes_Available() const
{
	return m_DataBuffer.num_readable();
}


void RageSound::RateChange( char *pBuffer, int &iCount, int iInputSpeed, int iOutputSpeed, int iChannels )
{
	if( iInputSpeed == iOutputSpeed )
		return;

	/* Rate change.  Change iInputSpeed into iOutputSpeed per-channel. */
	static char *pInbufTmp = NULL;
	static int iInbufTmpSize = 0;
	if( iCount > iInbufTmpSize )
	{
		iInbufTmpSize = iCount;
		delete [] pInbufTmp;
		pInbufTmp = new char[iCount];
	}

	memcpy( pInbufTmp, pBuffer, iCount );

	for( int c = 0; c < iChannels; ++c )
	{
		const int16_t *pInput = (const int16_t *) pInbufTmp;
		int16_t *pOutput = (int16_t *) pBuffer;
		pInput += c;
		pOutput += c;
		for( unsigned n = 0; n < iCount/(iChannels * sizeof(int16_t)); n += iInputSpeed )
		{

			/* Input 4 samples, output 5; 25% slowdown with no
			 * rounding error. */

			int16_t samps[20];	// max 2x rate
			ASSERT( size_t(iInputSpeed) <= sizeof(samps)/sizeof(*samps) );
			int s;
			for( s = 0; s < iInputSpeed; ++s )
			{
				samps[s] = *pInput;
				pInput += iChannels;
			}

			float fPosition = 0;
			float fIncrement = float(iInputSpeed) / iOutputSpeed;

			for( s = 0; s < iOutputSpeed; ++s )
			{
				float frac = fPosition - floorf( fPosition );
				int iPosition = int(fPosition);
				int iValue = int(samps[iPosition] * (1-frac));
				if( s+1 < iOutputSpeed )
					iValue += int(samps[iPosition+1] * frac);

				*pOutput = int16_t(iValue);
				fPosition += fIncrement;
				pOutput += iChannels;
			}
		}
	}
	iCount = (iCount * iOutputSpeed) / iInputSpeed;
}

/* Fill the buffer by about "bytes" worth of data.  (We might go a little
 * over, and we won't overflow our buffer.)  Return the number of bytes
 * actually read; 0 = EOF.  Convert mono input to stereo. */
int RageSound::FillBuf( int iFrames )
{
	ASSERT( m_pSource );

	bool bGotSomething = false;

	while( iFrames > 0 )
	{
		if( read_block_size > m_DataBuffer.num_writable() )
			break; /* full */

		char inbuf[10240];
		unsigned iReadSize = read_block_size;

		if( m_Param.speed_input_samples != m_Param.speed_output_samples )
		{
			/* Read enough data to produce read_block_size. */
			iReadSize = iReadSize * m_Param.speed_input_samples / m_Param.speed_output_samples;

			/* Read in blocks that are a multiple of a sample, the number of
			 * channels and the number of input samples. */
			int block_size = sizeof(int16_t) * channels * m_Param.speed_input_samples;
			iReadSize = (iReadSize / block_size) * block_size;
			ASSERT(iReadSize < sizeof(inbuf));
		}

		/* channels == 2; we want stereo.  If the input data is mono, read half as many
		 * samples. */
		if( m_pSource->GetNumChannels() == 1 )
			iReadSize /= 2;

		ASSERT( iReadSize < sizeof(inbuf) );

		int iCount = m_pSource->Read( inbuf, iReadSize );
		if( iCount == 0 )
			return bGotSomething; /* EOF */

		if( iCount == -1 )
		{
			Fail( m_pSource->GetError() );

			/* Pretend we got EOF. */
			return 0;
		}

		if( m_pSource->GetNumChannels() == 1 )
		{
			RageSoundUtil::ConvertMonoToStereoInPlace( (int16_t *) inbuf, iCount / sizeof(int16_t) );
			iCount *= 2;
		}

		RateChange( inbuf, iCount, m_Param.speed_input_samples, m_Param.speed_output_samples, channels );

		/* Add the data to the buffer. */
		m_DataBuffer.write( (const char *) inbuf, iCount );
		iFrames -= iCount / framesize;
		bGotSomething = true;
	}

	return bGotSomething;
}

/* Get a block of data from the input.  If buffer is NULL, just return the amount
 * that would be read. */
int RageSound::GetData( char *pBuffer, int iFrames )
{
	if( m_Param.m_LengthSeconds != -1 )
	{
		/* We have a length; only read up to the end. */
		const float fLastSecond = m_Param.m_StartSecond + m_Param.m_LengthSeconds;
		int iFramesToRead = int(fLastSecond*samplerate()) - m_iDecodePosition;

		/* If it's negative, we're past the end, so cap it at 0. Don't read
		 * more than size. */
		iFrames = clamp( iFramesToRead, 0, iFrames );
	}

	int iGot;
	if( m_iDecodePosition < 0 )
	{
		/* We havn't *really* started playing yet, so just feed silence.  How
		 * many more bytes of silence do we need? */
		iGot = -m_iDecodePosition;
		iGot = min( iGot, iFrames );
		if( pBuffer )
			memset( pBuffer, 0, iGot*framesize );
	} else {
		/* Feed data out of our streaming buffer. */
		ASSERT( m_pSource );
		iGot = min( int(m_DataBuffer.num_readable()/framesize), iFrames );
		if( pBuffer )
			m_DataBuffer.read( pBuffer, iGot*framesize );
	}

	return iGot;
}

/* RageSound::GetDataToPlay and RageSound::FillBuf are the main threaded API.  These
 * need to execute without blocking other threads from calling eg. GetPositionSeconds,
 * since they may take some time to run.
 */
/* Retrieve audio data, for mixing.  At the time of this call, the frameno at which the
 * sound will be played doesn't have to be known.  Once committed, and the frameno
 * is known, call CommitPCMData.  size is in bytes.
 *
 * If the data returned is at the end of the stream, return false.
 *
 * size is in frames
 * iSoundFrame is in frames (abstract)
 */
bool RageSound::GetDataToPlay( int16_t *pBuffer, int iSize, int &iSoundFrame, int &iFramesStored )
{
	int iNumRewindsThisCall = 0;

	/* We only update m_iDecodePosition; only take a shared lock, so we don't block the main thread. */
//	LockMut(m_Mutex);

	ASSERT_M( m_bPlaying, ssprintf("%p", this) );

	iFramesStored = 0;
	iSoundFrame = m_iDecodePosition;

	while( 1 )
	{
		/* If we don't have any data left buffered, fill the buffer by
		 * up to as much as we need. */
		if( !Bytes_Available() )
			FillBuf( iSize );

		/* Get a block of data. */
		int iGotFrames = GetData( (char *) pBuffer, iSize );

		/* If we didn't get any data, see if we need to pad the end of the file with
		 * silence for m_LengthSeconds. */
		if( !iGotFrames && m_Param.m_LengthSeconds != -1 )
		{
			const float LastSecond = m_Param.m_StartSecond + m_Param.m_LengthSeconds;
			int LastFrame = int(LastSecond*samplerate());
			int FramesOfSilence = LastFrame - m_iDecodePosition;
			FramesOfSilence = clamp( FramesOfSilence, 0, iSize );
			if( FramesOfSilence > 0 )
			{
				memset( pBuffer, 0, FramesOfSilence * framesize );
				iGotFrames = FramesOfSilence;
			}
		}

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

				/* Make sure we can get some data.  If we can't, then we'll have
				 * nothing to send and we'll just end up coming back here. */
				if( !Bytes_Available() )
					FillBuf( iSize );
				if( GetData(NULL, iSize) == 0 )
				{
					LOG->Warn( "Can't loop data in %s; no data available at start point %f",
						GetLoadedFilePath().c_str(), m_Param.m_StartSecond );

					/* Stop here. */
					return false;
				}
				continue;

			case RageSoundParams::M_CONTINUE:
				/* Keep playing silence. */
				memset( pBuffer, 0, iSize*framesize );
				iGotFrames = iSize;
				break;

			default:
				ASSERT(0);
			}
		}

		/* This block goes from m_iDecodePosition to m_iDecodePosition+iGotFrames. */

		/* We want to fade when there's m_FadeLength seconds left, but if
		 * m_LengthFrames is -1, we don't know the length we're playing.
		 * (m_LengthFrames is the length to play, not the length of the
		 * source.)  If we don't know the length, don't fade. */
		if( m_Param.m_FadeLength != 0 && m_Param.m_LengthSeconds != -1 )
		{
			const float fFinishFadingOutAt = m_Param.m_StartSecond + m_Param.m_LengthSeconds;
			const float fStartFadingOutAt = fFinishFadingOutAt - m_Param.m_FadeLength;
			const float fStartSecond = float(m_iDecodePosition) / samplerate();
			const float fEndSecond = float(m_iDecodePosition+iGotFrames) / samplerate();
			const float fStartVolume = SCALE( fStartSecond, fStartFadingOutAt, fFinishFadingOutAt, 1.0f, 0.0f );
			const float fEndVolume = SCALE( fEndSecond, fStartFadingOutAt, fFinishFadingOutAt, 1.0f, 0.0f );
			RageSoundUtil::Fade( pBuffer, iGotFrames, fStartVolume, fEndVolume );
		}

		RageSoundUtil::Pan( pBuffer, iGotFrames, m_Param.m_Balance );

		iSoundFrame = m_iDecodePosition;

		iFramesStored = iGotFrames;
		m_iDecodePosition += iGotFrames;
		return true;
	}
}

/* Indicate that a block of audio data has been written to the device. */
void RageSound::CommitPlayingPosition( int64_t frameno, int pos, int iGotFrames )
{
	m_Mutex.Lock();
	m_PositionMapping.Insert( frameno, pos, iGotFrames );
	m_Mutex.Unlock();
}

/* Called by the mixer: return a block of sound data. 
 * Be careful; this is called in a separate thread. */
int RageSound::GetPCM( char *pBuffer, int iSize, int64_t iFrameno )
{
	ASSERT( m_bPlaying );

	/*
	 * "frameno" is the audio driver's conception of time.  "position"
	 * is ours. Keep track of frameno->position mappings.
	 *
	 * This way, when we query the time later on, we can derive position
	 * values from the frameno values returned from GetPosition.
	 */

	/* Now actually put data from the correct buffer into the output. */
	int iBytesStored = 0;
	while( iBytesStored < iSize )
	{
		int iPosition, iGotFrames;
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

	SOUNDMAN->StartMixing( this );

//	LOG->Trace("StartPlaying %p finished (%s)", this, this->GetLoadedFilePath().c_str());
}

void RageSound::StopPlaying()
{
	if( !m_bPlaying )
		return;

	m_iStoppedPosition = (int) GetPositionSecondsInternal();

	/* Tell the sound driver to stop mixing this sound. */
	SOUNDMAN->StopMixing(this);

	/* Lock the mutex after calling UnregisterPlayingSound.  We must not make driver
	 * calls with our mutex locked (driver mutex < sound mutex).  Nobody else will
	 * see our sound as not playing until we set playing = false. */
	m_Mutex.Lock();

//	LOG->Trace("set playing false for %p (StopPlaying) (%s)", this, this->GetLoadedFilePath().c_str());
	m_bPlaying = false;
	
	m_iMaxDriverFrame = 0;
	m_PositionMapping.Clear();

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

	m_iStoppedPosition = (int) GetPositionSecondsInternal();

//	LOG->Trace("set playing false for %p (SoundIsFinishedPlaying) (%s)", this, this->GetLoadedFilePath().c_str());
	m_bPlaying = false;

	m_PositionMapping.Clear();
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
		return m_iStoppedPosition;

	/* If we don't yet have any position data, GetPCM hasn't yet been called at all,
	 * so guess what we think the real time is. */
	if( m_PositionMapping.IsEmpty() )
	{
		LOG->Trace( "no data yet; %i", m_iStoppedPosition );
		if( bApproximate )
			*bApproximate = true;
		return m_iStoppedPosition;
	}

	/* Get our current hardware position. */
	int64_t iCurrentFrame = SOUNDMAN->GetPosition(this);

	/* It's sometimes possible for the hardware position to move backwards, usually
	 * on underrun.  We can try to prevent this in each driver, but it's an obscure
	 * error, so let's clamp the result here instead.  Be sure to reset this on stop,
	 * since the position may reset. */
	if( iCurrentFrame < m_iMaxDriverFrame )
	{
		/* Clamp the output to one per second, so one underruns don't cascade due to
		 * output spam. */
		static RageTimer last(RageZeroTimer);
		if( last.IsZero() || last.Ago() > 1.0f )
		{
			LOG->Trace( "Sound %s: driver returned a lesser position (%i < %i)",
				this->GetLoadedFilePath().c_str(), (int) iCurrentFrame, (int) m_iMaxDriverFrame );
			last.Touch();
		}
	}
	m_iMaxDriverFrame = iCurrentFrame = max( iCurrentFrame, m_iMaxDriverFrame );

	return m_PositionMapping.Search( iCurrentFrame, bApproximate );
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

	const float fPosition = GetPositionSecondsInternal( bApproximate ) / float(samplerate());

	if( pTimestamp )
		HOOKS->ExitTimeCriticalSection();

	return GetPlaybackRate() * fPosition;
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

	{
		/* "m_iDecodePosition" records the number of frames we've output to the
		 * speaker.  If the rate isn't 1.0, this will be different from the
		 * position in the sound data itself.  For example, if we're playing
		 * at 0.5x, and we're seeking to the 10th frame, we would have actually
		 * played 20 frames, and it's the number of real speaker frames that
		 * "m_iDecodePosition" represents. */
	    const int iScaledFrames = int( iFrames / GetPlaybackRate() );

	    /* If we're already there, don't do anything. */
	    if( m_iDecodePosition == iScaledFrames )
		    return true;

	    m_iStoppedPosition = m_iDecodePosition = iScaledFrames;
	}

	/* The position we're going to seek the input stream to.  We have
	 * to do this in floating point to avoid overflow. */
	int ms = int( float(iFrames) * 1000.f / samplerate() );
	ms = max( ms, 0 );

	m_DataBuffer.clear();

	int iRet;
	if( m_Param.m_bAccurateSync )
		iRet = m_pSource->SetPosition_Accurate(ms);
	else
		iRet = m_pSource->SetPosition_Fast(ms);

	if( iRet == -1 )
	{
		/* XXX untested */
		Fail( m_pSource->GetError() );
		return false; /* failed */
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

void RageSoundParams::SetPlaybackRate( float fSpeed )
{
	if( fSpeed == 1.00f )
	{
		speed_input_samples = 1;
		speed_output_samples = 1;
	}
	else
	{
		/* Approximate it to the nearest tenth. */
		speed_input_samples = int( roundf(fSpeed * 10) );
		speed_output_samples = 10;
	}
}

float RageSound::GetAbsoluteVolume() const
{
	float f = m_Param.m_Volume;
	f *= SOUNDMAN->GetMixVolume();
	return f;
}

float RageSound::GetPlaybackRate() const
{
	return float(m_Param.speed_input_samples) / m_Param.speed_output_samples;
}

RageTimer RageSound::GetStartTime() const
{
	return m_Param.m_StartTime;
}

void RageSound::SetParams( const RageSoundParams &p )
{
	m_Param = p;
}

RageSoundParams::StopMode_t RageSound::GetStopMode() const
{
	if( m_Param.StopMode != RageSoundParams::M_AUTO )
		return m_Param.StopMode;

	if( m_sFilePath.Find("loop") != -1 )
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

