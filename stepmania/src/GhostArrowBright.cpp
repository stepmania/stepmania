#include "stdafx.h"
//
// GhostArrowBright.cpp: implementation of the GhostArrowBright class.
//
//////////////////////////////////////////////////////////////////////

#include "GhostArrowBright.h"
#include "ThemeManager.h"


const float  GRAY_ARROW_TWEEN_TIME = 0.5f;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GhostArrowBright::GhostArrowBright()
{
//	Load( THEME->GetPathTo(GRAPHIC_BRIGHT_GHOST_ARROW) );
	SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	TurnShadowOff();
}

void GhostArrowBright::SetBeat( const float fSongBeat )
{
	//SetState( fmodf(fSongBeat,1)<0.25 ? 1 : 0 );
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
