#include "stdafx.h"
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
