#include "stdafx.h"
//
// GhostArrow.cpp: implementation of the GhostArrow class.
//
//////////////////////////////////////////////////////////////////////

#include "GhostArrow.h"


const CString GHOST_ARROW_SPRITE = "Sprites\\Ghost Arrow.sprite";
const float  GRAY_ARROW_TWEEN_TIME = 0.5f;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GhostArrow::GhostArrow()
{
	LoadFromSpriteFile( GHOST_ARROW_SPRITE );
	SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
}

void GhostArrow::SetBeat( const float fSongBeat )
{
	//SetState( fmod(fSongBeat,1)<0.25 ? 1 : 0 );
}

void GhostArrow::Step( StepScore score )
{
	switch( score )
	{
	case perfect:	SetDiffuseColor( D3DXCOLOR(1.0f,1.0f,0.3f,1.0f) );	break;	// yellow
	case great:		SetDiffuseColor( D3DXCOLOR(0.0f,1.0f,0.4f,1.0f) );	break;	// green
	case good:		SetDiffuseColor( D3DXCOLOR(0.3f,0.8f,1.0f,1.0f) );	break;
	case boo:		SetDiffuseColor( D3DXCOLOR(0.8f,0.0f,0.6f,1.0f) );	break;
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
