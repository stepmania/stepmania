#include "global.h"
#include "Screen.h"
#include "GameManager.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "RageSound.h"
#include "ThemeManager.h"
#include "ScreenManager.h"
#include "GameSoundManager.h"
#include "ProfileManager.h"

#define NEXT_SCREEN					THEME->GetMetric (m_sName,"NextScreen")
#define PREV_SCREEN					THEME->GetMetric (m_sName,"PrevScreen")

Screen::Screen( CString sName )
{
	SetName( sName );
	m_bIsTransparent = false;
}

Screen::~Screen()
{

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
		CHECKPOINT_M( ssprintf("ScreenMessage(%i)", SM) );
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
	switch( type )
	{
	case IET_FIRST_PRESS:
	case IET_SLOW_REPEAT:
	case IET_FAST_REPEAT:
		break; /* OK */
	default:
		return; // don't care
	}

	/* Don't make the user hold the back button if they're pressing escape and escape is the back button. */
	if( MenuI.button == MENU_BUTTON_BACK && DeviceI.device == DEVICE_KEYBOARD  &&  DeviceI.button == KEY_ESC )
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

void Screen::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_MenuTimer:
		FOREACH_HumanPlayer(p)
			MenuStart( p );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		break;
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( PREV_SCREEN );
		break;
	}
}

