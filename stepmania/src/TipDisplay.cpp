#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: TipDisplay.h

 Desc: A BitmapText that .

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

	m_textTip.Load( THEME->GetPathTo(FONT_NORMAL) );
	m_textTip.SetEffectBlinking();
	m_textTip.TurnShadowOff();
	this->AddSubActor( &m_textTip );

	m_iCurTipIndex = 0;
	m_fSecsUntilSwitch = TIP_SHOW_TIME;
}


void TipDisplay::SetTips( CStringArray &arrayTips )
{ 
	m_arrayTips.RemoveAll();
	m_arrayTips.Copy( arrayTips );
	if( m_arrayTips.GetSize() > 0 )
		m_textTip.SetText( m_arrayTips[0] );
}


void TipDisplay::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	m_fSecsUntilSwitch -= fDeltaTime;
	if( m_fSecsUntilSwitch < 0 )		// time to switch states
	{
		m_iCurTipIndex++;
		m_iCurTipIndex = m_iCurTipIndex % m_arrayTips.GetSize();
		m_fSecsUntilSwitch = TIP_SHOW_TIME;
		m_textTip.SetText( m_arrayTips[m_iCurTipIndex] );
	}
}
