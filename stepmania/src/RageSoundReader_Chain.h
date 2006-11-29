/* RageSoundReader_Chain - Chain sounds together */

#ifndef RAGE_SOUND_READER_CHAIN
#define RAGE_SOUND_READER_CHAIN

#include "RageSoundReader.h"

#include <map>
#include <set>

class RageSoundReader_Chain: public SoundReader
{
public:
	RageSoundReader_Chain();
	~RageSoundReader_Chain();
	RageSoundReader_Chain *Copy() const;

	/* Set the preferred sample rate.  This will only be used if the source sounds
	 * use different sample rates. */
	void SetPreferredSampleRate( int iSampleRate ) { m_iPreferredSampleRate = iSampleRate; }

	/* Add the given sound to play after fOffsetSecs seconds.  Takes ownership
	 * of pSound. */
	bool AddSound( RString sPath, float fOffsetSecs, float fPan );

	/* Finish adding sounds. */
	void Finish();

	int GetLength() const;
	int GetLength_Fast() const;
	int SetPosition_Accurate( int ms );
	/* It rarely makes sense to set an approximate time for a chained sound, since any
	 * error in overlap will become obvious. */
	int SetPosition_Fast( int ms ) { return SetPosition_Accurate( ms ); }
	int Read( char *buf, unsigned len );
	int GetSampleRate() const { return m_iActualSampleRate; }
	unsigned GetNumChannels() const { return m_iChannels; }
	bool IsStreamingFromDisk() const;
	int GetNextStreamFrame() const;
	float GetStreamToSourceRatio() const;

private:
	int ReadBlock( int16_t *pBuffer, int iFrames );
	int GetSampleRateInternal() const;

	int m_iPreferredSampleRate;
	int m_iActualSampleRate;
	unsigned m_iChannels;

	map<RString, SoundReader *> m_apLoadedSounds;

	struct sound
	{
		RString sPath;
		int iOffsetMS;
		float fPan;
		int GetOffsetFrame( int iSampleRate ) const { return int( int64_t(iOffsetMS) * iSampleRate / 1000 ); }
		bool operator<( const sound &rhs ) const { return iOffsetMS < rhs.iOffsetMS; }
	};
	vector<sound> m_Sounds;
	unsigned GetNextSoundIndex() const;

	/* Read state: */
	int m_iCurrentFrame;
	unsigned m_iNextSound;
	struct ActiveSound
	{
		SoundReader *pSound;
		float fPan;
		bool operator< ( const ActiveSound &rhs ) const { return pSound < rhs.pSound; }
	};
	vector<ActiveSound> m_apActiveSounds;
	unsigned ActivateSound( const sound &s );
	void ReleaseSound( unsigned n );
};

#endif

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
