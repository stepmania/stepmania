#ifndef RAGE_SOUND_DRIVER
#define RAGE_SOUND_DRIVER

class RageSoundDriver
{
	friend class RageSound;

protected:
	friend class RageSoundManager;
	/* A RageSound calls this to request to be played.
	 * XXX: define what we should do when it can't be played (eg. out of
	 * channels) */
	virtual void StartMixing(RageSound *snd) = 0;

	/* A RageSound calls this to request it not be played.  When this function
	 * returns, snd is no longer valid; ensure no running threads are still
	 * accessing it before returning.  This must handle gracefully the case where 
	 * snd was not actually being played, though it may print a warning. */
	virtual void StopMixing(RageSound *snd) = 0;

	/* Get the current position of a given buffer, in the same units and time base
	 * as passed to RageSound::GetPCM. */
	virtual int GetPosition(const RageSound *snd) const = 0;

	/* When a sound is finished playing (GetPCM returns less than requested) and
	 * the sound has been completely flushed (so GetPosition is no longer meaningful),
	 * call RageSound::StopPlaying().  Do *not* call it when StopMixing is called. */

	/* Optional, if needed:  */
	virtual void Update(float delta) { }

	/* Sound startup latency--delay between Play() being called and actually
	 * hearing it.  (This isn't necessarily the same as the buffer latency.) */
	virtual float GetPlayLatency() const { return 0.0f; }

	/* This is called if the volume changed; call SOUNDMAN->GetMixVolume() to
	 * get it. */
	virtual void VolumeChanged() { }

	virtual int GetSampleRate() const { return 44100; }

public:
	virtual ~RageSoundDriver() { }
};

/*
 * Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */

#endif
