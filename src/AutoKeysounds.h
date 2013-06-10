/* AutoKeysounds - handle playback of auto keysounds notes. */

#ifndef AUTO_KEYSOUNDS_H
#define AUTO_KEYSOUNDS_H

#include "NoteData.h"
#include "PlayerNumber.h"
#include "RageSound.h"

class RageSoundReader;
class RageSoundReader_Chain;
class Song;
class AutoKeysounds
{
public:
	void Load( PlayerNumber pn, const NoteData& ndAutoKeysoundsOnly );
	void Update( float fDelta );
	void FinishLoading();
	RageSound *GetSound() { return &m_sSound; }
	RageSoundReader *GetSharedSound() { return m_pSharedSound; }
	RageSoundReader *GetPlayerSound( PlayerNumber pn ) { if( pn == PLAYER_INVALID ) return NULL; return m_pPlayerSounds[pn]; }

protected:	
	void LoadAutoplaySoundsInto( RageSoundReader_Chain *pChain );
	static void LoadTracks( const Song *pSong, RageSoundReader *&pGlobal, RageSoundReader *&pPlayer1, RageSoundReader *&pPlayer2 );

	NoteData		m_ndAutoKeysoundsOnly[NUM_PLAYERS];
	vector<RageSound>	m_vKeysounds;
	RageSound		m_sSound;
	RageSoundReader		*m_pChain; // owned by m_sSound
	RageSoundReader		*m_pPlayerSounds[NUM_PLAYERS]; // owned by m_sSound
	RageSoundReader		*m_pSharedSound; // owned by m_sSound
};

#endif

/*
 * (c) 2004 Chris Danford, Glenn Maynard
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
