#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: TipDisplay.h

 Desc: A graphic displayed in the TipDisplay during Dancing.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "TipDisplay.h"
#include "RageUtil.h"
#include "ThemeManager.h"


const float TIP_FADE_TIME	=	0.3f;
const float TIP_SHOW_TIME	=	2;


TipDisplay::TipDisplay()
{
	RageLog( "TipDisplay::TipDisplay()" );

	m_textTip.Load( THEME->GetPathTo(FONT_NORMAL) );
	this->AddActor( &m_textTip );

	iLastTipShown = -1;
	m_TipState = STATE_SHOWING;
	m_fTimeLeftInState = 0;
}


void TipDisplay::SetTips( CStringArray &arrayTips )
{ 
	m_arrayTips.RemoveAll();
	m_arrayTips.Copy( arrayTips );
}


void TipDisplay::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	if( m_arrayTips.GetSize() == 0 )
		return;

	m_fTimeLeftInState -= fDeltaTime;
	if( m_fTimeLeftInState <= 0 )		// time to switch states
	{
		switch( m_TipState )
		{
		case STATE_SHOWING:
			m_TipState = STATE_FADING_OUT;
			m_fTimeLeftInState = TIP_FADE_TIME;

			// fade out
			m_textTip.SetZoomY( 1 );
			m_textTip.BeginTweening( TIP_FADE_TIME );
			m_textTip.SetTweenZoomY( 0 );
			break;
		case STATE_FADING_OUT:
			m_TipState = STATE_FADING_IN;
			m_fTimeLeftInState = TIP_FADE_TIME;

			// switch the tip
			int iTipToShow;
			iTipToShow = iLastTipShown + 1;
			if( iTipToShow > m_arrayTips.GetSize()-1 )
				iTipToShow = 0;
			m_textTip.SetText( m_arrayTips[iTipToShow] );
			iLastTipShown = iTipToShow;

			// fade in
			m_textTip.SetZoomY( 0 );
			m_textTip.BeginTweening( TIP_FADE_TIME );
			m_textTip.SetTweenZoomY( 1 );

			break;
		case STATE_FADING_IN:
			m_TipState = STATE_SHOWING;
			m_fTimeLeftInState = TIP_SHOW_TIME;
			break;

		}
	}
}
