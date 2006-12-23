/* RageSoundReader_ChannelSplit - Split a sound channels into separate sounds. */

#ifndef RAGE_SOUND_READER_CHANNEL_SPLIT
#define RAGE_SOUND_READER_CHANNEL_SPLIT

#include "RageSoundReader.h"

class RageSoundSplitterImpl;

class RageSoundReader_Split: public RageSoundReader
{
public:
	RageSoundReader_Split( const RageSoundReader_Split &cpy );
	~RageSoundReader_Split();
	virtual RageSoundReader_Split *Copy() const { return new RageSoundReader_Split(*this); }

	virtual int GetLength() const;
	virtual int GetLength_Fast() const;
	virtual int SetPosition( int iFrame );
	virtual int Read( char *pBuf, int iFrames );
	virtual int GetSampleRate() const;
	virtual unsigned GetNumChannels() const;
	virtual bool SetProperty( const RString &sProperty, float fValue );
	virtual int GetNextSourceFrame() const;
	virtual float GetStreamToSourceRatio() const;

	void AddSourceChannelToSound( int iFromChannel, int iToChannel );

private:
	RageSoundReader_Split( RageSoundSplitterImpl *pImpl ); // create with RageSoundSplitter
	friend class RageSoundSplitterImpl;
	friend class RageSoundSplitter;

	RageSoundSplitterImpl *m_pImpl;
	struct ChannelMap
	{
		int m_iFromChannel;
		int m_iToChannel;
		ChannelMap( int iFromChannel, int iToChannel ) { m_iFromChannel = iFromChannel; m_iToChannel = iToChannel; }
	};
	vector<ChannelMap> m_aChannels;

	int m_iPositionFrame;
	int m_iRequestFrames;
	int m_iNumOutputChannels;
};

class RageSoundSplitter
{
public:
	RageSoundSplitter( RageSoundReader *pSource );
	~RageSoundSplitter();
	RageSoundReader_Split *CreateSound();

private:
	RageSoundSplitterImpl *m_pImpl;
};

#endif

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
