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
#include "Song.h"
#include "RageSoundReader_Chain.h"
#include "RageSoundReader_ChannelSplit.h"
#include "RageSoundReader_Extend.h"
#include "RageSoundReader_Merge.h"
#include "RageSoundReader_Pan.h"
#include "RageSoundReader_PitchChange.h"
#include "RageSoundReader_PostBuffering.h"
#include "RageSoundReader_ThreadedBuffer.h"
#include "RageSoundManager.h"
#include "RageLog.h"
#include "RageSoundReader_FileReader.h"

#include <vector>


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
	 * Add all current autoplay sounds in both players to the chain.
	 */
	int iNumTracks = m_ndAutoKeysoundsOnly[GAMESTATE->GetMasterPlayerNumber()].GetNumTracks();
	for( int t = 0; t < iNumTracks; t++ )
	{
		int iRow = -1;
		for(;;)
		{
			/* Find the next row that either player has a note on. */
			int iNextRow = INT_MAX;
			FOREACH_EnabledPlayer(pn)
			{
				// XXX Hack. Enabled players need not have their own note data.
				if( t >= m_ndAutoKeysoundsOnly[pn].GetNumTracks() )
					continue;
				int iNextRowForPlayer = iRow;
				/* XXX: If a BMS file only has one tap note per track,
				 * this will prevent any keysounds from loading.
				 * This leads to failure later on.
				 * We need a better way to prevent this. */
				if( m_ndAutoKeysoundsOnly[pn].GetNextTapNoteRowForTrack( t, iNextRowForPlayer ) )
					iNextRow = std::min( iNextRow, iNextRowForPlayer );
			}

			if( iNextRow == INT_MAX )
				break;
			iRow = iNextRow;

			TapNote tn[NUM_PLAYERS];
			FOREACH_EnabledPlayer(pn)
				tn[pn] = m_ndAutoKeysoundsOnly[pn].GetTapNote( t, iRow );

			FOREACH_EnabledPlayer(pn)
			{
				if( tn[pn] == TAP_EMPTY )
					continue;

				ASSERT( tn[pn].type == TapNoteType_AutoKeysound );
				if( tn[pn].iKeysoundIndex >= 0 )
				{
					RString sKeysoundFilePath = sSongDir + pSong->m_vsKeysoundFile[tn[pn].iKeysoundIndex];
					float fSeconds = GAMESTATE->m_pCurSteps[pn]->GetTimingData()->GetElapsedTimeFromBeatNoOffset( NoteRowToBeat(iRow) ) + SOUNDMAN->GetPlayLatency();

					float fPan = 0;
					// If two players are playing, pan the keysounds to each player's respective side
					if( GAMESTATE->GetNumPlayersEnabled() == 2 )
						fPan = (pn == PLAYER_1)? -1.0f:+1.0f;
					int iIndex = pChain->LoadSound( sKeysoundFilePath );
					pChain->AddSound( iIndex, fSeconds, fPan );
				}
			}
		}
	}
}

