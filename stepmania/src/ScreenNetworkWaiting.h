/*
-----------------------------------------------------------------------------
 Class: ScreenNetworkWaiting

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "SongSelector.h"
#include "BitmapText.h"
#include "TransitionFade.h"
#include "RandomSample.h"

class ScreenNetworkWaiting : public Screen
{
public:
	ScreenNetworkWaiting();
	virtual ~ScreenNetworkWaiting();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual void MenuUp( PlayerNumber pn );
	virtual void MenuDown( PlayerNumber pn );

private:

	MenuElements	m_Menu;

	BitmapText		m_textServerInfo;
	BitmapText		m_textPlayerList;
	bool			m_bReady;
	float			m_fUpdateTimer;
};



