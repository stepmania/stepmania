#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: HoldGhostArrow

 Desc: See header

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "HoldGhostArrow.h"
#include "NoteSkinManager.h"

HoldGhostArrow::HoldGhostArrow()
{
	m_bHoldIsActive = false;
}

void HoldGhostArrow::Load( CString sNoteSkin, CString sButton, CString sElement )
{
	Sprite::Load( NOTESKIN->GetPathToFromNoteSkinAndButton(sNoteSkin, sButton, sElement) );	// not optional
	this->Command( NOTESKIN->GetMetric(sNoteSkin,"HoldGhostArrow","OnCommand") );
}

void HoldGhostArrow::Update( float fDeltaTime )
{
	Sprite::Update( fDeltaTime );
	
	m_bHoldIsActive = false;
}
