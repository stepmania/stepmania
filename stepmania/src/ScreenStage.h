/*
-----------------------------------------------------------------------------
 Class: ScreenStage

 Desc: Shows the stage number.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "BitmapText.h"


class ScreenStage : public Screen
{
public:
	ScreenStage();
	ScreenStage( bool bTryExtraStage );

	virtual void HandleScreenMessage( const ScreenMessage SM );

private:
	bool			m_bTryExtraStage;

	BitmapText		m_textStage;
};


