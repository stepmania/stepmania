// GhostArrow.cpp: implementation of the GhostArrow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GhostArrow.h"




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GhostArrow::GhostArrow()
{
	Arrow::Arrow();

	m_sprOutline.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	m_sprMidSection.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	m_sprCenter.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
}


void GhostArrow::Step()
{
	m_sprOutline.SetZoom( 1 );
	m_sprOutline.SetDiffuseColor( D3DXCOLOR(1,1,0.5f,1) );
	m_sprOutline.BeginTweening( 0.3f );
	m_sprOutline.SetTweenZoom( 1.5 );
	m_sprOutline.SetTweenDiffuseColor( D3DXCOLOR(1,1,0.5f,0) );		

	m_sprMidSection.SetZoom( 1 );
	m_sprMidSection.SetDiffuseColor( D3DXCOLOR(1,1,0.5f,1) );
	m_sprMidSection.BeginTweening( 0.3f );
	m_sprMidSection.SetTweenZoom( 1.5 );
	m_sprMidSection.SetTweenDiffuseColor( D3DXCOLOR(1,1,0.5f,0) );		

	m_sprCenter.SetZoom( 1 );
	m_sprCenter.SetDiffuseColor( D3DXCOLOR(1,1,0.5f,1) );
	m_sprCenter.BeginTweening( 0.3f );
	m_sprCenter.SetTweenZoom( 1.5 );
	m_sprCenter.SetTweenDiffuseColor( D3DXCOLOR(1,1,0.5f,0) );		
}