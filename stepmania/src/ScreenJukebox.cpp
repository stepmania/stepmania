#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenJukebox

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenJukebox.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "SongManager.h"
#include "StepMania.h"



const ScreenMessage	SM_NotesEnded				= ScreenMessage(SM_User+101);	// MUST be same as in ScreenGameplay


bool PrepareForJukebox()		// always return true.
{
	// ScreeJukeboxMenu must set this
	ASSERT( GAMESTATE->m_CurStyle != STYLE_INVALID );
	GAMESTATE->m_PlayMode = PLAY_MODE_ARCADE;

	vector<Song*> vSongs;
	if( GAMESTATE->m_sPreferredGroup.CompareNoCase("all music") == 0 )
		SONGMAN->GetSongs( vSongs );
	else
		SONGMAN->GetSongs( vSongs, GAMESTATE->m_sPreferredGroup );

	//
	// Search for a Song and Notes to play during the demo
	//
	for( int i=0; i<600; i++ )
	{
		if( vSongs.size() == 0 )
			return true;

		Song* pSong = vSongs[rand()%vSongs.size()];

		if( !pSong->HasMusic() )
			continue;	// skip

		Difficulty dc = GAMESTATE->m_PreferredDifficulty[PLAYER_1];
		Notes* pNotes = NULL;
		if( dc != DIFFICULTY_INVALID )
			pNotes = pSong->GetNotes( GAMESTATE->GetCurrentStyleDef()->m_NotesType, GAMESTATE->m_PreferredDifficulty[PLAYER_1] );
		else	// "all difficulties"
		{
			vector<Notes*> vNotes;
			pSong->GetNotes( vNotes, GAMESTATE->GetCurrentStyleDef()->m_NotesType );
			if( vNotes.size() > 0 )
				pNotes = vNotes[rand()%vNotes.size()];
		}

		if( pNotes == NULL )
			continue;	// skip

		// Found something we can use!
		GAMESTATE->m_pCurSong = pSong;
		for( int p=0; p<NUM_PLAYERS; p++ )
			GAMESTATE->m_pCurNotes[p] = pNotes;
		
		break;	// done looking
	}

	ASSERT( GAMESTATE->m_pCurSong );

	GAMESTATE->m_MasterPlayerNumber = PLAYER_1;

	// choose some cool options
	int Benchmark = 0;
	if(Benchmark)
	{
		/* Note that you also need to make sure you benchmark with the
		 * same notes.  I use a copy of MaxU with only heavy notes included. */
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsPlayerEnabled(p) )
				continue;

			/* Lots and lots of arrows.  This might even bias to arrows a little
			 * too much. */
			GAMESTATE->m_PlayerOptions[p] = PlayerOptions();
			GAMESTATE->m_PlayerOptions[p].m_fScrollSpeed = .25f;
			GAMESTATE->m_PlayerOptions[p].m_bEffects[ PlayerOptions::EFFECT_SPACE ] = true;
			GAMESTATE->m_PlayerOptions[p].m_bEffects[ PlayerOptions::EFFECT_MINI ] = true;
		}
		GAMESTATE->m_SongOptions.m_LifeType = SongOptions::LIFE_BATTERY;
		PREFSMAN->m_FailType = PrefsManager::FAIL_OFF;
	}

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;

		GAMESTATE->m_PlayerOptions[p] = PlayerOptions();

		if( GAMESTATE->m_bJukeboxUsesModifiers )
			GAMESTATE->m_PlayerOptions[p].ChooseRandomMofifiers();
	}

	GAMESTATE->m_SongOptions = SongOptions();

	GAMESTATE->m_SongOptions.m_LifeType = (randomf(0,1)>0.8f) ? SongOptions::LIFE_BATTERY : SongOptions::LIFE_BAR;
	PREFSMAN->m_FailType = PrefsManager::FAIL_OFF;

	GAMESTATE->m_bDemonstration = true;

	return true;
}

