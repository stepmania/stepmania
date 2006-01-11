#ifndef ScreenJoinMultiplayer_H
#define ScreenJoinMultiplayer_H

#include "ScreenWithMenuElements.h"
#include "RageSound.h"
#include "ControllerStateDisplay.h"

enum MultiPlayerStatus
{
	MultiPlayerStatus_Joined,
	MultiPlayerStatus_NotJoined,
	MultiPlayerStatus_Unplugged,
	MultiPlayerStatus_MissingMultitap,
	NUM_MultiPlayerStatus,
	MultiPlayerStatus_INVALID
};
const CString& MultiPlayerStatusToString( MultiPlayerStatus i );


class ScreenJoinMultiplayer : public ScreenWithMenuElements
{
public:
	ScreenJoinMultiplayer( CString sName );
	virtual void Init();

	virtual void Input( const InputEventPlus &input );
	virtual void HandleScreenMessage( const ScreenMessage SM );
	virtual void Update(float f);
	virtual void DrawPrimitives();

protected:
	void UpdatePlayerStatus( bool bFirstUpdate );
	void PositionItem( Actor *pActor, int iItemIndex, int iNumItems );

	virtual void MenuBack( PlayerNumber pn );
	virtual void MenuStart( PlayerNumber pn );

	InputDeviceState m_InputDeviceState[NUM_MultiPlayer];
	MultiPlayerStatus m_MultiPlayerStatus[NUM_MultiPlayer];

	AutoActor m_sprPlayer[NUM_MultiPlayer];
	ControllerStateDisplay m_ControllerState[NUM_MultiPlayer];

	LuaExpression m_exprOnCommandFunction;	// params: self,itemIndex,numItems

	RageSound	m_soundPlugIn;
	RageSound	m_soundUnplug;
	RageSound	m_soundJoin;
	RageSound	m_soundUnjoin;
};

#endif

/*
 * (c) 2005 Chris Danford
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

