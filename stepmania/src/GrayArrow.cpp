#include "stdafx.h"
//
// GrayArrow.cpp: implementation of the GrayArrow class.
//
//////////////////////////////////////////////////////////////////////

#include "GrayArrow.h"


const CString GRAY_ARROW_TEXTURE = "Textures\\Gray Arrow 1x2.png";
const float GRAY_ARROW_POP_UP_TIME			= 0.3f;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GrayArrow::GrayArrow()
{
	LoadFromTexture( GRAY_ARROW_TEXTURE );
	StopAnimating();
}

void GrayArrow::SetBeat( const float fSongBeat )
{
	SetState( fmod(fSongBeat,1)<0.25 ? 1 : 0 );
}

void GrayArrow::Step( StepScore score )
{
	SetZoom( 0.50 );
	BeginTweening( GRAY_ARROW_POP_UP_TIME );
	SetTweenZoom( 1.0f );
}
