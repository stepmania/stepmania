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

	// zero alpha for the center. don't show it.
	m_sprCenter.SetDiffuseColor( D3DXCOLOR(0.4f,0.4f,0.4f,0) );
}


void GrayArrow::Step()
{
	m_sprOutline.SetZoom( 0.50 );
	m_sprOutline.BeginTweening( GRAY_ARROW_POP_UP_TIME );
	m_sprOutline.SetTweenZoom( 1.0f );

	m_sprMidSection.SetZoom( 0.50 );
	m_sprMidSection.BeginTweening( GRAY_ARROW_POP_UP_TIME );
	m_sprMidSection.SetTweenZoom( 1.0f );

	//m_sprCenter.SetZoom( 0.50 );
	//m_sprCenter.BeginTweening( GRAY_ARROW_POP_UP_TIME );
	//m_sprCenter.SetTweenZoom( 1.0f );
}

void GrayArrow::CalculateColor( const float fBeatsTilStep )
{
	float fOutline = 0.0f;
	float fMidSection = 0.0f;
	float fAlpha = 0.8f;


	// get a float between 0 and 2
	// this will determine what shade you are
	float fStage = fBeatsTilStep * 2.0f;
	while( fStage >= 2.0f )
		fStage -= 2.0f;

	// less than 1, we are in the first stage, gettin darker
	if( fStage < 1.0 )
	{
		fOutline = 1.0f - fStage;
		fMidSection = fStage;
	}
	// second stage, getting lighter again.
	else 
	{
		fOutline = fBeatsTilStep - 1.0f;
		fMidSection = 2.0f - fStage;
	}

	// shift the level a little so we never have entirely black or white.
	fOutline *= 0.5f;
	fOutline += 0.25f;
	fMidSection *= 0.4f;
	fMidSection += 0.4f;

	m_sprOutline.SetDiffuseColor( 
		D3DXCOLOR( fOutline,fOutline,fOutline,fAlpha ) );
	m_sprMidSection.SetDiffuseColor( 
		D3DXCOLOR( fMidSection,fMidSection,fMidSection,fAlpha ) );
}
