/*
-----------------------------------------------------------------------------
 Class: ScreenStage

 Desc: Shows the stage number.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"

class BitmapText;
class Sprite;
class TransitionFadeWipe;


class ScreenStage : public Screen
{
public:
	ScreenStage();
	ScreenStage( bool bTryExtraStage );
	~ScreenStage();

	virtual void HandleScreenMessage( const ScreenMessage SM );

private:
	bool			m_bTryExtraStage;

	BitmapText*		m_ptextStage;
	
	Sprite*			m_psprUnderscore;

	TransitionFadeWipe*		m_pWipe;
};


