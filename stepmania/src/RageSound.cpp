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
 * Known problems:
 * I hear a click in one speaker at the beginning of some MP3s.  This is probably
 * something wrong with my SDL_sound MAD wrapper ...
 *
 * TODO:
 * Rate (speed)
 * Configurable buffer sizes (stored in SoundManager) and so on
 *
 * We need (yet) another layer of abstraction: RageSoundSource.  It'll just
 * implement the SDL_sound interface (in a class).  Two implementations;
 * one, used normally, that just wraps SDL_sound; and another that is given
 * a list of sounds and time offsets and transparently mixes them together
 * at the given times, filling the gaps with silence.  This should be an easy
 * way to handle autoplay tracks in keyed games.  Normal background music can
 * be passed to it with an offset of 0 (or the gap, however it works out).
 */

#include "stdafx.h"
#include "RageSound.h"
#include "RageSoundManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"
#include "RageTimer.h"

#include "SDL_sound-1.0.0/SDL_sound.h"
#ifdef _DEBUG
#pragma comment(lib, "SDL_sound-1.0.0/lib/sdl_sound_static_d.lib")
#else
#pragma comment(lib, "SDL_sound-1.0.0/lib/sdl_sound_static.lib")
#endif

const int channels = 2;
const int samplesize = 2 * channels; /* 16-bit */
const int samplerate = 44100;

/* If a sound is smaller than this, we'll load it entirely into memory. */
const int max_prebuf_size = 1024*256;

/* The most data to buffer when streaming.  This should generally be at least as large
 * as the largest hardware buffer. */
const int internal_buffer_size = 1024*16;

/* The amount of data to read from SDL_sound at once. */
const int read_block_size = 1024;

RageSound::RageSound()
{
	ASSERT(SOUNDMAN);
	LockMutex L(SOUNDMAN->lock);

	static bool initialized = false;
	if(!initialized)
	{
		if(!Sound_Init())
			throw RageException( "RageSoundManager::RageSoundManager: error initializing sound loader: %s", Sound_GetError());
		initialized = true;
	}

	stream.Sample = NULL;
	position = 0;
	playing = false;
	Loop = false;
	AutoStop = true;
	speed = 1.0f;
	stream.buf.reserve(internal_buffer_size);
	m_StartSeconds = 0;
	m_LengthSeconds = -1;
	AccurateSync = false;
	/* Register ourselves, so we receive Update()s. */
	SOUNDMAN->all_sounds.insert(this);
}

RageSound::~RageSound()
{
	Unload();

	/* Unregister ourselves. */
	SOUNDMAN->all_sounds.erase(this);
}

RageSound::RageSound(const RageSound &cpy)
{
	ASSERT(SOUNDMAN);
	LockMutex L(SOUNDMAN->lock);

	stream.Sample = NULL;

	full_buf = cpy.full_buf;
	big = cpy.big;
	m_sFilePath = cpy.m_sFilePath;
	m_StartSeconds = cpy.m_StartSeconds;
	m_LengthSeconds = cpy.m_LengthSeconds;
	Loop = cpy.Loop;
	position = cpy.position;
	playing = false;
	speed = cpy.speed;
	AccurateSync = cpy.AccurateSync;
	AutoStop = cpy.AutoStop;
	
	if(big)
	{
		/* We can't copy the Sound_Sample, so load a new one. 
		 * Don't bother trying to load it in a small buffer. */
		stream.buf.reserve(internal_buffer_size);
		Load(cpy.GetLoadedFilePath(), false);
	}

	/* Register ourselves, so we receive Update()s. */
	SOUNDMAN->all_sounds.insert(this);
}

void RageSound::Unload()
{
	if(IsPlaying())
		Stop();

	Sound_FreeSample(stream.Sample);
	stream.Sample = NULL;

	m_sFilePath = "";
	stream.buf.clear();

	full_buf.erase();
}

