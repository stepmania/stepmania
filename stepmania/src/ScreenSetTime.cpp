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

static const CString SetTimeSelectionNames[NUM_SET_TIME_SELECTIONS] = {
	"Year", 
	"Month", 
	"Day",
	"Hour", 
	"Minute", 
	"Second", 
};
XToString( SetTimeSelection );
#define FOREACH_SetTimeSelection( s ) FOREACH_ENUM( SetTimeSelection, NUM_SET_TIME_SELECTIONS, s )

const float g_X[NUM_SET_TIME_SELECTIONS] =
{
	320, 320, 320, 320, 320, 320
};

const float g_Y[NUM_SET_TIME_SELECTIONS] =
{
	140, 180, 220, 260, 300, 340
};

static float GetTitleX( SetTimeSelection s ) { return g_X[s] - 80; }
static float GetTitleY( SetTimeSelection s ) { return g_Y[s]; }
static float GetValueX( SetTimeSelection s ) { return g_X[s] + 80; }
static float GetValueY( SetTimeSelection s ) { return g_Y[s]; }

ScreenSetTime::ScreenSetTime( CString sClassName ) : ScreenWithMenuElements( sClassName )
{
	LOG->Trace( "ScreenSetTime::ScreenSetTime()" );
	
	m_Selection = hour;

	FOREACH_PlayerNumber( pn )
		GAMESTATE->m_bSideIsJoined[pn] = true;

	FOREACH_SetTimeSelection( s )
	{
		BitmapText &title = m_textTitle[s];
		BitmapText &value = m_textValue[s];

		title.LoadFromFont( THEME->GetPathToF("Common title") );
		title.SetDiffuse( RageColor(1,1,1,1) );
		title.SetText( SetTimeSelectionToString(s) );
		title.SetXY( GetTitleX(s), GetTitleY(s) );
		this->AddChild( &title );

		value.LoadFromFont( THEME->GetPathToF("Common normal") );
		value.SetDiffuse( RageColor(1,1,1,1) );
		value.SetXY( GetValueX(s), GetValueY(s) );
		this->AddChild( &value );
	}

	m_TimeOffset = 0;
	m_Selection = (SetTimeSelection)0;
	ChangeSelection( 0 );

	SOUND->PlayMusic( THEME->GetPathS(m_sName,"music") );

	this->SortByDrawOrder();
}

void ScreenSetTime::Update( float fDelta )
{
	Screen::Update( fDelta );

	time_t iNow = time(NULL);
	iNow += m_TimeOffset;

	tm now;
	localtime_r( &iNow, &now );
	
	m_textValue[hour].SetText(	ssprintf("%02d",now.tm_hour) );
	m_textValue[minute].SetText( ssprintf("%02d",now.tm_min) );
	m_textValue[second].SetText( ssprintf("%02d",now.tm_sec) );
	m_textValue[year].SetText(	ssprintf("%02d",now.tm_year+1900) );
	m_textValue[month].SetText(	ssprintf("%02d",now.tm_mon) );
	m_textValue[day].SetText(	ssprintf("%02d",now.tm_mday) );
}

void ScreenSetTime::DrawPrimitives()
{
	Screen::DrawPrimitives();
}

void ScreenSetTime::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( type != IET_FIRST_PRESS && type != IET_SLOW_REPEAT )
		return;	// ignore

	if( IsTransitioning() )
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
	time_t iNow = time(NULL);
	time_t iAdjusted = iNow + m_TimeOffset;

	tm adjusted;
	localtime_r( &iAdjusted, &adjusted );
	
	//tm now = GetLocalTime();
	switch( m_Selection )
	{
	case hour:		adjusted.tm_hour += iDirection;	break;
	case minute:	adjusted.tm_min += iDirection;	break;
	case second:	adjusted.tm_sec += iDirection;	break;
	case year:		adjusted.tm_year += iDirection;	break;
	case month:		adjusted.tm_mon += iDirection;	break;
	case day:		adjusted.tm_mday += iDirection;	break;
	default:		ASSERT(0);
	}

	/* Normalize: */
	iAdjusted = mktime( &adjusted );

	m_TimeOffset = iAdjusted - iNow;

	SOUND->PlayOnce( THEME->GetPathS("ScreenSetTime","ChangeValue") );
}

void ScreenSetTime::ChangeSelection( int iDirection )
{
	// set new value of m_Selection
	SetTimeSelection OldSelection = m_Selection;
	enum_add( m_Selection, iDirection );

	CLAMP( (int&)m_Selection, 0, NUM_SET_TIME_SELECTIONS-1 );
	if( iDirection != 0 && m_Selection == OldSelection )
		return; // can't move any more

	m_textValue[OldSelection].SetEffectNone();
	m_textValue[m_Selection].SetEffectDiffuseShift();

	SOUND->PlayOnce( THEME->GetPathS("ScreenSetTime","ChangeSelection") );
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
	bool bHoldingLeftAndRight = 
		INPUTMAPPER->IsButtonDown( MenuInput(pn, MENU_BUTTON_RIGHT) ) &&
		INPUTMAPPER->IsButtonDown( MenuInput(pn, MENU_BUTTON_LEFT) );

	if( bHoldingLeftAndRight )
		ChangeSelection( -1 );
	else if( m_Selection == NUM_SET_TIME_SELECTIONS -1 )	// last row
	{
		/* Save the new time. */
		time_t iNow = time(NULL);
		time_t iAdjusted = iNow + m_TimeOffset;

		tm adjusted;
		localtime_r( &iAdjusted, &adjusted );

		HOOKS->SetTime( adjusted );

		/* We're going to draw a little more while we transition out.  We've already
		 * set the new time; don't over-adjust visually. */
		m_TimeOffset = 0;

		SOUND->PlayOnce( THEME->GetPathS("Common","start") );
		StartTransitioning( SM_GoToNextScreen );
	}
	else
		ChangeSelection( +1 );
}

void ScreenSetTime::MenuBack( PlayerNumber pn )
{
	StartTransitioning( SM_GoToPrevScreen );
}
	
