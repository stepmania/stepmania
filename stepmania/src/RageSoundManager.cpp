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
}

RageSoundManager::~RageSoundManager()
{
	lock.Lock();
	/* Clear any sounds that we own and havn't freed yet. */
	set<RageSoundBase *>::iterator j = owned_sounds.begin();
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

	/* The sound is finished, and should be deleted if it's in owned_sounds.
	 * However, this call might be *from* the sound itself, and it'd be
	 * a mess to delete it while it's on the call stack.  Instead, put it
	 * in a queue to delete, and delete it on the next update. */
	LockMut(lock);
	if(owned_sounds.find(snd) != owned_sounds.end()) {
		sounds_to_delete.insert(snd);
		owned_sounds.erase(snd);
	}
}

int64_t RageSoundManager::GetPosition( const RageSoundBase *snd ) const
{
	return driver->GetPosition(snd);
}

void RageSoundManager::Update(float delta)
{
	LockMut(lock);
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

int RageSoundManager::GetDriverSampleRate( int rate ) const
{
	return driver->GetSampleRate( rate );
}

RageSound *RageSoundManager::PlaySound(RageSound &snd)
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

	// Move to the start position.
	sound_to_play->SetPositionSeconds();
	sound_to_play->StartPlaying();

	return sound_to_play;
}

void RageSoundManager::StopPlayingSound(RageSound &snd)
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


void RageSoundManager::PlayOnce( CString sPath )
{
	/* We want this to start quickly, so don't try to prebuffer it. */
	RageSound *snd = new RageSound;
	snd->Load(sPath, false);

	/* We're responsible for freeing it. */
	lock.Lock();
	owned_sounds.insert(snd);
	lock.Unlock();

	snd->Play();
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

void SoundMixBuffer::write( const Sint16 *buf, unsigned size, float volume )
{
	int factor = vol;
	if( volume != -1 )
		factor = int( 256*volume );

	if(bufsize < size)
	{
		mixbuf = (Sint32 *) realloc(mixbuf, sizeof(Sint32) * size);
		bufsize = size;
	}

	if(used < size)
	{
		memset(mixbuf + used, 0, (size - used) * sizeof(Sint32));
		used = size;
	}

	/* Scale volume and add. */
	for(unsigned pos = 0; pos < size; ++pos)
		mixbuf[pos] += buf[pos] * factor;
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
