/* Screen - Class representing a game state.  It also holds Actors. */

#ifndef SCREEN_H
#define SCREEN_H

#include "ActorFrame.h"
#include "ScreenMessage.h"
#include "InputFilter.h"
#include "GameInput.h"
#include "MenuInput.h"
#include "StyleInput.h"


class Screen : public ActorFrame
{
public:
	Screen( CString sName );	// enforce that all screens have m_sName filled in
	virtual ~Screen();

	virtual void Update( float fDeltaTime );
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void PostScreenMessage( const ScreenMessage SM, const float fDelay );
	void ClearMessageQueue();
	void ClearMessageQueue( const ScreenMessage SM );	// clear of a specific SM

	bool IsTransparent() const { return m_bIsTransparent; }

	static Screen* Create( CString sClassName );
	static bool ChangeCoinModeInput( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );	// return true if CoinMode changed
	static bool JoinInput( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );	// return true if a player joined

protected:
	// structure for holding messages sent to a Screen
	struct QueuedScreenMessage {
		ScreenMessage SM;  
		float fDelayRemaining;
	};
	vector<QueuedScreenMessage>	m_QueuedMessages;
	static bool SortMessagesByDelayRemaining(const QueuedScreenMessage &m1, const QueuedScreenMessage &m2);

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

/*
 * (c) 2001-2004 Chris Danford
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
