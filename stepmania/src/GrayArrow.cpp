// GrayArrow.cpp: implementation of the GrayArrow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GrayArrow.h"


const float GRAY_ARROW_POP_UP_TIME			= 0.30f;



//////////////////////////////////////////////////////////////////////
// Construction/Destruction

GrayArrow::GrayArrow()
{
	Arrow::Arrow();

	m_sprOutline.SetDiffuseColor( D3DXCOLOR(0.6f,0.6f,0.6f,1) );
	m_sprMidSection.SetDiffuseColor( D3DXCOLOR(0.3f,0.3f,0.3f,1) );
	m_sprCenter.SetDiffuseColor( D3DXCOLOR(0.4f,0.4f,0.4f,1) );
}


void GrayArrow::Step()
{
	m_sprOutline.SetZoom( 0.50 );
	m_sprOutline.BeginTweening( GRAY_ARROW_POP_UP_TIME );
	m_sprOutline.SetTweenZoom( 1.0f );

	m_sprMidSection.SetZoom( 0.50 );
	m_sprMidSection.BeginTweening( GRAY_ARROW_POP_UP_TIME );
	m_sprMidSection.SetTweenZoom( 1.0f );

	m_sprCenter.SetZoom( 0.50 );
	m_sprCenter.BeginTweening( GRAY_ARROW_POP_UP_TIME );
	m_sprCenter.SetTweenZoom( 1.0f );
}
