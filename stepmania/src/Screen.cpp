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

Screen::Screen()
{
}

Screen::~Screen()
{

}

void Screen::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );


	// update the times of queued ScreenMessages and send if timer has expired
	// The order you remove messages in must be very careful!  Sending a message can 
	// potentially clear all m_QueuedMessages, and set a new state!
	for( int i=0; i<m_QueuedMessages.GetSize(); i++ )
	{
		if( m_QueuedMessages[i].fDelayRemaining <= 0.0f )		// send this sucker!
		{
			this->HandleScreenMessage( m_QueuedMessages[i].SM );
			m_QueuedMessages.RemoveAt( i );
			i--;
		}
		else
		{
			m_QueuedMessages[i].fDelayRemaining -= fDeltaTime;
		}
	}
}

void Screen::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
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
	}
}


void Screen::SendScreenMessage( ScreenMessage SM, float fDelay )
{
	assert( fDelay >= 0.0 );

	QueuedScreenMessage QSM;
	QSM.SM = SM;
	QSM.fDelayRemaining = fDelay;
	m_QueuedMessages.Add( QSM );
}