#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: GhostArrowBright

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GhostArrowBright.h"
#include "PrefsManager.h"


const float  GRAY_ARROW_TWEEN_TIME = 0.5f;


GhostArrowBright::GhostArrowBright()
{
//	Load( THEME->GetPathTo(GRAPHIC_BRIGHT_GHOST_ARROW) );
	SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	TurnShadowOff();
}

void GhostArrowBright::Update( float fDeltaTime )
{
	Sprite::Update( fDeltaTime );
}

void GhostArrowBright::Step( TapNoteScore score )
{
	switch( score )
	{
	case TNS_PERFECT:	SetDiffuseColor( D3DXCOLOR(1.0f,1.0f,0.3f,1) );	break;	// yellow
	case TNS_GREAT:		SetDiffuseColor( D3DXCOLOR(0.0f,1.0f,0.4f,1) );	break;	// green
	case TNS_GOOD:		SetDiffuseColor( D3DXCOLOR(0.3f,0.8f,1.0f,1) );	break;
	case TNS_BOO:		SetDiffuseColor( D3DXCOLOR(0.8f,0.0f,0.6f,1) );	break;
	case TNS_MISS:		ASSERT( false );								break;
	}
	SetState( 0 );
	SetZoom( 1.0f );
	BeginTweening( 0.25f );
	SetTweenZoom( 1.5f );
	D3DXCOLOR colorTween = GetDiffuseColor();
	colorTween.a = 0;
	SetTweenDiffuseColor( colorTween );
}
