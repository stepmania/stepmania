#include "global.h"
#include "RageSoundReader_Chain.h"
#include "RageSoundReader_FileReader.h"
#include "RageSoundReader_Resample.h"
#include "RageSoundReader_Preload.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageSoundMixBuffer.h"
#include "RageSoundUtil.h"

/*
 * Keyed sounds should pass this object to SoundReader_Preload, to preprocess it.
 * Streaming more than two or three sounds is too expensive (keyed games can play
 * two dozen), and reading from disk is too latent.
 *
 * This can also be used for chained background music, which should always stream,
 * so we don't do the preloading in here.
 */
RageSoundReader_Chain::RageSoundReader_Chain()
{
	m_iPreferredSampleRate = 44100;
	m_iActualSampleRate = -1;
	m_iChannels = 0;
	m_iCurrentFrame = 0;
}

RageSoundReader_Chain::~RageSoundReader_Chain()
{
	/* Clear m_apActiveSounds. */
	while( !m_apActiveSounds.empty() )
		ReleaseSound( 0 );

	map<CString, SoundReader *>::iterator it;
	for( it = m_apLoadedSounds.begin(); it != m_apLoadedSounds.end(); ++it )
		delete it->second;
}

SoundReader *RageSoundReader_Chain::Copy() const
{
	// XXX
	FAIL_M("unimplemented");
}

/* The same sound may be used several times, and by several different chains.  Avoid
 * loading the same sound multiple times.  We need to make a Copy() if we need to
 * read it more than once at a time. */
bool RageSoundReader_Chain::AddSound( CString sPath, float fOffsetSecs, float fPan )
{
	sPath.ToLower();

	map<CString, SoundReader *>::const_iterator it;
	it = m_apLoadedSounds.find( sPath );
	if( it == m_apLoadedSounds.end() )
	{
		CString error;
		SoundReader *pReader = SoundReader_FileReader::OpenFile( sPath, error );
		if( pReader == NULL )
		{
			LOG->Warn( "RageSoundReader_Chain: error opening sound \"%s\": %s",
				sPath.c_str(), error.c_str() );
			return false;
		}

		m_apLoadedSounds[sPath] = pReader;
	}

	sound s;
	s.sPath = sPath;
	s.iOffsetMS = lrintf( fOffsetSecs * 1000 );
	s.fPan = fPan;
	m_Sounds.push_back( s );

	return true;
}

/* If every sound has the same sample rate, return it.  Otherwise, return -1. */
int RageSoundReader_Chain::GetSampleRateInternal() const
{
	map<CString, SoundReader *>::const_iterator it;
	int iRate = -1;
	for( it = m_apLoadedSounds.begin(); it != m_apLoadedSounds.end(); ++it )
	{
		if( iRate == -1 )
			iRate = it->second->GetSampleRate();
		else if( iRate != it->second->GetSampleRate() )
			return -1;
	}
	return iRate;
}

void RageSoundReader_Chain::Finish()
{
	/* Remove any sounds that don't have corresponding SoundReaders. */
	for( unsigned i = 0; i < m_Sounds.size(); )
	{
		sound &sound = m_Sounds[i];

		map<CString, SoundReader *>::iterator it = m_apLoadedSounds.find( sound.sPath );
		if( it == m_apLoadedSounds.end() )
		{
			m_Sounds.erase( m_Sounds.begin()+i );
			continue;
		}

		++i;
	}

	/* Figure out how many channels we have. */
	m_iChannels = 1;
	map<CString, SoundReader *>::iterator it;
	for( it = m_apLoadedSounds.begin(); it != m_apLoadedSounds.end(); ++it )
		m_iChannels = max( m_iChannels, it->second->GetNumChannels() );

	/* If any sounds have a non-0 pan, we're stereo. */
	for( unsigned i = 0; i < m_Sounds.size(); ++i )
		if( fabs(m_Sounds[i].fPan) > 0.0001f )
			m_iChannels = 2;
	
	/*
	 * We might get different sample rates fro mour sources.  If they're all the same
	 * sample rate, just leave it alone, so the whole sound can be resampled as a group.
	 * If not, resample eveything to the preferred rate.  (Using the preferred rate
	 * should avoid redundant resampling later.)
	 */
	m_iActualSampleRate = GetSampleRateInternal();
	if( m_iActualSampleRate == -1 )
	{
		for( it = m_apLoadedSounds.begin(); it != m_apLoadedSounds.end(); ++it )
		{
			SoundReader *&pSound = it->second;

			/* We're preprocessing this; let's just use high quality resampling. */
			RageSoundReader_Resample *pResample = RageSoundReader_Resample::MakeResampler( RageSoundReader_Resample::RESAMP_HIGHQUALITY );
			pResample->Open( pSound );
			pResample->SetSampleRate( m_iPreferredSampleRate );
			pSound = pResample;
		}

		m_iActualSampleRate = m_iPreferredSampleRate;
	}

	/* Attempt to preload all sounds. */
	for( it = m_apLoadedSounds.begin(); it != m_apLoadedSounds.end(); ++it )
	{
		SoundReader *&pSound = it->second;
		RageSoundReader_Preload::PreloadSound( pSound );
	}

	for( it = m_apLoadedSounds.begin(); it != m_apLoadedSounds.end(); ++it )
	{
		SoundReader *pSound = it->second;
	    m_LoadedSoundIsUsed[pSound] = false;
	}

	/* Sort the sounds by start time. */
	sort( m_Sounds.begin(), m_Sounds.end() );

}

