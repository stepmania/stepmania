/*
-----------------------------------------------------------------------------
 Class: GameManager

 Desc: Manages "Game" and "StyleDef", and "Instrument" definitions, which define
	different ways of playing - like "dance" and "pump".

 Copyright (c) 2001-2002 by the names listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#ifndef _GameManager_H_
#define _GameManager_H_

#include "StyleDef.h"

class GameManager
{
public:
	GameManager();
	~GameManager();

	DanceStyle		m_DanceStyle;			// the current style


	bool IsPlayerEnabled( PlayerNumber PlayerNo );

	StyleDef GetStyleDef( DanceStyle mode );
	StyleDef GetCurrentStyleDef()	{ return GetStyleDef(m_DanceStyle); };

protected:

};


extern GameManager*	GAME;	// global and accessable from anywhere in our program


#endif