#ifndef RAGE_SOUND_OBJ_H
#define RAGE_SOUND_OBJ_H

#include "RageThreads.h"
#include "RageSoundManager.h"

#include <deque>

class CircBuf
{
	string buf;
	unsigned cnt, start;
	
public:

	CircBuf() { clear(); }
	unsigned size() const { return cnt; }
	unsigned capacity() const { return buf.size(); }
	void reserve(unsigned n);
	void clear();
	void write(const char *buf, unsigned size);
	void read(char *buf, unsigned size);
};

class SoundReader;

class RageSound
{
public:
	/* M_STOP (default) stops the sound after m_LengthSamples have been played.
	 * M_LOOP restarts from m_StartSample.
	 * M_CONTINUE feeds silence, which is useful to continue timing longer than the
	 * actual sound. */
	enum StopMode_t {
		M_STOP, M_LOOP, M_CONTINUE
	} StopMode;

	RageSound();
	~RageSound();
	RageSound(const RageSound &cpy);

	/* If cache == true (1), we'll preload the entire file into memory if
	 * it's small enough.  If this is done, a large number of copies of the
	 * sound can be played without much performance penalty.  This is useful
	 * for BM (keyed sounds), and for rapidly-repeating  sound effects, such
	 * as the music wheel.
	 *
	 * If cache == false (0), we'll never preload the file (always stream
	 * it).  This makes loads much faster.
	 * 
	 * If cache is 2 (the default), it means "don't care".  Currently, this
	 * means false; this may become a preference.
	 * 
	 * If the file failed to load, false is returned, Error() is set
	 * and a null sample will be loaded.  (This makes failed loads nonfatal;
	 * they can be ignored most of the time, so we continue to work if a file
	 * is broken or missing.) */
	bool Load(CString fn, int precache = 2);
	void Unload();

	void SetStopMode(StopMode_t m) { StopMode = m; }
	StopMode_t GetStopMode() const { return StopMode; }

	void SetStartSeconds(float secs = 0); /* default = beginning */
	void SetLengthSeconds(float secs = -1); /* default = no length limit */
	void StartPlaying();
	void StopPlaying();

	CString GetError() const { return error; }
	bool Error() const { return !error.empty(); }

	RageSound *Play();
	void Stop();

	float GetLengthSeconds();
	float GetPositionSeconds() const;
	int GetSampleRate() const;
	bool SetPositionSeconds( float fSeconds = -1);
	void SetAccurateSync(bool yes=true) { AccurateSync = yes; }
	void SetPlaybackRate( float fScale );
	void SetFadeLength( float fSeconds );
	void SetNoFade() { SetFadeLength(0); }
	float GetPlaybackRate() const { return float(speed_input_samples) / speed_output_samples; }
	bool IsPlaying() const { return playing; }
	CString GetLoadedFilePath() const { return m_sFilePath; }

private:
	/* If we were copied from another RageSound, this will point to it; otherwise
	 * this is ourself. */
	RageSound *original;

	SoundReader *Sample;
	CircBuf databuf;
	int FillBuf(int bytes);

	/* Sound blocks we've sent out recently through GetPCM.  We keep track
	 * of each block for the last four calls of GetPCM. */
	struct pos_map_t {
		/* Sample number from the POV of the sound driver: */
		int sampleno;

		/* Actual sound position within the sample: */
		int position;

		/* The number of samples in this block: */
		int samples;

		pos_map_t(int samp, int pos, int cnt) { sampleno=samp, position=pos; samples=cnt; }
	};
	deque<pos_map_t> pos_map;

	CString m_sFilePath;

	/* The amount of data to play (or loop): */
	int m_StartSample, m_LengthSamples;
	
	/* Current position of the output sound; if < 0, nothing will play until it
	 * becomes positive.  This is recorded in samples, to avoid rounding error. */
	int		position;

	/* Amount of time to fade out at the end. */
	float fade_length;

	/* Hack: When we stop a playing sound, we can't ask the driver the position
	 * (we're not playing); and we can't seek back to the current playing position
	 * when we stop (too slow), but we want to be able to report the position we
	 * were at when we stopped without jumping to the last position we buffered. */
	float stopped_position;
	bool    playing;

	/* Number of samples input and output when changing speed.  Currently,
	 * this is either 1/1, 5/4 or 4/5. */
	int speed_input_samples, speed_output_samples;
	bool AccurateSync;

	CString error;

	bool SetPositionSamples( int samples = -1 );
	int GetData(char *buffer, int size);
	void Fail(CString reason);
	int Bytes_Available() const;

	static void RateChange(char *buf, int &cnt,
				int speed_input_samples, int speed_output_samples, int channels);

public:
	/* Used by RageSoundManager: */
	RageSound *GetOriginal() { return original; }

	/* Called only by the sound drivers: */
	/* This function should return the number of bytes actually put into buffer.
	 * If less than size is returned, it signals the stream to stop; once it's
	 * flushed, SoundStopped will be called.  Until then, SOUNDMAN->GetPosition
	 * can still be called (the sound is still playing). */
	int GetPCM(char *buffer, int size, int sampleno);
	void Update(float delta);
};

#endif
/*
-----------------------------------------------------------------------------
 Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
-----------------------------------------------------------------------------
*/