int RageSoundReader_Chain::SetPosition_Accurate( int ms )
{
	/* Clear m_apActiveSounds. */
	while( !m_apActiveSounds.empty() )
		ReleaseSound( 0 );

	m_iCurrentFrame = int( int64_t(ms) * m_iActualSampleRate / 1000 );

	/* Run through all sounds in the chain, and activate all sounds which have data
	 * at ms. */
	for( unsigned i = 0; i < m_Sounds.size(); ++i )
	{
		sound &sound = m_Sounds[i];

		/* If this sound is in the future, skip it. */
		if( sound.iOffsetMS > ms )
			continue;

		/* Find the SoundReader. */
		int n = ActivateSound( sound );
		SoundReader *pSound = m_apActiveSounds[n].pSound;

		int iOffsetMS = ms - sound.iOffsetMS;
		if( pSound->SetPosition_Accurate(iOffsetMS) == 0 )
		{
			/* We're past the end of this sound. */
			ReleaseSound( n );
			continue;
		}
	}

	/* If no sounds were started, and we have no sounds ahead of us, we've seeked
	 * past EOF. */
	if( m_apActiveSounds.empty() && GetNextSoundIndex() == m_Sounds.size() )
		return 0;

	return ms;
}

unsigned RageSoundReader_Chain::ActivateSound( const sound &s )
{
	SoundReader *pSound = m_apLoadedSounds[s.sPath];

	map<SoundReader *, bool>::iterator it;
	it = m_LoadedSoundIsUsed.find( pSound );
	ASSERT( it != m_LoadedSoundIsUsed.end() );

	ActiveSound add;
	add.fPan = s.fPan;

	if( !it->second )
	{
		it->second = true;
		add.pSound = pSound;

		/* The sound may have been used in the past; rewind it.
		 * We do this when starting a sound, not when finishing it,
		 * so we don't' waste time rewinding a sound that we won't
		 * be reusing. */
		pSound->SetPosition_Accurate( 0 );
	}
	else
	{
		add.pSound = pSound->Copy();
	}

	m_apActiveSounds.push_back( add );
	return m_apActiveSounds.size() - 1;
}

void RageSoundReader_Chain::ReleaseSound( unsigned n )
{
	ASSERT_M( n < m_apActiveSounds.size(), ssprintf("%u, %u", n, unsigned(m_apActiveSounds.size())) );
	SoundReader *pSound = m_apActiveSounds[n].pSound;

	/* If pSoundToFree is in m_apLoadedSounds, just set it unused. */
	map<SoundReader *, bool>::iterator it;
	it = m_LoadedSoundIsUsed.find( pSound );
	if( it != m_LoadedSoundIsUsed.end() )
	{
		ASSERT( it->second );
		it->second = false;
	}
	else
		delete pSound;

	m_apActiveSounds.erase( m_apActiveSounds.begin()+n );
}

bool RageSoundReader_Chain::IsStreamingFromDisk() const
{
	map<CString, SoundReader *>::const_iterator it;
	for( it = m_apLoadedSounds.begin(); it != m_apLoadedSounds.end(); ++it )
		if( it->second->IsStreamingFromDisk() )
			return true;

	return false;
}

/* Find the next sound we'll need to start, if any.  m_Sounds is sorted by time. */
unsigned RageSoundReader_Chain::GetNextSoundIndex() const
{
	unsigned iNextSound = 0;
	while( iNextSound < m_Sounds.size() && m_iCurrentFrame > m_Sounds[iNextSound].GetOffsetFrame(m_iActualSampleRate) )
		++iNextSound;
	return iNextSound;
}

