#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: Screen

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "GameManager.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "RageSound.h"
#include "ThemeManager.h"
#include "ScreenManager.h"

Screen::Screen()
{
	m_bIsTransparent = false;
}

Screen::~Screen()
{

}

void Screen::AddChild( Actor* pActor )
{
	// add only if the actor is on screen
//	float fX = pActor->GetX();
//	float fY = pActor->GetY();
//	if( SCREEN_LEFT>=fX && fX<=SCREEN_RIGHT && SCREEN_TOP>=fY && fY<=SCREEN_BOTTOM )
		ActorFrame::AddChild( pActor );
}

void Screen::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );


	// update the times of queued ScreenMessages and send if timer has expired
	// The order you remove messages in must be very careful!  Sending a message can 
	// potentially clear all m_QueuedMessages, and set a new state!
	for( unsigned i=0; i<m_QueuedMessages.size(); i++ )
	{
		/* Er, wait.  Shouldn't we subtract first?  This will make messages
		 * get delayed an extra frame. -glenn */
		if( m_QueuedMessages[i].fDelayRemaining <= 0.0f )		// send this sucker!
		{
			this->HandleScreenMessage( m_QueuedMessages[i].SM );
			if(i < m_QueuedMessages.size())
				m_QueuedMessages.erase(m_QueuedMessages.begin()+i);
			i--;
		}
		else
		{
			m_QueuedMessages[i].fDelayRemaining -= fDeltaTime;
		}
	}
}

void Screen::MenuBack(	PlayerNumber pn, const InputEventType type )
{
	if(!PREFSMAN->m_bDelayedEscape || type==IET_SLOW_REPEAT || type==IET_FAST_REPEAT)
		MenuBack(pn); 
}

void Screen::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	/* Don't send release messages with the default handler. */
	if(type == IET_RELEASE) return; // don't care

	/* Don't make the user hold the back button if they're pressing escape. */
	if( DeviceI.device == DEVICE_KEYBOARD  &&  DeviceI.button == SDLK_ESCAPE )
	{
		this->MenuBack( MenuI.player );
		return;
	}

	// default input handler used by most menus
	if( !MenuI.IsValid() )
		return;

	if( !GAMESTATE->IsPlayerEnabled(MenuI.player) )
		return;

	switch( MenuI.button )
	{
	case MENU_BUTTON_UP:	this->MenuUp( MenuI.player, type );		return;
	case MENU_BUTTON_DOWN:	this->MenuDown( MenuI.player, type );	return;
	case MENU_BUTTON_LEFT:	this->MenuLeft( MenuI.player, type );	return;
	case MENU_BUTTON_RIGHT:	this->MenuRight( MenuI.player, type );	return;
	case MENU_BUTTON_BACK:	this->MenuBack( MenuI.player, type );	return;
	case MENU_BUTTON_START:	this->MenuStart( MenuI.player, type );	return;
	case MENU_BUTTON_COIN:	this->MenuCoin( MenuI.player, type );	return;
	}
}

bool Screen::ChangeCoinModeInput( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( type != IET_FIRST_PRESS )
		return false;
	if( DeviceI.device == DEVICE_KEYBOARD && DeviceI.button == SDLK_F3 )
	{
		(int&)PREFSMAN->m_CoinMode = (PREFSMAN->m_CoinMode+1) % PrefsManager::NUM_COIN_MODES;
		/* ResetGame();
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
		return true;
	}
	return false;
}

bool Screen::JoinInput( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( !GAMESTATE->m_bPlayersCanJoin )
		return false;

	if( MenuI.IsValid()  &&  MenuI.button==MENU_BUTTON_START )
	{
		/* If this side is already in, don't re-join (and re-pay!). */
		if(GAMESTATE->m_bSideIsJoined[MenuI.player])
			return false;

		/* subtract coins */
		int iCoinsToCharge = 0;
		if( PREFSMAN->m_CoinMode == PrefsManager::COIN_PAY )
			iCoinsToCharge = PREFSMAN->m_iCoinsPerCredit;
		
		if( PREFSMAN->m_bJointPremium )
			if( GAMESTATE->m_MasterPlayerNumber!=PLAYER_INVALID )	// one side already joined
				iCoinsToCharge = 0;

		if( GAMESTATE->m_iCoins < iCoinsToCharge )
			return false;	// not enough coins
		else
			GAMESTATE->m_iCoins -= iCoinsToCharge;

		GAMESTATE->m_bSideIsJoined[MenuI.player] = true;
		if( GAMESTATE->m_MasterPlayerNumber == PLAYER_INVALID )
			GAMESTATE->m_MasterPlayerNumber = MenuI.player;

		SCREENMAN->RefreshCreditsMessages();

		SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","Common start") );

		return true;
	}

	return false;
}


