#ifndef PERCENTAGE_DISPLAY_H
#define PERCENTAGE_DISPLAY_H

#include "ActorFrame.h"
#include "PlayerNumber.h"
#include "BitmapText.h"
#include "StageStats.h"

class PercentageDisplay: public ActorFrame
{
public:
	PercentageDisplay();
	void Load( PlayerNumber pn, StageStats *pSource, bool bAutoRefresh );
	void Update( float fDeltaTime );
	void TweenOffScreen();

private:
	void Refresh();
	PlayerNumber m_PlayerNumber;
	StageStats *m_pSource;
	bool m_bAutoRefresh;
	int m_Last;
	BitmapText	m_textPercent;
	BitmapText	m_textPercentRemainder;
};

#endif
/*
 * Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
 *  Chris Danford
 */
