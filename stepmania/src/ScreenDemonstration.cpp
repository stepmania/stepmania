#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenDemonstration

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenDemonstration.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "SongManager.h"
#include "StepMania.h"
#include "SDL_utils.h"


#define SECONDS_TO_SHOW			THEME->GetMetricF("ScreenDemonstration","SecondsToShow")
#define NEXT_SCREEN				THEME->GetMetric("ScreenDemonstration","NextScreen")


const ScreenMessage	SM_NotesEnded				= ScreenMessage(SM_User+101);	// MUST be same as in ScreenGameplay
const ScreenMessage	SM_BeginFadingToNextScreen	= ScreenMessage(SM_User+1000);


bool SetUpSongOptions()		// always return true.
{
	//
	// Set the current song to prepare for a demonstration
	//

	switch( GAMESTATE->m_CurGame )
	{
	case GAME_DANCE:	GAMESTATE->m_CurStyle = STYLE_DANCE_VERSUS;			break; 
	case GAME_PUMP:		GAMESTATE->m_CurStyle = STYLE_PUMP_VERSUS;			break; 
	case GAME_EZ2:		GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE_VERSUS;	break; 
	case GAME_PARA:		GAMESTATE->m_CurStyle = STYLE_PARA_SINGLE;			break; 
	case GAME_DS3DDX:	GAMESTATE->m_CurStyle = STYLE_DS3DDX_SINGLE;		break;
	case GAME_BM:		GAMESTATE->m_CurStyle = STYLE_BM_SINGLE;			break;
	default:	ASSERT(0);
	}

	GAMESTATE->m_PlayMode = PLAY_MODE_ARCADE;


	//
	// Search for a Song and Steps to play during the demo
	//
	for( int i=0; i<600; i++ )	// try 600 times
	{
		Song* pSong = SONGMAN->GetRandomSong();
		if( pSong == NULL )	// returns NULL there are no songs
			return true;	// we need to detect this and abort demonstration later

		if( pSong->m_apNotes.empty() )
			continue;	// skip
		
		if( !pSong->HasMusic() )
			continue;	// skip

		vector<Notes*> apNotes;
		pSong->GetNotes( apNotes, GAMESTATE->GetCurrentStyleDef()->m_NotesType );

		if( apNotes.empty() )
			continue;	// skip

		// Found something we can use!
		Notes* pNotes = apNotes[ rand()%apNotes.size() ];

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
			GAMESTATE->m_PlayerOptions[p].Init();
			GAMESTATE->m_PlayerOptions[p].m_fScrollSpeed = .25f;
			GAMESTATE->m_PlayerOptions[p].m_fEffects[ PlayerOptions::EFFECT_SPACE ] = 1;
			GAMESTATE->m_PlayerOptions[p].m_fEffects[ PlayerOptions::EFFECT_MINI ] = 1;
		}
		GAMESTATE->m_SongOptions.m_LifeType = SongOptions::LIFE_BATTERY;
		GAMESTATE->m_SongOptions.m_FailType = SongOptions::FAIL_OFF;
	}

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;

		GAMESTATE->m_PlayerOptions[p].Init();
		GAMESTATE->m_PlayerOptions[p].ChooseRandomMofifiers();
	}

	GAMESTATE->m_SongOptions = SongOptions();

	GAMESTATE->m_SongOptions.m_LifeType = (randomf(0,1)>0.8f) ? SongOptions::LIFE_BATTERY : SongOptions::LIFE_BAR;
	GAMESTATE->m_SongOptions.m_FailType = SongOptions::FAIL_OFF;

	GAMESTATE->m_bDemonstration = true;

	return true;
}

ScreenDemonstration::ScreenDemonstration() : ScreenGameplay(SetUpSongOptions())	// this is a hack to get some code to execute before the ScreenGameplay constructor
{
	LOG->Trace( "ScreenDemonstration::ScreenDemonstration()" );

	if( GAMESTATE->m_pCurSong == NULL )	// we didn't find a song.
	{
		this->SendScreenMessage( SM_GoToNextScreen, 0 );	// Abort demonstration.
		return;
	}


	m_sprDemonstrationOverlay.Load( THEME->GetPathTo("Graphics","demonstration overlay") );
	m_sprDemonstrationOverlay.SetXY( CENTER_X, CENTER_Y );
	this->AddChild( &m_sprDemonstrationOverlay );

	m_sprDemonstrationBlink.Load( THEME->GetPathTo("Graphics","demonstration blink") );
	m_sprDemonstrationBlink.SetXY( CENTER_X, CENTER_Y );
	m_sprDemonstrationBlink.SetEffectDiffuseBlink();
	this->AddChild( &m_sprDemonstrationBlink );

	this->MoveToTail( &m_Fade );

	m_Fade.OpenWipingRight();

	ClearMessageQueue();	// remove all of the messages set in ScreenGameplay that animate "ready", "here we go", etc.

	GAMESTATE->m_bPastHereWeGo = true;

	m_StarWipe.SetOpened();
	m_DancingState = STATE_DANCING;
	this->SendScreenMessage( SM_BeginFadingToNextScreen, SECONDS_TO_SHOW );	
}

ScreenDemonstration::~ScreenDemonstration()
{
	GAMESTATE->m_bDemonstration = false;
	GAMESTATE->Reset();
}

void ScreenDemonstration::Update( float fDeltaTime )
{
	ScreenGameplay::Update( fDeltaTime );

	// hide status icons
	for( int i=0; i<NUM_STATUS_ICONS; i++ )
		m_sprStatusIcons[i].SetDiffuse( RageColor(1,1,1,0) );	
}

void ScreenDemonstration::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	//LOG->Trace( "ScreenDemonstration::Input()" );


	//
	// This should be the same as ScreenAttract::Input()
	//

	if(type != IET_FIRST_PRESS) return; // don't care

	if( DeviceI.device == DEVICE_KEYBOARD && DeviceI.button == SDLK_F3 )
	{
		(int&)PREFSMAN->m_CoinMode = (PREFSMAN->m_CoinMode+1) % PrefsManager::NUM_COIN_MODES;
		/*ResetGame();
				This causes problems on ScreenIntroMovie, which results in the
				movie being restarted and/or becoming out-of-synch -- Miryokuteki */

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
			SDL_Delay( 800 );	// do a little pause, like the arcade does
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
				SDL_Delay( 800 );	// do a little pause, like the arcade does
				SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
				break;
			default:
				ASSERT(0);
			}
			break;
		}
	}

}

void ScreenDemonstration::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_NotesEnded:
		this->SendScreenMessage( SM_BeginFadingToNextScreen, 0 );
		return;
	case SM_BeginFadingToNextScreen:
		m_Fade.CloseWipingRight( SM_GoToNextScreen );
		return;
	case SM_GoToNextScreen:
		m_soundMusic.Stop();
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		return;
	}

	ScreenGameplay::HandleScreenMessage( SM );
}
