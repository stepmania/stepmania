#ifndef SCREEN_H
#define SCREEN_H
/*
-----------------------------------------------------------------------------
 Class: Screen

 Desc: Class representing a game state.  It also holds Actors.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

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

	virtual void AddChild( Actor* pActor );

	// let subclass override if they want
	virtual void Restore() {};
	virtual void Invalidate() {};

	virtual void Update( float fDeltaTime );
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM ) {};

	void SendScreenMessage( const ScreenMessage SM, const float fDelay );
	void ClearMessageQueue();
	void ClearMessageQueue( const ScreenMessage SM );	// clear of a specific SM

	bool IsTransparent() { return m_bIsTransparent; }

protected:
	// structure for holding messages sent to a Screen
	struct QueuedScreenMessage {
		ScreenMessage SM;  
		float fDelayRemaining;
	};
	vector<QueuedScreenMessage>	m_QueuedMessages;

	bool m_bIsTransparent;	// screens below us need to be drawn first

public:

	// let subclass override if they want
	virtual void MenuUp(	PlayerNumber pn, const InputEventType type )	{ if(type==IET_FIRST_PRESS) MenuUp(pn); }
	virtual void MenuDown(	PlayerNumber pn, const InputEventType type )	{ if(type==IET_FIRST_PRESS) MenuDown(pn); }
	virtual void MenuLeft(	PlayerNumber pn, const InputEventType type )	{ if(type==IET_FIRST_PRESS) MenuLeft(pn); }
	virtual void MenuRight( PlayerNumber pn, const InputEventType type )	{ if(type==IET_FIRST_PRESS) MenuRight(pn); }
	virtual void MenuStart( PlayerNumber pn, const InputEventType type )	{ if(type==IET_FIRST_PRESS) MenuStart(pn); }
	virtual void MenuBack(	PlayerNumber pn, const InputEventType type );
	virtual void MenuCoin(	PlayerNumber pn, const InputEventType type )	{ if(type==IET_FIRST_PRESS) MenuCoin(pn); }

	virtual void MenuUp(	PlayerNumber pn )	{}
	virtual void MenuDown(	PlayerNumber pn )	{}
	virtual void MenuLeft(	PlayerNumber pn )	{}
	virtual void MenuRight( PlayerNumber pn )	{}
	virtual void MenuStart( PlayerNumber pn )	{}
	virtual void MenuBack(	PlayerNumber pn )	{}
	virtual void MenuCoin(	PlayerNumber pn );

private:
	bool m_FirstUpdate;
};

#endif
