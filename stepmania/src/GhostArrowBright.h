#pragma once
/*
-----------------------------------------------------------------------------
 Class: GhostArrowBright

 Desc: Ghost arrow used when over 100 combo.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"
#include "GameConstantsAndTypes.h"

class GhostArrowBright : public Sprite
{
public:
	GhostArrowBright();

	virtual void  Update( float fDeltaTime );

	void  Step( TapNoteScore score );

protected:
	float m_fPopUpSeconds;
	float m_fZoomStart, m_fZoomEnd;
	D3DXCOLOR m_colorPerfect, m_colorGreat, m_colorGood, m_colorBoo;
};
