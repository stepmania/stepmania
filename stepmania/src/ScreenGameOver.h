/*
-----------------------------------------------------------------------------
 Class: ScreenGameOver

 Desc: Shows the stage number.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Sprite.h"


class ScreenGameOver : public Screen
{
public:
	ScreenGameOver();

	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual void MenuStart( const PlayerNumber p );

private:

	bool m_bClosing;
	Sprite			m_sprGameOver;
};


