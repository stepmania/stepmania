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
#include "PrefsManager.h"
#include "RageException.h"
#include "RageTimer.h"
#include "NoteSkinManager.h"
#include "GameState.h"

HoldGhostArrow::HoldGhostArrow()
{
	m_bHoldIsActive = false;
}

void HoldGhostArrow::Load( CString sNoteSkin, CString sButton, CString sElement )
{
	Sprite::Load( NOTESKIN->GetPathToFromNoteSkinAndButton(sNoteSkin, sButton, sElement) );	// not optional
	m_sOnCommand = NOTESKIN->GetMetric(sNoteSkin,"HoldGhostArrow","OnCommand");
	this->Command( m_sOnCommand );
}

void HoldGhostArrow::Update( float fDeltaTime )
{
	Sprite::Update( fDeltaTime );
	
	m_bHoldIsActive = false;
}

void HoldGhostArrow::DrawPrimitives()
{
	if( !m_bHoldIsActive )
		return;

	Sprite::DrawPrimitives();
}
