#ifndef RAGE_SOUND_OBJ_H
#define RAGE_SOUND_OBJ_H

#include <deque>
#include "RageTimer.h"
#include "RageUtil_CircularBuffer.h"

class SoundReader;

/* Driver interface for sounds: this is what drivers see. */
class RageSoundBase
{
public:
	virtual ~RageSoundBase() { }
	virtual void StopPlaying() = 0;
	virtual int GetPCM( char *buffer, int size, int64_t sampleno ) = 0;
	virtual int GetSampleRate() const = 0;
	virtual RageTimer GetStartTime() const { return RageZeroTimer; }
	virtual float GetVolume() const = 0;
	virtual CString GetLoadedFilePath() const = 0;
};

class RageSound: public RageSoundBase
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
	 * for BM (keyed sounds), and for rapidly-repeating sound effects, such
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
	float GetPositionSeconds( bool *approximate=NULL, RageTimer *Timestamp=NULL ) const;
	int GetSampleRate() const;
	bool SetPositionSeconds( float fSeconds = -1);
	void SetAccurateSync(bool yes=true) { AccurateSync = yes; }
	void SetPlaybackRate( float fScale );
	void SetFadeLength( float fSeconds );
	void SetNoFade() { SetFadeLength(0); }
	void SetVolume( float fVolume ) { m_Volume = fVolume; }
	float GetVolume() const { return m_Volume; }
	float GetPlaybackRate() const { return float(speed_input_samples) / speed_output_samples; }
	bool IsPlaying() const { return playing; }
	CString GetLoadedFilePath() const { return m_sFilePath; }
	void SetStartTime( const RageTimer &tm ) { StartTime = tm; }
	RageTimer GetStartTime() const { return StartTime; }
	void SetBalance( float f ) { m_Balance = f; }

private:
	/* If we were copied from another RageSound, this will point to it; otherwise
	 * this is ourself. */
	RageSound *original;

	SoundReader *Sample;
	CircBuf<char> databuf;
	int FillBuf(int bytes);

	/* Sound blocks we've sent out recently through GetPCM.  We keep track
	 * of each block for the last four calls of GetPCM. */
	struct pos_map_t {
		/* Frame number from the POV of the sound driver: */
		int64_t frameno;

		/* Actual sound position within the sample: */
		int64_t position;

		/* The number of frames in this block: */
		int64_t frames;

		pos_map_t( int64_t frame, int pos, int cnt ) { frameno=frame, position=pos; frames=cnt; }
	};
	deque<pos_map_t> pos_map;
	static int64_t SearchPosMap( const deque<pos_map_t> &pos_map, int64_t cur_frames, bool *approximate );
	static void CleanPosMap( deque<pos_map_t> &pos_map );
	
	CString m_sFilePath;

	/* The amount of data to play (or loop): */
	int m_StartSample, m_LengthSamples;
	
	/* Current position of the output sound; if < 0, nothing will play until it
	 * becomes positive.  This is recorded in samples, to avoid rounding error. */
	int		position;

	/* Amount of time to fade out at the end. */
	float fade_length;

	float m_Volume;

	/* Pan: -1, left; 1, right */
	float m_Balance;

	/* Hack: When we stop a playing sound, we can't ask the driver the position
	 * (we're not playing); and we can't seek back to the current playing position
	 * when we stop (too slow), but we want to be able to report the position we
	 * were at when we stopped without jumping to the last position we buffered. */
	int64_t stopped_position;
	bool    playing;

	/* Number of samples input and output when changing speed.  Currently,
	 * this is either 1/1, 5/4 or 4/5. */
	int speed_input_samples, speed_output_samples;
	bool AccurateSync;

	/* Optional driver feature: time to actually start playing sounds.  If zero, or if not
	 * supported, it'll start immediately. */
	RageTimer StartTime;

	CString error;

	int64_t GetPositionSecondsInternal( bool *approximate=NULL ) const;
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
	int GetPCM( char *buffer, int size, int64_t frameno );
	void Update(float delta);
};

#endif
/*
-----------------------------------------------------------------------------
 Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
-----------------------------------------------------------------------------
*/
