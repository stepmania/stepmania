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
	virtual void MenuUp(	const PlayerNumber p, const InputEventType type )	{ MenuUp(p); };
	virtual void MenuDown(	const PlayerNumber p, const InputEventType type )	{ MenuDown(p); };
	virtual void MenuLeft(	const PlayerNumber p, const InputEventType type )	{ MenuLeft(p); };
	virtual void MenuRight( const PlayerNumber p, const InputEventType type )	{ MenuRight(p); };
	virtual void MenuStart( const PlayerNumber p, const InputEventType type )	{ MenuStart(p); };
	virtual void MenuBack(	const PlayerNumber p, const InputEventType type )	{ MenuBack(p); };

	virtual void MenuUp(	const PlayerNumber p )	{};
	virtual void MenuDown(	const PlayerNumber p )	{};
	virtual void MenuLeft(	const PlayerNumber p )	{};
	virtual void MenuRight( const PlayerNumber p )	{};
	virtual void MenuStart( const PlayerNumber p )	{};
	virtual void MenuBack(	const PlayerNumber p )	{};

};
