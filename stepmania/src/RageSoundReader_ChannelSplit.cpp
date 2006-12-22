/*
 * Output a sound from the channels of another sound.
 *
 * This is intended for splitting a FileReader, and assumes the GetStreamToSourceRatio
 * of the source is 1.0.
 *
 * If multiple readers are created, they should be read in parallel.  If their source
 * position drifts apart, the data between will be buffered to prevent seeking the source.
 *
 * The resulting sounds have as many channels as the largest destination channel
 * specified; multiple sounds on the same channel are mixed together; empty
 * channels are silent.
 *
 * Create two mono sounds from one stereo sound, one playing the left channel and
 * one playing the right:
 *   RageSoundSplitter split(pSource);
 *   RageSoundReader_Split *pLeft = split.CreateSound();
 *   pLeft->AddSourceChannelToSound( 0, 0 );
 *   RageSoundReader_Split *pRight = split.CreateSound();
 *   pRight->AddSourceChannelToSound( 1, 0 );
 *
 * Convert a stereo sound to mono:
 *   RageSoundSplitter split(pSource);
 *   RageSoundReader_Split *pMono = split.CreateSound();
 *   pMono->AddSourceChannelToSound( 0, 0 );
 *   pMono->AddSourceChannelToSound( 1, 0 );
 *
 * Convert a mono sound to stereo (RageSoundReader_Pan will do this faster):
 *   RageSoundSplitter split(pSource);
 *   RageSoundReader_Split *pStereo = split.CreateSound();
 *   pStereo->AddSourceChannelToSound( 0, 0 );
 *   pStereo->AddSourceChannelToSound( 0, 1 );
 */

#include "global.h"
#include "RageSoundReader_ChannelSplit.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageSoundMixBuffer.h"
#include "RageSoundUtil.h"
#include "Foreach.h"
#include <set>

class RageSoundReader_Split;

/* This class is refcounted, freed when all RageSoundReader_Split and RageSoundSplitter
 * classes release it. */
class RageSoundSplitterImpl
{
public:
	RageSoundSplitterImpl( RageSoundReader *pSource )
	{
		m_iRefCount = 1;
		m_pSource = pSource;
		m_iBufferPositionFrames = 0;
	}

	~RageSoundSplitterImpl()
	{
		delete m_pSource;
	}

	static void Release( RageSoundSplitterImpl *pImpl )
	{
		--pImpl->m_iRefCount;
		if( pImpl->m_iRefCount == 0 )
			delete pImpl;
	}

	/* Request that m_sBuffer contain frames [iStartFrame,iStartFrame+iFrames). */
	int ReadBuffer();
	int m_iRefCount;

	RageSoundReader *m_pSource;

	set<RageSoundReader_Split *> m_apSounds;

	/* m_sBuffer[0] corresponds to frame number m_iBufferPositionFrames. */
	int m_iBufferPositionFrames;
	RString m_sBuffer;
};

int RageSoundReader_Split::GetLength() const { return m_pImpl->m_pSource->GetLength(); }
int RageSoundReader_Split::GetLength_Fast() const { return m_pImpl->m_pSource->GetLength_Fast(); }
int RageSoundReader_Split::GetSampleRate() const { return m_pImpl->m_pSource->GetSampleRate(); }
unsigned RageSoundReader_Split::GetNumChannels() const { return m_iNumOutputChannels; }
bool RageSoundReader_Split::IsStreamingFromDisk() const { return m_pImpl->m_pSource->IsStreamingFromDisk(); }
int RageSoundReader_Split::GetNextSourceFrame() const { return m_iPositionFrame; }
float RageSoundReader_Split::GetStreamToSourceRatio() const { return 1.0f; }

RageSoundReader_Split::RageSoundReader_Split( RageSoundSplitterImpl *pImpl )
{
	m_pImpl = pImpl;
	++m_pImpl->m_iRefCount;
	m_pImpl->m_apSounds.insert( this );
	m_iNumOutputChannels = 0;
	m_iPositionFrame = 0;
	m_iRequestFrames = 0;
}

RageSoundReader_Split::RageSoundReader_Split( const RageSoundReader_Split &cpy ):
	RageSoundReader( cpy )
{
	m_pImpl = cpy.m_pImpl;
	++m_pImpl->m_iRefCount;
	m_pImpl->m_apSounds.insert( this );
	m_aChannels = cpy.m_aChannels;
	m_iNumOutputChannels = cpy.m_iNumOutputChannels;
	m_iPositionFrame = cpy.m_iPositionFrame;
	m_iRequestFrames = cpy.m_iRequestFrames;
}

RageSoundReader_Split::~RageSoundReader_Split()
{
	m_pImpl->m_apSounds.erase( this );
	RageSoundSplitterImpl::Release( m_pImpl );
}

int RageSoundReader_Split::SetPosition( int iFrame )
{
	m_iPositionFrame = iFrame;

	/* We can't tell whether we're past EOF.  We can't ReadBuffer here, because the
	 * other sounds using this same source probably havn't seeked yet, so seeking
	 * to the beginning of the file would buffer between there and the position
	 * of those sounds.  Just return success, and we'll return EOF in Read if needed. */
	return 1;
}