void AutoKeysounds::LoadTracks( const Song *pSong, RageSoundReader *&pShared, RageSoundReader *&pPlayer1, RageSoundReader *&pPlayer2 )
{
	// If we have two players, prefer a three-track sound; otherwise prefer a
	// two-track sound.
	//bool bTwoPlayers = GAMESTATE->GetNumPlayersEnabled() == 2;

	pPlayer1 = nullptr;
	pPlayer2 = nullptr;
	pShared = nullptr;

	std::vector<RString> vsMusicFile;
	const RString sMusicPath = GAMESTATE->m_pCurSteps[GAMESTATE->GetMasterPlayerNumber()]->GetMusicPath();

	if( !sMusicPath.empty() )
		vsMusicFile.push_back( sMusicPath );

	FOREACH_ENUM( InstrumentTrack, it )
	{
		if( it == InstrumentTrack_Guitar )
			continue;
		if( pSong->HasInstrumentTrack(it) )
			vsMusicFile.push_back( pSong->GetInstrumentTrackPath(it) );
	}


	std::vector<RageSoundReader *> vpSounds;
	for (RString const &s : vsMusicFile)
	{
		RString sError;
		RageSoundReader *pSongReader = RageSoundReader_FileReader::OpenFile( s, sError );
		vpSounds.push_back( pSongReader );
	}

	if( vpSounds.size() == 1 )
	{
		RageSoundReader *pSongReader = vpSounds[0];

		// Load the buffering filter before the effects filters, so effects aren't delayed.
		pSongReader = new RageSoundReader_Extend( pSongReader );
		pSongReader = new RageSoundReader_ThreadedBuffer( pSongReader );
		pShared = pSongReader;
	}
	else if( !vpSounds.empty() )
	{
		RageSoundReader_Merge *pMerge = new RageSoundReader_Merge;

		for (RageSoundReader *so : vpSounds)
			pMerge->AddSound( so );
		pMerge->Finish( SOUNDMAN->GetDriverSampleRate() );

		RageSoundReader *pSongReader = pMerge;

		// Load the buffering filter before the effects filters, so effects aren't delayed.
		pSongReader = new RageSoundReader_Extend( pSongReader );
		pSongReader = new RageSoundReader_ThreadedBuffer( pSongReader );
		pShared = pSongReader;
	}


	if( pSong->HasInstrumentTrack(InstrumentTrack_Guitar) )
	{
		RString sError;
		RageSoundReader *pGuitarTrackReader = RageSoundReader_FileReader::OpenFile( pSong->GetInstrumentTrackPath(InstrumentTrack_Guitar), sError );
		// Load the buffering filter before the effects filters, so effects aren't delayed.
		pGuitarTrackReader = new RageSoundReader_Extend( pGuitarTrackReader );
		pGuitarTrackReader = new RageSoundReader_ThreadedBuffer( pGuitarTrackReader );
		pPlayer1 = pGuitarTrackReader;
	}

	return;

	//if( pSongReader->GetNumChannels() <= 2 )
	//{
	//	/* If we only have one track, return it as the shared track. */
	//	pShared = pSongReader;
	//	return;
	//}

	// TODO: Make this work for player 2, and for 2 players

	/* The code below is used to split the main sound stream into per-player
	 * sounds. The results of this method doesn't seem interesting enough to
	 * bother supporting this. */

	//RageSoundSplitter Splitter( pSongReader );

	//RageSoundReader_Split *pMainSound = Splitter.CreateSound();
	//pMainSound->AddSourceChannelToSound( 0, 0 );
	//if( pSongReader->GetNumChannels() >= 2 ) // stereo
	//	pMainSound->AddSourceChannelToSound( 1, 1 );
	//pShared = pMainSound;

	//RageSoundReader_Split *pLeadSound = Splitter.CreateSound();
	//pLeadSound->AddSourceChannelToSound( 2, 0 );
	//if( pSongReader->GetNumChannels() >= 4 ) // stereo
	//	pLeadSound->AddSourceChannelToSound( 3, 1 );
	//pPlayer1 = pLeadSound;

	//if( pSongReader->GetNumChannels() >= 5 )
	//{
	//	if( bTwoPlayers )
	//	{
	//		RageSoundReader_Split *pSecondarySound = Splitter.CreateSound();
	//		pSecondarySound->AddSourceChannelToSound( 4, 0 );
	//		if( pSongReader->GetNumChannels() >= 6 )
	//			pSecondarySound->AddSourceChannelToSound( 5, 1 ); // stereo
	//		else
	//			pSecondarySound->AddSourceChannelToSound( 4, 1 ); // mono
	//		pPlayer2 = pSecondarySound;
	//	}
	//	else
	//	{
	//		/* We have a secondary track, but we only have one player.  Mix it into
	//		 * the background track. */
	//		pMainSound->AddSourceChannelToSound( 4, 0 );
	//		if( pSongReader->GetNumChannels() >= 6 )
	//			pMainSound->AddSourceChannelToSound( 5, 1 ); // stereo
	//		else
	//			pMainSound->AddSourceChannelToSound( 4, 1 ); // mono
	//	}

	//}
	//else if( bTwoPlayers )
	//{
	//	/* We have two players, but only two tracks.  Use the same track for both
	//	 * players. */
	//	pPlayer2 = pPlayer1->Copy();
	//	pPlayer1->SetProperty( "Balance", -1.0f );
	//	pPlayer2->SetProperty( "Balance", +1.0f );
	//}
}

