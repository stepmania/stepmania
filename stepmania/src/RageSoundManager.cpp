/* Handle and provide an interface to the sound driver.  Delete sounds that
 * have been detached from their owner when they're finished playing.  Distribute Update
 * calls to all sounds. */
#include "global.h"

#include "RageSoundManager.h"
#include "RageException.h"
#include "RageUtil.h"
#include "RageSound.h"
#include "RageLog.h"
#include "RageTimer.h"

#include "arch/arch.h"
#include "arch/Sound/RageSoundDriver.h"
#include "SDL_audio.h"

/*
 * This mutex is locked before Update() deletes old sounds from owned_sounds.  Lock
 * this mutex if you want to ensure that sounds remain valid.  (Other threads may
 * still delete them; this only guarantees that RageSoundManager won't.)
 *
 * The lock ordering requirements are:
 * g_DeletionMutex before RageSound::Lock
 * RageSound::Lock before g_SoundManMutex
 * RageSound::Lock must not be locked when calling driver calls (since the driver
 * may lock a mutex and then make RageSound calls back)
 *
 * (This is important: you must not make RageSound calls that might lock while holding
 * g_SoundManMutex, but you can do so while holding g_DeletionMutex.)
 */

static RageMutex g_DeletionMutex("SoundDeletionMutex");
static RageMutex g_SoundManMutex("SoundMan");

RageSoundManager *SOUNDMAN = NULL;

RageSoundManager::RageSoundManager(CString drivers)
{
	/* needs to be done first */
	SOUNDMAN = this;

	try
	{
		MixVolume = 1.0f;

		driver = MakeRageSoundDriver(drivers);
		if(!driver)
			RageException::Throw("Couldn't find a sound driver that works");
	} catch(...) {
		SOUNDMAN = NULL;
		throw;
	}

	pos_map_queue.reserve( 1024 );
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

void RageSoundManager::StartMixing( RageSoundBase *snd )
{
	driver->StartMixing(snd);
}

void RageSoundManager::StopMixing( RageSoundBase *snd )
{
	driver->StopMixing(snd);
}

int64_t RageSoundManager::GetPosition( const RageSoundBase *snd ) const
{
	return driver->GetPosition(snd);
}

void RageSoundManager::Update(float delta)
{
	FlushPosMapQueue();

	g_DeletionMutex.Lock();

	/* Scan the owned_sounds list for sounds that are no longer playing, and delete them. */
	g_SoundManMutex.Lock(); /* lock for access to owned_sounds */
	set<RageSound *>::iterator it;
	set<RageSound *> ToDelete;
	for( it = owned_sounds.begin(); it != owned_sounds.end(); ++it )
		if( !(*it)->IsPlaying() )
			ToDelete.insert( *it );

	for( it = ToDelete.begin(); it != ToDelete.end(); ++it )
		owned_sounds.erase( *it );
	g_SoundManMutex.Unlock(); /* finished with owned_sounds */

	/* We can safely delete sounds while holding g_DeletionMutex, but not while
	 * holding g_SoundManMutex (see the mutex ordering at the top of the file). */
	for( it = ToDelete.begin(); it != ToDelete.end(); ++it )
		delete *it;
	g_DeletionMutex.Unlock();

//	for(set<RageSound *>::iterator i = all_sounds.begin();
//		i != all_sounds.end(); ++i)
//		(*i)->Update(delta);

	driver->Update(delta);
}

/* Register the given sound, and return a unique ID. */
int RageSoundManager::RegisterSound( RageSound *p )
{
	LockMut(g_SoundManMutex); /* lock for access to all_sounds and iID */

	all_sounds.insert( p );

	static int iID = 0;
	return ++iID;
}

void RageSoundManager::UnregisterSound( RageSound *p )
{
	g_SoundManMutex.Lock(); /* lock for access to all_sounds */
	all_sounds.erase( p );
	g_SoundManMutex.Unlock(); /* finished with all_sounds */
}

void RageSoundManager::RegisterPlayingSound( RageSound *p )
{
	g_SoundManMutex.Lock(); /* lock for access to playing_sounds */
	SOUNDMAN->playing_sounds.insert( p );
	g_SoundManMutex.Unlock(); /* finished with playing_sounds */
}

void RageSoundManager::UnregisterPlayingSound( RageSound *p )
{
	g_SoundManMutex.Lock(); /* lock for access to playing_sounds */
	SOUNDMAN->playing_sounds.erase( p );
	g_SoundManMutex.Unlock(); /* finished with playing_sounds */
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
	set<RageSound *>::iterator it;
	for( it = all_sounds.begin(); it != all_sounds.end(); ++it )
		if( (*it)->GetID() == ID )
			return *it;
	return NULL;
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
			LOG->Trace("ignored unknown (stale?) commit ID %i", p.ID);
			continue;
		}

		pSound->CommitPlayingPosition( p.frameno, p.pos, p.got_frames );
	}
}

float RageSoundManager::GetPlayLatency() const
{
	return driver->GetPlayLatency();
}

int RageSoundManager::GetDriverSampleRate( int rate ) const
{
	return driver->GetSampleRate( rate );
}

RageSound *RageSoundManager::PlaySound( RageSound &snd, const RageSoundParams *params )
{
	RageSound *sound_to_play;
	if(!snd.IsPlaying())
		sound_to_play = &snd;
	else
	{
		sound_to_play = new RageSound(snd);

		/* We're responsible for freeing it. */
		g_SoundManMutex.Lock(); /* lock for access to owned_sounds */
		owned_sounds.insert(sound_to_play);
		g_SoundManMutex.Unlock(); /* finished with owned_sounds */
	}

	if( params )
		sound_to_play->SetParams( *params );

	// Move to the start position.
	sound_to_play->SetPositionSeconds( sound_to_play->GetParams().m_StartSecond );

	sound_to_play->StartPlaying();

	return sound_to_play;
}