void Screen::MenuCoin( PlayerNumber pn )
{
	// This is now handled globally by Stepmania.cpp  --  Miryokuteki
}

void Screen::SendScreenMessage( const ScreenMessage SM, float fDelay )
{
	ASSERT( fDelay >= 0.0 );

	QueuedScreenMessage QSM;
	QSM.SM = SM;
	QSM.fDelayRemaining = fDelay;
	m_QueuedMessages.push_back( QSM );
}

void Screen::ClearMessageQueue()
{
	m_QueuedMessages.clear(); 
}

void Screen::ClearMessageQueue( const ScreenMessage SM )
{
	for( int i=m_QueuedMessages.size()-1; i>=0; i-- )
		m_QueuedMessages.erase( m_QueuedMessages.begin()+i ); 
}


// Screen classes
#include "ScreenAppearanceOptions.h"
#include "ScreenCaution.h"
#include "ScreenEdit.h"
#include "ScreenEditMenu.h"
#include "ScreenEvaluation.h"
#include "ScreenEz2SelectPlayer.h"
#include "ScreenSelectMode.h"
#include "ScreenGameOver.h"
#include "ScreenGameplay.h"
#include "ScreenGraphicOptions.h"
#include "ScreenHowToPlay.h"
#include "ScreenInputOptions.h"
#include "ScreenMachineOptions.h"
#include "ScreenMapControllers.h"
#include "ScreenMusicScroll.h"
#include "ScreenPlayerOptions.h"
#include "ScreenSelectCourse.h"
#include "ScreenSelectDifficulty.h"
#include "ScreenSelectDifficultyEX.h"
#include "ScreenSelectGame.h"
#include "ScreenSelectGroup.h"
#include "ScreenSelectMusic.h"
#include "ScreenSelectStyle5th.h"
#include "ScreenSelectStyle.h"
#include "ScreenSongOptions.h"
#include "ScreenSoundOptions.h"
#include "ScreenStage.h"
#include "ScreenTest.h"
#include "ScreenTestFonts.h"
#include "ScreenTestSound.h"
#include "ScreenTitleMenu.h"
#include "ScreenEz2SelectMusic.h"
#include "ScreenWarning.h"
#include "ScreenRanking.h"
#include "ScreenMemoryCard.h"
#include "ScreenCompany.h"
#include "ScreenIntroMovie.h"
#include "ScreenAlbums.h"
#include "ScreenLogo.h"
#include "ScreenUnlock.h"
#include "ScreenDemonstration.h"
#include "ScreenInstructions.h"
#include "ScreenNameEntry.h"
#include "ScreenJukebox.h"
#include "ScreenJukeboxMenu.h"
#include "ScreenOptionsMenu.h"
#include "ScreenGameplayOptions.h"
#include "ScreenStyleSplash.h"