int RageSoundReader_Chain::Read( char *pBuffer, unsigned iLength )
{
	RageSoundMixBuffer mix;

	unsigned iNextSound = GetNextSoundIndex();

	int iNumFramesToRead = iLength / (sizeof(int16_t) * m_iChannels);

	int iFramesRead = 0;
	while( iFramesRead < iNumFramesToRead )
	{
		/* How many samples should we read before we need to start up a sound? */
		int iFramesToRead = 999999999; /* inf */
		if( iNextSound < m_Sounds.size() )
		{
			int iStartFrame = m_iCurrentFrame;
			int iOffsetFrame = m_Sounds[iNextSound].GetOffsetFrame(m_iActualSampleRate);
			ASSERT_M( iOffsetFrame >= iStartFrame, ssprintf("%i %i", iOffsetFrame, iStartFrame) );
			iFramesToRead = iOffsetFrame - iStartFrame;
		}
		iFramesToRead = min( iFramesToRead, iNumFramesToRead-iFramesRead );
		iFramesToRead = min( iFramesToRead, 1024 );

		mix.SetWriteOffset( iFramesRead * m_iChannels );

		/* While we need more data, and have a source for it, read. */
		int iMaxFramesRead = 0;
		if( iFramesToRead > 0 && !m_apActiveSounds.empty() )
		{
			/* Read iFramesToRead from each sound. */
			int16_t Buffer[2048];
			for( unsigned i = 0; i < m_apActiveSounds.size(); )
			{
				ActiveSound &s = m_apActiveSounds[i];
				SoundReader *pSound = s.pSound;
				int iSamples = min( iFramesToRead * pSound->GetNumChannels(), ARRAYSIZE(Buffer) );
				int iBytesRead = pSound->Read( (char *) Buffer, iSamples*sizeof(int16_t) );
				if( iBytesRead == -1 || iBytesRead == 0 )
				{
					/* The sound is at EOF.  Release it. */
					ReleaseSound( i );
					continue;
				}

				int iSamplesRead = iBytesRead / sizeof(int16_t);
				int iFramesRead = iSamplesRead / pSound->GetNumChannels();

				iMaxFramesRead = max( iMaxFramesRead, iFramesRead );

				if( m_iChannels == 2 && pSound->GetNumChannels() == 1 )
				{
					RageSoundUtil::ConvertMonoToStereoInPlace( Buffer, iSamplesRead );
					iSamplesRead *= 2;
				}

				if( fabs(s.fPan) > 0.0001f )
					RageSoundUtil::Pan( Buffer, iSamplesRead, s.fPan );

				mix.write( Buffer, iSamplesRead );
				++i;
			}
		}

		/* If we have more sounds ahead of us, pretend we read the entire block, since
		 * there's silence in between.  Otherwise, we're at EOF. */
		if( iNextSound < m_Sounds.size() )
		{
			iMaxFramesRead = iFramesToRead;
			mix.Extend( iFramesToRead*m_iChannels );
		}

		m_iCurrentFrame += iMaxFramesRead;
		iFramesRead += iMaxFramesRead;

		while( iNextSound < m_Sounds.size() && m_iCurrentFrame == m_Sounds[iNextSound].GetOffsetFrame(m_iActualSampleRate) )
		{
			sound &sound = m_Sounds[iNextSound];
			ActivateSound( sound );
			++iNextSound;
		}

		/* If we have no sources, and no more sounds to play, EOF. */
		if( m_apActiveSounds.empty() && iNextSound == m_Sounds.size() )
			break;
	}

	/* Read mixed frames into the output buffer. */
	ASSERT_M( mix.size() <= iFramesRead * m_iChannels, ssprintf("%i, %i", mix.size(), iFramesRead * m_iChannels) );
	mix.read( (int16_t *) pBuffer );

	return iFramesRead * sizeof(int16_t) * m_iChannels;
}

int RageSoundReader_Chain::GetLength() const
{
	int iLength = 0;
	for( unsigned i = 0; i < m_Sounds.size(); ++i )
	{
		const sound &sound = m_Sounds[i];
		const SoundReader *pSound = m_apLoadedSounds.find( sound.sPath )->second;
		int iThisLength = pSound->GetLength();
		if( iThisLength )
			iLength = max( iLength, iThisLength + sound.iOffsetMS );
	}
	return iLength;
}

int RageSoundReader_Chain::GetLength_Fast() const
{
	int iLength = 0;
	for( unsigned i = 0; i < m_Sounds.size(); ++i )
	{
		const sound &sound = m_Sounds[i];
		const SoundReader *pSound = m_apLoadedSounds.find( sound.sPath )->second;
		int iThisLength = pSound->GetLength_Fast();
		if( iThisLength )
			iLength = max( iLength, iThisLength + sound.iOffsetMS );
	}
	return iLength;
}


/*
 * Copyright (c) 2004 Glenn Maynard
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
