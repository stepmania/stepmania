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
	m_fTimeLeftInState = 0;
	m_CountingState = holding_down;

	m_textBPM.LoadFromNumbers( THEME->GetPathToN("BPMDisplay") );
	m_textBPM.EnableShadow( false );
	m_textBPM.SetHorizAlign( Actor::align_right );
	m_textBPM.SetDiffuse( NORMAL_COLOR );
	m_textBPM.SetXY( 0, 0 );

	m_sprLabel.Load( THEME->GetPathToG("BPMDisplay label") );
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
	
	m_fTimeLeftInState -= fDeltaTime;
	if( m_fTimeLeftInState < 0 )
	{
		// go to next state
		switch( m_CountingState )
		{
		case counting_up:
			m_CountingState = holding_up;
			m_fTimeLeftInState = 1;		// reset timer
			break;
		case holding_up:
			m_CountingState = counting_down;
			m_fTimeLeftInState = 1;		// reset timer
			break;
		case counting_down:
			m_CountingState = holding_down;
			m_fTimeLeftInState = 1;		// reset timer
			break;
		case holding_down:
			m_CountingState = counting_up;
			m_fTimeLeftInState = 1;		// reset timer
			break;
		case cycle_randomly:
			m_textBPM.SetText( (RandomFloat(0,1)>0.90) ? "xxx" : ssprintf("%03.0f",RandomFloat(0,600)) ); 
			m_fTimeLeftInState = 0.2f;		// reset timer
			break;
		case no_bpm:
			m_fTimeLeftInState = 0;
			break;
		default:
			ASSERT(0);
		}
	}

	// update m_fCurrentBPM
//	int iLastCurBPM = (int)m_fCurrentBPM;
	switch( m_CountingState )
	{
	case counting_down:	m_fCurrentBPM = m_fLowBPM + (m_fHighBPM-m_fLowBPM)*m_fTimeLeftInState;	break;
	case counting_up:	m_fCurrentBPM = m_fHighBPM + (m_fLowBPM-m_fHighBPM)*m_fTimeLeftInState;	break;
	case holding_up:	m_fCurrentBPM = m_fHighBPM;												break;
	case holding_down:	m_fCurrentBPM = m_fLowBPM;												break;
	case cycle_randomly:																		break;
	case no_bpm:																				break;
	default:
		ASSERT(0);
	}

	// update text
	switch( m_CountingState )
	{
	case counting_down:
	case counting_up:
	case holding_up:
	case holding_down:
		//if( (int)m_fCurrentBPM != iLastCurBPM )
		//	m_textBPM.SetText( ssprintf("%03.0f", m_fCurrentBPM) ); 

		/* BUG FIXED:: Just set the BPM.. This was causing an error that would not change
			the BPM to what it should be, if you changed the selection from a Group
			to a song. There's no great reason to check if it is higher or lower
			than the previous BPM -- Miryokuteki */
		m_textBPM.SetText( ssprintf("%03.0f", m_fCurrentBPM) );
		break;
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
		m_textBPM.SetDiffuse( EXTRA_COLOR );
		m_sprLabel.SetDiffuse( EXTRA_COLOR );		
	}
	else if( m_fLowBPM != m_fHighBPM )
	{
		m_textBPM.SetDiffuse( CHANGE_COLOR );
		m_sprLabel.SetDiffuse( CHANGE_COLOR );
	}
	else
	{
		m_textBPM.SetDiffuse( NORMAL_COLOR );
		m_sprLabel.SetDiffuse( NORMAL_COLOR );
	}

}

void BPMDisplay::CycleRandomly()
{
	m_CountingState = cycle_randomly;
	m_fTimeLeftInState = 0;

	m_textBPM.SetDiffuse( NORMAL_COLOR );
	m_sprLabel.SetDiffuse( NORMAL_COLOR );
}

void BPMDisplay::NoBPM()
{
	m_CountingState = no_bpm;
	m_textBPM.SetText( "..." ); 

	m_textBPM.SetDiffuse( NORMAL_COLOR );
	m_sprLabel.SetDiffuse( NORMAL_COLOR );
}

void BPMDisplay::SetBPM( const Song* pSong )
{
	ASSERT( pSong );
	switch( pSong->m_DisplayBPMType )
	{
	case Song::DISPLAY_ACTUAL:
		{
			float fMinBPM, fMaxBPM;
			pSong->GetActualBPM( fMinBPM, fMaxBPM );
			SetBPMRange( fMinBPM, fMaxBPM );
		}
		break;
	case Song::DISPLAY_SPECIFIED:
		SetBPMRange( pSong->m_fSpecifiedBPMMin, pSong->m_fSpecifiedBPMMax );
		break;
	case Song::DISPLAY_RANDOM:
		CycleRandomly();
		break;
	default:
		ASSERT(0);
	}
}

void BPMDisplay::DrawPrimitives()
{
	ActorFrame::DrawPrimitives();
}