Screen* Screen::Create( CString sClassName )
{
	Screen *ret = NULL;

	if(		 0==stricmp(sClassName, "ScreenAppearanceOptions") )	ret = new ScreenAppearanceOptions;
	else if( 0==stricmp(sClassName, "ScreenCaution") )				ret = new ScreenCaution;
	else if( 0==stricmp(sClassName, "ScreenEdit") )					ret = new ScreenEdit;
	else if( 0==stricmp(sClassName, "ScreenEditMenu") )				ret = new ScreenEditMenu;
	else if( 0==stricmp(sClassName, "ScreenEvaluation") )			ret = new ScreenEvaluation;
	else if( 0==stricmp(sClassName, "ScreenFinalEvaluation") )		ret = new ScreenFinalEvaluation;
	else if( 0==stricmp(sClassName, "ScreenEz2SelectPlayer") )		ret = new ScreenEz2SelectPlayer;
	else if( 0==stricmp(sClassName, "ScreenSelectMode") )			ret = new ScreenSelectMode;
	else if( 0==stricmp(sClassName, "ScreenGameOver") )				ret = new ScreenGameOver;
	else if( 0==stricmp(sClassName, "ScreenGameplay") )				ret = new ScreenGameplay;
	else if( 0==stricmp(sClassName, "ScreenGraphicOptions") )		ret = new ScreenGraphicOptions;
	else if( 0==stricmp(sClassName, "ScreenHowToPlay") )			ret = new ScreenHowToPlay;
	else if( 0==stricmp(sClassName, "ScreenInputOptions") )			ret = new ScreenInputOptions;
	else if( 0==stricmp(sClassName, "ScreenMachineOptions") )		ret = new ScreenMachineOptions;
	else if( 0==stricmp(sClassName, "ScreenMapControllers") )		ret = new ScreenMapControllers;
	else if( 0==stricmp(sClassName, "ScreenInputOptions") )			ret = new ScreenInputOptions;
	else if( 0==stricmp(sClassName, "ScreenMusicScroll") )			ret = new ScreenMusicScroll;
	else if( 0==stricmp(sClassName, "ScreenPlayerOptions") )		ret = new ScreenPlayerOptions;
	else if( 0==stricmp(sClassName, "ScreenSelectCourse") )			ret = new ScreenSelectCourse;
	else if( 0==stricmp(sClassName, "ScreenSelectDifficulty") )		ret = new ScreenSelectDifficulty;
	else if( 0==stricmp(sClassName, "ScreenSelectDifficultyEX") )	ret = new ScreenSelectDifficultyEX;
	else if( 0==stricmp(sClassName, "ScreenSelectGame") )			ret = new ScreenSelectGame;
	else if( 0==stricmp(sClassName, "ScreenSelectGroup") )			ret = new ScreenSelectGroup;
	else if( 0==stricmp(sClassName, "ScreenSelectMusic") )			ret = new ScreenSelectMusic;
	else if( 0==stricmp(sClassName, "ScreenSelectStyle5th") )		ret = new ScreenSelectStyle5th;
	else if( 0==stricmp(sClassName, "ScreenSelectStyle") )		ret = new ScreenSelectStyle;
	else if( 0==stricmp(sClassName, "ScreenSongOptions") )			ret = new ScreenSongOptions;
	else if( 0==stricmp(sClassName, "ScreenStage") )				ret = new ScreenStage;
	else if( 0==stricmp(sClassName, "ScreenTest") )					ret = new ScreenTest;
	else if( 0==stricmp(sClassName, "ScreenTestFonts") )			ret = new ScreenTestFonts;
	else if( 0==stricmp(sClassName, "ScreenTestSound") )			ret = new ScreenTestSound;
	else if( 0==stricmp(sClassName, "ScreenTitleMenu") )			ret = new ScreenTitleMenu;
	else if( 0==stricmp(sClassName, "ScreenEz2SelectMusic") )		ret = new ScreenEz2SelectMusic;
	else if( 0==stricmp(sClassName, "ScreenWarning") )				ret = new ScreenWarning;
	else if( 0==stricmp(sClassName, "ScreenRanking") )				ret = new ScreenRanking;
	else if( 0==stricmp(sClassName, "ScreenMemoryCard") )			ret = new ScreenMemoryCard;
	else if( 0==stricmp(sClassName, "ScreenCompany") )				ret = new ScreenCompany;
	else if( 0==stricmp(sClassName, "ScreenIntroMovie") )			ret = new ScreenIntroMovie;
	else if( 0==stricmp(sClassName, "ScreenAlbums") )				ret = new ScreenAlbums;
	else if( 0==stricmp(sClassName, "ScreenLogo") )					ret = new ScreenLogo;
	else if( 0==stricmp(sClassName, "ScreenUnlock") )				ret = new ScreenUnlock;
	else if( 0==stricmp(sClassName, "ScreenDemonstration") )		ret = (ScreenGameplay*)new ScreenDemonstration;
	else if( 0==stricmp(sClassName, "ScreenInstructions") )			ret = new ScreenInstructions;
	else if( 0==stricmp(sClassName, "ScreenNameEntry") )			ret = new ScreenNameEntry;
	else if( 0==stricmp(sClassName, "ScreenJukebox") )				ret = new ScreenJukebox;
	else if( 0==stricmp(sClassName, "ScreenJukeboxMenu") )			ret = new ScreenJukeboxMenu;
	else if( 0==stricmp(sClassName, "ScreenOptionsMenu") )			ret = new ScreenOptionsMenu;
	else if( 0==stricmp(sClassName, "ScreenSoundOptions") )			ret = new ScreenSoundOptions;
	else if( 0==stricmp(sClassName, "ScreenGameplayOptions") )		ret = new ScreenGameplayOptions;
	else if( 0==stricmp(sClassName, "ScreenStyleSplash") )			ret = new ScreenStyleSplash;
	else
		RageException::Throw( "Invalid Screen class name '%s'", sClassName.GetString() );
	return ret;
}