#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: BPMDisplay.h

 Desc: A graphic displayed in the BPMDisplay during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "BPMDisplay.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameState.h"


#define NORMAL_COLOR_TOP		THEME->GetMetricC("BPMDisplay","NormalColorTop")
#define NORMAL_COLOR_BOTTOM		THEME->GetMetricC("BPMDisplay","NormalColorBottom")
#define CHANGE_COLOR_TOP		THEME->GetMetricC("BPMDisplay","ChangeColorTop")
#define CHANGE_COLOR_BOTTOM		THEME->GetMetricC("BPMDisplay","ChangeColorBottom")
#define EXTRA_COLOR_TOP			THEME->GetMetricC("BPMDisplay","ExtraColorTop")
#define EXTRA_COLOR_BOTTOM		THEME->GetMetricC("BPMDisplay","ExtraColorBottom")


BPMDisplay::BPMDisplay()
{
	m_fCurrentBPM = m_fLowBPM = m_fHighBPM = 0;
	m_CountingState = holding_down;
	m_fTimeLeftInState = 0;
	m_bExtraStage = GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2();

	m_rectFrame.SetDiffuseColor( D3DXCOLOR(0,0,0,0.3f) );
	m_rectFrame.SetZoomX( 120 );
	m_rectFrame.SetZoomY( 40 );

	m_textBPM.LoadFromFont( THEME->GetPathTo("Fonts","header1") );
	m_textBPM.TurnShadowOff();
	m_textBPM.SetXY( CENTER_X, SCREEN_HEIGHT - 50 );
	//m_textBPM.SetSequence( ssprintf("999") );
	m_textBPM.SetXY( -23, 0 );
	m_textBPM.SetZoom( 1.0f );
	m_textBPM.SetDiffuseColorTopEdge( NORMAL_COLOR_TOP );	// yellow
	m_textBPM.SetDiffuseColorBottomEdge( NORMAL_COLOR_BOTTOM );	// orange

	m_textLabel.LoadFromFont( THEME->GetPathTo("Fonts","header1") );
	m_textLabel.TurnShadowOff();
	m_textLabel.SetXY( 34, 2 );
	m_textLabel.SetText( "BPM" );
	m_textLabel.SetZoom( 0.7f );
	m_textLabel.SetZoomX( 0.5f );
	m_textLabel.SetDiffuseColorTopEdge( NORMAL_COLOR_TOP );	// yellow
	m_textLabel.SetDiffuseColorBottomEdge( NORMAL_COLOR_BOTTOM );	// orange

	//this->AddSubActor( &m_rectFrame );
	this->AddSubActor( &m_textBPM );
	this->AddSubActor( &m_textLabel );
}


void BPMDisplay::Update( float fDeltaTime ) 
{ 
	ActorFrame::Update( fDeltaTime ); 
	
	if( m_bExtraStage )
	{
		m_fTimeLeftInState -= fDeltaTime;
		if( m_fTimeLeftInState < 0 )
		{
			m_textBPM.SetText( (RandomFloat(0,1)>0.90) ? "???" : ssprintf("%03.0f",RandomFloat(0,600)) ); 
			m_fTimeLeftInState = 0.2f;		// reset timer
		}
	}
	else	// !m_bExtraStage
	{
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
		m_textBPM.SetText( ssprintf("%03.0f", m_fCurrentBPM) ); 
	}
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

	m_textBPM.BeginTweening(0.5f);
	m_textLabel.BeginTweening(0.5f);

	if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
	{
		m_textBPM.SetTweenDiffuseColorTopEdge( EXTRA_COLOR_TOP );
		m_textBPM.SetTweenDiffuseColorBottomEdge( EXTRA_COLOR_BOTTOM );
		m_textLabel.SetTweenDiffuseColorTopEdge( EXTRA_COLOR_TOP );
		m_textLabel.SetTweenDiffuseColorBottomEdge( EXTRA_COLOR_BOTTOM );		
	}
	else if( m_fLowBPM != m_fHighBPM )
	{
		m_textBPM.SetTweenDiffuseColorTopEdge( CHANGE_COLOR_TOP );
		m_textBPM.SetTweenDiffuseColorBottomEdge( CHANGE_COLOR_BOTTOM );
		m_textLabel.SetTweenDiffuseColorTopEdge( CHANGE_COLOR_TOP );
		m_textLabel.SetTweenDiffuseColorBottomEdge( CHANGE_COLOR_BOTTOM );
	}
	else
	{
		m_textBPM.SetTweenDiffuseColorTopEdge( NORMAL_COLOR_TOP );
		m_textBPM.SetTweenDiffuseColorBottomEdge( NORMAL_COLOR_BOTTOM );
		m_textLabel.SetTweenDiffuseColorTopEdge( NORMAL_COLOR_TOP );
		m_textLabel.SetTweenDiffuseColorBottomEdge( NORMAL_COLOR_BOTTOM );
	}

}