void AutoKeysounds::FinishLoading()
{
	m_sSound.Unload();

	Song* pSong = GAMESTATE->m_pCurSong;

	std::vector<RageSoundReader *> apSounds;
	LoadTracks( pSong, m_pSharedSound, m_pPlayerSounds[0], m_pPlayerSounds[1] );

	// Load autoplay sounds, if any.
	{
		RageSoundReader_Chain *pChain = new RageSoundReader_Chain;
		pChain->SetPreferredSampleRate( SOUNDMAN->GetDriverSampleRate() );
		LoadAutoplaySoundsInto( pChain );

		if( pChain->GetNumSounds() > 0 || !m_pSharedSound )
		{
			if( m_pSharedSound )
			{
				int iIndex = pChain->LoadSound( m_pSharedSound );
				pChain->AddSound( iIndex, 0.0f, 0 );
			}
			pChain->Finish();
			m_pSharedSound = new RageSoundReader_Extend(pChain);
		}
		else
		{
			delete pChain;
		}
	}
	ASSERT_M( m_pSharedSound != nullptr, ssprintf("No keysounds were loaded for the song %s!", pSong->m_sMainTitle.c_str() ));

	m_pSharedSound = new RageSoundReader_PitchChange( m_pSharedSound );
	m_pSharedSound = new RageSoundReader_PostBuffering( m_pSharedSound );
	m_pSharedSound = new RageSoundReader_Pan( m_pSharedSound );
	apSounds.push_back( m_pSharedSound );

	if( m_pPlayerSounds[0] != nullptr )
	{
		m_pPlayerSounds[0] = new RageSoundReader_PitchChange( m_pPlayerSounds[0] );
		m_pPlayerSounds[0] = new RageSoundReader_PostBuffering( m_pPlayerSounds[0] );
		m_pPlayerSounds[0] = new RageSoundReader_Pan( m_pPlayerSounds[0] );
		apSounds.push_back( m_pPlayerSounds[0] );
	}

	if( m_pPlayerSounds[1] != nullptr )
	{
		m_pPlayerSounds[1] = new RageSoundReader_PitchChange( m_pPlayerSounds[1] );
		m_pPlayerSounds[1] = new RageSoundReader_PostBuffering( m_pPlayerSounds[1] );
		m_pPlayerSounds[1] = new RageSoundReader_Pan( m_pPlayerSounds[1] );
		apSounds.push_back( m_pPlayerSounds[1] );
	}

	if( GAMESTATE->GetNumPlayersEnabled() == 1 && GAMESTATE->GetMasterPlayerNumber() == PLAYER_2 )
		std::swap( m_pPlayerSounds[PLAYER_1], m_pPlayerSounds[PLAYER_2] );

	if( apSounds.size() > 1 )
	{
		RageSoundReader_Merge *pMerge = new RageSoundReader_Merge;

		for (RageSoundReader *ps : apSounds)
			pMerge->AddSound( ps );

		pMerge->Finish( SOUNDMAN->GetDriverSampleRate() );

		m_pChain = pMerge;
	}
	else
	{
		ASSERT( !apSounds.empty() );
		m_pChain = apSounds[0];
	}

	m_sSound.LoadSoundReader( m_pChain );
}

void AutoKeysounds::Update( float fDelta )
{
	// Play keysounds for crossed rows.
/*
	bool bCrossedABeat = false;
	{
		float fPositionSeconds = GAMESTATE->m_fMusicSeconds;
		float fSongBeat = GAMESTATE->m_pCurSong->GetBeatFromElapsedTime( fPositionSeconds );

		int iRowNow = BeatToNoteRowNotRounded( fSongBeat );
		iRowNow = std::max( 0, iRowNow );
		static int iRowLastCrossed = 0;

		float fBeatLast = std::round(NoteRowToBeat(iRowLastCrossed));
		float fBeatNow = std::round(NoteRowToBeat(iRowNow));

		bCrossedABeat = fBeatLast != fBeatNow;

		FOREACH_EnabledPlayer( pn )
		{
			const NoteData &nd = m_ndAutoKeysoundsOnly[pn];

			for( int t=0; t<nd.GetNumTracks(); t++ )
			{
				FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( nd, t, r, iRowLastCrossed+1, iRowNow )
				{
					const TapNote &tn = nd.GetTapNote( t, r );
					ASSERT( tn.type == TapNoteType_AutoKeysound );
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
