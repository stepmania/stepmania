/* Handle and provide an interface to the sound driver.  Delete sounds that
 * have been detached from their owner when they're finished playing. */

#include "global.h"
#include "RageSoundManager.h"
#include "RageException.h"
#include "RageUtil.h"
#include "RageSound.h"
#include "RageLog.h"
#include "RageTimer.h"
#include "RageSoundReader_Preload.h"
#include "Foreach.h"
#include "ThemeMetric.h"

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
	m_fMixVolume = 1.0f;
	m_bPlayOnlyCriticalSounds = false;
}

static ThemeMetric<CString> COULDNT_FIND_SOUND_DRIVER( "RageSoundManager", "Couldn't find a sound driver that works" );

void RageSoundManager::Init( CString sDrivers )
{
	m_pDriver = MakeRageSoundDriver( sDrivers );
	if( m_pDriver == NULL )
		RageException::Throw( COULDNT_FIND_SOUND_DRIVER.GetValue() );
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
	delete m_pDriver;
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
	SAFE_DELETE( m_pDriver );
}

void RageSoundManager::StartMixing( RageSoundBase *pSound )
{
	if( m_pDriver != NULL )
		m_pDriver->StartMixing( pSound );
}

void RageSoundManager::StopMixing( RageSoundBase *pSound )
{
	if( m_pDriver != NULL )
		m_pDriver->StopMixing( pSound );
}

bool RageSoundManager::Pause( RageSoundBase *pSound, bool bPause )
{
	if( m_pDriver == NULL )
		return false;
	else
		return m_pDriver->PauseMixing( pSound, bPause );
}

int64_t RageSoundManager::GetPosition( const RageSoundBase *pSound ) const
{
	if( m_pDriver == NULL )
		return 0;
	return m_pDriver->GetPosition( pSound );
}

void RageSoundManager::Update( float fDeltaTime )
{
	FlushPosMapQueue();

	/* Scan m_mapPreloadedSounds for sounds that are no longer loaded, and delete them. */
	g_SoundManMutex.Lock(); /* lock for access to m_mapPreloadedSounds, owned_sounds */
	{
		map<CString, RageSoundReader_Preload *>::iterator it, next;
		it = m_mapPreloadedSounds.begin();
		
		while( it != m_mapPreloadedSounds.end() )
		{
			next = it; ++next;
			if( it->second->GetReferenceCount() == 1 )
			{
				LOG->Trace( "Deleted old sound \"%s\"", it->first.c_str() );
				delete it->second;
				m_mapPreloadedSounds.erase( it );
			}

			it = next;
		}
	}

	/* Scan the owned_sounds list for sounds that are no longer playing, and delete them. */
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

	if( m_pDriver != NULL )
		m_pDriver->Update( fDeltaTime );
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
	if( m_pDriver == NULL )
		return 0;

	return m_pDriver->GetPlayLatency();
}

int RageSoundManager::GetDriverSampleRate( int iRate ) const
{
	if( m_pDriver == NULL )
		return 44100;

	return m_pDriver->GetSampleRate( iRate );
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

RageSound *RageSoundManager::PlayCopyOfSound( RageSound &snd, const RageSoundParams *pParams )
{
	RageSound *pSound = new RageSound( snd );
	DeleteSoundWhenFinished( pSound );

	if( pParams )
		pSound->SetParams( *pParams );

	// Move to the start position.
	pSound->SetPositionSeconds( pSound->GetParams().m_StartSecond );

	pSound->StartPlaying();

	return pSound;
}

void RageSoundManager::DeleteSound( RageSound *pSound )
{
	/* Stop playing the sound. */
	pSound->StopPlaying();

	/* We might be in a thread, so don't delete it here. */
	DeleteSoundWhenFinished( pSound );
}

void RageSoundManager::DeleteSoundWhenFinished( RageSound *pSound )
{
	g_SoundManMutex.Lock(); /* lock for access to owned_sounds */
	owned_sounds.insert( pSound );
	g_SoundManMutex.Unlock(); /* finished with owned_sounds */
}

/* If the given path is loaded, return a copy; otherwise return NULL.
 * It's the caller's responsibility to delete the result. */
SoundReader *RageSoundManager::GetLoadedSound( const CString &sPath_ )
{
	LockMut(g_SoundManMutex); /* lock for access to m_mapPreloadedSounds */

	CString sPath(sPath_);
	sPath.MakeLower();
	map<CString, RageSoundReader_Preload *>::const_iterator it;
	it = m_mapPreloadedSounds.find( sPath );
	if( it == m_mapPreloadedSounds.end() )
		return NULL;

	return it->second->Copy();
}

/* Add the sound to the set of loaded sounds that can be copied for reuse.
 * The sound will be kept in memory as long as there are any other references
 * to it; once we hold the last one, we'll release it. */
void RageSoundManager::AddLoadedSound( const CString &sPath_, RageSoundReader_Preload *pSound )
{
	LockMut(g_SoundManMutex); /* lock for access to m_mapPreloadedSounds */

	/* Don't AddLoadedSound a sound that's already registered.  It should have been
	 * used in GetLoadedSound. */
	CString sPath(sPath_);
	sPath.MakeLower();
	map<CString, RageSoundReader_Preload *>::const_iterator it;
	it = m_mapPreloadedSounds.find( sPath );
	ASSERT_M( it == m_mapPreloadedSounds.end(), sPath );
	
	m_mapPreloadedSounds[sPath] = (RageSoundReader_Preload *) pSound->Copy();
}


/* Don't hold the lock when we don't have to.  We call this function from other
 * threads, to avoid stalling the gameplay thread. */
void RageSoundManager::PlayOnce( CString sPath )
{
	/* We want this to start quickly, so don't try to prebuffer it. */
	RageSound *pSound = new RageSound;
	pSound->Load( sPath, false );

	pSound->Play();

	/* We're responsible for freeing it.  Add it to owned_sounds *after* we start
	 * playing, so RageSoundManager::Update doesn't free it before we actually start
	 * it. */
	DeleteSoundWhenFinished( pSound );
}

void RageSoundManager::SetMixVolume( float fMixVol )
{
	ASSERT_M( fMixVol >= 0 && fMixVol <= 1, ssprintf("%f",fMixVol) );

	g_SoundManMutex.Lock(); /* lock for access to m_fMixVolume */
	m_fMixVolume = fMixVol;
	g_SoundManMutex.Unlock(); /* finished with m_fMixVolume */
}

void RageSoundManager::SetPlayOnlyCriticalSounds( bool bPlayOnlyCriticalSounds )
{
	g_SoundManMutex.Lock(); /* lock for access to m_bPlayOnlyCriticalSounds */
	m_bPlayOnlyCriticalSounds = bPlayOnlyCriticalSounds;
	g_SoundManMutex.Unlock(); /* finished with m_bPlayOnlyCriticalSounds */
}

/* Standalone helpers: */
void RageSoundManager::AttenuateBuf( int16_t *pBuf, int iSamples, float fVolume )
{
	while( iSamples-- )
	{
		*pBuf = int16_t( (*pBuf) * fVolume );
		++pBuf;
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
