/* RageSound - High-level sound object. */

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
	virtual float GetAbsoluteVolume() const = 0;
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

	/* Number of seconds to spend fading out. */
	float m_FadeLength;

	void SetNoFade() { m_FadeLength = 0; }

	float m_Volume;	// multiplies with SOUNDMAN->GetMixVolume()

	/* Pan: -1, left; 1, right */
	float m_Balance;

	/* Number of samples input and output when changing speed.  Currently,
	 * this is either 1/1, 5/4 or 4/5. */
	int speed_input_samples, speed_output_samples;
	void SetPlaybackRate( float fScale );

	/* If enabled, file seeking will prefer accuracy over speed. */
	bool m_bAccurateSync;

	/* Optional driver feature: time to actually start playing sounds.  If zero, or if not
	 * supported, the sound will start immediately. */
	RageTimer m_StartTime;

	/*
	 * M_STOP stops the sound at the end.
	 * M_LOOP restarts.
	 * M_CONTINUE feeds silence, which is useful to continue timing longer than the actual sound.
	 * M_AUTO (default) stops, obeying filename hints.
	 */
	enum StopMode_t {
		M_STOP,
		M_LOOP,
		M_CONTINUE,
		M_AUTO
	} StopMode;

	bool m_bIsCriticalSound;	// "is a sound that should be played even during attract"
};

class RageSound: public RageSoundBase
{
public:
	RageSound();
	~RageSound();
	RageSound( const RageSound &cpy );
	RageSound &operator=( const RageSound &cpy );

	/*
	 * If bPrecache == true, we'll preload the entire file into memory if
	 * small enough.  If this is done, a large number of copies of the sound
	 * can be played without much performance penalty.  This is useful for
	 * efficiently playing keyed sounds, and for rapidly-repeating sound
	 * effects, such as the music wheel.
	 *
	 * If cache == false, we'll always stream the sound on demand, which
	 * makes loads much faster.
	 * 
	 * If the file failed to load, false is returned, Error() is set
	 * and a null sample will be loaded.  This makes failed loads nonfatal;
	 * they can be ignored most of the time, so we continue to work if a file
	 * is broken or missing.
	 */
	bool Load( CString sFile, bool bPrecache );

	/* 
	 * Using this version means the "don't care" about caching.  Currently, 
	 * this always will not cache the sound; this may become a preference.
	 */
	bool Load( CString sFile );

	/* Load a SoundReader that you've set up yourself.  Sample rate conversion will
	 * be set up only if needed.  Doesn't fail. */
	void LoadSoundReader( SoundReader *pSound );
	void Unload();
	bool IsLoaded() const;

	void StartPlaying();
	void StopPlaying();

	CString GetError() const { return m_sError; }
	bool Error() const { return !m_sError.empty(); }

	RageSound *Play( const RageSoundParams *params=NULL );
	void Stop();

	/* Cleanly pause or unpause the sound.  If the sound wasn't already playing,
	 * return true and do nothing. */
	bool Pause( bool bPause );

	float GetLengthSeconds();
	float GetPositionSeconds( bool *approximate=NULL, RageTimer *Timestamp=NULL ) const;
	int GetSampleRate() const;
	bool IsStreamingFromDisk() const;
	bool SetPositionSeconds( float fSeconds );
	CString GetLoadedFilePath() const { return m_sFilePath; }
	bool IsPlaying() const { return m_bPlaying; }

	float GetPlaybackRate() const;
	RageTimer GetStartTime() const;
	float GetAbsoluteVolume() const;	// factors in SOUNDMAN->GetMixVolume()
	int GetID() const { return m_iID; }
	void SetParams( const RageSoundParams &p );
	const RageSoundParams &GetParams() const { return m_Param; }

private:
	mutable RageMutex m_Mutex;

	SoundReader *m_pSource;
	CircBuf<char> m_DataBuffer;
	bool FillBuf( int iFrames );

	/* We keep track of sound blocks we've sent out recently through GetDataToPlay. */
	pos_map_queue m_PositionMapping;
	
	CString m_sFilePath;

	RageSoundParams m_Param;
	
	/* Current position of the output sound, in frames.  If < 0, nothing will play
	 * until it becomes positive. */
	int m_iDecodePosition;

	/* Hack: When we stop a playing sound, we can't ask the driver the position
	 * (we're not playing); and we can't seek back to the current playing position
	 * when we stop (too slow), but we want to be able to report the position we
	 * were at when we stopped without jumping to the last position we buffered. 
	 * Keep track of the position after a seek or stop, so we can return a sane
	 * position when stopped, and when playing but pos_map hasn't yet been filled. */
	int m_iStoppedPosition;
	bool m_bPlaying;

	/* Keep track of the max SOUNDMAN->GetPosition result (see GetPositionSecondsInternal). */
	mutable int64_t m_iMaxDriverFrame;

	/* Unique ID number for this instance of RageSound. */
	int m_iID;

	CString m_sError;

	int64_t GetPositionSecondsInternal( bool *bApproximate=NULL ) const;
	bool SetPositionFrames( int frames = -1 );
	int GetData( char *pBuffer, int iSize );
	void Fail( CString sReason );
	int Bytes_Available() const;
	RageSoundParams::StopMode_t GetStopMode() const; /* resolves M_AUTO */

	void SoundIsFinishedPlaying(); // called by sound drivers

	static void RateChange( char *pBuf, int &iCount, int iInputSpeed, int iOutputSpeed, int iChannels );

public:
	/* These functions are called only by sound drivers. */

	/* Returns the number of bytes actually put into pBuffer. If less than iSize is
	 * returned, it signals the stream to stop; once it's flushed, SoundStopped will
	 * be called.  Until then, SOUNDMAN->GetPosition can still be called; the sound
	 * is still playing. */
	int GetPCM( char *pBuffer, int iSize, int64_t iFrameno );
	bool GetDataToPlay( int16_t *pBuffer, int iSize, int &iPosition, int &iBytesRead );
	void CommitPlayingPosition( int64_t iFrameno, int iPosition, int iBytesRead );
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
