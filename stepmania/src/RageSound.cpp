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
 * TODO:
 * Configurable buffer sizes (stored in SoundManager) and so on
 *
 * Error handling:
 * Decoding errors (eg. CRC failures) will be recovered from when possible.
 *
 * When they can't be recovered, the sound will stop (unless loop or !autostop)
 * and the error will be available in GetError().
 *
 * Seeking past the end of the file will throw a warning and rewind.
 *
 * We need (yet) another layer of abstraction: RageSoundSource.  It'll just
 * implement the SDL_sound interface (in a class).  Two implementations;
 * one, used normally, that just wraps SDL_sound; and another that is given
 * a list of sounds and time offsets and transparently mixes them together
 * at the given times, filling the gaps with silence.  This should be an easy
 * way to handle autoplay tracks in keyed games.  Normal background music can
 * be passed to it with an offset of 0 (or the gap, however it works out).
 */

#include "global.h"
#include "RageSound.h"
#include "RageSoundManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"
#include "RageTimer.h"

#include "RageSoundReader_SDL_Sound.h"

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

/* The number of samples we should keep pos_map data for.  This being too high
 * is mostly harmless (the data is small).  Let's keep a second; no sane audio
 * driver will have that much latency. */
const int pos_map_backlog_samples = samplerate;

RageSound::RageSound()
{
	ASSERT(SOUNDMAN);
	LockMut(SOUNDMAN->lock);

	original = this;
	stream.Sample = NULL;
	position = 0;
	stopped_position = -1;
	playing = false;
	StopMode = M_STOP;
	speed_input_samples = speed_output_samples = 1;
	stream.buf.reserve(internal_buffer_size);
	m_StartSample = 0;
	m_LengthSamples = -1;
	AccurateSync = false;
	fade_length = 0;
	/* Register ourselves, so we receive Update()s. */
	SOUNDMAN->all_sounds.insert(this);
}

RageSound::~RageSound()
{
	/* If we're a "master" sound (not a copy), tell RageSoundManager to
	 * stop mixing us and everything that's copied from us. */
	if(original == this)
		SOUNDMAN->StopPlayingSound(*this);

	Unload();

	/* Unregister ourselves. */
	SOUNDMAN->all_sounds.erase(this);
}

RageSound::RageSound(const RageSound &cpy)
{
	ASSERT(SOUNDMAN);
	LockMut(SOUNDMAN->lock);

	stream.Sample = NULL;

	original = cpy.original;
	full_buf = cpy.full_buf;
	big = cpy.big;
	m_StartSample = cpy.m_StartSample;
	m_LengthSamples = cpy.m_LengthSamples;
	StopMode = cpy.StopMode;
	position = cpy.position;
	playing = false;
	AccurateSync = cpy.AccurateSync;
	fade_length = cpy.fade_length;
	speed_input_samples = cpy.speed_input_samples;
	speed_output_samples = cpy.speed_output_samples;

	if(big)
	{
		/* We can't copy the Sound_Sample, so load a new one. 
		 * Don't bother trying to load it in a small buffer. */
		stream.buf.reserve(internal_buffer_size);
		Load(cpy.GetLoadedFilePath(), false);
	}

	/* Load() won't work on a copy if m_sFilePath is already set, so
	 * copy this down here. */
	m_sFilePath = cpy.m_sFilePath;

	/* Register ourselves, so we receive Update()s. */
	SOUNDMAN->all_sounds.insert(this);
}

void RageSound::Unload()
{
	if(IsPlaying())
		StopPlaying();

	delete stream.Sample;
	stream.Sample = NULL;
	
	m_sFilePath = "";
	stream.buf.clear();

	full_buf.erase();
}

