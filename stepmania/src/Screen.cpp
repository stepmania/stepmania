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
#include "RageSounds.h"

Screen::Screen( CString sName )
{
	SetName( sName );
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

bool Screen::SortMessagesByDelayRemaining(const Screen::QueuedScreenMessage &m1,
										 const Screen::QueuedScreenMessage &m2)
{
	return m1.fDelayRemaining < m2.fDelayRemaining;
}

void Screen::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	/* We need to ensure two things:
	 * 1. Messages must be sent in the order of delay.  If two messages are sent
	 *    simultaneously, one with a .001 delay and another with a .002 delay, the
	 *    .001 delay message must be sent first.
	 * 2. Messages to be delivered simultaneously must be sent in the order queued.
	 * 
	 * Stable sort by time to ensure #2. */
	stable_sort(m_QueuedMessages.begin(), m_QueuedMessages.end(), SortMessagesByDelayRemaining);

	// update the times of queued ScreenMessages and send if timer has expired
	// The order you remove messages in must be very careful!  Sending a message can 
	// potentially clear all m_QueuedMessages, and set a new state!
	/* Also, it might call ClearMessageQueue() to clear a single message type.
	 * This might clear previous messages on the queue.  So, first apply time to
	 * everything. */
	
	unsigned i;
	for( i=0; i<m_QueuedMessages.size(); i++ )
	{
		/* Hack:
		 *
		 * If we simply subtract time and then send messages, we have a problem.
		 * Messages are queued to arrive at specific times, and those times line
		 * up with things like tweens finishing.  If we send the message at the
		 * exact time given, then it'll be on the same cycle that would be rendering
		 * the last frame of a tween (such as an object going off the screen).  However,
		 * when we send the message, we're likely to set up a new screen, which
		 * causes everything to stop in place; this results in actors occasionally
		 * not quite finishing their tweens.
		 *
		 * Let's delay all messages that have a non-zero time an extra frame. 
		 */
		if(m_QueuedMessages[i].fDelayRemaining > 0.0001f)
		{
			m_QueuedMessages[i].fDelayRemaining -= fDeltaTime;
			m_QueuedMessages[i].fDelayRemaining = max(m_QueuedMessages[i].fDelayRemaining, 0.0001f);
		} else {
			m_QueuedMessages[i].fDelayRemaining -= fDeltaTime;
		}
	}

	/* Now dispatch messages.  If the number of messages on the queue changes
	 * within HandleScreenMessage, someone cleared messages on the queue.  This
	 * means we have no idea where 'i' is, so start over. Since we applied time
	 * already, this won't cause messages to be mistimed. */
	for( i=0; i<m_QueuedMessages.size(); i++ )
	{
		if( m_QueuedMessages[i].fDelayRemaining > 0.0f )
			continue; /* not yet */

		/* Remove the message from the list. */
		const ScreenMessage SM = m_QueuedMessages[i].SM;
		m_QueuedMessages.erase(m_QueuedMessages.begin()+i);
		i--;

		unsigned size = m_QueuedMessages.size();

		// send this sucker!
		this->HandleScreenMessage( SM );

		/* If the size changed, start over. */
		if(size != m_QueuedMessages.size())
			i = 0;
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

	if( !GAMESTATE->IsHumanPlayer(MenuI.player) )
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
		(int&)PREFSMAN->m_iCoinMode = (PREFSMAN->m_iCoinMode+1) % NUM_COIN_MODES;
		/* ResetGame();
				This causes problems on ScreenIntroMovie, which results in the
				movie being restarted and/or becoming out-of-synch -- Miryokuteki */

		CString sMessage = CoinModeToString( (CoinMode)PREFSMAN->m_iCoinMode );
		sMessage.MakeUpper();
		sMessage = "Coin Mode: " + sMessage;
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
		if( PREFSMAN->m_iCoinMode == COIN_PAY )
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

		SOUND->PlayOnce( THEME->GetPathToS("Common start") );

		return true;
	}

	return false;
}


void Screen::MenuCoin( PlayerNumber pn )
{
	// This is now handled globally by Stepmania.cpp  --  Miryokuteki
}

void Screen::PostScreenMessage( const ScreenMessage SM, float fDelay )
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
		if( m_QueuedMessages[i].SM == SM )
			m_QueuedMessages.erase( m_QueuedMessages.begin()+i ); 
}