void RageSound::Load(CString sSoundFilePath, bool cache)
{
	LOG->Trace( "RageSound::LoadSound( '%s' )", sSoundFilePath.GetString() );

	Unload();

	m_sFilePath = sSoundFilePath;

	Sound_AudioInfo sound_desired;
	sound_desired.channels = channels;
	sound_desired.format = AUDIO_S16SYS;
	sound_desired.rate = samplerate;

    Sound_Sample *NewSample = Sound_NewSampleFromFile(sSoundFilePath.GetString(),
                    &sound_desired, read_block_size);

	if( NewSample == NULL )
		throw RageException( "RageSound::LoadSound: error loading %s: %s",
		sSoundFilePath.GetString(), Sound_GetError() );

	/* Try to decode into full_buf. */
	big = false;
	if(!cache)
		big = true; /* Don't. */

	/* Check the length, and see if we think it'll fit in the buffer. */
	{
		int len = Sound_Length(NewSample);
		/* This will fail with EAGAIN if it'll take a while.  We only want
		 * to do this if it's fast. */
		if(len != -1) {
			float secs = len / 1000.f;

			int pcmsize = int(secs * samplerate * samplesize); /* seconds -> bytes */
			if(pcmsize > max_prebuf_size)
				big = true; /* Don't bother trying to preload it. */
			else
				full_buf.reserve(pcmsize);
		}

		Sound_Rewind(NewSample);
	}

	while(!big) {
		int cnt = Sound_Decode(NewSample);

		if(cnt < 0)
			throw RageException("Read error on %s: %s", 
				sSoundFilePath.GetString(), Sound_GetError() ); /* XXX (see other error-handling XXX) */

		/* Add the buffer. */
		full_buf.append((const char *)NewSample->buffer, 
			(const char *)NewSample->buffer+cnt);

		if(full_buf.size() > max_prebuf_size) {
			full_buf.erase();
			big = true; /* too big */
		}

		if(NewSample->flags & SOUND_SAMPLEFLAG_EOF)
			break;
	}

	if(big) {
		/* Oops; we need to stream it. */
		stream.Sample = NewSample;
		Sound_Rewind(stream.Sample);
	} else {
		/* We're done with the stream. */
		Sound_FreeSample(NewSample);
	}

	position = 0;
}

void RageSound::SetStartSeconds( float secs )
{
	m_StartSeconds = secs;
}

void RageSound::SetLengthSeconds(float secs)
{
	m_LengthSeconds = secs;
}

/* Start playing from the current position.  If the sound is already
 * playing, Stop is called. */
void RageSound::Play()
{
	LockMutex L(SOUNDMAN->lock);

	if(playing) Stop();
	playing = true;

	// Tell the sound manager to start mixing us
	SOUNDMAN->StartMixing(this);
}

void RageSound::Update(float delta)
{
	if(playing && big)
		stream.FillBuf(int(delta * samplerate * samplesize));
}

/* Fill the buffer by about "bytes" worth of data.  (We might go a little
 * over, and we won't overflow our buffer.) */
int RageSound::stream_t::FillBuf(int bytes)
{
	ASSERT(Sample);

	bool got_something = false;
	if(Sample->flags & SOUND_SAMPLEFLAG_EOF)
		return got_something; /* EOF */

	while(bytes > 0)
	{
		if(buf.size()+read_block_size > buf.capacity())
			break; /* full */

		int cnt = Sound_Decode(Sample);
		if(Sample->flags & SOUND_SAMPLEFLAG_EOF)
			return got_something; /* EOF */

		if(Sample->flags & SOUND_SAMPLEFLAG_ERROR)
		{
			/* There was a fatal error; get it with Sound_GetError().
			 * XXX: How should we handle sound errors?  We can't
			 * just return error, since we're in a separate thread.
			 * Most of the time we should probably just warn and move
			 * on (no big deal), but the gameplay screen should query
			 * periodically and do something more intelligent when we
			 * fail (so we don't play out the rest of the song in
			 * silence) ... */
			throw RageException("Read error: %s",
					Sound_GetError() );
		}

		buf.write((const char *)Sample->buffer, cnt);
		bytes -= cnt;
		got_something = true;
	}

	return got_something;
}