/* This is called upon fatal failure.  Replace the sound with silence. */
void RageSound::Fail(CString reason)
{
	delete stream.Sample;
	stream.Sample = NULL;

	big = false;

	full_buf.erase();
	/* XXX untested
	 * full_buf.append(0, 1024); should be OK, but VC6 is broken ... */
	basic_string<char> empty(1024, 0);
	full_buf.insert(full_buf.end(), empty.begin(), empty.end());
	position = 0;
	
	LOG->Warn("Decoding %s failed: %s",
		GetLoadedFilePath().GetString(), reason.GetString() );

	error = reason;
}

bool RageSound::Load(CString sSoundFilePath, int precache)
{
	LOG->Trace( "RageSound::LoadSound( '%s' )", sSoundFilePath.GetString() );

	if(precache == 2)
		precache = false;

	/* Don't load over copies. */
	ASSERT(original == this || m_sFilePath == "");

	Unload();

	m_sFilePath = sSoundFilePath;

	Sound_AudioInfo sound_desired;
	sound_desired.channels = channels;
	sound_desired.format = AUDIO_S16SYS;
	sound_desired.rate = samplerate;

    SoundReader *NewSample = new SoundReader_SDL_Sound;
	if(!NewSample->Open(sSoundFilePath.GetString()))
		RageException::Throw( "RageSoundManager::RageSoundManager: error opening sound %s: %s",
			sSoundFilePath.GetString(), NewSample->GetError().c_str());

	/* Try to decode into full_buf. */
	big = false;
	if(!precache)
		big = true; /* Don't. */

	/* Check the length, and see if we think it'll fit in the buffer. */
	{
		int len = NewSample->GetLength_Fast();
		if(len != -1)
		{
			float secs = len / 1000.f;

			int pcmsize = int(secs * samplerate * samplesize); /* seconds -> bytes */
			if(pcmsize > max_prebuf_size)
				big = true; /* Don't bother trying to preload it. */
			else
				full_buf.reserve(pcmsize);
		}
	}

	while(!big) {
		char buf[1024];
		int cnt = NewSample->Read(buf, sizeof(buf));

		if(cnt < 0) {
			/* XXX untested */
			Fail(stream.Sample->GetError());
			delete NewSample;
			return false;
		}

		if(!cnt) break; /* eof */

		/* Add the buffer. */
		full_buf.append(buf, buf+cnt);

		if(full_buf.size() > max_prebuf_size) {
			full_buf.erase();
			big = true; /* too big */
		}
	}

	if(big) {
		/* Oops; we need to stream it. */
		stream.Sample = NewSample;
		stream.Sample->SetPosition_Accurate(0);
	} else {
		/* We're done with the stream. */
		delete NewSample;
	}

	position = 0;
	return true;
}

void RageSound::SetStartSeconds( float secs )
{
	ASSERT(!playing);
	m_StartSample = int(secs*samplerate);
}

void RageSound::SetLengthSeconds(float secs)
{
	ASSERT(secs == -1 || secs >= 0);
	ASSERT(!playing);
	
	if(secs == -1)
		m_LengthSamples = -1;
	else
		m_LengthSamples = int(secs*samplerate);
}

void RageSound::Update(float delta)
{
	if(playing && big && delta)
		FillBuf(int(delta * samplerate * samplesize));
}

/* Return the number of bytes available in the input buffer. */
int RageSound::Bytes_Available() const
{
	if(big)
		return stream.buf.size();

	unsigned byte_pos = position * samplesize; /* samples -> bytes */
	if(byte_pos > full_buf.size())
		return 0; /* eof */

	return full_buf.size() - byte_pos;
}

