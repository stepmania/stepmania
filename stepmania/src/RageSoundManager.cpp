/* Handle and provide an interface to the sound driver.  Delete sounds that
 * have been detached from their owner when they're finished playing.  Distribute Update
 * calls to all sounds. */
#include "stdafx.h"

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
	MixVolume = 1.0f;

	driver = MakeRageSoundDriver(drivers);
	if(!driver)
		RageException::Throw("Couldn't find a sound driver that works");

	music = new RageSound;
}

RageSoundManager::~RageSoundManager()
{
	delete music;

	/* Clear any sounds that we own and havn't freed yet. */
	set<RageSound *>::iterator j = owned_sounds.begin();
	while(j != owned_sounds.end())
		delete *(j++);

	delete driver;
}

void RageSoundManager::StartMixing(RageSound *snd)
{
	playing_sounds.insert(snd);
	driver->StartMixing(snd);
}

void RageSoundManager::StopMixing(RageSound *snd)
{
	playing_sounds.erase(snd);

	driver->StopMixing(snd);

	/* The sound is finished, and should be deleted if it's in owned_sounds.
	 * However, this call might be *from* the sound itself, and it'd be
	 * a mess to delete it while it's on the call stack.  Instead, put it
	 * in a queue to delete, and delete it on the next update. */
	if(owned_sounds.find(snd) != owned_sounds.end()) {
		sounds_to_delete.insert(snd);
		owned_sounds.erase(snd);
	}

	map<RageSound *, FakeSound>::iterator fake = 
		fake_sounds.find(const_cast<RageSound *>(snd));
	if(fake != fake_sounds.end())
		fake_sounds.erase(fake);
}

int RageSoundManager::GetPosition(const RageSound *snd) const
{
	map<RageSound *, FakeSound>::const_iterator fake = 
		fake_sounds.find(const_cast<RageSound *>(snd));
	if(fake != fake_sounds.end())
	{
		float time_since = RageTimer::GetTimeSinceStart() - fake->second.begin;
		
		return int(time_since * 44100); // XXX
	}
	
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

	/* We keep a list of "fake" sound effects.  These are sound effects that
	 * have been requested to play, but the driver couldn't play them; usually
	 * because it's run out of sound channels.  Pretend we're playing them,
	 * since this should be uncommon. */
	{
		char buf[1024*16];
		map<RageSound *, FakeSound>::iterator j = fake_sounds.begin(),
			next = j;
		while(j != fake_sounds.end())
		{
			next++;

			RageSound *s = j->first;
			FakeSound &fake = j->second;

			int bytes = min(unsigned(delta * 44100*4), sizeof(buf));
			
			int now = int(RageTimer::GetTimeSinceStart() - fake.begin) * 44100;

			int got = s->GetPCM(buf, bytes, now);
			if(got < bytes)
				s->StopPlaying();

			j = next;
		}
	}
}

void RageSoundManager::AddFakeSound(RageSound *snd)
{
	map<RageSound *, FakeSound>::const_iterator fake = 
		fake_sounds.find(const_cast<RageSound *>(snd));

	ASSERT(fake == fake_sounds.end());
	FakeSound newfake;
	newfake.begin = RageTimer::GetTimeSinceStart();
	newfake.samples_read = 0;
	fake_sounds[snd] = newfake;
}

float RageSoundManager::GetPlayLatency() const
{
	return driver->GetPlayLatency();
}

RageSound *RageSoundManager::PlaySound(RageSound &snd)
{
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
	owned_sounds.insert(snd);

	snd->Play();
}

void RageSoundManager::PlayOnceFromDir( CString sDir )
{
	if( sDir == "" )
		return;

	// make sure there's a slash at the end of this path
	sDir.Replace("\\", "/");
	if( sDir[sDir.GetLength()-1] != '/' )
		sDir += "/";

	CStringArray arraySoundFiles;
	GetDirListing( sDir + "*.mp3", arraySoundFiles );
	GetDirListing( sDir + "*.wav", arraySoundFiles );
	GetDirListing( sDir + "*.ogg", arraySoundFiles );

	if( arraySoundFiles.empty() )
		return;

	int index = rand() % arraySoundFiles.size();
	SOUNDMAN->PlayOnce( sDir + arraySoundFiles[index] );
}

/* Standalone helpers: */

/* Mix audio.  This is from SDL, but doesn't depend on the sound being
 * initialized. */
void RageSoundManager::MixAudio(Sint16 *dst, const Sint16 *src, Uint32 len, float volume)
{
	if ( volume == 0 )
		return;

	int factor = int(volume * 256);
	len /= 2;
	while ( len-- ) {
		Sint16 src1 = *src;
		src1 = Sint16((src1*factor)/256);
		Sint16 src2 = *dst;

		int dst_sample = src1+src2;
		dst_sample = clamp(dst_sample, -32768, 32767);
		*dst = Sint16(dst_sample);

		src++;
		dst++;
	}
}

void RageSoundManager::PlayMusic(CString file, bool loop, float start_sec, float length_sec, float fade_len)
{
//	LOG->Trace("play '%s' (current '%s')", file.GetString(), music->GetLoadedFilePath().GetString());
	if(music->IsPlaying())
	{
		if( music->GetLoadedFilePath() == file )
			return;		// do nothing

		music->StopPlaying();
	}

	/* If file is blank, just stop. */
	if(file.empty())
		return;

	music->Load( file, false );

	music->SetStopMode(loop? RageSound::M_LOOP:RageSound::M_STOP);

	if(start_sec == -1)
		music->SetStartSeconds();
	else
		music->SetStartSeconds(start_sec);

	if(length_sec == -1)
		music->SetLengthSeconds();
	else
		music->SetLengthSeconds(length_sec);

	music->SetFadeLength(fade_len);
	music->SetPositionSeconds();
	music->StartPlaying();
}

void RageSoundManager::SetPrefs(float MixVol)
{
	MixVolume = MixVol;
	driver->VolumeChanged();
}

SoundMixBuffer::SoundMixBuffer()
{
	vol = SOUNDMAN->GetMixVolume();
}

void SoundMixBuffer::write(const Sint16 *buf, unsigned size)
{
	if(mixbuf.size() < size)
	{
		basic_string<Sint32, char_traits_Sint32> empty(size-mixbuf.size(), 0);

		mixbuf.insert(mixbuf.end(), empty.begin(), empty.end());
	}

	for(unsigned pos = 0; pos < size; ++pos)
	{
		Sint32 samp = buf[pos] * 32768;

		/* Scale volume: */
		samp = Sint32(samp * vol);

		/* Add and clip.  Can't use clamp() here--we're clamping to
		 * the min and max of Sint32. */
		if(samp > 0 && mixbuf[pos] + samp < mixbuf[pos])
			mixbuf[pos] = INT_MAX;
		else if(samp < 0 && mixbuf[pos] + samp > mixbuf[pos])
			mixbuf[pos] = INT_MIN;
		else
			mixbuf[pos] = mixbuf[pos] + samp;
	}
}

void SoundMixBuffer::read(Sint16 *buf)
{
	for(unsigned pos = 0; pos < mixbuf.size(); ++pos)
	{
		/* XXX: dither */
		Sint32 out = (mixbuf[pos]) / 32768;
		out = clamp(out, -32768, 32767);
		buf[pos] = Sint16(out);
	}

	mixbuf.erase();
}
