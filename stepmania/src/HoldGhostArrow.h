#ifndef HOLDGHOSTARROW_H
#define HOLDGHOSTARROW_H
/*
-----------------------------------------------------------------------------
 Class: HoldGhostArrow

 Desc: The graphic shown while holding a HoldNote.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"

class HoldGhostArrow : public Sprite
{
public:
	HoldGhostArrow();

	virtual void Load( CString sNoteSkin, CString sButton, CString sElement );

	virtual void Update( float fDeltaTime );
	virtual bool EarlyAbortDraw() { return !m_bHoldIsActive; }

	void  SetHoldIsActive( bool bHoldIsActive ) { m_bHoldIsActive = bHoldIsActive; }

private:
	bool m_bHoldIsActive;
};


#endif
