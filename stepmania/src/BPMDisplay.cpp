#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: BPMDisplay.h

 Desc: A graphic displayed in the BPMDisplay during Dancing.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "BPMDisplay.h"
#include "RageUtil.h"
#include "ScreenDimensions.h"



BPMDisplay::BPMDisplay()
{
	m_fCurrentBPM = m_fLowBPM = m_fHighBPM = 0;
	m_CountingState = holding_down;
	m_fTimeLeftInState = 0;

	m_rectFrame.SetDiffuseColor( D3DXCOLOR(0,0,0,0.3f) );
	m_rectFrame.SetZoomX( 120 );
	m_rectFrame.SetZoomY( 40 );

	m_seqBPM.LoadFromSequenceFile( "SpriteSequences\\LED Numbers.seq" );
	m_seqBPM.SetXY( CENTER_X, SCREEN_HEIGHT - 50 );
	//m_seqBPM.SetSequence( ssprintf("999") );
	m_seqBPM.SetXY( -30, 0 );
	m_seqBPM.SetZoom( 0.7f );
	m_seqBPM.SetDiffuseColorTopEdge( D3DXCOLOR(1,1,0,1) );	// yellow
	m_seqBPM.SetDiffuseColorBottomEdge( D3DXCOLOR(1,0.5f,0,1) );	// orange

	m_textLabel.LoadFromFontName( "Black Wolf" );
	m_textLabel.SetXY( 54, 10 );
	m_textLabel.SetText( "BPM" );
	m_textLabel.SetDiffuseColorTopEdge( D3DXCOLOR(1,1,0,1) );	// yellow
	m_textLabel.SetDiffuseColorBottomEdge( D3DXCOLOR(1,0.5f,0,1) );	// orange

	//this->AddActor( &m_rectFrame );
	this->AddActor( &m_seqBPM );
	this->AddActor( &m_textLabel );
}


void BPMDisplay::Update( float fDeltaTime ) 
{ 
	ActorFrame::Update( fDeltaTime ); 
	
	m_fTimeLeftInState -= fDeltaTime;
	if( m_fTimeLeftInState < 0 )
	{
		// go to next state
		switch( m_CountingState )
		{
		case counting_up:	m_CountingState = holding_up;		break;
		case holding_up:	m_CountingState = counting_down;	break;
		case counting_down:	m_CountingState = holding_down;		break;
		case holding_down:	m_CountingState = counting_up;		break;
		}
		m_fTimeLeftInState = 1;		// reset timer
	}

	switch( m_CountingState )
	{
	case counting_down:	m_fCurrentBPM = m_fLowBPM + (m_fHighBPM-m_fLowBPM)*m_fTimeLeftInState;	break;
	case counting_up:	m_fCurrentBPM = m_fHighBPM + (m_fLowBPM-m_fHighBPM)*m_fTimeLeftInState;	break;
	case holding_up:	m_fCurrentBPM = m_fHighBPM;												break;
	case holding_down:	m_fCurrentBPM = m_fLowBPM;												break;
	}
	m_seqBPM.SetSequence( ssprintf("%03.0f", m_fCurrentBPM) ); 
}


void BPMDisplay::SetBPMRange( float fLowBPM, float fHighBPM )
{
	m_fLowBPM = fLowBPM;
	m_fHighBPM = fHighBPM;
	if( m_fCurrentBPM > m_fHighBPM )
		m_CountingState = counting_down;
	else
		m_CountingState = counting_up;
	m_fTimeLeftInState = 1;

	if( m_fLowBPM != m_fHighBPM )
	{
		m_seqBPM.BeginTweening(0.5f);
		m_seqBPM.SetTweenDiffuseColorTopEdge( D3DXCOLOR(1,0,0,1) );			// red
		m_seqBPM.SetTweenDiffuseColorBottomEdge( D3DXCOLOR(0.6f,0,0,1) );	// dark red
		m_textLabel.BeginTweening(0.5f);
		m_textLabel.SetTweenDiffuseColorTopEdge( D3DXCOLOR(1,0,0,1) );			// red
		m_textLabel.SetTweenDiffuseColorBottomEdge( D3DXCOLOR(0.6f,0,0,1) );	// dark red
	}
	else
	{
		m_seqBPM.BeginTweening(0.5f);
		m_seqBPM.SetTweenDiffuseColorTopEdge( D3DXCOLOR(1,1,0,1) );			// yellow
		m_seqBPM.SetTweenDiffuseColorBottomEdge( D3DXCOLOR(1,0.5f,0,1) );	// orange
		m_textLabel.BeginTweening(0.5f);
		m_textLabel.SetTweenDiffuseColorTopEdge( D3DXCOLOR(1,1,0,1) );			// yellow
		m_textLabel.SetTweenDiffuseColorBottomEdge( D3DXCOLOR(1,0.5f,0,1) );	// orange
	}

}