#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: TipDisplay

 Desc: See header

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "TipDisplay.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "RageLog.h"


const float TIP_SHOW_TIME	=	3.5f;


TipDisplay::TipDisplay()
{
	LOG->Trace( "TipDisplay::TipDisplay()" );

	m_textTip.LoadFromFont( THEME->GetPathTo("Fonts","help") );
	m_textTip.SetEffectBlinking();
	m_textTip.TurnShadowOff();
	this->AddChild( &m_textTip );

	m_iCurTipIndex = 0;
	m_fSecsUntilSwitch = TIP_SHOW_TIME;
}


void TipDisplay::SetTips( CStringArray &arrayTips )
{ 
	m_arrayTips = arrayTips;
	if( !m_arrayTips.empty() )
		m_textTip.SetText( m_arrayTips[0] );
}


void TipDisplay::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	if( !m_arrayTips.empty() )
	{
		m_fSecsUntilSwitch -= fDeltaTime;
		if( m_fSecsUntilSwitch < 0 )		// time to switch states
		{
			m_iCurTipIndex++;
			m_iCurTipIndex = m_iCurTipIndex % m_arrayTips.size();
			m_fSecsUntilSwitch = TIP_SHOW_TIME;
			m_textTip.SetText( m_arrayTips[m_iCurTipIndex] );
		}
	}
}
