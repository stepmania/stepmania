/*
 * Handle loading and decoding of sounds through SDL_sound.  This file
 * is portable; actual playing is handled in RageSoundManager.
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

#include "RageSoundReader_Preload.h"
#include "RageSoundReader_Resample.h"
#include "RageSoundReader_FileReader.h"

const int channels = 2;
const int framesize = 2 * channels; /* 16-bit */
#define samplerate() Sample->GetSampleRate()

/* The most data to buffer when streaming. */
const int internal_buffer_size = 1024*1;

/* The amount of data to read at once. */
const unsigned read_block_size = 1024;

RageSoundParams::RageSoundParams():
	StartTime( RageZeroTimer )
{
	m_StartSecond = 0;
	m_LengthSeconds = -1;
	m_FadeLength = 0;
	m_Volume = -1.0f; // use SOUNDMAN->GetMixVolume()
	m_Balance = 0; // center
	speed_input_samples = speed_output_samples = 1;
	AccurateSync = false;
	StopMode = M_AUTO;
}

RageSound::RageSound():
	m_Mutex( "RageSound" )
{
	ASSERT(SOUNDMAN);

	original = this;
	Sample = NULL;
	decode_position = 0;
	stopped_position = 0;
	max_driver_frame = 0;
	playing = false;
	playing_thread = 0;
	databuf.reserve(internal_buffer_size);

	ID = SOUNDMAN->GetUniqueID();

	/* Register ourself last, once everything is initialized. */
	SOUNDMAN->RegisterSound( this );
}

RageSound::~RageSound()
{
	/* If we're a "master" sound (not a copy), tell RageSoundManager to
	 * stop mixing us and everything that's copied from us. */
	if(original == this)
		SOUNDMAN->StopPlayingAllCopiesOfSound(*this);

	Unload();

	/* Unregister ourself. */
	SOUNDMAN->UnregisterSound( this );
}

RageSound::RageSound(const RageSound &cpy):
	RageSoundBase( cpy ),
	m_Mutex( "RageSound" )
{
	ASSERT(SOUNDMAN);

	Sample = NULL;

	*this = cpy;

	/* We have a different ID than our parent. */
	ID = SOUNDMAN->GetUniqueID();

	/* Register ourself. */
	SOUNDMAN->RegisterSound( this );
}

RageSound &RageSound::operator=( const RageSound &cpy )
{
	LockMut(cpy.m_Mutex);

	original = cpy.original;
	m_Param = cpy.m_Param;
	decode_position = cpy.decode_position;
	stopped_position = cpy.stopped_position;
	max_driver_frame = 0;
	playing = false;
	playing_thread = 0;

	databuf.reserve(internal_buffer_size);
	delete Sample;
	Sample = cpy.Sample->Copy();

	m_sFilePath = cpy.m_sFilePath;

	return *this;
}

void RageSound::Unload()
{
	LockMut(m_Mutex);

	if(IsPlaying())
		StopPlaying();

	delete Sample;
	Sample = NULL;
	
	m_sFilePath = "";
	databuf.clear();
}

