/*
 * This class handles two things: auto-play preload, and runtime auto-play sounds.
 *
 * On song start, all autoplay sounds and the main BGM track (if any) are combined
 * into a single chain, which is used as the song background.  Sounds added this way
 * are removed from the NoteData.
 *
 * Any sounds not added to the sound chain and any autoplay sounds added to the NoteData
 * during play (usually due to battle mode mods) are played dynamically, via Update().
 *
 * Note that autoplay sounds which are played before the BGM starts will never be
 * placed in the sound chain, since the sound chain becomes the BGM; the BGM can't
 * play sound before it starts.  These sounds will be left in the NoteData, and played
 * as dynamic sounds; this means that they don't get robust sync.  This isn't a problem
 * for imported BMS files, which don't have an offset value, but it's annoying.
 */

#include "global.h"
#include "AutoKeysounds.h"
#include "GameState.h"
#include "song.h"
#include "RageSoundReader_Chain.h"
#include "RageSoundReader_PitchChange.h"
#include "RageSoundManager.h"
#include "RageLog.h"

void AutoKeysounds::Load( PlayerNumber pn, const NoteData& ndAutoKeysoundsOnly )
{
	m_ndAutoKeysoundsOnly[pn] = ndAutoKeysoundsOnly;
}

void AutoKeysounds::LoadAutoplaySoundsInto( RageSoundReader_Chain *pChain )
{
	//
	// Load sounds.
	//
	Song* pSong = GAMESTATE->m_pCurSong;
	RString sSongDir = pSong->GetSongDir();
/*
	m_vKeysounds.clear();
	m_vKeysounds.resize( pSong->m_vsKeysoundFile.size() );
	for( unsigned i=0; i<m_vKeysounds.size(); i++ )
	{
		 RString sKeysoundFilePath = sSongDir + pSong->m_vsKeysoundFile[i];
		 RageSound& sound = m_vKeysounds[i];
		 sound.Load( sKeysoundFilePath );


	}
*/

	/*
	 * Add all current autoplay sounds in both players to the chain.  If a sound is
	 * common to both players, don't pan it; otherwise pan it to that player's side.
	 */
	int iNumTracks = m_ndAutoKeysoundsOnly[GAMESTATE->m_MasterPlayerNumber].GetNumTracks();
	for( int t = 0; t < iNumTracks; t++ )
	{
		int iRow = -1;
		while(1)
		{
			/* Find the next row that either player has a note on. */
			int iNextRow = INT_MAX;
			FOREACH_EnabledPlayer(pn)
			{
				// XXX Hack. Enabled players need not have their own note data.
				if( t >= m_ndAutoKeysoundsOnly[pn].GetNumTracks() )
					continue;
				int iNextRowForPlayer = iRow;
				if( m_ndAutoKeysoundsOnly[pn].GetNextTapNoteRowForTrack( t, iNextRowForPlayer ) )
					iNextRow = min( iNextRow, iNextRowForPlayer );
			}

			if( iNextRow == INT_MAX )
				break;
			iRow = iNextRow;

			TapNote tn[NUM_PLAYERS];
			FOREACH_EnabledPlayer(pn)
				tn[pn] = m_ndAutoKeysoundsOnly[pn].GetTapNote( t, iRow );

			/* Do all enabled players have the same note here?  (Having no note at all
			 * counts as having a different note.) */
			bool bSoundIsGlobal = true;
			{
				PlayerNumber pn = GetNextEnabledPlayer((PlayerNumber)-1);
				const TapNote &t = tn[pn];
				pn = GetNextEnabledPlayer(pn);
				while( pn != PLAYER_INVALID )
				{
					if( tn[pn].type != TapNote::autoKeysound || tn[pn].iKeysoundIndex != t.iKeysoundIndex )
						bSoundIsGlobal = false;
					pn = GetNextEnabledPlayer(pn);
				}
			}

			FOREACH_EnabledPlayer(pn)
			{
				if( tn[pn] == TAP_EMPTY )
					continue;

				ASSERT( tn[pn].type == TapNote::autoKeysound );
				if( tn[pn].iKeysoundIndex >= 0 )
				{
					RString sKeysoundFilePath = sSongDir + pSong->m_vsKeysoundFile[tn[pn].iKeysoundIndex];
					float fSeconds = pSong->m_Timing.GetElapsedTimeFromBeat( NoteRowToBeat(iRow) );

					float fPan = 0;
					if( !bSoundIsGlobal )
						fPan = (pn == PLAYER_1)? -1.0f:+1.0f;
					int iIndex = pChain->LoadSound( sKeysoundFilePath );
					pChain->AddSound( iIndex, fSeconds, fPan );
				}
			}
		}		
	}
}

void AutoKeysounds::FinishLoading()
{
	m_sSound.Unload();

	/* Load the BGM. */
	RageSoundReader_Chain *pChain = new RageSoundReader_Chain;

	Song* pSong = GAMESTATE->m_pCurSong;
	pChain->SetPreferredSampleRate( SOUNDMAN->GetDriverSampleRate(44100) );
	pChain->AddSound( pChain->LoadSound(pSong->GetMusicPath()), 0, 0 );

	LoadAutoplaySoundsInto( pChain );
	
	pChain->Finish();

	RageSoundReader *pReader = pChain;

	/* Load a pitch shifter for the whole sound. */
	pReader = new RageSoundReader_PitchChange( pReader );

	m_sSound.LoadSoundReader( pReader );
}

void AutoKeysounds::Update( float fDelta )
{
	//
	// Play keysounds for crossed rows.
	//
/*
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
*/
}

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
