#include "global.h"
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
#include "ThemeManager.h"


#define NORMAL_COLOR		THEME->GetMetricC("BPMDisplay","NormalColor")
#define CHANGE_COLOR		THEME->GetMetricC("BPMDisplay","ChangeColor")
#define EXTRA_COLOR			THEME->GetMetricC("BPMDisplay","ExtraColor")


BPMDisplay::BPMDisplay()
{
	m_fCurrentBPM = m_fLowBPM = m_fHighBPM = 0;
	m_CountingState = holding_down;
	m_fTimeLeftInState = 0;
	m_bExtraStage = GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2();

	m_textBPM.LoadFromNumbers( THEME->GetPathTo("Numbers","select music bpm numbers") );
	m_textBPM.EnableShadow( false );
	m_textBPM.SetHorizAlign( Actor::align_right );
	m_textBPM.SetDiffuse( NORMAL_COLOR );
	m_textBPM.SetXY( 0, 0 );

	m_sprLabel.Load( THEME->GetPathTo("Graphics","select music bpm label") );
	m_sprLabel.EnableShadow( false );
	m_sprLabel.SetDiffuse( NORMAL_COLOR );
	m_sprLabel.SetHorizAlign( Actor::align_left );
	m_sprLabel.SetXY( 0, 0 );

	this->AddChild( &m_textBPM );
	this->AddChild( &m_sprLabel );
}


void BPMDisplay::Update( float fDeltaTime ) 
{ 
	ActorFrame::Update( fDeltaTime ); 
	
	if( m_bExtraStage )
	{
		m_fTimeLeftInState -= fDeltaTime;
		if( m_fTimeLeftInState < 0 )
		{
			// XXX: the numbers font doesn't have "?".
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

	m_textBPM.StopTweening();
	m_sprLabel.StopTweening();
	m_textBPM.BeginTweening(0.5f);
	m_sprLabel.BeginTweening(0.5f);

	if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
	{
		m_textBPM.SetTweenDiffuse( EXTRA_COLOR );
		m_sprLabel.SetTweenDiffuse( EXTRA_COLOR );		
	}
	else if( m_fLowBPM != m_fHighBPM )
	{
		m_textBPM.SetTweenDiffuse( CHANGE_COLOR );
		m_sprLabel.SetTweenDiffuse( CHANGE_COLOR );
	}
	else
	{
		m_textBPM.SetTweenDiffuse( NORMAL_COLOR );
		m_sprLabel.SetTweenDiffuse( NORMAL_COLOR );
	}

}
