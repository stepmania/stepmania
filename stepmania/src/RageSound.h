#ifndef RAGE_SOUND_H
#define RAGE_SOUND_H

#include <deque>
#include "RageThreads.h"
#include "RageTimer.h"
#include "RageUtil_CircularBuffer.h"
#include "RageSoundPosMap.h"

class SoundReader;

/* Driver interface for sounds: this is what drivers see. */
class RageSoundBase
{
public:
	virtual ~RageSoundBase() { }
	virtual void SoundIsFinishedPlaying() = 0;
	virtual bool GetDataToPlay( int16_t *buffer, int size, int &pos, int &got_bytes ) = 0;
	virtual int GetPCM( char *buffer, int size, int64_t frameno ) = 0;
	virtual int GetSampleRate() const = 0;
	virtual RageTimer GetStartTime() const { return RageZeroTimer; }
	virtual float GetVolume() const = 0;
	virtual int GetID() const = 0;
	virtual CString GetLoadedFilePath() const = 0;
	virtual bool IsStreamingFromDisk() const = 0;
};

/* These are parameters to play a sound.  These are normally changed before playing begins,
 * and are constant from then on. */
struct RageSoundParams
{
	RageSoundParams();

	/* The amount of data to play (or loop): */
	float m_StartSecond;
	float m_LengthSeconds;

	/* Amount of time to fade out at the end. */
	float m_FadeLength;
	void SetNoFade() { m_FadeLength = 0; }

	float m_Volume;

	/* Pan: -1, left; 1, right */
	float m_Balance;

	/* Number of samples input and output when changing speed.  Currently,
	 * this is either 1/1, 5/4 or 4/5. */
	int speed_input_samples, speed_output_samples;
	void SetPlaybackRate( float fScale );

	bool AccurateSync;

	/* Optional driver feature: time to actually start playing sounds.  If zero, or if not
	 * supported, it'll start immediately. */
	RageTimer StartTime;

	/* M_STOP (default) stops the sound at the end.
	 * M_LOOP restarts.
	 * M_CONTINUE feeds silence, which is useful to continue timing longer than the actual sound. */
	enum StopMode_t {
		M_STOP, /* stop when finished */
		M_LOOP, /* loop */
		M_CONTINUE, /* keep playing silence */
		M_AUTO /* obey filename hints */
	} StopMode;
};

class RageSound: public RageSoundBase
{
public:
	RageSound();
	~RageSound();
	RageSound(const RageSound &cpy);
	RageSound &operator=( const RageSound &cpy );

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

	void StartPlaying();
	void StopPlaying();

	CString GetError() const { return error; }
	bool Error() const { return !error.empty(); }

	RageSound *Play( const RageSoundParams *params=NULL );
	void Stop();

	float GetLengthSeconds();
	float GetPositionSeconds( bool *approximate=NULL, RageTimer *Timestamp=NULL ) const;
	int GetSampleRate() const;
	bool IsStreamingFromDisk() const;
	bool SetPositionSeconds( float fSeconds );
	CString GetLoadedFilePath() const { return m_sFilePath; }
	bool IsPlaying() const { return playing; }
	uint64_t GetPlayingThread() const { return playing_thread; }

	/* Lock and unlock this sound. */
	void LockSound();
	void UnlockSound();

	float GetPlaybackRate() const;
	RageTimer GetStartTime() const;
	float GetVolume() const;
	int GetID() const { return ID; }
	void SetParams( const RageSoundParams &p );
	const RageSoundParams &GetParams() const { return m_Param; }

private:
	/* If we were copied from another RageSound, this will point to it; otherwise
	 * this is ourself. */
	RageSound *original;

	mutable RageMutex m_Mutex;

	SoundReader *Sample;
	CircBuf<char> databuf;
	int FillBuf(int bytes);

	/* We keep track of sound blocks we've sent out recently through GetDataToPlay. */
	pos_map_queue pos_map;
	
	CString m_sFilePath;

	RageSoundParams m_Param;
	
	/* Current position of the output sound, in frames.  If < 0, nothing will play
	 * until it becomes positive. */
	int		decode_position;

	/* Hack: When we stop a playing sound, we can't ask the driver the position
	 * (we're not playing); and we can't seek back to the current playing position
	 * when we stop (too slow), but we want to be able to report the position we
	 * were at when we stopped without jumping to the last position we buffered. 
	 * Keep track of the position after a seek or stop, so we can return a sane
	 * position when stopped, and when playing but pos_map hasn't yet been filled. */
	int stopped_position;
	bool    playing;

	/* Keep track of the max SOUNDMAN->GetPosition result (see GetPositionSecondsInternal). */
	mutable int64_t max_driver_frame;

	/* If playing, record the thread that called Play(). */
	uint64_t playing_thread;

	/* Unique ID number for this instance of RageSound. */
	int ID;

	CString error;

	int64_t GetPositionSecondsInternal( bool *approximate=NULL ) const;
	bool SetPositionFrames( int frames = -1 );
	int GetData(char *buffer, int size);
	void Fail(CString reason);
	int Bytes_Available() const;
	RageSoundParams::StopMode_t GetStopMode() const; /* resolves M_AUTO */

	void SoundIsFinishedPlaying(); // called by sound drivers

	static void RateChange(char *buf, int &cnt, int speed_input_samples, int speed_output_samples, int channels);

public:
	/* Used by RageSoundManager: */
	RageSound *GetOriginal() { return original; }

	/* Called only by the sound drivers: */
	/* This function should return the number of bytes actually put into buffer.
	 * If less than size is returned, it signals the stream to stop; once it's
	 * flushed, SoundStopped will be called.  Until then, SOUNDMAN->GetPosition
	 * can still be called (the sound is still playing). */
	int GetPCM( char *buffer, int size, int64_t frameno );
	bool GetDataToPlay( int16_t *buffer, int size, int &pos, int &got_bytes );
	void CommitPlayingPosition( int64_t frameno, int pos, int got_bytes );
};

#endif

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
