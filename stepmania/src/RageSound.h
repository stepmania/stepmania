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
	virtual void StopPlaying() = 0; // deprecated
	virtual void SoundIsFinishedPlaying() = 0;
	virtual bool GetDataToPlay( int16_t *buffer, int size, int &pos, int &got_bytes ) = 0;
	virtual int GetPCM( char *buffer, int size, int64_t frameno ) = 0;
	virtual int GetSampleRate() const = 0;
	virtual RageTimer GetStartTime() const { return RageZeroTimer; }
	virtual float GetVolume() const = 0;
	virtual int GetID() const = 0;
	virtual CString GetLoadedFilePath() const = 0;
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
	bool SetPositionSeconds( float fSeconds );
	CString GetLoadedFilePath() const { return m_sFilePath; }
	bool IsPlaying() const { return playing; }
	unsigned GetPlayingThread() const { return playing_thread; }

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

	SoundReader *Sample;
	CircBuf<char> databuf;
	int FillBuf(int bytes);

	/* Sound blocks we've sent out recently through GetPCM.  We keep track
	 * of each block for the last four calls of GetPCM. */
	struct pos_map_t
	{
		/* Frame number from the POV of the sound driver: */
		int64_t frameno;

		/* Actual sound position within the sample: */
		int64_t position;

		/* The number of frames in this block: */
		int64_t frames;

		pos_map_t() { frameno=0; position=0; frames=0; }
		pos_map_t( int64_t frame, int pos, int cnt ) { frameno=frame; position=pos; frames=cnt; }
	};
	deque<pos_map_t> pos_map;
	static int64_t SearchPosMap( const deque<pos_map_t> &pos_map, int64_t cur_frames, bool *approximate );
	static void CleanPosMap( deque<pos_map_t> &pos_map );
	
	CString m_sFilePath;

	RageSoundParams m_Param;
	
	/* Current position of the output sound; if < 0, nothing will play until it
	 * becomes positive.  This is recorded in frames, to avoid rounding error. */
	int		position;

	/* Hack: When we stop a playing sound, we can't ask the driver the position
	 * (we're not playing); and we can't seek back to the current playing position
	 * when we stop (too slow), but we want to be able to report the position we
	 * were at when we stopped without jumping to the last position we buffered. 
	 * Keep track of the position after a seek or stop, so we can return a sane
	 * position when stopped, and when playing but pos_map hasn't yet been filled. */
	int stopped_position;
	bool    playing;

	/* If playing, record the thread that called Play(). */
	unsigned playing_thread;

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
	void Update(float delta);
};

#endif
/*
-----------------------------------------------------------------------------
 Copyright (c) 2002-2004 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
-----------------------------------------------------------------------------
*/