/* Stop playing all playing sounds derived from the same parent as snd. */
void RageSoundManager::StopPlayingAllCopiesOfSound(RageSound &snd)
{
	g_DeletionMutex.Lock();

	vector<RageSound *> snds;
	GetCopies( snd, snds );

	vector<RageSound *>::iterator it;
	for( it = snds.begin(); it != snds.end(); ++it )
	{
		if( (*it)->IsPlaying() )
			(*it)->StopPlaying();
	}

	g_DeletionMutex.Unlock();
}

/* XXX: If this is ever called from a thread, it should take a bLockSounds parameter,
 * like GetCopies. */
set<RageSound *> RageSoundManager::GetPlayingSounds() const
{
	LockMut(g_SoundManMutex); /* lock for access to playing_sounds */
	return playing_sounds;
}

void RageSoundManager::DeleteSound( RageSound *p )
{
	/* Stop playing the sound. */
	p->StopPlaying();

	/* Add it to owned_sounds.  It'll be deleted the next time we come around
	 * to Update(). */
	g_SoundManMutex.Lock(); /* lock for access to owned_sounds */
	owned_sounds.insert( p );
	g_SoundManMutex.Unlock(); /* finished with owned_sounds */
}

void RageSoundManager::StopPlayingSoundsForThisThread()
{
	/* Lock to make sure sounds don't become invalidated below before we get to them. */
	g_DeletionMutex.Lock();

	set<RageSound *> Sounds = GetPlayingSounds();
	set<RageSound *>::iterator it;
	for( it = Sounds.begin(); it != Sounds.end(); ++it )
	{
		if( (*it)->GetPlayingThread() != RageThread::GetCurrentThreadID() )
			continue;
		(*it)->Stop();
	}
	g_DeletionMutex.Unlock();
}

/*
 * If bLockSounds is true, all returned sounds will be locked; you must call Unlock()
 * on all returned sounds when you're done.  This is used for thread safety: without
 * it, if this is called in a separate thread, returned copies might stop playing
 * and be deleted by RageSoundManager::Update before you're done with them.
 *
 * If bLockSounds is false, sounds are not locked.  This is only safe to use in the same
 * thread as RageSoundManager::Update (the gameplay thread).
 */
void RageSoundManager::GetCopies( RageSound &snd, vector<RageSound *> &snds, bool bLockSounds )
{
	snds.clear();

	/* Locking this means that Update() will not delete sounds.  g_SoundManMutex does that,
	 * too, but we can't hold g_SoundManMutex when we lock sounds. */
	g_DeletionMutex.Lock();

	g_SoundManMutex.Lock(); /* lock for access to all_sounds */
	set<RageSound *> sounds = all_sounds;
	g_SoundManMutex.Unlock(); /* finished with all_sounds */
	
	RageSound *parent = snd.GetOriginal();

	set<RageSound *>::iterator it;
	for( it = sounds.begin(); it != sounds.end(); ++it )
	{
		if( (*it)->GetOriginal() != parent )
			continue;
		if( bLockSounds )
			(*it)->LockSound();

		snds.push_back( *it );
	}

	g_DeletionMutex.Unlock();
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
	g_SoundManMutex.Lock(); /* lock for access to owned_sounds */
	owned_sounds.insert(snd);
	g_SoundManMutex.Unlock(); /* finished with owned_sounds */
}

void RageSoundManager::SetPrefs(float MixVol)
{
	g_SoundManMutex.Lock(); /* lock for access to MixVolume */
	MixVolume = MixVol;
	g_SoundManMutex.Unlock(); /* finished with MixVolume */

	driver->VolumeChanged();
}

/* Standalone helpers: */
void RageSoundManager::AttenuateBuf( Sint16 *buf, int samples, float vol )
{
	while( samples-- )
	{
		*buf = Sint16( (*buf) * vol );
		++buf;
	}
}

	
SoundMixBuffer::SoundMixBuffer()
{
	bufsize = used = 0;
	mixbuf = NULL;
	SetVolume( SOUNDMAN->GetMixVolume() );
}

SoundMixBuffer::~SoundMixBuffer()
{
	free(mixbuf);
}

void SoundMixBuffer::SetVolume( float f )
{
	vol = int(256*f);
}

void SoundMixBuffer::write( const Sint16 *buf, unsigned size, float volume, int offset )
{
	int factor = vol;
	if( volume != -1 )
		factor = int( 256*volume );

	const unsigned realsize = size+offset;
	if( bufsize < realsize )
	{
		mixbuf = (Sint32 *) realloc( mixbuf, sizeof(Sint32) * realsize );
		bufsize = realsize;
	}

	if( used < realsize )
	{
		memset( mixbuf + used, 0, (realsize - used) * sizeof(Sint32) );
		used = realsize;
	}

	/* Scale volume and add. */
	for(unsigned pos = 0; pos < size; ++pos)
		mixbuf[pos+offset] += buf[pos] * factor;
}

void SoundMixBuffer::read(Sint16 *buf)
{
	for( unsigned pos = 0; pos < used; ++pos )
	{
		Sint32 out = (mixbuf[pos]) / 256;
		buf[pos] = (Sint16) clamp( out, -32768, 32767 );
	}
	used = 0;
}

void SoundMixBuffer::read( float *buf )
{
	const int Minimum = -32768 * 256;
	const int Maximum = 32767 * 256;

	for( unsigned pos = 0; pos < used; ++pos )
		buf[pos] = SCALE( (float)mixbuf[pos], Minimum, Maximum, -1.0f, 1.0f );

	used = 0;
}
