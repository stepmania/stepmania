#ifndef ReceptorArrow_H
#define ReceptorArrow_H
/*
-----------------------------------------------------------------------------
 Class: ReceptorArrow

 Desc: A gray arrow that "receives" the colorful note arrows.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ActorFrame.h"
#include "ActorUtil.h"
#include "PlayerNumber.h"

class ReceptorArrow : public ActorFrame
{
public:
	ReceptorArrow();
	bool Load( CString NoteSkin, PlayerNumber pn, int iColNo );

	virtual void DrawPrimitives();
	virtual void Update( float fDeltaTime );
	void Step();
	void SetPressed() { m_bIsPressed = true; };
private:

	PlayerNumber m_PlayerNumber;
	int m_iColNo;

	AutoActor m_pReceptorWaiting;
	AutoActor m_pReceptorGo;
	CString m_sStepCommand;
	
	AutoActor m_pPressBlock;
	bool m_bIsPressed;
};

#endif 
