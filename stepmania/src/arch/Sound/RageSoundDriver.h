#ifndef RAGE_SOUND_DRIVER
#define RAGE_SOUND_DRIVER

class RageSoundBase;
class RageSoundDriver
{
public:
	friend class RageSoundManager;

	/* Initialize.  On failure, an error message is returned. */
	virtual CString Init() { return ""; }

	/* A RageSound calls this to request to be played.
	 * XXX: define what we should do when it can't be played (eg. out of
	 * channels) */
	virtual void StartMixing( RageSoundBase *snd ) = 0;

	/* A RageSound calls this to request it not be played.  When this function
	 * returns, snd is no longer valid; ensure no running threads are still
	 * accessing it before returning.  This must handle gracefully the case where 
	 * snd was not actually being played, though it may print a warning. */
	virtual void StopMixing( RageSoundBase *snd ) = 0;

	/* Pause or unpause the given sound.  If the sound was stopped (not paused),
	 * return false and do nothing; otherwise return true and pause or unpause
	 * the sound.  Unlike StopMixing, pausing and unpause a sound will not lose
	 * any buffered sound (but will not release any resources associated with
	 * playing the sound, either). */
	virtual bool PauseMixing( RageSoundBase *snd, bool bStop ) = 0;

	/* Get the current position of a given buffer, in the same units and time base
	 * as passed to RageSound::GetPCM. */
	virtual int64_t GetPosition( const RageSoundBase *snd ) const = 0;

	/* When a sound is finished playing (GetPCM returns less than requested) and
	 * the sound has been completely flushed (so GetPosition is no longer meaningful),
	 * call RageSoundBase::SoundIsFinishedPlaying(). */

	/* Optional, if needed:  */
	virtual void Update(float delta) { }

	/* Sound startup latency--delay between Play() being called and actually
	 * hearing it.  (This isn't necessarily the same as the buffer latency.) */
	virtual float GetPlayLatency() const { return 0.0f; }

	virtual int GetSampleRate( int rate ) const { return 44100; }

	virtual ~RageSoundDriver() { }
};

/*
 * (c) 2002-2004 Glenn Maynard
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

#endif