// Screen classes
#include "ScreenAppearanceOptions.h"
#include "ScreenCaution.h"
#include "ScreenEdit.h"
#include "ScreenEditMenu.h"
#include "ScreenEvaluation.h"
#include "ScreenEz2SelectPlayer.h"
#include "ScreenGameOver.h"
#include "ScreenGameplay.h"
#include "ScreenGraphicOptions.h"
#include "ScreenHowToPlay.h"
#include "ScreenInputOptions.h"
#include "ScreenMachineOptions.h"
#include "ScreenMapControllers.h"
#include "ScreenMusicScroll.h"
#include "ScreenPlayerOptions.h"
#include "ScreenSandbox.h"
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
#include "ScreenAutogenOptions.h"
#include "ScreenCredits.h"
#include "ScreenSelectCharacter.h"
#include "ScreenRaveOptions.h"
#include "ScreenSelectMode.h"
#include "ScreenBackgroundOptions.h"
#include "ScreenSelectMaster.h"

Screen* Screen::Create( CString sClassName )
{
#define IF_RETURN(X)	if(sClassName.CompareNoCase(#X)==0)	return new X;

	IF_RETURN( ScreenAppearanceOptions );
	IF_RETURN( ScreenCaution );
	IF_RETURN( ScreenEdit );
	IF_RETURN( ScreenEditMenu );
	IF_RETURN( ScreenEvaluationStage );
	IF_RETURN( ScreenEvaluationSummary );
	IF_RETURN( ScreenEvaluationNonstop );
	IF_RETURN( ScreenEvaluationOni );
	IF_RETURN( ScreenEvaluationEndless );
	IF_RETURN( ScreenEvaluationCpubattle );
	IF_RETURN( ScreenEvaluationHumanbattle );
	IF_RETURN( ScreenEvaluationRave );
	IF_RETURN( ScreenEz2SelectPlayer );
	IF_RETURN( ScreenGameOver );
	IF_RETURN( ScreenGameplay );
	IF_RETURN( ScreenGraphicOptions );
	IF_RETURN( ScreenHowToPlay );
	IF_RETURN( ScreenInputOptions );
	IF_RETURN( ScreenMachineOptions );
	IF_RETURN( ScreenMapControllers );
	IF_RETURN( ScreenInputOptions );
	IF_RETURN( ScreenMusicScroll );
	IF_RETURN( ScreenPlayerOptions );
	IF_RETURN( ScreenSandbox );
	IF_RETURN( ScreenSelectCourse );
	IF_RETURN( ScreenSelectDifficulty );
	IF_RETURN( ScreenSelectDifficultyEX );
	IF_RETURN( ScreenSelectGame );
	IF_RETURN( ScreenSelectGroup );
	IF_RETURN( ScreenSelectMusic );
	IF_RETURN( ScreenSelectStyle5th );
	IF_RETURN( ScreenSelectStyle );
	IF_RETURN( ScreenSelectMode );
	IF_RETURN( ScreenSongOptions );
	IF_RETURN( ScreenStage );
	IF_RETURN( ScreenTest );
	IF_RETURN( ScreenTestFonts );
	IF_RETURN( ScreenTestSound );
	IF_RETURN( ScreenTitleMenu );
	IF_RETURN( ScreenEz2SelectMusic );
	IF_RETURN( ScreenWarning );
	IF_RETURN( ScreenRanking );
	IF_RETURN( ScreenCompany );
	IF_RETURN( ScreenIntroMovie );
	IF_RETURN( ScreenAlbums );
	IF_RETURN( ScreenLogo );
	IF_RETURN( ScreenUnlock );
	IF_RETURN( ScreenDemonstration );
	IF_RETURN( ScreenInstructions );
	IF_RETURN( ScreenNameEntry );
	IF_RETURN( ScreenJukebox );
	IF_RETURN( ScreenJukeboxMenu );
	IF_RETURN( ScreenOptionsMenu );
	IF_RETURN( ScreenSoundOptions );
	IF_RETURN( ScreenGameplayOptions );
	IF_RETURN( ScreenStyleSplash );
	IF_RETURN( ScreenAutogenOptions );
	IF_RETURN( ScreenCredits );
	IF_RETURN( ScreenSelectCharacter );
	IF_RETURN( ScreenRaveOptions );
	IF_RETURN( ScreenBackgroundOptions );
	IF_RETURN( ScreenSelectMaster );

	RageException::Throw( "Invalid Screen class name '%s'", sClassName.c_str() );
}
