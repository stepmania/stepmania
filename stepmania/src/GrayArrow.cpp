#include "stdafx.h"
//
// GrayArrow.cpp: implementation of the GrayArrow class.
//
//////////////////////////////////////////////////////////////////////

#include "GrayArrow.h"
#include "ThemeManager.h"

const float GRAY_ARROW_POP_UP_TIME			= 0.3f;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GrayArrow::GrayArrow()
{
	Load( THEME->GetPathTo(GRAPHIC_GRAY_ARROW) );
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