bool RageSoundReader_Split::SetProperty( const RString &sProperty, float fValue )
{
	return false;
}

int RageSoundReader_Split::Read( char *pBuf, int iFrames )
{
	m_iRequestFrames = iFrames;
	int iRet = m_pImpl->ReadBuffer();
	if( iRet < 0 )
	{
		this->SetError( m_pImpl->m_pSource->GetError() );
		return iRet;
	}

	int iBytesAvailable = m_pImpl->m_sBuffer.size();
	const char *pSrc = m_pImpl->m_sBuffer.data();
	if( m_pImpl->m_iBufferPositionFrames < m_iPositionFrame )
	{
		int iSkipFrames = m_iPositionFrame - m_pImpl->m_iBufferPositionFrames;
		int iSkipBytes = iSkipFrames * sizeof(int16_t) * m_pImpl->m_pSource->GetNumChannels();
		pSrc += iSkipBytes;
		iBytesAvailable -= iSkipBytes;
	}

	int iFramesWanted = iFrames;
	int iFramesAvailable = iBytesAvailable / (sizeof(int16_t) * m_pImpl->m_pSource->GetNumChannels());
	iFramesAvailable = min( iFramesAvailable, iFramesWanted );

	{
		const int16_t *pSrcSamples = (const int16_t *) pSrc;
		RageSoundMixBuffer mix;
		for( int i = 0; i < (int) m_aChannels.size(); ++i )
		{
			const ChannelMap &chan = m_aChannels[i];
			mix.SetWriteOffset( chan.m_iToChannel );
			mix.write( pSrcSamples + chan.m_iFromChannel, iFramesAvailable, m_pImpl->m_pSource->GetNumChannels(), m_iNumOutputChannels );
		}

		mix.read( (int16_t *) pBuf );
	}

	m_iPositionFrame += iFramesAvailable;

	/* We no longer need the data we requested.  Clear our request, so the
	 * memory can be freed. */
	m_iRequestFrames = 0;
	m_pImpl->ReadBuffer();
	return iFramesAvailable;
}

int RageSoundSplitterImpl::ReadBuffer()
{
	/* Discard any bytes that are no longer requested by any sound. */
	int iMinFrameRequested = INT_MAX;
	int iMaxFrameRequested = 0;
	FOREACHS( RageSoundReader_Split *, m_apSounds, snd )
	{
		iMinFrameRequested = min( iMinFrameRequested, (*snd)->m_iPositionFrame );
		iMaxFrameRequested = max( iMaxFrameRequested, (*snd)->m_iPositionFrame + (*snd)->m_iRequestFrames );
	}

	if( iMinFrameRequested > m_iBufferPositionFrames )
	{
		int iEraseFrames = iMinFrameRequested - m_iBufferPositionFrames;
		iEraseFrames = min( iEraseFrames, (int) m_sBuffer.size() );
		m_sBuffer.erase( 0, iEraseFrames * sizeof(int16_t) * m_pSource->GetNumChannels() );
		m_iBufferPositionFrames += iEraseFrames;
	}

	if( iMinFrameRequested != m_iBufferPositionFrames )
	{
		m_pSource->SetPosition( iMinFrameRequested );
		m_iBufferPositionFrames = iMinFrameRequested;
		m_sBuffer.clear();
	}

	int iFramesBuffered = m_sBuffer.size() / (sizeof(int16_t) * m_pSource->GetNumChannels() );

	int iFramesToRead = iMaxFrameRequested - (m_iBufferPositionFrames + iFramesBuffered);
	if( iFramesToRead <= 0 )
		return 1; // requested data already buffered

	int iBytesToRead = iFramesToRead * sizeof(int16_t) * m_pSource->GetNumChannels();
	int iOldSizeBytes = m_sBuffer.size();
	m_sBuffer.resize( iOldSizeBytes + iBytesToRead );
	int iGotFrames = m_pSource->Read( &m_sBuffer[0] + iOldSizeBytes, iFramesToRead );
	if( iGotFrames < 0 )
	{
		m_sBuffer.resize( iOldSizeBytes );
		return iGotFrames;
	}

	int iGotBytes = iGotFrames * sizeof(int16_t) * m_pSource->GetNumChannels();
	m_sBuffer.resize( iOldSizeBytes + iGotBytes );
	return 1;
}

void RageSoundReader_Split::AddSourceChannelToSound( int iFromChannel, int iToChannel )
{
	m_aChannels.push_back( ChannelMap(iFromChannel, iToChannel) );
	m_iNumOutputChannels = max( m_iNumOutputChannels, iToChannel + 1 );
}

RageSoundSplitter::RageSoundSplitter( RageSoundReader *pSource )
{
	m_pImpl = new RageSoundSplitterImpl( pSource );
}

RageSoundSplitter::~RageSoundSplitter()
{
	RageSoundSplitterImpl::Release( m_pImpl );
}

RageSoundReader_Split *RageSoundSplitter::CreateSound()
{
	return new RageSoundReader_Split( m_pImpl );
}

/*
 * Copyright (c) 2006 Glenn Maynard
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
