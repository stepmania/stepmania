#include "stdafx.h"
//
// GhostArrow.cpp: implementation of the GhostArrow class.
//
//////////////////////////////////////////////////////////////////////

#include "GhostArrow.h"


const CString GHOST_ARROW_TEXTURE = "Textures\\Gray Arrow 1x2.png";
const float  GRAY_ARROW_TWEEN_TIME = 0.5f;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GhostArrow::GhostArrow()
{
	LoadFromTexture( GHOST_ARROW_TEXTURE );
	SetState( 1 );
	SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	TurnShadowOff();
}

void GhostArrow::SetBeat( const float fSongBeat )
{
	//SetState( fmod(fSongBeat,1)<0.25 ? 1 : 0 );
}

void GhostArrow::Step( StepScore score )
{
	switch( score )
	{
	case perfect:	SetDiffuseColor( D3DXCOLOR(1.0f,1.0f,0.3f,0.7f) );	break;	// yellow
	case great:		SetDiffuseColor( D3DXCOLOR(0.0f,1.0f,0.4f,0.7f) );	break;	// green
	case good:		SetDiffuseColor( D3DXCOLOR(0.3f,0.8f,1.0f,0.7f) );	break;
	case boo:		SetDiffuseColor( D3DXCOLOR(0.8f,0.0f,0.6f,0.7f) );	break;
	case miss:		ASSERT(true);	break;
	}
	SetZoom( 1.0f );
	BeginTweening( 0.3f );
	SetTweenZoom( 1.5f );
	D3DXCOLOR colorTween = GetDiffuseColor();
	colorTween.a = 0;
	SetTweenDiffuseColor( colorTween );

}
