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
	Load( THEME->GetPathTo("Graphics","music status icons 1x4") );
	StopAnimating();

	SetType( none );
}

void MusicStatusDisplay::SetType( IconType type )
{
	m_type = type;

	SetDiffuse( RageColor(1,1,1,1) );

	switch( type )
	{
	case none:
		SetEffectNone();
		SetDiffuse( RageColor(1,1,1,0) );
		break;
	case easy:
		SetEffectNone();
		SetState( 0 );	
		break;
	case crown1:
		SetState( 1 );	
		break;
	case crown2:
		SetState( 2 );	
		break;
	case crown3:
		SetState( 3 );	
		break;
	default:
		ASSERT(0);
	}
}

void MusicStatusDisplay::Update( float fDeltaTime )
{
	Sprite::Update( fDeltaTime );
}

void MusicStatusDisplay::DrawPrimitives()
{
	switch( m_type )
	{
	case none:
	case easy:
		break;
	case crown1:
	case crown2:
	case crown3:
		if( fmodf(TIMER->GetTimeSinceStart(), 1) > 0.5f )
			return;	// blink
		break;
	default:
		ASSERT(0);
	}
	Sprite::DrawPrimitives();
}