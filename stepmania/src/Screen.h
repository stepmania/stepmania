#pragma once
/*
-----------------------------------------------------------------------------
 Class: Screen

 Desc: Class representing a game state.  It also holds Actors.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageInput.h"
#include "RageDisplay.h"
#include "PrefsManager.h"
#include "ActorFrame.h"
#include "ScreenMessage.h"
#include "InputFilter.h"
#include "GameInput.h"
#include "MenuInput.h"
#include "StyleInput.h"


class Screen : public ActorFrame
{
public:
	Screen();
	virtual ~Screen();

	// let subclass override if they want
	virtual void Restore() {};
	virtual void Invalidate() {};

	virtual void Update( float fDeltaTime );
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM ) {};

	void SendScreenMessage( const ScreenMessage SM, const float fDelay );
	void ClearMessageQueue() { m_QueuedMessages.SetSize(0,5); };

protected:

	// structure for holding messages sent to a Screen
	struct QueuedScreenMessage {
		ScreenMessage SM;  
		float fDelayRemaining;
	};
	CArray<QueuedScreenMessage, QueuedScreenMessage&>	m_QueuedMessages;

public:

	// let subclass override if they want
	virtual void MenuUp(	PlayerNumber p, const InputEventType type )	{ if(type==IET_FIRST_PRESS) MenuUp(p); }
	virtual void MenuDown(	PlayerNumber p, const InputEventType type )	{ if(type==IET_FIRST_PRESS) MenuDown(p); }
	virtual void MenuLeft(	PlayerNumber p, const InputEventType type )	{ if(type==IET_FIRST_PRESS) MenuLeft(p); }
	virtual void MenuRight( PlayerNumber p, const InputEventType type )	{ if(type==IET_FIRST_PRESS) MenuRight(p); }
	virtual void MenuStart( PlayerNumber p, const InputEventType type )	{ if(type==IET_FIRST_PRESS) MenuStart(p); }
	virtual void MenuBack(	PlayerNumber p, const InputEventType type );

	virtual void MenuUp(	PlayerNumber p )	{};
	virtual void MenuDown(	PlayerNumber p )	{};
	virtual void MenuLeft(	PlayerNumber p )	{};
	virtual void MenuRight( PlayerNumber p )	{};
	virtual void MenuStart( PlayerNumber p )	{};
	virtual void MenuBack(	PlayerNumber p )	{};
};