ScreenJukebox::ScreenJukebox() : ScreenGameplay(PrepareForJukebox())	// this is a hack to get some code to execute before the ScreenGameplay constructor
{
	LOG->Trace( "ScreenJukebox::ScreenJukebox()" );

	if( GAMESTATE->m_pCurSong == NULL )	// we didn't find a song.
	{
		this->SendScreenMessage( SM_GoToNextScreen, 0 );	// Abort demonstration.
		return;
	}

	m_Fade.OpenWipingRight();

	ClearMessageQueue();	// remove all of the messages set in ScreenGameplay that animate "ready", "here we go", etc.

	GAMESTATE->m_bPastHereWeGo = true;

	m_StarWipe.SetOpened();
	m_DancingState = STATE_DANCING;
}

ScreenJukebox::~ScreenJukebox()
{
//	GAMESTATE->m_bDemonstration = false;
	//GAMESTATE->Reset();
}

void ScreenJukebox::Update( float fDeltaTime )
{
	ScreenGameplay::Update( fDeltaTime );

	// hide status icons
	for( int i=0; i<NUM_STATUS_ICONS; i++ )
		m_sprStatusIcons[i].SetDiffuse( RageColor(1,1,1,0) );	
}

void ScreenJukebox::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	//LOG->Trace( "ScreenJukebox::Input()" );


	//
	// This should be the same as ScreenAttract::Input()
	//

	if(type != IET_FIRST_PRESS) return; // don't care

	if( DeviceI.device == DEVICE_KEYBOARD && DeviceI.button == SDLK_F3 )
	{
		(int&)PREFSMAN->m_CoinMode = (PREFSMAN->m_CoinMode+1) % PrefsManager::NUM_COIN_MODES;
		/*ResetGame();
				You only changed the coin mode.. as with the arcade versions,
				the machine should not reset.	-- Miryokuteki				*/

		CString sMessage = "Coin Mode: ";
		switch( PREFSMAN->m_CoinMode )
		{
		case PrefsManager::COIN_HOME:	sMessage += "HOME";	break;
		case PrefsManager::COIN_PAY:	sMessage += "PAY";	break;
		case PrefsManager::COIN_FREE:	sMessage += "FREE";	break;
		}
		SCREENMAN->RefreshCreditsMessages();
		SCREENMAN->SystemMessage( sMessage );
		return;
	}

	if( MenuI.IsValid() )
	{
		switch( MenuI.button )
		{
		case MENU_BUTTON_LEFT:
		case MENU_BUTTON_RIGHT:
			if( !m_Fade.IsOpening() && !m_Fade.IsClosing() )
				m_Fade.CloseWipingRight( SM_GoToNextScreen );
			break;
		case MENU_BUTTON_COIN:
			Screen::MenuCoin( MenuI.player );	// increment coins, play sound
			m_soundMusic.Stop();
			::Sleep( 800 );	// do a little pause, like the arcade does
			SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
			break;
		case MENU_BUTTON_START:
		case MENU_BUTTON_BACK:

			switch( PREFSMAN->m_CoinMode )
			{
			case PrefsManager::COIN_PAY:
				if( GAMESTATE->m_iCoins < PREFSMAN->m_iCoinsPerCredit )
					break;	// don't fall through
				// fall through
			case PrefsManager::COIN_FREE:
			case PrefsManager::COIN_HOME:
				m_soundMusic.Stop();
				SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","insert coin") );
				::Sleep( 800 );	// do a little pause, like the arcade does
				SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
				break;
			default:
				ASSERT(0);
			}
			break;
		}
	}

}

void ScreenJukebox::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_NotesEnded:
		if( m_Fade.IsClosing() )
			return;	// ignore - we're already fading
		m_Fade.CloseWipingRight( SM_GoToNextScreen );
		return;
	case SM_GoToNextScreen:
		/* We're actually under Update(), so make sure ScreenGameplay doesn't
		 * continue grading for this call. */
		m_soundMusic.StopPlaying();

		SCREENMAN->SetNewScreen( "ScreenJukebox" );
		return;
	}

	ScreenGameplay::HandleScreenMessage( SM );
}