#include <math.h>

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

			Sint16 samps[16];
			ASSERT(speed_input_samples <= sizeof(samps)/sizeof(*samps));
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

	if(!big)
		return 0; /* prebuffer is already fully loaded */

	ASSERT(stream.Sample);

	bool got_something = false;

	while(bytes > 0)
	{
		if(read_block_size > stream.buf.capacity() - stream.buf.size())
			break; /* full */

		char inbuf[10240];
		int read_size = read_block_size;
		int cnt = 0;

		if(speed_input_samples != speed_output_samples)
		{
			/* Read enough data to produce read_block_size. */
			read_size = read_size * speed_input_samples / speed_output_samples;

			/* Read in blocks that are a multiple of a sample, the number of
			 * channels and the number of input samples. */
			int block_size = sizeof(Sint16) * channels * speed_input_samples;
			read_size = (read_size / block_size) * block_size;
			ASSERT(read_size < sizeof(inbuf));
		}

		ASSERT(read_size < sizeof(inbuf));

		cnt = stream.Sample->Read(inbuf, read_size);
		if(cnt == 0)
			return got_something; /* EOF */

		RateChange(inbuf, cnt, speed_input_samples, speed_output_samples, channels);

		if(cnt == -1)
		{
			/* XXX untested */
			Fail(stream.Sample->GetError());

			/* Pretend we got data; we actually just switched to a non-streaming
			 * buffer. */
			return true;
		}

		/* Add the data to the buffer. */
		stream.buf.write((const char *) inbuf, cnt);
		bytes -= cnt;
		got_something = true;
	}

	return got_something;
}

/* Get a block of data from the input.  If buffer is NULL, just return the amount
 * that would be read. */
int RageSound::GetData(char *buffer, int size)
{
	if(m_LengthSamples != -1)
	{
		/* We have a length; only read up to the end.  MaxPosition is the
		 * sample position of the end. */
		int SamplesToRead = m_StartSample + m_LengthSamples - position;

		/* If it's negative, we're past the end, so cap it at 0. Don't read
		 * more than size. */
		size = clamp(SamplesToRead * samplesize, 0, size);
	}

	int got;
	if(position < 0) {
		/* We havn't *really* started playing yet, so just feed silence.  How
		 * many more bytes of silence do we need? */
		got = -position * samplesize;
		got = min(got, size);
		if(buffer)
			memset(buffer, 0, got);
	} else if(big) {
		/* Feed data out of our streaming buffer. */
		ASSERT(stream.Sample);
		got = min(int(stream.buf.size()), size);
		if(buffer)
			stream.buf.read(buffer, got);
	} else {
		/* Feed data out of our full buffer. */
		int byte_pos = position * samplesize;
		got = min(int(full_buf.size())-byte_pos, size);
		got = max(got, 0);
		if(buffer)
			memcpy(buffer, full_buf.data()+byte_pos, got);
	}

	return got;
}

/* Called by the mixer: return a block of sound data. 
 * Be careful; this is called in a separate thread. */
