#ifndef RAGE_SOUND_MANAGER_H
#define RAGE_SOUND_MANAGER_H

#include <set>
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

	RageSoundDriver *driver;

public:
	RageMutex lock;

	RageSoundManager();
	~RageSoundManager();

	void Update(float delta);
	void StartMixing(RageSound *snd);	/* used by RageSound */
	void StopMixing(RageSound *snd);		/* used by RageSound */
	int GetPosition(const RageSound *snd) const;	/* used by RageSound */

	float GetPlayLatency() const;

	void PlayOnce( CString sPath );
	void PlayOnceFromDir( CString sDir );
	void PlayCopy( const RageSound &snd );


	/* A list of all sounds that currently exist.  RageSound adds and removes
	 * itself to this. */
	set<RageSound *> all_sounds;
};

extern RageSoundManager *SOUNDMAN;

#endif

