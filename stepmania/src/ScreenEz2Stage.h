/*
-----------------------------------------------------------------------------
 Class: ScreenEz2Stage

 Desc: Shows the stage number.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Andrew Livy
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "BitmapText.h"


class ScreenEz2Stage : public Screen
{
public:
	ScreenEz2Stage();

	virtual void HandleScreenMessage( const ScreenMessage SM );

private:
	bool			m_bTryExtraStage;

	BitmapText		m_textStage;
	BitmapText		m_textStageString;
	BitmapText		m_blobs_a[20];
	BitmapText		m_blobs_b[20];
	BitmapText		m_ez2ukm[2];
	Screen*			m_pNextScreen;
};