bool Screen::ChangeCoinModeInput( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( type != IET_FIRST_PRESS )
		return false;
	if( DeviceI.device == DEVICE_KEYBOARD && DeviceI.button == KEY_F3 )
	{
		PREFSMAN->m_iCoinMode++;
		wrap( PREFSMAN->m_iCoinMode, NUM_COIN_MODES );

		/* Show the real coin mode, not GetCoinMode(), or F3 will go "home, free, free"
		 * in event mode.  XXX: move GetCoinMode to GameState to keep PrefsManager dumb? */
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
	if( !GAMESTATE->PlayersCanJoin() )
		return false;

	if( MenuI.IsValid()  &&  MenuI.button==MENU_BUTTON_START )
	{
		/* If this side is already in, don't re-join (and re-pay!). */
		if(GAMESTATE->m_bSideIsJoined[MenuI.player])
			return false;

		/* subtract coins */
		int iCoinsToCharge = 0;
		if( PREFSMAN->GetCoinMode() == COIN_PAY )
			iCoinsToCharge = PREFSMAN->m_iCoinsPerCredit;

		// If joint premium don't take away a credit for the 2nd join.
		if( PREFSMAN->GetPremium() == PrefsManager::JOINT_PREMIUM  &&  
			GAMESTATE->GetNumSidesJoined() == 1 )
			iCoinsToCharge = 0;

		if( GAMESTATE->m_iCoins < iCoinsToCharge )
			return false;	// not enough coins
		else
			GAMESTATE->m_iCoins -= iCoinsToCharge;

		SCREENMAN->PlayStartSound();
		GAMESTATE->JoinPlayer( MenuI.player );

		// don't load memory card profiles here.  It's slow and can cause a big skip.
		PROFILEMAN->LoadLocalProfileFromMachine( MenuI.player );
		SCREENMAN->RefreshCreditsMessages();

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
#include "ScreenCaution.h"
#include "ScreenEdit.h"
#include "ScreenEditMenu.h"
#include "ScreenEvaluation.h"
#include "ScreenEz2SelectPlayer.h"
#include "ScreenGameOver.h"
#include "ScreenGameplay.h"
#include "ScreenHowToPlay.h"
#include "ScreenMapControllers.h"
#include "ScreenMusicScroll.h"
#include "ScreenPlayerOptions.h"
#include "ScreenSandbox.h"
#include "ScreenSelectDifficulty.h"
#include "ScreenSelectGroup.h"
#include "ScreenSelectMusic.h"
#include "ScreenSelectStyle.h"
#include "ScreenSongOptions.h"
#include "ScreenStage.h"
#include "ScreenTest.h"
#include "ScreenTestFonts.h"
#include "ScreenTestSound.h"
#include "ScreenTitleMenu.h"
#include "ScreenNetSelectMusic.h"
#include "ScreenNetEvaluation.h"
#include "ScreenEz2SelectMusic.h"
#include "ScreenRanking.h"
#include "ScreenLogo.h"
#include "ScreenUnlock.h"
#include "ScreenDemonstration.h"
#include "ScreenInstructions.h"
#include "ScreenNameEntry.h"
#include "ScreenNameEntryTraditional.h"
#include "ScreenJukebox.h"
#include "ScreenJukeboxMenu.h"
#include "ScreenStyleSplash.h"
#include "ScreenCredits.h"
#include "ScreenSelectCharacter.h"
#include "ScreenSelectMode.h"
#include "ScreenSelectMaster.h"
#include "ScreenEditCoursesMenu.h"
#include "ScreenNetworkOptions.h"
#include "ScreenProfileOptions.h"
#include "ScreenExit.h"
#include "ScreenAttract.h"
#include "ScreenReloadSongs.h"
#include "ScreenOptionsMaster.h"
#include "ScreenCenterImage.h"
#include "ScreenTestInput.h"
#include "ScreenBookkeeping.h"
#include "ScreenBranch.h"
#include "ScreenEnding.h"
#include "ScreenDownloadMachineStats.h"
#include "ScreenSetTime.h"
#include "ScreenTestLights.h"
#include "ScreenClearMachineStats.h"
#include "ScreenResetToDefaults.h"
#include "ScreenClearBookkeepingData.h"
#include "ScreenInsertCredit.h"

Screen* Screen::Create( CString sClassName )
{
	CString sName = sClassName;
	// Look up the class in the metrics group sName
	if( THEME->HasMetric(sClassName,"Class") )
		sClassName = THEME->GetMetric(sClassName,"Class");

#define IF_RETURN(X)	if(sClassName.CompareNoCase(#X)==0)	return new X(sName);

	IF_RETURN( ScreenAttract );
	IF_RETURN( ScreenCaution );
	IF_RETURN( ScreenEdit );
	IF_RETURN( ScreenEditMenu );
	IF_RETURN( ScreenEvaluation );
	IF_RETURN( ScreenEz2SelectPlayer );
	IF_RETURN( ScreenGameOver );
	IF_RETURN( ScreenGameplay );
	IF_RETURN( ScreenHowToPlay );
	IF_RETURN( ScreenMapControllers );
	IF_RETURN( ScreenMusicScroll );
	IF_RETURN( ScreenPlayerOptions );
	IF_RETURN( ScreenSandbox );
	IF_RETURN( ScreenSelectDifficulty );
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
	IF_RETURN( ScreenRanking );
	IF_RETURN( ScreenLogo );
	IF_RETURN( ScreenUnlock );
	IF_RETURN( ScreenDemonstration );
	IF_RETURN( ScreenInstructions );
	IF_RETURN( ScreenNameEntry );
	IF_RETURN( ScreenNameEntryTraditional );
	IF_RETURN( ScreenJukebox );
	IF_RETURN( ScreenJukeboxMenu );
	IF_RETURN( ScreenStyleSplash );
	IF_RETURN( ScreenCredits );
	IF_RETURN( ScreenSelectCharacter );
	IF_RETURN( ScreenSelectMaster );
	IF_RETURN( ScreenEditCoursesMenu );
	IF_RETURN( ScreenProfileOptions );
	IF_RETURN( ScreenExit );
	IF_RETURN( ScreenReloadSongs );
	IF_RETURN( ScreenOptionsMaster );
	IF_RETURN( ScreenCenterImage );
	IF_RETURN( ScreenTestInput );
	IF_RETURN( ScreenBookkeeping );
	IF_RETURN( ScreenBranch );
	IF_RETURN( ScreenEnding );
	IF_RETURN( ScreenDownloadMachineStats );
	IF_RETURN( ScreenSetTime );
	IF_RETURN( ScreenTestLights );
	IF_RETURN( ScreenClearMachineStats );
	IF_RETURN( ScreenResetToDefaults );
	IF_RETURN( ScreenClearBookkeepingData );
	IF_RETURN( ScreenInsertCredit );

#if !defined(WITHOUT_NETWORKING)
	IF_RETURN( ScreenNetworkOptions );
	IF_RETURN( ScreenNetSelectMusic );
	IF_RETURN( ScreenNetEvaluation );
#endif

	RageException::Throw( "Invalid Screen class name '%s'", sClassName.c_str() );
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
