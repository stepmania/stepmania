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

#include <math.h>

#include "RageSoundReader_Preload.h"
#include "RageSoundReader_Resample.h"
#include "RageSoundReader_FileReader.h"

const int channels = 2;
const int framesize = 2 * channels; /* 16-bit */
#define samplerate() Sample->GetSampleRate()

/* The most data to buffer when streaming.  This should generally be at least as large
 * as the largest hardware buffer. */
const int internal_buffer_size = 1024*16;

/* The amount of data to read from SDL_sound at once. */
const unsigned read_block_size = 1024;

/* The number of frames we should keep pos_map data for.  This being too high
 * is mostly harmless; the data is small. */
const int pos_map_backlog_frames = 100000;

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

RageSound::RageSound()
{
	ASSERT(SOUNDMAN);
	LockMut(SOUNDMAN->lock);

	original = this;
	Sample = NULL;
	position = 0;
	stopped_position = -1;
	playing = false;
	databuf.reserve(internal_buffer_size);

	/* Register ourself, so we have a unique ID and receive Update()s. */
	ID = SOUNDMAN->RegisterSound( this );
}

RageSound::~RageSound()
{
	/* If we're a "master" sound (not a copy), tell RageSoundManager to
	 * stop mixing us and everything that's copied from us. */
	if(original == this)
		SOUNDMAN->StopPlayingSound(*this);

	Unload();

	/* Unregister ourself. */
	SOUNDMAN->UnregisterSound( this );
}

RageSound::RageSound(const RageSound &cpy):
	RageSoundBase( cpy )
{
	ASSERT(SOUNDMAN);
	LockMut(SOUNDMAN->lock);

	Sample = NULL;

	original = cpy.original;
	m_Param = cpy.m_Param;
	position = cpy.position;
	playing = false;

	databuf.reserve(internal_buffer_size);
	Sample = cpy.Sample->Copy();

	/* Load() won't work on a copy if m_sFilePath is already set, so
	 * copy this down here. */
	m_sFilePath = cpy.m_sFilePath;

	/* Register ourself, so we receive Update()s.  We have a different ID than
	 * our parent. */
	ID = SOUNDMAN->RegisterSound( this );
}

void RageSound::Unload()
{
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


bool RageSound::Load(CString sSoundFilePath, int precache)
{
	LOG->Trace( "RageSound::LoadSound( '%s' )", sSoundFilePath.c_str() );

	if(precache == 2)
		precache = false;

	/* Don't load over copies. */
	ASSERT(original == this || m_sFilePath == "");

	Unload();

	m_sFilePath = sSoundFilePath;
	position = 0;

	CString error;
	Sample = SoundReader_FileReader::OpenFile( m_sFilePath, error );
	if( Sample == NULL )
		RageException::Throw( "RageSoundManager::RageSoundManager: error opening sound '%s': '%s'",
			m_sFilePath.c_str(), error.c_str());

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

	return true;
}

/* Read data at the rate we're playing it.  We only do this to smooth out the rate
 * we read data; the sound thread will always read more if it's needed. 
 *
 * Actually, this isn't a good idea.  The sound driver will read in small chunks,
 * interleaving between files.  For example, if four files are playing, and each
 * is two chunks behind, it'll read a chunk from each file twice, instead of reading
 * two chunks for each file at a time, which reduces the chance of underrun. */
void RageSound::Update(float delta)
{
//	LockMut(SOUNDMAN->lock);

//	if( playing && delta )
//		FillBuf(int(delta * GetSampleRate() * framesize));
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
		const Sint16 *in = (const Sint16 *) inbuf_tmp;
		Sint16 *out = (Sint16 *) buf;
		in += c;
		out += c;
		for(unsigned n = 0; n < cnt/(channels * sizeof(Sint16)); n += speed_input_samples)
		{

			/* Input 4 samples, output 5; 25% slowdown with no
			 * rounding error. */

			Sint16 samps[20];	// max 2x rate
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

				*out = Sint16(val);
				pos += incr;
				out += channels;
			}
		}
	}
	cnt = (cnt * speed_output_samples) / speed_input_samples;
}

/* Fill the buffer by about "bytes" worth of data.  (We might go a little
 * over, and we won't overflow our buffer.)  Return the number of bytes
 * actually read; 0 = EOF. */
