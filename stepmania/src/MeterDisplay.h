#ifndef MeterDisplay_H
#define MeterDisplay_H
/*
-----------------------------------------------------------------------------
 File: MeterDisplay.h

 Desc: 

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "Sprite.h"
#include "ActorUtil.h"


class MeterDisplay : public ActorFrame
{
public:
	MeterDisplay();
	void Load( CString sStreamPath, float fStreamWidth, CString sTipPath = "" );

	void SetPercent( float fPercent );

private:
	float	m_fStreamWidth;
	float	m_fPercent;
	Sprite  m_sprStream;
	AutoActor  m_sprTip;
};

#endif