int RageSound::GetPCM(char *buffer, int size, int sampleno)
{
	int NumRewindsThisCall = 0;

	LockMut(SOUNDMAN->lock);

	ASSERT(playing);
	/* Erase old pos_map data. */
	while(pos_map.size() > 1 && pos_map.back().sampleno - pos_map.front().sampleno > pos_map_backlog_samples)
		pos_map.pop_front();

	/*
	 * "sampleno" is the audio driver's conception of time.  "position"
	 * is ours. Keep track of sampleno->position mappings.
	 *
	 * This way, when we query the time later on, we can derive position
	 * values from the sampleno values returned from GetPosition.
	 */

	/* Now actually put data from the correct buffer into the output. */
	int bytes_stored = 0;
	while(size)
	{
		/* Get a block of data. */
		int got = GetData(buffer, size);

		if(!got)
		{
			/* If we don't have any data left buffered, fill the buffer by
			 * up to as much as we need. */
			if(!Bytes_Available())
				FillBuf(size);

			/* If we got some data, we're OK. */
			if(GetData(NULL, size) != 0)
				continue; /* we have more */

			/* We're at the end of the data.  If we're looping, rewind and restart. */
			if(StopMode == M_LOOP)
			{
				NumRewindsThisCall++;
				if(NumRewindsThisCall > 3)
				{
					/* We're rewinding a bunch of times in one call.  This probably means
					 * that the length is too short.  It might also mean that the start
					 * position is very close to the end of the file, so we're looping
					 * over the remainder.  If we keep doing this, we'll chew CPU rewinding,
					 * so stop. */
					LOG->Warn("Sound %s is busy looping.  Sound stopped (start = %i, length = %i)",
						GetLoadedFilePath().GetString(), m_StartSample, m_LengthSamples);

					return 0;
				}

				/* Rewind and start over. */
				SetPositionSamples(m_StartSample);

				/* Make sure we can get some data.  If we can't, then we'll have
				 * nothing to send and we'll just end up coming back here. */
				if(!Bytes_Available()) FillBuf(size);
				if(GetData(NULL, size) == 0)
				{
					LOG->Warn("Can't loop data in %s; no data available at start point %i",
						GetLoadedFilePath().GetString(), m_StartSample);

					/* Stop here. */
					return bytes_stored;
				}

				continue;
			}

			/* Not looping.  Normally, we'll just stop here. */
			if(StopMode == M_STOP)
				break;

			/* We're out of data, but we're not going to stop, so fill in the
			 * rest with silence. */
			memset(buffer, 0, size);
			got = size;
		}

		/* This block goes from position to position+got_samples. */
		int got_samples = got / samplesize;  /* bytes -> samples */

		/* Save this sampleno/position map. */
		pos_map.push_back(pos_map_t(sampleno, position, got_samples));

		/* We want to fade when there's FADE_TIME seconds left, but if
		 * m_LengthSamples is -1, we don't know the length we're playing.
		 * (m_LengthSamples is the length to play, not the length of the
		 * source.)  If we don't know the length, don't fade. */
		if(fade_length != 0 && m_LengthSamples != -1) {
			Sint16 *p = (Sint16 *) buffer;
			int this_position = position;

			for(int samp = 0; samp < got_samples; ++samp)
			{
				float fSecsUntilSilent = float(m_StartSample + m_LengthSamples - this_position) / samplerate;
				float fVolPercent = fSecsUntilSilent / fade_length;

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

/* Start playing from the current position.  If the sound is already
 * playing, Stop is called. */
void RageSound::StartPlaying()
{
	LockMut(SOUNDMAN->lock);
	stopped_position = -1;

	ASSERT(!playing);

	/* Tell the sound manager to start mixing us. */
	playing = true;
	SOUNDMAN->StartMixing(this);
}

void RageSound::StopPlaying()
{
	if(!playing)
		return;

	stopped_position = GetPositionSeconds();

	/* Tell the sound manager to stop mixing this sound. */
	SOUNDMAN->StopMixing(this);
	playing = false;

	pos_map.clear();
}

RageSound *RageSound::Play()
{
	return SOUNDMAN->PlaySound(*this);
}

void RageSound::Stop()
{
	SOUNDMAN->StopPlayingSound(*this);
}


float RageSound::GetLengthSeconds()
{
	if(big) {
		ASSERT(stream.Sample);
		int len = stream.Sample->GetLength();

		if(len < 0)
		{
			LOG->Warn("GetLengthSeconds failed on %s: %s",
				GetLoadedFilePath().GetString(), stream.Sample->GetError().c_str() );
			return -1;
		}

		return len / 1000.f; /* ms -> secs */
	} else {
		/* We have the whole file loaded; just return the position. */
		return full_buf.size() / (float(samplerate)*samplesize);
	}
}

float RageSound::GetPositionSeconds() const
{
	LockMut(SOUNDMAN->lock);

	/* If we're not playing, just report the static position. */
	if( !IsPlaying() )
	{
		if(stopped_position != -1)
			return stopped_position;
		return GetPlaybackRate() * position / float(samplerate);
	}

	/* If we don't yet have any position data, GetPCM hasn't yet been called at all,
	 * so report the static position. */
	{
		if(pos_map.empty()) {
			LOG->Trace("no data yet; %i", position);
			return GetPlaybackRate() * position / float(samplerate);
		}
	}

	/* Get our current hardware position. */
	int cur_sample = SOUNDMAN->GetPosition(this);

	/* sampleno is probably in pos_maps.  Search to figure out what position
	 * this sampleno maps to. */

	int closest_position = 0, closest_position_dist = INT_MAX;
	for(unsigned i = 0; i < pos_map.size(); ++i) {
		if(cur_sample >= pos_map[i].sampleno &&
		   cur_sample < pos_map[i].sampleno+pos_map[i].samples)
		{
			/* cur_sample lies in this block; it's an exact match.  Figure
			 * out the exact position. */
			int diff = pos_map[i].position - pos_map[i].sampleno;
			return GetPlaybackRate() * float(cur_sample + diff) / samplerate;
		}

		/* See if the current position is close to the beginning of this block. */
		int dist = abs(pos_map[i].sampleno - cur_sample);
		if(dist < closest_position_dist)
		{
			closest_position_dist = dist;
			closest_position = pos_map[i].position;
		}

		/* See if the current position is close to the end of this block. */
		dist = abs(pos_map[i].sampleno + pos_map[i].samples - cur_sample);
		if(dist < closest_position_dist)
		{
			closest_position_dist = dist + pos_map[i].samples;
			closest_position = pos_map[i].position + pos_map[i].samples;
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
	LOG->Trace("Approximate sound time: sample %i, dist %i, closest %i", cur_sample, closest_position_dist, closest_position);

	return GetPlaybackRate() * closest_position / float(samplerate);
}

bool RageSound::SetPositionSeconds( float fSeconds )
{
	return SetPositionSamples( fSeconds == -1? -1: int(fSeconds * samplerate) );
}

bool RageSound::SetPositionSamples( int samples )
{
	if(samples == -1)
		samples = m_StartSample;

	/* This can take a while.  Only lock the sound buffer if we're actually playing. */
	LockMutex L(SOUNDMAN->lock);

	/* This can take a while.  Only hold the sound buffer if we're actually playing. */
	if(!playing)
		L.Unlock();

	/* If we're already there, don't do anything. */
	if(position == samples)
		return true;

	position = samples;
	if( samples < 0 )
		samples = 0;

	int ms = int(float(samples) * 1000.f / samplerate);

	if(!big) {
		/* Just make sure the position is in range. */
		if(position*samplesize < int(full_buf.size()))
			return true;

		/* We were told to seek beyond EOF.  This could be a truncated file
		 * or invalid data.  Warn about it and jump back to the beginning. */
		LOG->Warn("SetPositionSamples: %i ms is beyond EOF in %s",
			ms, GetLoadedFilePath().GetString());
		position = 0;
		return false;
	}

	stream.buf.clear();

	ASSERT(stream.Sample);

	int ret;
	if(AccurateSync)
		ret = stream.Sample->SetPosition_Accurate(ms);
	else
		ret = stream.Sample->SetPosition_Fast(ms);

	if(ret == -1)
	{
		/* XXX untested */
		Fail(stream.Sample->GetError());
		return false; /* failed */
	}

	if(ret == 0 && ms != 0)
	{
		/* We were told to seek somewhere, and we got 0 instead, which means
		 * we passed EOF.  This could be a truncated file or invalid data.  Warn
		 * about it and jump back to the beginning. */
		LOG->Warn("SetPositionSamples: %i ms is beyond EOF in %s",
			ms, GetLoadedFilePath().GetString());

		position = 0;
		return false; /* failed (but recoverable) */
	}

	return true;
}

void RageSound::SetPlaybackRate( float NewSpeed )
{
	LockMut(SOUNDMAN->lock);

	if(GetPlaybackRate() == NewSpeed)
		return;

	if(NewSpeed == 1.00f) {
		speed_input_samples = 1; speed_output_samples = 1;
	} else {
		/* Approximate it to the nearest tenth. */
		speed_input_samples = int(NewSpeed * 10);
		speed_output_samples = 10;
	}
}

void RageSound::SetFadeLength( float fSeconds )
{
	fade_length = fSeconds;
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