int RageSound::FillBuf(int bytes)
{
	LockMut(SOUNDMAN->lock);

	ASSERT(Sample);

	bool got_something = false;

	while(bytes > 0)
	{
		if(read_block_size > databuf.num_writable())
			break; /* full */

		char inbuf[10240];
		unsigned read_size = read_block_size;
		int cnt = 0;

		if( m_Param.speed_input_samples != m_Param.speed_output_samples )
		{
			/* Read enough data to produce read_block_size. */
			read_size = read_size * m_Param.speed_input_samples / m_Param.speed_output_samples;

			/* Read in blocks that are a multiple of a sample, the number of
			 * channels and the number of input samples. */
			int block_size = sizeof(Sint16) * channels * m_Param.speed_input_samples;
			read_size = (read_size / block_size) * block_size;
			ASSERT(read_size < sizeof(inbuf));
		}

		ASSERT(read_size < sizeof(inbuf));

		cnt = Sample->Read(inbuf, read_size);
		if(cnt == 0)
			return got_something; /* EOF */

		if(cnt == -1)
		{
			Fail(Sample->GetError());

			/* Pretend we got EOF. */
			return 0;
		}

		RateChange( inbuf, cnt, m_Param.speed_input_samples, m_Param.speed_output_samples, channels );

		/* Add the data to the buffer. */
		databuf.write((const char *) inbuf, cnt);
		bytes -= cnt;
		got_something = true;
	}

	return got_something;
}

/* Get a block of data from the input.  If buffer is NULL, just return the amount
 * that would be read. */
int RageSound::GetData(char *buffer, int size)
{
	if( m_Param.m_LengthSeconds != -1 )
	{
		/* We have a length; only read up to the end. */
		const float LastSecond = m_Param.m_StartSecond + m_Param.m_LengthSeconds;
		int FramesToRead = int(LastSecond*samplerate()) - position;

		/* If it's negative, we're past the end, so cap it at 0. Don't read
		 * more than size. */
		size = clamp( FramesToRead * framesize, 0, size );
	}

	int got;
	if(position < 0) {
		/* We havn't *really* started playing yet, so just feed silence.  How
		 * many more bytes of silence do we need? */
		got = -position * framesize;
		got = min(got, size);
		if(buffer)
			memset(buffer, 0, got);
	} else {
		/* Feed data out of our streaming buffer. */
		ASSERT(Sample);
		got = min(int(databuf.num_readable()), size);
		if(buffer)
			databuf.read(buffer, got);
	}

	return got;
}

/* Retrieve audio data, for mixing.  At the time of this call, the frameno at which the
 * sound will be played doesn't have to be known.  Once committed, and the frameno
 * is known, call CommitPCMData.  size is in bytes.
 *
 * If the data returned is at the end of the stream, return false.
 *
 * size is in samples
 * sound_frame is in frames (abstract)
 */
bool RageSound::GetDataToPlay( int16_t *buffer, int size, int &sound_frame, int &frames_stored )
{
	int NumRewindsThisCall = 0;

	LockMut(SOUNDMAN->lock);

	ASSERT(playing);

	frames_stored = 0;
	sound_frame = position;

	while( 1 )
	{
		/* If we don't have any data left buffered, fill the buffer by
		 * up to as much as we need. */
		if( !Bytes_Available() )
			FillBuf( size*sizeof(Sint16) );

		/* Get a block of data. */
		int got = GetData( (char *) buffer, size*sizeof(Sint16) );
		if( !got )
		{
			/* We're at the end of the data.  If we're looping, rewind and restart. */
			if( GetStopMode() == RageSoundParams::M_LOOP )
			{
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

				/* Rewind and start over. */
				SetPositionSeconds( m_Param.m_StartSecond );

				/* Make sure we can get some data.  If we can't, then we'll have
				 * nothing to send and we'll just end up coming back here. */
				if( !Bytes_Available() )
					FillBuf( size*sizeof(Sint16) );
				if( GetData(NULL, size*sizeof(Sint16)) == 0 )
				{
					LOG->Warn( "Can't loop data in %s; no data available at start point %f",
						GetLoadedFilePath().c_str(), m_Param.m_StartSecond );

					/* Stop here. */
					return false;
				}

				continue;
			}

			/* Not looping.  Normally, we'll just stop here. */
			if( GetStopMode() == RageSoundParams::M_STOP )
				return false;

			/* We're out of data, but we're not going to stop, so fill in the
			 * rest with silence. */
			memset( buffer, 0, size*sizeof(Sint16) );
			got = size*sizeof(Sint16);
		}

		/* This block goes from position to position+got_frames. */
		int got_frames = got / framesize;  /* bytes -> frames */
		RAGE_ASSERT_M( (got % framesize) == 0, ssprintf("%i isn't divisible by %i", got, framesize) );

		/* We want to fade when there's FADE_TIME seconds left, but if
		 * m_LengthFrames is -1, we don't know the length we're playing.
		 * (m_LengthFrames is the length to play, not the length of the
		 * source.)  If we don't know the length, don't fade. */
		if( m_Param.m_FadeLength != 0 && m_Param.m_LengthSeconds != -1 )
		{
			Sint16 *p = (Sint16 *) buffer;
			int this_position = position;

			for(int samp = 0; samp < got_frames; ++samp)
			{
				const float fLastSecond = m_Param.m_StartSecond+m_Param.m_LengthSeconds;
				const float fSecsUntilSilent = fLastSecond - float(this_position) / samplerate();
				float fVolPercent = fSecsUntilSilent / m_Param.m_FadeLength;

				fVolPercent = clamp(fVolPercent, 0.f, 1.f);
				for(int i = 0; i < channels; ++i) {
					*p = short(*p * fVolPercent);
					p++;
				}
				this_position++;
			}
		}

		if( m_Param.m_Balance != 0 )
		{
			Sint16 *p = (Sint16 *) buffer;

			float fBalance = m_Param.m_Balance;
			bool bSwap = fBalance < 0;
			if( bSwap )
				fBalance = -fBalance;

			float fLeftFactors[2] ={ 1-fBalance, 0 };
			float fRightFactors[2] =
			{
				SCALE( fBalance, 0, 1, 0.5f, 0 ),
				SCALE( fBalance, 0, 1, 0.5f, 1 )
			};

			if( bSwap )
			{
				swap( fLeftFactors[0], fRightFactors[0] );
				swap( fLeftFactors[1], fRightFactors[1] );
			}

			RAGE_ASSERT_M( channels == 2, ssprintf("%i", channels) );
			for( int samp = 0; samp < got_frames; ++samp )
			{
				Sint16 l = Sint16(p[0]*fLeftFactors[0] + p[1]*fLeftFactors[1]);
				Sint16 r = Sint16(p[0]*fRightFactors[0] + p[1]*fRightFactors[1]);
				p[0] = l;
				p[1] = r;
				p+=2;
			}
		}

		sound_frame = position;

		frames_stored = got_frames;
		position += got_frames;
		return true;
	}
}

