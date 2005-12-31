/* RageSoundManager - A global singleton to interface RageSound and RageSoundDriver. */

#ifndef RAGE_SOUND_MANAGER_H
#define RAGE_SOUND_MANAGER_H

#include <set>
#include <map>
#include "RageUtil_CircularBuffer.h"

class RageSound;
class RageSoundBase;
class RageSoundDriver;
struct RageSoundParams;
class SoundReader;
class RageSoundReader_Preload;

class RageSoundManager
{
public:
	RageSoundManager();
	~RageSoundManager();

	/* This may be called when shutting down, in order to stop all sounds.  This
	 * should be called before shutting down threads that may have running sounds,
	 * in order to prevent DirectSound delays and glitches.  Further attempts to
	 * start sounds will do nothing, and threads may be shut down. */
	void Shutdown();

	void Init();

	float GetMixVolume() const { return m_fMixVolume; }
	void SetMixVolume( float fMixVol );
	bool GetPlayOnlyCriticalSounds() const { return m_bPlayOnlyCriticalSounds; }
	void SetPlayOnlyCriticalSounds( bool bPlayOnlyCriticalSounds );

	void Update( float fDeltaTime );
	void StartMixing( RageSoundBase *snd );	/* used by RageSound */
	void StopMixing( RageSoundBase *snd );	/* used by RageSound */
	bool Pause( RageSoundBase *snd, bool bPause );	/* used by RageSound */
	int64_t GetPosition( const RageSoundBase *snd ) const;	/* used by RageSound */
	void RegisterSound( RageSound *p );		/* used by RageSound */
	void UnregisterSound( RageSound *p );	/* used by RageSound */
	int GetUniqueID();						/* used by RageSound */
	void CommitPlayingPosition( int ID, int64_t frameno, int pos, int got_bytes );	/* used by drivers */
	float GetPlayLatency() const;
	int GetDriverSampleRate( int iRate ) const;

	/* When deleting a sound from any thread except the one calling Update(), this
	 * must be used to prevent race conditions. */
	void DeleteSound( RageSound *pSound );
	void DeleteSoundWhenFinished( RageSound *pSound );

	SoundReader *GetLoadedSound( const CString &sPath );
	void AddLoadedSound( const CString &sPath, RageSoundReader_Preload *pSound );

	void PlayOnce( CString sPath );

	RageSound *PlaySound( RageSound &snd, const RageSoundParams *params = NULL );
	RageSound *PlayCopyOfSound( RageSound &snd, const RageSoundParams *params = NULL );

private:
	/* Set of sounds that we've taken over (and are responsible for deleting
	 * when they're finished playing): */
	set<RageSound *> owned_sounds;

	/* A list of all sounds that currently exist, by ID. */
	map<int,RageSound *> all_sounds;

	map<CString, RageSoundReader_Preload *> m_mapPreloadedSounds;

	RageSoundDriver *m_pDriver;

	/* Prefs: */
	float m_fMixVolume;
	bool m_bPlayOnlyCriticalSounds;

	struct queued_pos_map_t
	{
		int ID, pos, got_frames;
		int64_t frameno;
	};

	CircBuf<queued_pos_map_t> pos_map_queue;
	void FlushPosMapQueue();

	RageSound *GetSoundByID( int ID );
};

extern RageSoundManager *SOUNDMAN;

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
