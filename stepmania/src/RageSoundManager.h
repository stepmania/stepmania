#ifndef RAGE_SOUND_MANAGER_H
#define RAGE_SOUND_MANAGER_H

#include <set>
#include <map>
#include "arch/Sound/RageSoundDriver.h"

#include "RageThreads.h"

class RageSound;

//#define PlayOnceStreamed PlayOnce
//#define PlayOnceStreamedFromDir PlayOnceFromDir

class RageSoundManager
{
	/* Set of sounds that we've taken over (and are responsible for deleting
	 * when they're finished playing): */
	set<RageSound *> owned_sounds;

	/* Set of sounds that are finished and should be deleted. */
	set<RageSound *> sounds_to_delete;

	set<RageSound *> playing_sounds;

	struct FakeSound {
		float begin;
		int samples_read;
	};
	map<RageSound *, FakeSound> fake_sounds;

	RageSoundDriver *driver;

public:
	RageMutex lock;

	RageSoundManager(CString drivers);
	~RageSoundManager();

	void Update(float delta);
	void StartMixing(RageSound *snd);	/* used by RageSound */
	void StopMixing(RageSound *snd);	/* used by RageSound */
	int GetPosition(const RageSound *snd) const;	/* used by RageSound */
	void AddFakeSound(RageSound *snd);		/* used by drivers */
	float GetPlayLatency() const;

	void PlayOnce( CString sPath );
	static void PlayOnceFromDir( CString sDir );

	RageSound *PlaySound(RageSound &snd);
	void StopPlayingSound(RageSound &snd);
	void GetCopies(RageSound &snd, vector<RageSound *> &snds);
	/* A list of all sounds that currently exist.  RageSound adds and removes
	 * itself to this. */
	set<RageSound *> all_sounds;

	static void MixAudio(Uint8 *dst, const Uint8 *src, Uint32 len, int volume);
};

extern RageSoundManager *SOUNDMAN;

#endif