/* Indicate that a block of audio data has been written to the device. */
void RageSound::CommitPlayingPosition( int64_t frameno, int pos, int got_frames )
{
	LockMut(SOUNDMAN->lock);
	pos_map_t p( frameno, pos, got_frames );
	pos_map.push_back( p );

	/* Erase old pos_map data. */
	CleanPosMap( pos_map );
}

/* Called by the mixer: return a block of sound data. 
 * Be careful; this is called in a separate thread. */
int RageSound::GetPCM( char *buffer, int size, int64_t frameno )
{
	LockMut(SOUNDMAN->lock);

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
		bool eof = !GetDataToPlay( (int16_t *)(buffer+bytes_stored), (size-bytes_stored)/2, pos, got_frames );

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
	LockMut(SOUNDMAN->lock);

	// If no volume is set, use the default.
	if( m_Param.m_Volume == -1 )
		m_Param.m_Volume = SOUNDMAN->GetMixVolume();

	stopped_position = -1;

	ASSERT(!playing);

	/* If StartTime is in the past, then we probably set a start time but took too
	 * long loading.  We don't want that; log it, since it can be unobvious. */
	if( !m_Param.StartTime.IsZero() && m_Param.StartTime.Ago() > 0 )
		LOG->Trace("Sound \"%s\" has a start time %f seconds in the past",
			GetLoadedFilePath().c_str(), m_Param.StartTime.Ago() );

	/* Tell the sound manager to start mixing us. */
	playing = true;
	SOUNDMAN->StartMixing(this);
	SOUNDMAN->playing_sounds.insert( this );
}

void RageSound::StopPlaying()
{
	if(!playing)
		return;

	stopped_position = GetPositionSecondsInternal();

	/* Tell the sound manager to stop mixing this sound. */
	SOUNDMAN->StopMixing(this);

	SOUNDMAN->lock.Lock();
	SOUNDMAN->playing_sounds.erase( this );
	SOUNDMAN->lock.Unlock();

	playing = false;

	pos_map.clear();
}

RageSound *RageSound::Play( const RageSoundParams *params )
{
	return SOUNDMAN->PlaySound( *this, params );
}