/* Called by the mixer: return a block of sound data. 
 * Be careful; this is called in a separate thread. */
int RageSound::GetPCM(char *buffer, int size, int sampleno)
{
	LockMutex L(SOUNDMAN->lock);

	/* If the sound is paused, just fill the buffer with silence. 
	 * Hmm.  Pausing is annoying, since we'll get startup latency if
	 * we keep our buffer playing; we should stop the stream completely
	 * when we pause ... */
	ASSERT(playing);

	int bytes_stored = 0;

	pos_map[0] = pos_map[1];
	pos_map[1] = pos_map[2];
	pos_map[2] = pos_map[3];
	pos_map[3].clear();

	/* "sampleno" is the audio driver's conception of time.  "position"
	 * is ours. Keep track of sampleno->position mappings for two GetPCM calls.
	 *
	 * This way, when we query the time later on, we can derive position
	 * values from the sampleno values returned from GetPosition.
	 *
	 * We need to keep two buffers worth of values, since we might loop at
	 * the end of a buffer. */

	/* Now actually put data from the correct buffer into the output. */
	while(size)
	{
		int got;
		int MaxBytes;
		if(m_LengthSeconds != -1)
		{
			/* We have a length; only read up to the end.  MaxPosition is the
			 * sample position of the end. */
			int MaxPosition = int((m_StartSeconds + m_LengthSeconds) * samplerate);

			/* Number of bytes until MaxPosition. */
			MaxBytes = (MaxPosition - position) * samplesize;

			/* If it's negative, we're past the end, so cap it at 0. */
			MaxBytes = max(0, MaxBytes);

			/* Don't read more than size. */
			MaxBytes = min(MaxBytes, size);
		}
		else
			MaxBytes = size;

		if(position < 0) {
			/* We havn't *really* started playing yet, so just feed silence.  How
			 * many more bytes of silence do we need? */
			got = -position * samplesize;
			got = min(got, MaxBytes);
			memset(buffer, 0, got);
		} else if(big) {
			/* Feed data out of our streaming buffer. */
			ASSERT(stream.Sample);
			got = min(int(stream.buf.size()), MaxBytes);
			stream.buf.read(buffer, got);
		} else {
			/* Feed data out of our full buffer. */
			int byte_pos = position * samplesize;
			got = min(int(full_buf.size())-byte_pos, MaxBytes);
			got = max(got, 0);
			if(got)
				memcpy(buffer, full_buf.data()+byte_pos, got);
		}

		if(!got)
		{
			/* We need more data.  Find out if we've hit EOF. */
			bool HitEOF = true;
			if(big) {
				/* If we don't have any data left buffered, fill the buffer by up to
				 * as much as we need. */
				if(stream.buf.size() || stream.FillBuf(size))
					HitEOF = false; /* we have more */
			} else {
				unsigned byte_pos = position * samplesize; /* samples -> bytes */
				if(byte_pos < full_buf.size())
					HitEOF = false; /* we have more */
			}

			/* If we've passed the stop point (m_StartSeconds+m_LengthSeconds), pretend
			 * we've hit EOF. */
			if(m_LengthSeconds != -1 &&
					float(position)/samplerate >= m_StartSeconds+m_LengthSeconds)
				HitEOF = true;

			if(!HitEOF)
				continue;

			if(Loop && m_LengthSeconds == 0)
			{
				/* Oops.  Looping with seconds == 0 doesn't make much sense.
				 * It might happen if we're given an empty sound file as input,
				 * though.  Let's just stop. */
				break;
			}

			/* We're at EOF.  If we're not looping, just stop. */
			if(Loop && m_LengthSeconds != 0)
			{
				/* Rewind and start over. */
				SetPositionSeconds(m_StartSeconds);
				continue;
			}
			if(AutoStop)
				break;

			/* We're out of data, but we're not going to stop, so fill in the
			 * rest with silence. */
			memset(buffer, 0, size);
			got = size;
		}

		/* Save this sampleno/position map. */
		pos_map[3].push_back(pos_map_t(sampleno, position, got/samplesize));

		int got_samples = got / samplesize;  /* bytes -> samples */

		/* This block goes from position to position+got_samples. */
		const float FADE_TIME = 1.5f;

		/* XXX: Loop shouldn't set fading; add a Fade_Time member?
			* 
			* We want to fade when there's FADE_TIME seconds left, but if
			* m_LengthSeconds is -1, we don't know the length we're playing.
			* (m_LengthSeconds is the length to play, not the length of the
			* source.)  If we don't know the length, don't fade. */
		if(Loop && m_LengthSeconds != -1) {
			Sint16 *p = (Sint16 *) buffer;
			int this_position = position;

			for(int samp = 0; samp < got_samples; ++samp)
			{
				float fSecsUntilSilent = (m_StartSeconds + m_LengthSeconds) - float(this_position)/samplerate;
				float fVolPercent = fSecsUntilSilent / FADE_TIME;

				fVolPercent = clamp(fVolPercent, 0.f, 1.f);
				for(int i = 0; i < channels; ++i) {
					*p = short(*p * fVolPercent);
					p++;
				}
				this_position++;
			}
		}


		bytes_stored += got;
		position += got_samples;
		size -= got;
		buffer += got;
		sampleno += got_samples;
	}

	return bytes_stored;
}

