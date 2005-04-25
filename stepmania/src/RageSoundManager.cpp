/* Handle and provide an interface to the sound driver.  Delete sounds that
 * have been detached from their owner when they're finished playing. */

#include "global.h"
#include "RageSoundManager.h"
#include "RageException.h"
#include "RageUtil.h"
#include "RageSound.h"
#include "RageLog.h"
#include "RageTimer.h"

#include "arch/Sound/RageSoundDriver.h"

#include "arch/arch.h"

/*
 * The lock ordering requirements are:
 * RageSound::Lock before g_SoundManMutex
 * RageSound::Lock must not be locked when calling driver calls (since the driver
 * may lock a mutex and then make RageSound calls back)
 *
 * Do not make RageSound calls that might lock while holding g_SoundManMutex.
 */

static RageMutex g_SoundManMutex("SoundMan");

RageSoundManager *SOUNDMAN = NULL;

RageSoundManager::RageSoundManager()
{
	pos_map_queue.reserve( 1024 );
	MixVolume = 1.0f;
}

void RageSoundManager::Init( CString drivers )
{
	driver = MakeRageSoundDriver(drivers);
	if( driver == NULL )
		RageException::Throw( "Couldn't find a sound driver that works" );
}

RageSoundManager::~RageSoundManager()
{
	g_SoundManMutex.Lock(); /* lock for access to owned_sounds */
	set<RageSound *> sounds = owned_sounds;
	g_SoundManMutex.Unlock(); /* finished with owned_sounds */

	/* Clear any sounds that we own and havn't freed yet. */
	set<RageSound *>::iterator j = sounds.begin();
	while(j != sounds.end())
		delete *(j++);

	/* Don't lock while deleting the driver (the decoder thread might deadlock). */
	delete driver;
}

/*
 * Previously, we went to some lengths to shut down sounds before exiting threads.
 * The only other thread that actually starts sounds is SOUND.  Doing this was ugly;
 * instead, let's shut down the driver early, stopping all sounds.  We don't want
 * to delete SOUNDMAN early, since those threads are still using it; just shut down
 * the driver.
 */
void RageSoundManager::Shutdown()
{
	if( driver != NULL )
	{
		delete driver;
		driver = NULL;
	}
}

void RageSoundManager::StartMixing( RageSoundBase *snd )
{
	if( driver != NULL )
		driver->StartMixing(snd);
}

void RageSoundManager::StopMixing( RageSoundBase *snd )
{
	if( driver != NULL )
		driver->StopMixing(snd);
}

bool RageSoundManager::Pause( RageSoundBase *snd, bool bPause )
{
	if( driver == NULL )
		return false;
	else
		return driver->PauseMixing( snd, bPause );
}

int64_t RageSoundManager::GetPosition( const RageSoundBase *snd ) const
{
	if( driver == NULL )
		return 0;
	return driver->GetPosition(snd);
}

void RageSoundManager::Update(float delta)
{
	FlushPosMapQueue();

	/* Scan the owned_sounds list for sounds that are no longer playing, and delete them. */
	g_SoundManMutex.Lock(); /* lock for access to owned_sounds */
	set<RageSound *> ToDelete;
	for( set<RageSound *>::iterator it = owned_sounds.begin(); it != owned_sounds.end(); ++it )
	{
		RageSound *pSound = *it;
		if( pSound->IsPlaying() )
			continue;

		LOG->Trace("XXX: deleting '%s'", pSound->GetLoadedFilePath().c_str());

		ToDelete.insert( pSound );
	}
	
	for( set<RageSound *>::iterator it = ToDelete.begin(); it != ToDelete.end(); ++it )
		owned_sounds.erase( *it );
	g_SoundManMutex.Unlock(); /* finished with owned_sounds */

	/* Be sure to release g_SoundManMutex before deleting sounds. */
	for( set<RageSound *>::iterator it = ToDelete.begin(); it != ToDelete.end(); ++it )
		delete *it;

	if( driver != NULL )
		driver->Update(delta);
}

/* Register the given sound, and return a unique ID. */
void RageSoundManager::RegisterSound( RageSound *p )
{
	g_SoundManMutex.Lock(); /* lock for access to all_sounds */
	all_sounds[p->GetID()] = p;
	g_SoundManMutex.Unlock(); /* finished with all_sounds */
}

void RageSoundManager::UnregisterSound( RageSound *p )
{
	g_SoundManMutex.Lock(); /* lock for access to all_sounds */
	all_sounds.erase( p->GetID() );
	g_SoundManMutex.Unlock(); /* finished with all_sounds */
}