void RageSound::Fail(CString reason)
{
	LOG->Warn("Decoding %s failed: %s", GetLoadedFilePath().c_str(), reason.c_str() );

	error = reason;
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


bool RageSound::Load( CString sSoundFilePath, int precache )
{
	LOG->Trace( "RageSound::LoadSound( '%s' )", sSoundFilePath.c_str() );

	if(precache == 2)
		precache = false;

	/* Don't load over copies. */
	ASSERT(original == this || m_sFilePath == "");

	Unload();

	m_sFilePath = sSoundFilePath;
	decode_position = stopped_position = 0;

	CString error;
	Sample = SoundReader_FileReader::OpenFile( m_sFilePath, error );
	if( Sample == NULL )
	{
		LOG->Warn( "RageSound::Load: error opening sound \"%s\": %s",
			m_sFilePath.c_str(), error.c_str() );

		Sample = new RageSoundReader_Silence;
	}

	const int NeededRate = SOUNDMAN->GetDriverSampleRate( Sample->GetSampleRate() );
	if( NeededRate != Sample->GetSampleRate() )
	{
		RageSoundReader_Resample *Resample = RageSoundReader_Resample::MakeResampler( (RageSoundReader_Resample::ResampleQuality) PREFSMAN->m_iSoundResampleQuality );
		Resample->Open(Sample);
		Resample->SetSampleRate( NeededRate );
		Sample = Resample;
	}

	/* Try to precache. */
	if(precache)
	{
		SoundReader_Preload *Preload = new SoundReader_Preload;
		if(Preload->Open(Sample)) {
			Sample = Preload;
		} else {
			/* Preload failed.  It read some data, so we need to rewind the
			 * reader. */
			Sample->SetPosition_Fast(0);
			delete Preload;
		}
	}

	m_Mutex.SetName( ssprintf("RageSound (%s)", Basename(sSoundFilePath).c_str() ) );

	return true;
}

/* Return the number of bytes available in the input buffer. */
int RageSound::Bytes_Available() const
{
	return databuf.num_readable();
}


void RageSound::RateChange(char *buf, int &cnt,
				int speed_input_samples, int speed_output_samples, int channels)
{
	if(speed_input_samples == speed_output_samples)
		return;

	/* Rate change.  Change speed_input_samples into speed_output_samples.
		* Do this per-channel. */
	static char *inbuf_tmp = NULL;
	static int maxcnt = 0;
	if(cnt > maxcnt)
	{
		maxcnt = cnt;
		delete [] inbuf_tmp;
		inbuf_tmp = new char[cnt];
	}

	memcpy(inbuf_tmp, buf, cnt);

	for(int c = 0; c < channels; ++c)
	{
		const int16_t *in = (const int16_t *) inbuf_tmp;
		int16_t *out = (int16_t *) buf;
		in += c;
		out += c;
		for(unsigned n = 0; n < cnt/(channels * sizeof(int16_t)); n += speed_input_samples)
		{

			/* Input 4 samples, output 5; 25% slowdown with no
			 * rounding error. */

			int16_t samps[20];	// max 2x rate
			ASSERT(size_t(speed_input_samples) <= sizeof(samps)/sizeof(*samps));
			int s;
			for(s = 0; s < speed_input_samples; ++s) {
				samps[s] = *in; in += channels;
			}

			float pos = 0;
			float incr = float(speed_input_samples) / speed_output_samples;

			for(s = 0; s < speed_output_samples; ++s) {
				float frac = pos - floorf(pos);
				int p = int(pos);
				int val = int(samps[p] * (1-frac));
				if(s+1 < speed_output_samples)
					val += int(samps[p+1] * frac);

				*out = int16_t(val);
				pos += incr;
				out += channels;
			}
		}
	}
	cnt = (cnt * speed_output_samples) / speed_input_samples;
}

/* Fill the buffer by about "bytes" worth of data.  (We might go a little
 * over, and we won't overflow our buffer.)  Return the number of bytes
 * actually read; 0 = EOF.  Convert mono input to stereo. */
int RageSound::FillBuf( int frames )
{
	ASSERT(Sample);

	bool got_something = false;

	while( frames > 0 )
	{
		if(read_block_size > databuf.num_writable())
			break; /* full */

		char inbuf[10240];
		unsigned read_size = read_block_size;

		if( m_Param.speed_input_samples != m_Param.speed_output_samples )
		{
			/* Read enough data to produce read_block_size. */
			read_size = read_size * m_Param.speed_input_samples / m_Param.speed_output_samples;

			/* Read in blocks that are a multiple of a sample, the number of
			 * channels and the number of input samples. */
			int block_size = sizeof(int16_t) * channels * m_Param.speed_input_samples;
			read_size = (read_size / block_size) * block_size;
			ASSERT(read_size < sizeof(inbuf));
		}

		/* channels == 2; we want stereo.  If the input data is mono, read half as many
		 * samples. */
		if( Sample->GetNumChannels() == 1 )
			read_size /= 2;

		ASSERT(read_size < sizeof(inbuf));

		int cnt = Sample->Read(inbuf, read_size);
		if(cnt == 0)
			return got_something; /* EOF */

		if( Sample->GetNumChannels() == 1 )
		{
			/* Dupe mono to stereo in-place; do it in reverse, to handle overlap. */
			int num_samples = cnt / sizeof(int16_t);
			int16_t *input = (int16_t *) inbuf;
			int16_t *output = input;
			input += num_samples;
			output += num_samples*2;
			while( num_samples-- )
			{
				input -= 1;
				output -= 2;
				output[0] = input[0];
				output[1] = input[0];
			}

			cnt *= 2;
		}

		if(cnt == -1)
		{
			Fail(Sample->GetError());

			/* Pretend we got EOF. */
			return 0;
		}

		RateChange( inbuf, cnt, m_Param.speed_input_samples, m_Param.speed_output_samples, channels );

		/* Add the data to the buffer. */
		databuf.write((const char *) inbuf, cnt);
		frames -= cnt/framesize;
		got_something = true;
	}

	return got_something;
}

/* Get a block of data from the input.  If buffer is NULL, just return the amount
 * that would be read. */
int RageSound::GetData( char *buffer, int frames )
{
	if( m_Param.m_LengthSeconds != -1 )
	{
		/* We have a length; only read up to the end. */
		const float LastSecond = m_Param.m_StartSecond + m_Param.m_LengthSeconds;
		int FramesToRead = int(LastSecond*samplerate()) - decode_position;

		/* If it's negative, we're past the end, so cap it at 0. Don't read
		 * more than size. */
		frames = clamp( FramesToRead, 0, frames );
	}

	int got;
	if( decode_position < 0 )
	{
		/* We havn't *really* started playing yet, so just feed silence.  How
		 * many more bytes of silence do we need? */
		got = -decode_position;
		got = min( got, frames );
		if( buffer )
			memset( buffer, 0, got*framesize );
	} else {
		/* Feed data out of our streaming buffer. */
		ASSERT(Sample);
		got = min( int(databuf.num_readable()/framesize), frames );
		if( buffer )
			databuf.read( buffer, got*framesize );
	}

	return got;
}

/* Pan buffer left or right; fPos is -1...+1. */
void PanSound( int16_t *buffer, int frames, float fPos )
{
	if( fPos == 0 )
		return; /* no change */

	bool bSwap = fPos < 0;
	if( bSwap )
		fPos = -fPos;

	float fLeftFactors[2] ={ 1-fPos, 0 };
	float fRightFactors[2] =
	{
		SCALE( fPos, 0, 1, 0.5f, 0 ),
		SCALE( fPos, 0, 1, 0.5f, 1 )
	};

	if( bSwap )
	{
		swap( fLeftFactors[0], fRightFactors[0] );
		swap( fLeftFactors[1], fRightFactors[1] );
	}

	ASSERT_M( channels == 2, ssprintf("%i", channels) );
	for( int samp = 0; samp < frames; ++samp )
	{
		int16_t l = int16_t(buffer[0]*fLeftFactors[0] + buffer[1]*fLeftFactors[1]);
		int16_t r = int16_t(buffer[0]*fRightFactors[0] + buffer[1]*fRightFactors[1]);
		buffer[0] = l;
		buffer[1] = r;
		buffer += 2;
	}
}

void FadeSound( int16_t *buffer, int frames, float fStartVolume, float fEndVolume  )
{
	for( int samp = 0; samp < frames; ++samp )
	{
		float fVolPercent = SCALE( samp, 0, frames, fStartVolume, fEndVolume );

		fVolPercent = clamp( fVolPercent, 0.f, 1.f );
		for(int i = 0; i < channels; ++i)
		{
			*buffer = short(*buffer * fVolPercent);
			buffer++;
		}
	}
}

/* RageSound::GetDataToPlay and RageSound::FillBuf are the main threaded API.  These
 * need to execute without blocking other threads from calling eg. GetPositionSeconds,
 * since they may take some time to run.
 Sample (r), databuf (r)
 decode_position (r), databuf (r)
 * 
 */
/* Retrieve audio data, for mixing.  At the time of this call, the frameno at which the
 * sound will be played doesn't have to be known.  Once committed, and the frameno
 * is known, call CommitPCMData.  size is in bytes.
 *
 * If the data returned is at the end of the stream, return false.
 *
 * size is in frames
 * sound_frame is in frames (abstract)
 */
bool RageSound::GetDataToPlay( int16_t *buffer, int size, int &sound_frame, int &frames_stored )
{
	int NumRewindsThisCall = 0;

	/* We only update decode_position; only take a shared lock, so we don't block the main thread. */
//	LockMut(m_Mutex);

	ASSERT_M( playing, ssprintf("%p", this) );

	frames_stored = 0;
	sound_frame = decode_position;

	while( 1 )
	{
		/* If we don't have any data left buffered, fill the buffer by
		 * up to as much as we need. */
		if( !Bytes_Available() )
			FillBuf( size );

		/* Get a block of data. */
		int got_frames = GetData( (char *) buffer, size );
		if( !got_frames )
		{
			/* EOF. */
			switch( GetStopMode() )
			{
			case RageSoundParams::M_STOP:
				/* Not looping.  Normally, we'll just stop here. */
				return false;

			case RageSoundParams::M_LOOP:
				/* Rewind and restart. */
				NumRewindsThisCall++;
				if(NumRewindsThisCall > 3)
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
					FillBuf( size );
				if( GetData(NULL, size) == 0 )
				{
					LOG->Warn( "Can't loop data in %s; no data available at start point %f",
						GetLoadedFilePath().c_str(), m_Param.m_StartSecond );

					/* Stop here. */
					return false;
				}
				continue;

			case RageSoundParams::M_CONTINUE:
				/* Keep playing silence. */
				memset( buffer, 0, size*framesize );
				got_frames = size;
				break;

			default:
				ASSERT(0);
			}
		}

		/* This block goes from decode_position to decode_position+got_frames. */

		/* We want to fade when there's FADE_TIME seconds left, but if
		 * m_LengthFrames is -1, we don't know the length we're playing.
		 * (m_LengthFrames is the length to play, not the length of the
		 * source.)  If we don't know the length, don't fade. */
		if( m_Param.m_FadeLength != 0 && m_Param.m_LengthSeconds != -1 )
		{
			const float fLastSecond = m_Param.m_StartSecond+m_Param.m_LengthSeconds;
			const float fStartVolume = fLastSecond - float(decode_position) / samplerate();
			const float fEndVolume = fLastSecond - float(decode_position+got_frames) / samplerate();
			FadeSound( buffer, got_frames, fStartVolume, fEndVolume );
		}

		PanSound( buffer, got_frames, m_Param.m_Balance );

		sound_frame = decode_position;

		frames_stored = got_frames;
		decode_position += got_frames;
		return true;
	}
}

/* Indicate that a block of audio data has been written to the device. */
void RageSound::CommitPlayingPosition( int64_t frameno, int pos, int got_frames )
{
	m_Mutex.Lock();
	pos_map.Insert( frameno, pos, got_frames );
	m_Mutex.Unlock();
}

/* Called by the mixer: return a block of sound data. 
 * Be careful; this is called in a separate thread. */
int RageSound::GetPCM( char *buffer, int size, int64_t frameno )
{
	ASSERT(playing);

	/*
	 * "frameno" is the audio driver's conception of time.  "position"
	 * is ours. Keep track of frameno->position mappings.
	 *
	 * This way, when we query the time later on, we can derive position
	 * values from the frameno values returned from GetPosition.
	 */

	/* Now actually put data from the correct buffer into the output. */
	int bytes_stored = 0;
	while( bytes_stored < size )
	{
		int pos, got_frames;
		bool eof = !GetDataToPlay( (int16_t *)(buffer+bytes_stored), (size-bytes_stored)/framesize, pos, got_frames );

		/* Save this frameno/position map. */
		SOUNDMAN->CommitPlayingPosition( GetID(), frameno, pos, got_frames );

		bytes_stored += got_frames * framesize;
		frameno += got_frames;

		if( eof )
			break;
	}

	return bytes_stored;
}

/* Start playing from the current position.  If the sound is already
 * playing, Stop is called. */
void RageSound::StartPlaying()
{
	// If no volume is set, use the default.
	if( m_Param.m_Volume == -1 )
		m_Param.m_Volume = SOUNDMAN->GetMixVolume();

	ASSERT(!playing);

	/* If StartTime is in the past, then we probably set a start time but took too
	 * long loading.  We don't want that; log it, since it can be unobvious. */
	if( !m_Param.StartTime.IsZero() && m_Param.StartTime.Ago() > 0 )
		LOG->Trace("Sound \"%s\" has a start time %f seconds in the past",
			GetLoadedFilePath().c_str(), m_Param.StartTime.Ago() );

	/* Tell the sound manager to start mixing us. */
//	LOG->Trace("set playing true for %p (StartPlaying) (%s)", this, this->GetLoadedFilePath().c_str());

	playing = true;
	playing_thread = RageThread::GetCurrentThreadID();

	SOUNDMAN->StartMixing(this);

	SOUNDMAN->RegisterPlayingSound( this );

//	LOG->Trace("StartPlaying %p finished (%s)", this, this->GetLoadedFilePath().c_str());
}

void RageSound::StopPlaying()
{
	if(!playing)
		return;

	stopped_position = (int) GetPositionSecondsInternal();

	/* Tell the sound driver to stop mixing this sound. */
	SOUNDMAN->StopMixing(this);

	SOUNDMAN->UnregisterPlayingSound( this );

	/* Lock the mutex after calling UnregisterPlayingSound.  We must not make driver
	 * calls with our mutex locked (driver mutex < sound mutex).  Nobody else will
	 * see our sound as not playing until we set playing = false. */
	m_Mutex.Lock();

//	LOG->Trace("set playing false for %p (StopPlaying) (%s)", this, this->GetLoadedFilePath().c_str());
	playing = false;
	playing_thread = 0;
	
	max_driver_frame = 0;
	pos_map.Clear();

	/* We may still have positions queued up in RageSoundManager.  We need to make sure
	 * that we don't accept those; otherwise, if we start playing again quickly, they'll
	 * confuse GetPositionSeconds().  Do this by changing our ID. */
	ID = SOUNDMAN->GetUniqueID();

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
	if(!playing)
		return;
	m_Mutex.Lock();

	stopped_position = (int) GetPositionSecondsInternal();

	SOUNDMAN->UnregisterPlayingSound( this );

//	LOG->Trace("set playing false for %p (SoundIsFinishedPlaying) (%s)", this, this->GetLoadedFilePath().c_str());
	playing = false;
	playing_thread = 0;

	pos_map.Clear();
//	LOG->Trace("SoundIsFinishedPlaying %p finished (%s)", this, this->GetLoadedFilePath().c_str());

	m_Mutex.Unlock();
}

RageSound *RageSound::Play( const RageSoundParams *params )
{
	ASSERT( Sample );
	return SOUNDMAN->PlaySound( *this, params );
}

void RageSound::Stop()
{
	SOUNDMAN->StopPlayingAllCopiesOfSound(*this);
}


float RageSound::GetLengthSeconds()
{
	ASSERT(Sample);
	int len = Sample->GetLength();

	if(len < 0)
	{
		LOG->Warn("GetLengthSeconds failed on %s: %s",
			GetLoadedFilePath().c_str(), Sample->GetError().c_str() );
		return -1;
	}

	return len / 1000.f; /* ms -> secs */
}

/* Get the position in frames. */
int64_t RageSound::GetPositionSecondsInternal( bool *approximate ) const
{
	LockMut(m_Mutex);

	if( approximate )
		*approximate = false;

	/* If we're not playing, just report the static position. */
	if( !IsPlaying() )
		return stopped_position;

	/* If we don't yet have any position data, GetPCM hasn't yet been called at all,
	 * so guess what we think the real time is. */
	if( pos_map.IsEmpty() )
	{
		LOG->Trace("no data yet; %i", stopped_position);
		if( approximate )
			*approximate = true;
		return stopped_position;
	}

	/* Get our current hardware position. */
	int64_t cur_frame = SOUNDMAN->GetPosition(this);

	/* It's sometimes possible for the hardware position to move backwards, usually
	 * on underrun.  We can try to prevent this in each driver, but it's an obscure
	 * error, so let's clamp the result here instead.  Be sure to reset this on stop,
	 * since the position may reset. */
	if( cur_frame < max_driver_frame )
	{
		/* Clamp the output to one per second, so one underruns don't cascade due to
		 * output spam. */
		static RageTimer last(RageZeroTimer);
		if( last.IsZero() || last.Ago() > 1.0f )
		{
			LOG->Trace( "Sound %s: driver returned a lesser position (%i < %i)",
				this->GetLoadedFilePath().c_str(), (int) cur_frame, (int) max_driver_frame );
			last.Touch();
		}
	}
	max_driver_frame = cur_frame = max( cur_frame, max_driver_frame );

	return pos_map.Search( cur_frame, approximate );
}

/*
 * If non-NULL, approximate is set to true if the returned time is approximated because of
 * underrun, the sound not having started (after Play()) or finished (after EOF) yet.
 *
 * If non-NULL, Timestamp is set to the real clock time associated with the returned sound
 * position.  We might take a variable amount of time before grabbing the timestamp (to
 * lock SOUNDMAN); we might lose the scheduler after grabbing it, when releasing SOUNDMAN.
 */

float RageSound::GetPositionSeconds( bool *approximate, RageTimer *Timestamp ) const
{
	LockMut(m_Mutex);

	if( Timestamp )
	{
		HOOKS->EnterTimeCriticalSection();
		Timestamp->Touch();
	}

	const float pos = GetPositionSecondsInternal( approximate ) / float(samplerate());

	if( Timestamp )
		HOOKS->ExitTimeCriticalSection();

	return GetPlaybackRate() * pos;
}


bool RageSound::SetPositionSeconds( float fSeconds )
{
	ASSERT( Sample );
	return SetPositionFrames( int(fSeconds * samplerate()) );
}

/* This is always the desired sample rate of the current driver. */
int RageSound::GetSampleRate() const
{
	ASSERT( Sample );
	return Sample->GetSampleRate();
}

bool RageSound::IsStreamingFromDisk() const
{
	ASSERT( Sample );
	return Sample->IsStreamingFromDisk();
}

bool RageSound::SetPositionFrames( int frames )
{
	LockMut(m_Mutex);

	{
		/* "decode_position" records the number of frames we've output to the
		 * speaker.  If the rate isn't 1.0, this will be different from the
		 * position in the sound data itself.  For example, if we're playing
		 * at 0.5x, and we're seeking to the 10th frame, we would have actually
		 * played 20 frames, and it's the number of real speaker frames that
		 * "decode_position" represents. */
	    const int scaled_frames = int( frames / GetPlaybackRate() );

	    /* If we're already there, don't do anything. */
	    if( decode_position == scaled_frames )
		    return true;

	    stopped_position = decode_position = scaled_frames;
	}

	/* The position we're going to seek the input stream to.  We have
	 * to do this in floating point to avoid overflow. */
	int ms = int( float(frames) * 1000.f / samplerate() );
	ms = max(ms, 0);

	databuf.clear();

	ASSERT(Sample);

	int ret;
	if( m_Param.AccurateSync )
		ret = Sample->SetPosition_Accurate(ms);
	else
		ret = Sample->SetPosition_Fast(ms);

	if(ret == -1)
	{
		/* XXX untested */
		Fail(Sample->GetError());
		return false; /* failed */
	}

	if(ret == 0 && ms != 0)
	{
		/* We were told to seek somewhere, and we got 0 instead, which means
		 * we passed EOF.  This could be a truncated file or invalid data. */
		LOG->Warn("SetPositionFrames: %i ms is beyond EOF in %s",
			ms, GetLoadedFilePath().c_str());

		return false; /* failed */
	}

	return true;
}

void RageSoundParams::SetPlaybackRate( float NewSpeed )
{
	if( NewSpeed == 1.00f )
	{
		speed_input_samples = 1; speed_output_samples = 1;
	} else {
		/* Approximate it to the nearest tenth. */
		speed_input_samples = int( roundf(NewSpeed * 10) );
		speed_output_samples = 10;
	}
}

float RageSound::GetVolume() const
{
	return m_Param.m_Volume;
}

void RageSound::LockSound()
{
	m_Mutex.Lock();
}

void RageSound::UnlockSound()
{
	m_Mutex.Unlock();
}

float RageSound::GetPlaybackRate() const
{
	return float(m_Param.speed_input_samples) / m_Param.speed_output_samples;
}

RageTimer RageSound::GetStartTime() const
{
	return m_Param.StartTime;
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

