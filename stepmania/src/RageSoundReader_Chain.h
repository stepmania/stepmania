/* RageSoundReader_Chain - Chain sounds together */

#ifndef RAGE_SOUND_READER_CHAIN
#define RAGE_SOUND_READER_CHAIN

#include "RageSoundReader.h"

#include <map>

class RageSoundReader_Chain: public RageSoundReader
{
public:
	RageSoundReader_Chain();
	~RageSoundReader_Chain();
	RageSoundReader_Chain *Copy() const;

	/* Set the preferred sample rate.  This will only be used if the source sounds
	 * use different sample rates. */
	void SetPreferredSampleRate( int iSampleRate ) { m_iPreferredSampleRate = iSampleRate; }

	int LoadSound( RString sPath );
	int LoadSound( RageSoundReader *pSound );

	/* Add the given sound to play after fOffsetSecs seconds.  Takes ownership
	 * of pSound. */
	void AddSound( int iIndex, float fOffsetSecs, float fPan );

	/* Finish adding sounds. */
	void Finish();

	/* Return the number of added sounds. */
	int GetNumSounds() const { return m_aSounds.size(); }
	
	int GetLength() const;
	int GetLength_Fast() const;
	int SetPosition( int iFrame );
	int Read( char *pBuf, int iFrames );
	int GetSampleRate() const { return m_iActualSampleRate; }
	unsigned GetNumChannels() const { return m_iChannels; }
	bool IsStreamingFromDisk() const;
	bool SetProperty( const RString &sProperty, float fValue );
	int GetNextSourceFrame() const;
	float GetStreamToSourceRatio() const;
	RString GetError() const { return ""; }

private:
	int GetSampleRateInternal() const;

	int m_iPreferredSampleRate;
	int m_iActualSampleRate;
	unsigned m_iChannels;

	map<RString, RageSoundReader *> m_apNamedSounds;
	vector<RageSoundReader *> m_apLoadedSounds;

	struct Sound
	{
		int iIndex; // into m_apLoadedSounds
		int iOffsetMS;
		float fPan;
		RageSoundReader *pSound; // NULL if not activated

		int GetOffsetFrame( int iSampleRate ) const { return int( int64_t(iOffsetMS) * iSampleRate / 1000 ); }
		bool operator<( const Sound &rhs ) const { return iOffsetMS < rhs.iOffsetMS; }
	};
	vector<Sound> m_aSounds;

	/* Read state: */
	int m_iCurrentFrame;
	unsigned m_iNextSound;
	vector<Sound *> m_apActiveSounds;

	void ActivateSound( Sound *s );
	void ReleaseSound( Sound *s );
};

#endif

/*
 * Copyright (c) 2004-2006 Glenn Maynard
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