/* After we finish via the GetPCM return value, this is called by
 * RageSoundManager once the sound is actually flushed, which means
 * we're *really* stopped. */
void RageSound::SoundStopped()
{
	LOG->Trace("stop completed");
	Stop();
}

void RageSound::Pause()
{
	if(!playing) return;

	// Tell the sound manager to stop mixing this sound.
	SOUNDMAN->StopMixing(this);

	playing = false;
}

void RageSound::Stop()
{
	/* Tell the sound manager to stop mixing this sound. */
	SOUNDMAN->StopMixing(this);

	SetPositionSeconds(m_StartSeconds);

	playing = false;
	for(int i = 0; i < 4; ++i)
		pos_map[i].clear();
}

float RageSound::GetLengthSeconds()
{
	if(big) {
		ASSERT(stream.Sample);

 		int len = Sound_Length(stream.Sample);
		if(len == -1 && stream.Sample->flags & SOUND_SAMPLEFLAG_EAGAIN) {
			/* This indicates the length check will take a little while; call
			 * it again to confirm. */
			len = Sound_Length(stream.Sample);
		}

		if(len < 0)
			return -1; /* XXX: put a Sound_GetError() error message somewhere */

		return len / 1000.f; /* ms -> secs */
	} else {
		/* We have the whole file loaded; just return the position. */
		return full_buf.size() / (float(samplerate)*samplesize);
	}
}

