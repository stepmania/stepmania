#include "stdafx.h"

#include "RageSoundManager.h"
#include "RageException.h"
#include "RageUtil.h"
#include "RageSound.h"
#include "RageLog.h"
#include "RageTimer.h"

#include "arch/arch.h"
#include "arch/Sound/RageSoundDriver.h"

RageSoundManager::RageSoundManager()
{
	driver = MakeRageSoundDriver();
	if(!driver)
		throw RageException("Couldn't find a sound driver that works");
}

RageSoundManager::~RageSoundManager()
{
	/* Clear any sounds that we own and havn't freed yet. */
	set<RageSound *>::iterator j = owned_sounds.begin();
	while(j != owned_sounds.end())
		delete *(j++);

	delete driver;
}

void RageSoundManager::StartMixing(RageSound *snd)
{
	driver->StartMixing(snd);
}

void RageSoundManager::StopMixing(RageSound *snd)
{
	driver->StopMixing(snd);

	/* The sound is finished, and should be deleted if it's in owned_sounds.
	 * However, this call might be *from* the sound itself, and it'd be
	 * a mess to delete it while it's on the call stack.  Instead, put it
	 * in a queue to delete, and delete it on the next update. */
	if(owned_sounds.find(snd) != owned_sounds.end()) {
		sounds_to_delete.insert(snd);
		owned_sounds.erase(snd);
	}
}

int RageSoundManager::GetPosition(const RageSound *snd) const
{
	return driver->GetPosition(snd);
}

void RageSoundManager::Update(float delta)
{
	while(!sounds_to_delete.empty())
	{
		delete *sounds_to_delete.begin();
		sounds_to_delete.erase(sounds_to_delete.begin());
	}

	for(set<RageSound *>::iterator i = all_sounds.begin();
		i != all_sounds.end(); ++i)
		(*i)->Update(delta);

	driver->Update(delta);
}

float RageSoundManager::GetPlayLatency() const
{
	return driver->GetPlayLatency();
}

void RageSoundManager::PlayCopy( const RageSound &snd )
{
	/* Make a copy of the sound and play it; this detaches the sound
	 * from the screen.  This has a few effects:
	 *
	 * 1. If this is called more than once, it actually plays a new
	 * copy of the sound, rather than rewinding the currently-playing
	 * one.  (This is important for keyed games.)
	 *
	 * 2. If the screen closes, the sound plays to completion.
	 *
	 * I'm not sure if copying will become a performance problem.  As a
	 * flag, we could play snd instead of a copy if snd isn't actually
	 * playing.  This would keep property 1 and lose 2, which would be
	 * fine for keyed games.  Copying a streamed sound reopens the sound;
	 * copying a prebuffered sound makes a new copy of the buffer (unless
	 * std::basic_string happens to be refcounted).  That's not too bad,
	 * but we might do five of these in the same frame ...
	 */
	RageSound *newsnd = new RageSound(snd);

	/* We're responsible for freeing it. */
	owned_sounds.insert(newsnd);

	newsnd->Play();
}

void RageSoundManager::PlayOnce( CString sPath )
{
	/* We want this to start quickly, so don't try to prebuffer it. */
	RageSound *snd = new RageSound;
	snd->Load(sPath, false);

	/* We're responsible for freeing it. */
	owned_sounds.insert(snd);

	snd->Play();
}

void RageSoundManager::PlayOnceFromDir( CString sDir )
{
	if( sDir == "" )
		return;

	// make sure there's a backslash at the end of this path
	if( sDir[sDir.GetLength()-1] != '\\' )
		sDir += "\\";

	CStringArray arraySoundFiles;
	GetDirListing( sDir + "*.mp3", arraySoundFiles );
	GetDirListing( sDir + "*.wav", arraySoundFiles );
	GetDirListing( sDir + "*.ogg", arraySoundFiles );

	if( arraySoundFiles.empty() )
		return;

	int index = rand() % arraySoundFiles.size();
	PlayOnce( sDir + arraySoundFiles[index] );
}
