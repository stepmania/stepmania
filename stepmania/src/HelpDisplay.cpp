#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: HelpDisplay

 Desc: See header

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "HelpDisplay.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "ActorUtil.h"

#define TIP_SHOW_TIME			THEME->GetMetricF(m_sName,"TipShowTime")


HelpDisplay::HelpDisplay()
{
	m_iCurTipIndex = 0;
	m_fSecsUntilSwitch = 0;
}

void HelpDisplay::Load()
{
	m_textTip.SetName( "Tip" );
	m_textTip.LoadFromFont( THEME->GetPathToF("HelpDisplay") );
	ON_COMMAND( m_textTip );
	this->AddChild( &m_textTip );

	m_fSecsUntilSwitch = TIP_SHOW_TIME;
}

void HelpDisplay::SetTips( const CStringArray &arrayTips )
{ 
	m_arrayTips = arrayTips;
	if( !m_arrayTips.empty() )
		m_textTip.SetText( m_arrayTips[0] );
	else
		m_textTip.SetText( "" );
}


void HelpDisplay::Update( float fDeltaTime )
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
