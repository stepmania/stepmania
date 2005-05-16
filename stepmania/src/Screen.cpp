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
#include "ActorUtil.h"

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

void Screen::Init()
{
	this->RunCommands( THEME->GetMetricA(m_sName, "InitCommand") );
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
	
	for( unsigned i=0; i<m_QueuedMessages.size(); i++ )
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
	for( unsigned i=0; i<m_QueuedMessages.size(); i++ )
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
	if(!PREFSMAN->m_bDelayedBack || type==IET_SLOW_REPEAT || type==IET_FAST_REPEAT)
		MenuBack(pn); 
}

/* ScreenManager sends input here first.  Overlay screens can use it to get a first
 * pass at input.  Return true if the input was handled and should not be passed
 * to lower screens, or false if not handled.  If true is returned, Input() will
 * not be called, either.  Normal screens should not overload this function. */
bool Screen::OverlayInput( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	return false;
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

	switch( MenuI.button )
	{
	case MENU_BUTTON_UP:	this->MenuUp( MenuI.player, type );		return;
	case MENU_BUTTON_DOWN:	this->MenuDown( MenuI.player, type );	return;
	case MENU_BUTTON_LEFT:	this->MenuLeft( MenuI.player, type );	return;
	case MENU_BUTTON_RIGHT:	this->MenuRight( MenuI.player, type );	return;
	case MENU_BUTTON_BACK:	this->MenuBack( MenuI.player, type );	return;
	case MENU_BUTTON_START:	this->MenuStart( MenuI.player, type );	return;
	case MENU_BUTTON_SELECT:this->MenuSelect( MenuI.player, type );	return;
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
		if( SCREENMAN->IsStackedScreen(this) )
			SCREENMAN->PopTopScreen( SM_None );
		else
			SCREENMAN->SetNewScreen( NEXT_SCREEN );
		break;
	case SM_GoToPrevScreen:
		SCREENMAN->DeletePreparedScreens();
		if( SCREENMAN->IsStackedScreen(this) )
			SCREENMAN->PopTopScreen( SM_None );
		else
			SCREENMAN->SetNewScreen( PREV_SCREEN );
		break;
	}
}

bool Screen::JoinInput( const MenuInput &MenuI )
{
	if( !GAMESTATE->PlayersCanJoin() )
		return false;

	if( MenuI.IsValid()  &&  MenuI.button==MENU_BUTTON_START )
	{
		/* If this side is already in, don't re-join (and re-pay!). */
		if(GAMESTATE->m_bSideIsJoined[MenuI.player])
			return false;

		/* subtract coins */
		int iCoinsNeededToJoin = GAMESTATE->GetCoinsNeededToJoin();

		if( GAMESTATE->m_iCoins < iCoinsNeededToJoin )
			return false;	// not enough coins
		else
			GAMESTATE->m_iCoins -= iCoinsNeededToJoin;

		// HACK: Only play start sound for the 2nd player who joins.  The 
		// start sound for the 1st player will be played by ScreenTitleMenu 
		// when the player makes a selection on the screen.
		if( GAMESTATE->GetNumSidesJoined() > 0 )
			SCREENMAN->PlayStartSound();

		GAMESTATE->JoinPlayer( MenuI.player );

		// don't load memory card profiles here.  It's slow and can cause a big skip.
		/* Don't load the local profile, either.  It causes a 150+ms skip on my A64 3000+,
		 * so it probably causes a skip for everyone.  We probably shouldn't load this here,
		 * anyway: leave it unloaded and display "INSERT CARD" until the normal time, and
		 * load the local profile at the time we would have loaded the memory card if none
		 * was inserted (via LoadFirstAvailableProfile). */
//		PROFILEMAN->LoadLocalProfileFromMachine( MenuI.player );
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