/* Return a unique ID. */
int RageSoundManager::GetUniqueID()
{
	LockMut(g_SoundManMutex); /* serialize iID */
	static int iID = 0;
	return ++iID;
}

void RageSoundManager::CommitPlayingPosition( int ID, int64_t frameno, int pos, int got_frames )
{
	/* This can be called from realtime threads; don't lock any mutexes. */
	queued_pos_map_t p;
	p.ID = ID;
	p.frameno = frameno;
	p.pos = pos;
	p.got_frames = got_frames;

	pos_map_queue.write( &p, 1 );
}

RageSound *RageSoundManager::GetSoundByID( int ID )
{
	LockMut( g_SoundManMutex );

	/* Find the sound with p.ID. */
	map<int,RageSound *>::iterator it;
	it = all_sounds.find( ID );
	if( it == all_sounds.end() )
		return NULL;
	return it->second;
}

/* This is only called by RageSoundManager::Update. */
void RageSoundManager::FlushPosMapQueue()
{
	queued_pos_map_t p;

	/* We don't need to lock to access pos_map_queue. */
	while( pos_map_queue.read( &p, 1 ) )
	{
		RageSound *pSound = GetSoundByID( p.ID );

		/* If we can't find the ID, the sound was probably deleted before we got here. */
		if( pSound == NULL )
		{
			// LOG->Trace("ignored unknown (stale?) commit ID %i", p.ID);
			continue;
		}

		pSound->CommitPlayingPosition( p.frameno, p.pos, p.got_frames );
	}
}

float RageSoundManager::GetPlayLatency() const
{
	if( driver == NULL )
		return 0;

	return driver->GetPlayLatency();
}

int RageSoundManager::GetDriverSampleRate( int rate ) const
{
	if( driver == NULL )
		return 44100;

	return driver->GetSampleRate( rate );
}

/* The return value of PlaySound and PlayCopyOfSound is only valid in the main
 * thread, until the next call to Update().  After that, it may be deleted. */
RageSound *RageSoundManager::PlaySound( RageSound &snd, const RageSoundParams *params )
{
	if( snd.IsPlaying() )
		return PlayCopyOfSound( snd, params );

	if( params )
		snd.SetParams( *params );

	// Move to the start position.
	snd.SetPositionSeconds( snd.GetParams().m_StartSecond );

	snd.StartPlaying();
	return &snd;
}

RageSound *RageSoundManager::PlayCopyOfSound( RageSound &snd, const RageSoundParams *params )
{
	RageSound *pSound = new RageSound(snd);
	DeleteSoundWhenFinished( pSound );

	if( params )
		pSound->SetParams( *params );

	// Move to the start position.
	pSound->SetPositionSeconds( pSound->GetParams().m_StartSecond );

	pSound->StartPlaying();

	return pSound;
}

void RageSoundManager::DeleteSound( RageSound *p )
{
	/* Stop playing the sound. */
	p->StopPlaying();

	/* We might be in a thread, so don't delete it here. */
	DeleteSoundWhenFinished( p );
}

void RageSoundManager::DeleteSoundWhenFinished( RageSound *pSound )
{
	g_SoundManMutex.Lock(); /* lock for access to owned_sounds */
	owned_sounds.insert( pSound );
	g_SoundManMutex.Unlock(); /* finished with owned_sounds */
}

/* Don't hold the lock when we don't have to.  We call this function from other
 * threads, to avoid stalling the gameplay thread. */
void RageSoundManager::PlayOnce( CString sPath )
{
	/* We want this to start quickly, so don't try to prebuffer it. */
	RageSound *snd = new RageSound;
	snd->Load(sPath, false);

	snd->Play();

	/* We're responsible for freeing it.  Add it to owned_sounds *after* we start
	 * playing, so RageSoundManager::Update doesn't free it before we actually start
	 * it. */
	DeleteSoundWhenFinished( snd );
}

void RageSoundManager::SetPrefs(float MixVol)
{
	g_SoundManMutex.Lock(); /* lock for access to MixVolume */
	MixVolume = MixVol;
	g_SoundManMutex.Unlock(); /* finished with MixVolume */
}

/* Standalone helpers: */
void RageSoundManager::AttenuateBuf( int16_t *buf, int samples, float vol )
{
	while( samples-- )
	{
		*buf = int16_t( (*buf) * vol );
		++buf;
	}
}

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