void RageSound::Stop()
{
	SOUNDMAN->StopPlayingSound(*this);
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

int64_t RageSound::SearchPosMap( const deque<pos_map_t> &pos_map, int64_t cur_frame, bool *approximate )
{
	/* cur_frame is probably in pos_map.  Search to figure out what position
	 * it maps to. */
	int64_t closest_position = 0, closest_position_dist = INT_MAX;
	int closest_block = 0; /* print only */
	for( unsigned i = 0; i < pos_map.size(); ++i )
	{
		if( cur_frame >= pos_map[i].frameno &&
			cur_frame < pos_map[i].frameno+pos_map[i].frames )
		{
			/* cur_frame lies in this block; it's an exact match.  Figure
			 * out the exact position. */
			int64_t diff = pos_map[i].position - pos_map[i].frameno;
			return cur_frame + diff;
		}

		/* See if the current position is close to the beginning of this block. */
		int64_t dist = llabs( pos_map[i].frameno - cur_frame );
		if( dist < closest_position_dist )
		{
			closest_position_dist = dist;
			closest_block = i;
			closest_position = pos_map[i].position - dist;
		}

		/* See if the current position is close to the end of this block. */
		dist = llabs( pos_map[i].frameno + pos_map[i].frames - cur_frame );
		if( dist < closest_position_dist )
		{
			closest_position_dist = dist;
			closest_position = pos_map[i].position + pos_map[i].frames + dist;
		}
	}

	/* The frame is out of the range of data we've actually sent.
	 * Return the closest position.
	 *
	 * There are three cases when this happens: 
	 * 1. After the first GetPCM call, but before it actually gets heard.
	 * 2. After GetPCM returns EOF and the sound has flushed, but before
	 *    SoundStopped has been called.
	 * 3. Underflow; we'll be given a larger frame number than we know about.
	 */
	/* XXX: %lli normally, %I64i in Windows */
	LOG->Trace( "Approximate sound time: driver frame %lli, pos_map frame %lli (dist %lli), closest position is %lli",
		cur_frame, pos_map[closest_block].frameno, closest_position_dist, closest_position );

	if( approximate )
		*approximate = true;
	return closest_position;
}

void RageSound::CleanPosMap( deque<pos_map_t> &pos_map )
{
	LockMut( SOUNDMAN->lock );

	/* Determine the number of frames of data we have. */
	int64_t total_frames = 0;
	for( unsigned i = 0; i < pos_map.size(); ++i )
		total_frames += pos_map[i].frames;

	/* Remove the oldest entry so long we'll stil have enough data.  Don't delete every
	 * frame, so we'll always have some data to extrapolate from. */
	while( pos_map.size() > 1 && total_frames - pos_map.front().frames > pos_map_backlog_frames )
	{
		total_frames -= pos_map.front().frames;
		pos_map.pop_front();
	}
}

/* Get the position in frames. */
int64_t RageSound::GetPositionSecondsInternal( bool *approximate ) const
{
	LockMut(SOUNDMAN->lock);

	if( approximate )
		*approximate = false;

	/* If we're not playing, just report the static position. */
	if( !IsPlaying() )
	{
		if(stopped_position != -1)
			return stopped_position;
		return position;
	}

	/* If we don't yet have any position data, GetPCM hasn't yet been called at all,
	 * so guess what we think the real time is. */
	if(pos_map.empty())
	{
		LOG->Trace("no data yet; %i", position);
		if( approximate )
			*approximate = true;
		return position - int(samplerate()*SOUNDMAN->GetPlayLatency());
	}

	/* Get our current hardware position. */
	int64_t cur_frame = SOUNDMAN->GetPosition(this);

	/* Before using pos_map, flush any incoming positions. */
	SOUNDMAN->FlushPosMapQueue();

	return SearchPosMap( pos_map, cur_frame, approximate );
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
	LockMut(SOUNDMAN->lock);

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
	if( fSeconds == -1 )
		fSeconds = m_Param.m_StartSecond;

	return SetPositionFrames( int(fSeconds * samplerate()) );
}

/* This is always the desired sample rate of the current driver. */
int RageSound::GetSampleRate() const
{
	return Sample->GetSampleRate();
}


bool RageSound::SetPositionFrames( int frames )
{
	/* This can take a while.  Only lock the sound buffer if we're actually playing. */
	LockMutex L(SOUNDMAN->lock);
	if(!playing)
		L.Unlock();

	{
		/* "position" records the number of frames we've output to the
		 * speaker.  If the rate isn't 1.0, this will be different from the
		 * position in the sound data itself.  For example, if we're playing
		 * at 0.5x, and we're seeking to the 10th frame, we would have actually
		 * played 20 frames, and it's the number of real speaker frames that
		 * "position" represents. */
	    const int scaled_frames = int( frames / GetPlaybackRate() );

	    /* If we're already there, don't do anything. */
	    if( position == scaled_frames )
		    return true;

	    position = scaled_frames;
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
-----------------------------------------------------------------------------
 Copyright (c) 2002-2004 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
-----------------------------------------------------------------------------
*/
