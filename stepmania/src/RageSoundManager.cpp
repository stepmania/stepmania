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

RageSoundManager *SOUNDMAN = NULL;

RageSoundManager::RageSoundManager(CString drivers):
	lock("RageSoundManager")
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
	lock.Lock();
	/* Clear any sounds that we own and havn't freed yet. */
	set<RageSound *>::iterator j = owned_sounds.begin();
	while(j != owned_sounds.end())
		delete *(j++);
	lock.Unlock();

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
	LockMut(lock);

	FlushPosMapQueue();

	/* Scan the owned_sounds list for sounds that are no longer playing, and delete them. */
	set<RageSound *>::iterator it;
	set<RageSound *> ToDelete;
	for( it = owned_sounds.begin(); it != owned_sounds.end(); ++it )
		if( !(*it)->IsPlaying() )
			ToDelete.insert( *it );

	for( it = ToDelete.begin(); it != ToDelete.end(); ++it )
	{
		delete *it;
		owned_sounds.erase( *it );
	}

	for(set<RageSound *>::iterator i = all_sounds.begin();
		i != all_sounds.end(); ++i)
		(*i)->Update(delta);

	driver->Update(delta);
}

/* Register the given sound, and return a unique ID. */
int RageSoundManager::RegisterSound( RageSound *p )
{
	LockMut(lock);

	all_sounds.insert( p );

	static int iID = 0;
	return ++iID;
}

void RageSoundManager::UnregisterSound( RageSound *p )
{
	LockMut(lock);
	all_sounds.erase( p );
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

void RageSoundManager::FlushPosMapQueue()
{
	LockMut(SOUNDMAN->lock);
	queued_pos_map_t p;

	while( pos_map_queue.read( &p, 1 ) )
	{
		/* Find the sound with p.ID. */
		set<RageSound *>::iterator it;
		for( it = all_sounds.begin(); it != all_sounds.end(); ++it )
			if( (*it)->GetID() == p.ID )
				break;

		/* If we can't find the ID, the sound was probably deleted before we got here. */
		if( it == all_sounds.end() )
		{
			LOG->Trace("ignored unknown (stale?) commit ID %i", p.ID);
			continue;
		}

		(*it)->CommitPlayingPosition( p.frameno, p.pos, p.got_frames );
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
	LockMut(lock);

	RageSound *sound_to_play;
	if(!snd.IsPlaying())
		sound_to_play = &snd;
	else
	{
		sound_to_play = new RageSound(snd);

		/* We're responsible for freeing it. */
		owned_sounds.insert(sound_to_play);
	}

	if( params )
		sound_to_play->SetParams( *params );

	// Move to the start position.
	sound_to_play->SetPositionSeconds( sound_to_play->GetParams().m_StartSecond );

	sound_to_play->StartPlaying();

	return sound_to_play;
}

void RageSoundManager::StopPlayingAllCopiesOfSound(RageSound &snd)
{
	LockMut(lock);

	/* Stop playing all playing sounds derived from the same parent as snd. */
	vector<RageSound *> snds;
	GetCopies(snd, snds);
	for(vector<RageSound *>::iterator i = snds.begin(); i != snds.end(); i++)
	{
		if((*i)->IsPlaying())
			(*i)->StopPlaying();
	}
}

void RageSoundManager::StopPlayingSoundsForThisThread()
{
	/* Lock to make sure sounds don't become invalidated below before we get to them. */
	LockMut(lock);

	set<RageSound *> Sounds = GetPlayingSounds();
	for( set<RageSound *>::iterator it = Sounds.begin(); it != Sounds.end(); ++it )
	{
		if( (*it)->GetPlayingThread() != RageThread::GetCurrentThreadID() )
			continue;
		(*it)->Stop();
	}
}

void RageSoundManager::GetCopies(RageSound &snd, vector<RageSound *> &snds)
{
	LockMut(lock);

	RageSound *parent = snd.GetOriginal();

	snds.clear();
	for(set<RageSound *>::iterator i = playing_sounds.begin();
		i != playing_sounds.end(); i++)
		if((*i)->GetOriginal() == parent)
			snds.push_back(*i);
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
	LockMut(lock);
	lock.Lock();
	owned_sounds.insert(snd);
	lock.Unlock();
}

void RageSoundManager::SetPrefs(float MixVol)
{
	LockMut(lock);

	MixVolume = MixVol;
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
