#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSetTime

 Desc: Where the player maps device input to pad input.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenSetTime.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "InputMapper.h"
#include "GameManager.h"
#include "GameState.h"
#include "RageSounds.h"
#include "ThemeManager.h"
#include "RageDisplay.h"
#include "Bookkeeper.h"
#include "arch/ArchHooks/ArchHooks.h"

ScreenSetTime::ScreenSetTime( CString sClassName ) : Screen( sClassName )
{
	LOG->Trace( "ScreenSetTime::ScreenSetTime()" );
	
	m_Selection = hour;


	for( int i=0; i<NUM_SELECTIONS; i++ )
	{
		m_text[i].LoadFromFont( THEME->GetPathToF("Common title") );
		m_text[i].SetDiffuse( RageColor(1,1,1,1) );
		this->AddChild( &m_text[i] );

		switch( i )
		{
		case hour:		m_text[i].SetXY( 220, 200 );	break;
		case minute:	m_text[i].SetXY( 320, 200 );	break;
		case second:	m_text[i].SetXY( 420, 200 );	break;
		case year:		m_text[i].SetXY( 220, 280 );	break;
		case month:		m_text[i].SetXY( 320, 280 );	break;
		case day:		m_text[i].SetXY( 420, 280 );	break;
		default:		ASSERT(0);
		}
	}

	m_Selection = (Selection)0;
	ChangeSelection( 0 );

	m_Menu.Load( m_sName );
	this->AddChild( &m_Menu );

	SOUND->PlayMusic( THEME->GetPathS(m_sName,"music") );
}

void ScreenSetTime::Update( float fDelta )
{
	Screen::Update( fDelta );

	tm now = GetLocalTime();
	m_text[hour].SetText(	ssprintf("%02d",now.tm_hour) );
	m_text[minute].SetText( ssprintf("%02d",now.tm_min) );
	m_text[second].SetText( ssprintf("%02d",now.tm_sec) );
	m_text[year].SetText(	ssprintf("%02d",now.tm_year+1900) );
	m_text[month].SetText(	ssprintf("%02d",now.tm_mon) );
	m_text[day].SetText(	ssprintf("%02d",now.tm_mday) );
}

void ScreenSetTime::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenSetTime::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( type != IET_FIRST_PRESS && type != IET_SLOW_REPEAT )
		return;	// ignore

	if( m_Menu.IsTransitioning() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default handler
}

void ScreenSetTime::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToNextScreen:
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
		break;
	}
}

void ScreenSetTime::ChangeValue( int iDirection )
{
	tm now = GetLocalTime();
	switch( m_Selection )
	{
	case hour:		now.tm_hour += iDirection;	break;
	case minute:	now.tm_min += iDirection;	break;
	case second:	now.tm_sec += iDirection;	break;
	case year:		now.tm_year += iDirection;	break;
	case month:		now.tm_mon += iDirection;	break;
	case day:		now.tm_mday += iDirection;	break;
	default:		ASSERT(0);
	}

	HOOKS->SetTime( now );
}

void ScreenSetTime::ChangeSelection( int iDirection )
{
	// turn off old effect
	m_text[m_Selection].SetEffectNone();

	// set new value of m_Selection
	((int&)m_Selection) += iDirection;
	CLAMP( (int&)m_Selection, 0, NUM_SELECTIONS-1 );

	m_text[m_Selection].SetEffectDiffuseShift();
}

void ScreenSetTime::MenuUp( PlayerNumber pn )
{
	ChangeSelection( -1 );
}

void ScreenSetTime::MenuDown( PlayerNumber pn )
{
	ChangeSelection( +1 );
}

void ScreenSetTime::MenuLeft( PlayerNumber pn )
{
	ChangeValue( -1 );
}

void ScreenSetTime::MenuRight( PlayerNumber pn )
{
	ChangeValue( +1 );
}

void ScreenSetTime::MenuStart( PlayerNumber pn )
{
	m_Menu.StartTransitioning( SM_GoToNextScreen );
}

void ScreenSetTime::MenuBack( PlayerNumber pn )
{
	m_Menu.StartTransitioning( SM_GoToPrevScreen );
}
	
