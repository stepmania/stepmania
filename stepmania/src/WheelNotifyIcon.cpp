#include "global.h"
#include "WheelNotifyIcon.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "MusicWheel.h"
#include "WheelNotifyIcon.h"
#include "RageTimer.h"
#include "ThemeManager.h"


static ThemeMetric<bool>	SHOW_TRAINING	("WheelNotifyIcon","ShowTraining");

WheelNotifyIcon::WheelNotifyIcon()
{
	Load( THEME->GetPathG("WheelNotifyIcon","icons 4x2") );
	StopAnimating();
}

void WheelNotifyIcon::SetFlags( Flags flags )
{
	m_vIconsToShow.clear();

	// push onto vector in highest to lowest priority

	switch( flags.iPlayersBestNumber )
	{
	case 1:	m_vIconsToShow.push_back( best1 );	break;
	case 2:	m_vIconsToShow.push_back( best2 );	break;
	case 3:	m_vIconsToShow.push_back( best3 );	break;
	}

	if( flags.bEdits )
		m_vIconsToShow.push_back( edits );

	switch( flags.iStagesForSong )
	{
	case 1:										break;
	case 2:	m_vIconsToShow.push_back( long_ver );	break;
	case 3:	m_vIconsToShow.push_back( marathon );	break;
	default:	ASSERT(0);
	}

	if( flags.bHasBeginnerOr1Meter && (bool)SHOW_TRAINING )
		m_vIconsToShow.push_back( training );


	// HACK:  Make players best blink if it's the only icon
	if( m_vIconsToShow.size() == 1 )
	{
		if( m_vIconsToShow[0] >= best1  &&  m_vIconsToShow[0] <= best3 )
			m_vIconsToShow.push_back( empty );		
	}


	m_vIconsToShow.resize( min(m_vIconsToShow.size(),2u) );	// crop to most important 2

	/* Make sure the right icon is selected, since we might be drawn before
	 * we get another update. */
	Update(0);
}

bool WheelNotifyIcon::EarlyAbortDraw() const
{
	if( m_vIconsToShow.empty() )
		return true;
	return Sprite::EarlyAbortDraw();
}

void WheelNotifyIcon::Update( float fDeltaTime )
{
	if( m_vIconsToShow.size() > 0 )
	{
		const float fSecondFraction = fmodf( RageTimer::GetTimeSinceStartFast(), 1 );
		const int index = (int)(fSecondFraction*m_vIconsToShow.size());
		Sprite::SetState( m_vIconsToShow[index] );
	}

	Sprite::Update( fDeltaTime );
}

/*
 * (c) 2001-2004 Chris Danford
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
