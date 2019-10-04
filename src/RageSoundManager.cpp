/*
 * This manager has several distinct purposes:
 *
 * Load the sound driver, and handle most communication between it and RageSound.
 * Factory and reference count RageSoundReader objects for RageSound.
 * User-level:
 *  - global volume management
 *  - sound detaching ("play and delete when done playing")
 */

#include "global.h"
#include "RageSoundManager.h"
#include "RageUtil.h"
#include "RageSound.h"
#include "RageLog.h"
#include "RageTimer.h"
#include "RageSoundReader_Preload.h"
#include "LocalizedString.h"
#include "Preference.h"
#include "RageSoundReader_PostBuffering.h"

#include "arch/Sound/RageSoundDriver.h"

/*
 * The lock ordering requirements are:
 * RageSound::Lock before g_SoundManMutex
 * RageSound::Lock must not be locked when calling driver calls (since the driver
 * may lock a mutex and then make RageSound calls back)
 *
 * Do not make RageSound calls that might lock while holding g_SoundManMutex.
 */

static RageMutex g_SoundManMutex("SoundMan");
static Preference<RString> g_sSoundDrivers( "SoundDrivers", "" ); // "" == DEFAULT_SOUND_DRIVER_LIST

RageSoundManager *SOUNDMAN = nullptr;

RageSoundManager::RageSoundManager(): m_pDriver(nullptr),
	m_fVolumeOfNonCriticalSounds(1.0f) {}

static LocalizedString COULDNT_FIND_SOUND_DRIVER( "RageSoundManager", "Couldn't find a sound driver that works" );
void RageSoundManager::Init()
{
	m_pDriver = RageSoundDriver::Create( g_sSoundDrivers );
	if( m_pDriver == nullptr )
		RageException::Throw( "%s", COULDNT_FIND_SOUND_DRIVER.GetValue().c_str() );
}

RageSoundManager::~RageSoundManager()
{
	/* Don't lock while deleting the driver (the decoder thread might deadlock). */
	delete m_pDriver;
	for (std::pair<RString const &, RageSoundReader_Preload *> s : m_mapPreloadedSounds)
		delete s.second;
	m_mapPreloadedSounds.clear();
}


void RageSoundManager::low_sample_count_workaround()
{
	m_pDriver->low_sample_count_workaround();
}

void RageSoundManager::fix_bogus_sound_driver_pref(RString const& valid_setting)
{
	g_sSoundDrivers.Set(valid_setting);
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
	if( m_pDriver != nullptr )
		m_pDriver->StartMixing( pSound );
}

void RageSoundManager::StopMixing( RageSoundBase *pSound )
{
	if( m_pDriver != nullptr )
		m_pDriver->StopMixing( pSound );
}

bool RageSoundManager::Pause( RageSoundBase *pSound, bool bPause )
{
	if( m_pDriver == nullptr )
		return false;
	else
		return m_pDriver->PauseMixing( pSound, bPause );
}

int64_t RageSoundManager::GetPosition( RageTimer *pTimer ) const
{
	if( m_pDriver == nullptr )
		return 0;
	return m_pDriver->GetHardwareFrame( pTimer );
}

void RageSoundManager::Update()
{
	/* Scan m_mapPreloadedSounds for sounds that are no longer loaded, and delete them. */
	g_SoundManMutex.Lock(); /* lock for access to m_mapPreloadedSounds, owned_sounds */
	{
		map<RString, RageSoundReader_Preload *>::iterator it, next;
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

	g_SoundManMutex.Unlock(); /* finished with m_mapPreloadedSounds */

	if( m_pDriver != nullptr )
		m_pDriver->Update();
}

float RageSoundManager::GetPlayLatency() const
{
	if( m_pDriver == nullptr )
		return 0;

	return m_pDriver->GetPlayLatency();
}

int RageSoundManager::GetDriverSampleRate() const
{
	if( m_pDriver == nullptr )
		return 44100;

	return m_pDriver->GetSampleRate();
}

/* If the given path is loaded, return a copy; otherwise return nullptr.
 * It's the caller's responsibility to delete the result. */
RageSoundReader *RageSoundManager::GetLoadedSound( const RString &sPath_ )
{
	LockMut(g_SoundManMutex); /* lock for access to m_mapPreloadedSounds */

	RString sPath(sPath_);
	sPath.MakeLower();
	map<RString, RageSoundReader_Preload *>::const_iterator it;
	it = m_mapPreloadedSounds.find( sPath );
	if( it == m_mapPreloadedSounds.end() )
		return nullptr;

	return it->second->Copy();
}

/* Add the sound to the set of loaded sounds that can be copied for reuse.
 * The sound will be kept in memory as long as there are any other references
 * to it; once we hold the last one, we'll release it. */
void RageSoundManager::AddLoadedSound( const RString &sPath_, RageSoundReader_Preload *pSound )
{
	LockMut(g_SoundManMutex); /* lock for access to m_mapPreloadedSounds */

	/* Don't AddLoadedSound a sound that's already registered.  It should have been
	 * used in GetLoadedSound. */
	RString sPath(sPath_);
	sPath.MakeLower();
	map<RString, RageSoundReader_Preload *>::const_iterator it;
	it = m_mapPreloadedSounds.find( sPath );
	ASSERT_M( it == m_mapPreloadedSounds.end(), sPath );
	
	m_mapPreloadedSounds[sPath] = pSound->Copy();
}

static Preference<float> g_fSoundVolume( "SoundVolume", 1.0f );

void RageSoundManager::SetMixVolume()
{
	RageSoundReader_PostBuffering::SetMasterVolume( g_fSoundVolume.Get() );
}

void RageSoundManager::SetVolumeOfNonCriticalSounds( float fVolumeOfNonCriticalSounds )
{
	g_SoundManMutex.Lock(); /* lock for access to m_fVolumeOfNonCriticalSounds */
	m_fVolumeOfNonCriticalSounds = fVolumeOfNonCriticalSounds;
	g_SoundManMutex.Unlock(); /* finished with m_fVolumeOfNonCriticalSounds */
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
