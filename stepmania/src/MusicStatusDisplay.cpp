#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: MusicStatusDisplay

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "MusicStatusDisplay.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "MusicWheel.h"
#include "MusicStatusDisplay.h"
#include "RageTimer.h"
#include <math.h>
#include "ThemeManager.h"



MusicStatusDisplay::MusicStatusDisplay()
{
	Load( THEME->GetPathTo("Graphics","music status icons 4x2") );
	StopAnimating();
}

void MusicStatusDisplay::SetFlags( Flags flags )
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

	if( flags.bHasBeginnerOr1Meter )
		m_vIconsToShow.push_back( training );


	// HACK:  Make players best blink if it's the only icon
	if( m_vIconsToShow.size() == 1 )
	{
		if( m_vIconsToShow[0] >= best1  &&  m_vIconsToShow[0] <= best3 )
			m_vIconsToShow.push_back( empty );		
	}


	m_vIconsToShow.resize( min(m_vIconsToShow.size(),2u) );	// crop to most important 2

	if( m_vIconsToShow.size() == 0 )
		Sprite::SetDiffuse( RageColor(1,1,1,0) );
	else
		Sprite::SetDiffuse( RageColor(1,1,1,1) );
}

void MusicStatusDisplay::DrawPrimitives()
{
		float fSecondFraction = fmodf(RageTimer::GetTimeSinceStart(), 1 );
	if( m_vIconsToShow.size() > 0 )
	{
		int index = (int)(fSecondFraction*m_vIconsToShow.size());
		Sprite::SetState( m_vIconsToShow[index] );
	}

	Sprite::DrawPrimitives();
}

