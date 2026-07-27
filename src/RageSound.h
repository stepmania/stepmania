/* RageSound - High-level sound object. */

#ifndef RAGE_SOUND_H
#define RAGE_SOUND_H

#include "RageThreads.h"
#include "RageTimer.h"
#include "RageSoundPosMap.h"

#include <cstdint>

class RageSoundReader;
struct lua_State;

/* Driver interface for sounds: this is what drivers see. */
class RageSoundBase
{
public:
	virtual ~RageSoundBase() { }
	virtual void SoundIsFinishedPlaying() = 0;
	virtual int GetDataToPlay( float *buffer, int size, std::int64_t &iStreamFrame, int &got_bytes ) = 0;
	virtual void CommitPlayingPosition( std::int64_t iFrameno, std::int64_t iPosition, int iBytesRead ) = 0;
	virtual RageTimer GetStartTime() const { return RageZeroTimer; }
	virtual RString GetLoadedFilePath() const = 0;
};

/**
 * @brief The parameters to play a sound.
 *
 * These are normally changed before playing begins,
 * and are constant from then on. */
struct RageSoundParams
{
	RageSoundParams();

	// The amount of data to play (or loop):
	float m_StartSecond;
	float m_LengthSeconds;

	// Number of seconds to spend fading in.
	float m_fFadeInSeconds;

	// Number of seconds to spend fading out.
	float m_fFadeOutSeconds;

	float m_Volume;	// multiplies with SOUNDMAN->GetMixVolume()
	float m_fAttractVolume;	// multiplies with m_Volume

	/* Number of samples input and output when changing speed.
	 * Currently, this is either 1/1, 5/4 or 4/5. */
	float m_fPitch;
	float m_fSpeed;

	/* Optional driver feature: time to actually start playing sounds.
	 * If zero, or if not supported, the sound will start immediately. */
	RageTimer m_StartTime;

	/** @brief How does the sound stop itself, if it does? */
	enum StopMode_t {
		M_STOP, /**< The sound is stopped at the end. */
		M_LOOP, /**< The sound restarts itself. */
		M_CONTINUE, /**< Silence is fed at the end to continue timing longer than the sound. */
		M_AUTO /**< The default, the sound stops while obeying filename hints. */
	} /** @brief How does the sound stop itself, if it does? */ StopMode;

	bool m_bIsCriticalSound; // "is a sound that should be played even during attract"
};

struct RageSoundLoadParams
{
	RageSoundLoadParams();

	/* If true, speed and pitch changes will be supported for this sound, at a
	 * small memory penalty if not used. */
	bool m_bSupportRateChanging;

	// If true, panning will be supported for this sound.
	bool m_bSupportPan;
};

class RageSound: public RageSoundBase
{
public:
	RageSound();
	~RageSound();
	RageSound( const RageSound &cpy );
	RageSound &operator=( const RageSound &cpy );

	/* If bPrecache == true, we'll preload the entire file into memory if
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
	bool Load( RString sFile, bool bPrecache, const RageSoundLoadParams *pParams = nullptr );

	/* Using this version means the "don't care" about caching. Currently,
	 * this always will not cache the sound; this may become a preference. */
	bool Load( RString sFile );

	/* Load a RageSoundReader that you've set up yourself. Sample rate conversion
	 * will be set up only if needed. Doesn't fail. */
	void LoadSoundReader( RageSoundReader *pSound );

	// Get the loaded RageSoundReader. While playing, only properties can be set.
	RageSoundReader *GetSoundReader() { return m_pSource; }

	void Unload();
	bool IsLoaded() const;
	void DeleteSelfWhenFinishedPlaying();

	void StartPlaying();
	void StopPlaying();

	RString GetError() const { return m_sError; }

	void Play(bool is_action, const RageSoundParams *params=nullptr);
	void PlayCopy(bool is_action, const RageSoundParams *pParams = nullptr) const;
	void Stop();

	/* Cleanly pause or unpause the sound. If the sound wasn't already playing,
	 * return true and do nothing. */
	bool Pause( bool bPause );

	float GetLengthSeconds();
	float GetPositionSeconds( bool *approximate=nullptr, RageTimer *Timestamp=nullptr ) const;
	RString GetLoadedFilePath() const { return m_sFilePath; }
	bool IsPlaying() const { return m_bPlaying; }

	float GetPlaybackRate() const;
	RageTimer GetStartTime() const;
	void SetParams( const RageSoundParams &p );
	const RageSoundParams &GetParams() const { return m_Param; }
	bool SetProperty( const RString &sProperty, float fValue );
	void SetStopModeFromString( const RString &sStopMode );

	// Lua
	virtual void PushSelf( lua_State *L );

private:
	mutable RageMutex m_Mutex;

	RageSoundReader *m_pSource;

	// We keep track of sound blocks we've sent out recently through GetDataToPlay.
	pos_map_queue m_HardwareToStreamMap;
	pos_map_queue m_StreamToSourceMap;

	RString m_sFilePath;

	void ApplyParams();
	RageSoundParams m_Param;

	/* Current position of the output sound, in frames. If < 0, nothing will play
	 * until it becomes positive. */
	std::int64_t m_iStreamFrame;

	/* Hack: When we stop a playing sound, we can't ask the driver the position
	 * (we're not playing); and we can't seek back to the current playing position
	 * when we stop (too slow), but we want to be able to report the position we
	 * were at when we stopped without jumping to the last position we buffered.
	 * Keep track of the position after a seek or stop, so we can return a sane
	 * position when stopped, and when playing but pos_map hasn't yet been filled. */
	int m_iStoppedSourceFrame;
	bool m_bPlaying;
	bool m_bDeleteWhenFinished;

	RString m_sError;

	int GetSourceFrameFromHardwareFrame( std::int64_t iHardwareFrame, bool *bApproximate = nullptr ) const;

	bool SetPositionFrames( int frames = -1 );
	RageSoundParams::StopMode_t GetStopMode() const; // resolves M_AUTO

	void SoundIsFinishedPlaying(); // called by sound drivers

public:
	// These functions are called only by sound drivers.

	/* Returns the number of bytes actually put into pBuffer. If 0 is returned,
	 * it signals the stream to stop; once it's flushed, SoundStopped will be
	 * called. Until then, SOUNDMAN->GetPosition can still be called; the sound
	 * is still playing. */
	int GetDataToPlay( float *pBuffer, int iSize, std::int64_t &iStreamFrame, int &iBytesRead );
	void CommitPlayingPosition( std::int64_t iHardwareFrame, std::int64_t iStreamFrame, int iGotFrames );
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
