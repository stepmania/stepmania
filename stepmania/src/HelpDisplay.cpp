#include "global.h"
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
	m_textTip.LoadFromFont( THEME->GetPathToF(m_sName) );
	ON_COMMAND( m_textTip );
	this->AddChild( &m_textTip );

	m_fSecsUntilSwitch = TIP_SHOW_TIME;
}

void HelpDisplay::SetTips( const CStringArray &arrayTips, const CStringArray &arrayTipsAlt )
{ 
	ASSERT( arrayTips.size() == arrayTipsAlt.size() );

	if( arrayTips == m_arrayTips && arrayTipsAlt == m_arrayTipsAlt )
		return;

	m_textTip.SetText( "" );

	m_arrayTips = arrayTips;
	m_arrayTipsAlt = arrayTipsAlt;

	m_iCurTipIndex = 0;
	m_fSecsUntilSwitch = 0;
	Update( 0 );
}


void HelpDisplay::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	if( m_arrayTips.empty() )
		return;

	m_fSecsUntilSwitch -= fDeltaTime;
	if( m_fSecsUntilSwitch > 0 )
		return;

	// time to switch states
	m_fSecsUntilSwitch = TIP_SHOW_TIME;
	m_textTip.SetText( m_arrayTips[m_iCurTipIndex], m_arrayTipsAlt[m_iCurTipIndex] );
	m_iCurTipIndex++;
	m_iCurTipIndex = m_iCurTipIndex % m_arrayTips.size();
}

/*
 * (c) 2001-2003 Chris Danford, Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
