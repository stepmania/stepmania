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
	MeterDisplay(
		CString sSteamPath,
		CString sFramePath,
		float fStreamWidth );
	virtual ~MeterDisplay();
	virtual void Update( float fDelta );
	virtual void DrawPrimitives();

	void SetPercent( float fPercent );

protected:

	float	m_fStreamWidth;
	float	m_fPercent;
	float	m_fTrailingPercent;
	Sprite m_sprStream;
	Quad	m_quad;
	Sprite m_sprFrame;
};

#endif
