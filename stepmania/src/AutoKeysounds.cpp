#include "global.h"
#include "AutoKeysounds.h"
#include "GameState.h"
#include "Song.h"

void AutoKeysounds::Load( PlayerNumber pn, const NoteData& ndAutoKeysoundsOnly )
{
	m_ndAutoKeysoundsOnly[pn] = ndAutoKeysoundsOnly;
	
	//
	// Load sounds.
	//
	Song* pSong = GAMESTATE->m_pCurSong;
	CString sSongDir = pSong->GetSongDir();
	m_vKeysounds.clear();
	m_vKeysounds.resize( pSong->m_vsKeysoundFile.size() );
	for( unsigned i=0; i<m_vKeysounds.size(); i++ )
	{
		 CString sKeysoundFilePath = sSongDir + pSong->m_vsKeysoundFile[i];
		 RageSound& sound = m_vKeysounds[i];
		 sound.Load( sKeysoundFilePath );
	}
}

void AutoKeysounds::Update( float fDelta )
{
	//
	// Play keysounds for crossed rows.
	//
	bool bCrossedABeat = false;
	{
		float fPositionSeconds = GAMESTATE->m_fMusicSeconds;
		float fSongBeat = GAMESTATE->m_pCurSong->GetBeatFromElapsedTime( fPositionSeconds );

		int iRowNow = BeatToNoteRowNotRounded( fSongBeat );
		iRowNow = max( 0, iRowNow );
		static int iRowLastCrossed = 0;

		float fBeatLast = roundf(NoteRowToBeat(iRowLastCrossed));
		float fBeatNow = roundf(NoteRowToBeat(iRowNow));

		bCrossedABeat = fBeatLast != fBeatNow;

		FOREACH_EnabledPlayer( pn )
		{
			const NoteData &nd = m_ndAutoKeysoundsOnly[pn];
		
			for( int t=0; t<nd.GetNumTracks(); t++ )
			{
				FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( nd, t, r, iRowLastCrossed+1, iRowNow )
				{
					const TapNote &tn = nd.GetTapNote( t, r );
					ASSERT( tn.type == TapNote::autoKeysound );
					if( tn.bKeysound )
						m_vKeysounds[tn.iKeysoundIndex].Play();
				}		
			}
		}

		iRowLastCrossed = iRowNow;
	}
}

/*
 * (c) 2003 Chris Danford
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