float RageSound::GetPositionSeconds() const
{
	LockMutex L(SOUNDMAN->lock);

	/* If we're not playing, just report the static position. */
	if( !IsPlaying() )
		return position / float(samplerate);

	/* If we don't yet have any position data, GetPCM hasn't yet been called at all,
	 * so report the static position. */
	{
		bool HasData = false;
		for(int i = 0; i < 4; ++i) {
			if(!pos_map[i].empty()) HasData = true;
		}
		if(!HasData) {
			LOG->Trace("no data yet; %i", position);
			return position / float(samplerate);
		}
	}

	/* Get our current hardware position. */
	int cur_sample = SOUNDMAN->GetPosition(this);

	/* Concatenate the pos_maps.  (Just a cheat to simplify this a little; this
	 * is small, so this isn't very expensive.) */
	vector<pos_map_t> posmaps;
	for(int n = 0; n < 4; ++n)
		posmaps.insert(posmaps.end(), pos_map[n].begin(), pos_map[n].end());

	/* sampleno is probably in one of the pos_maps.  Search through them
	 * to figure out what position this sampleno maps to. */

	int closest_position = 0, closest_position_dist = INT_MAX;
	for(unsigned i = 0; i < posmaps.size(); ++i) {
		if(cur_sample >= posmaps[i].sampleno &&
		   cur_sample < posmaps[i].sampleno+posmaps[i].samples)
		{
			/* cur_sample lies in this block; it's an exact match.  Figure
			 * out the exact position. */
			int diff = posmaps[i].position - posmaps[i].sampleno;
			return float(cur_sample + diff) / samplerate;
		}

		/* See if the current position is close to the beginning of this block. */
		int dist = abs(posmaps[i].sampleno - cur_sample);
		if(dist < closest_position_dist)
		{
			closest_position_dist = dist;
			closest_position = posmaps[i].position;
		}

		/* See if the current position is close to the end of this block. */
		dist = abs(posmaps[i].sampleno + posmaps[i].samples - cur_sample);
		if(dist < closest_position_dist)
		{
			closest_position_dist = dist + posmaps[i].samples;
			closest_position = posmaps[i].position + posmaps[i].samples;
		}
	}

	/* The sample is out of the range of data we've actually sent.
	 * Return the closest position.
	 *
	 * There are three cases when this happens: 
	 * 1. After the first GetPCM call, but before it actually gets heard.
	 * 2. After GetPCM returns EOF and the sound has flushed, but before
	 *    SoundStopped has been called.
	 * 3. Underflow; we'll be given a larger sample number than we know about.
	 */

	return closest_position / float(samplerate);
}

void RageSound::SetPositionSeconds( float fSeconds )
{
	LockMutex L(SOUNDMAN->lock);

	position = int(fSeconds * samplerate);
	if( fSeconds < 0 )
		fSeconds = 0;

	if(big) {
		ASSERT(stream.Sample);
		if(AccurateSync)
			Sound_AccurateSeek(stream.Sample, int(fSeconds * 1000));
		else
			Sound_FastSeek(stream.Sample, int(fSeconds * 1000));
		stream.buf.clear();
	}
}

void RageSound::SetPlaybackRate( float fScale )
{
	LockMutex L(SOUNDMAN->lock);

	speed = fScale;
}

/* This is used to start music.  It probably belongs in RageSoundManager. */
void RageSound::LoadAndPlayIfNotAlready( CString sSoundFilePath )
{
	LockMutex L(SOUNDMAN->lock);
	if( GetLoadedFilePath() == sSoundFilePath && IsPlaying() )
		return;		// do nothing

	Load( sSoundFilePath );
	SetPositionSeconds(0);
	SetStartSeconds(0);
	SetLengthSeconds();
	SetLooping();
	Play();
}

void CircBuf::reserve(unsigned n)
{
	clear();
	buf.erase();
	buf.insert(buf.end(), n, 0);
}

void CircBuf::clear()
{
	cnt = start = 0;
}

void CircBuf::write(const char *buffer, unsigned buffer_size)
{
	ASSERT(size() + buffer_size <= capacity()); /* overflow */

	while(buffer_size)
	{
		unsigned write_pos = start + size();
		if(write_pos >= buf.size()) write_pos -= buf.size();
		
		int cpy = min(buffer_size, buf.size() - write_pos);
		buf.replace(write_pos, cpy, buffer, cpy);

		cnt += cpy;

		buffer += cpy;
		buffer_size -= cpy;
	}
}

void CircBuf::read(char *buffer, unsigned buffer_size)
{
	ASSERT(size() >= buffer_size); /* underflow */
	
	while(buffer_size)
	{
		unsigned total = min(buf.size() - start, size());
		unsigned cpy = min(buffer_size, total);
		buf.copy(buffer, cpy, start);

		start += cpy;
		if(start == buf.size()) start = 0;
		cnt -= cpy;

		buffer += cpy;
		buffer_size -= cpy;
	}
}
