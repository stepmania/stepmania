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

#include "Actor.h"
#include "Sprite.h"
#include "Quad.h"


class MeterDisplay : public Actor
{
public:
	MeterDisplay();
	void Load( CString sSteamPath, float fStreamWidth );
	virtual void Update( float fDelta );
	virtual void DrawPrimitives();

	void SetPercent( float fPercent );


	float	m_fStreamWidth;
	float	m_fPercent;
	Sprite  m_sprStream;
};

#endif
