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
	Load( THEME->GetPathTo(GRAPHIC_GHOST_ARROW) );
	SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	TurnShadowOff();
}

void GhostArrowBright::SetBeat( const float fSongBeat )
{
	//SetState( fmod(fSongBeat,1)<0.25 ? 1 : 0 );
}

void GhostArrowBright::Step( StepScore score )
{
	switch( score )
	{
	case perfect:	SetDiffuseColor( D3DXCOLOR(1.0f,1.0f,0.3f,0.9f) );	break;	// yellow
	case great:		SetDiffuseColor( D3DXCOLOR(0.0f,1.0f,0.4f,0.9f) );	break;	// green
	case good:		SetDiffuseColor( D3DXCOLOR(0.3f,0.8f,1.0f,0.9f) );	break;
	case boo:		SetDiffuseColor( D3DXCOLOR(0.8f,0.0f,0.6f,0.9f) );	break;
	case miss:		ASSERT(true);	break;
	}
	SetState( 0 );
	SetZoom( 1.2f );
	BeginTweening( 0.3f );
	SetTweenZoom( 2.5f );
	D3DXCOLOR colorTween = GetDiffuseColor();
	colorTween.a = 0;
	SetTweenDiffuseColor( colorTween );

}
