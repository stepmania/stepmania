/*
-----------------------------------------------------------------------------
 Class: ScreenGameOver

 Desc: Shows the stage number.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"

class Sprite;
class TransitionFadeWipe;


class ScreenGameOver : public Screen
{
public:
	ScreenGameOver();

	~ScreenGameOver();

	virtual void HandleScreenMessage( const ScreenMessage SM );

private:
	
	Sprite*			m_psprGameOver;

	TransitionFadeWipe*		m_pWipe;
};


