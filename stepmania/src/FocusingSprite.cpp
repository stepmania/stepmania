#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: FocusingSprite.h

 Desc: A graphic that appears to blur and come into focus.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "FocusingSprite.h"
#include "RageUtil.h"



#define BLUR_DISTANCE	50

#define BLUR_TWEEN_TIME	0.5f



FocusingSprite::FocusingSprite()
{
	for( int i=0; i<3; i++ )
	{
		this->AddSubActor( &m_sprites[i] );
	}
	
	m_fPercentBlurred = 1.0f;
	m_BlurState = invisible;
}

void FocusingSprite::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );


	switch( m_BlurState )
	{
	case focused:
		break;
	case focusing:
		m_fPercentBlurred -= fDeltaTime / BLUR_TWEEN_TIME;
		if( m_fPercentBlurred <= 0.0f ) {
			m_fPercentBlurred = 0.0f;
			m_BlurState = focused;
		}
		break;
	case invisible:
		break;
	case blurring:
		m_fPercentBlurred += fDeltaTime / BLUR_TWEEN_TIME;
		if( m_fPercentBlurred >= 1.0f ) {
			m_fPercentBlurred = 1.0f;
			m_BlurState = invisible;
		}
		break;
	}

}

void FocusingSprite::DrawPrimitives()
{
	if( m_BlurState != invisible )
	{
		SetDiffuseColor( D3DXCOLOR(1,1,1,0.5f-m_fPercentBlurred/2) );

		m_sprites[0].SetXY( m_fPercentBlurred*BLUR_DISTANCE*2, m_fPercentBlurred*BLUR_DISTANCE );
		m_sprites[1].SetXY( -m_fPercentBlurred*BLUR_DISTANCE*2, -m_fPercentBlurred*BLUR_DISTANCE );
		m_sprites[2].SetXY( 0, 0 );

		ActorFrame::DrawPrimitives();
	}

}


void FocusingSprite::StartFocusing()
{
	m_fPercentBlurred = 1;
	m_BlurState = focusing;
}

void FocusingSprite::StartBlurring()
{
	m_fPercentBlurred = 0;
	m_BlurState = blurring;
}
