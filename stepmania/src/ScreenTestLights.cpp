#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenTestLights

 Desc: Where the player maps device input to pad input.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenTestLights.h"
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
#include "LightsManager.h"


ScreenTestLights::ScreenTestLights( CString sClassName ) : Screen( sClassName )
{
	LOG->Trace( "ScreenTestLights::ScreenTestLights()" );
	

	m_textInputs.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textInputs.SetText( "" );
	m_textInputs.SetXY( CENTER_X, CENTER_Y );
	m_textInputs.SetDiffuse( RageColor(1,1,1,1) );
	m_textInputs.SetZoom( 0.8f );
	this->AddChild( &m_textInputs );

	m_Menu.Load( "ScreenTestLights" );
	this->AddChild( &m_Menu );

	SOUND->PlayMusic( THEME->GetPathToS("ScreenTestLights music") );

	LIGHTSMAN->SetLightsMode( LIGHTSMODE_TEST );
}



ScreenTestLights::~ScreenTestLights()
{
	LOG->Trace( "ScreenTestLights::~ScreenTestLights()" );
}


void ScreenTestLights::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );

	int iSec = (int)RageTimer::GetTimeSinceStart();

	CString s;
	s += ssprintf("cabinet light %d\n", iSec%NUM_CABINET_LIGHTS);
	FOREACH_GameController( gc )
		s += ssprintf("controller %d light %d\n", gc+1, iSec%MAX_GAME_BUTTONS);

	m_textInputs.SetText( s );
}


void ScreenTestLights::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenTestLights::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( type != IET_FIRST_PRESS && type != IET_SLOW_REPEAT )
		return;	// ignore

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default handler
}

void ScreenTestLights::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToNextScreen:
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
		break;
	}
}

void ScreenTestLights::MenuStart( PlayerNumber pn )
{
	MenuBack(pn);
}

void ScreenTestLights::MenuBack( PlayerNumber pn )
{
	if(!m_Menu.IsTransitioning())
	{
		SOUND->PlayOnce( THEME->GetPathToS("Common start") );
		m_Menu.StartTransitioning( SM_GoToPrevScreen );		
	}
}
