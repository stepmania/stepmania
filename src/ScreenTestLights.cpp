#include "global.h"
#include "ScreenTestLights.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "LightsManager.h"
#include "Game.h"
#include "ScreenDimensions.h"
#include "InputEventPlus.h"
#include "LocalizedString.h"

REGISTER_SCREEN_CLASS( ScreenTestLights );

void ScreenTestLights::Init()
{
	ScreenWithMenuElements::Init();

	m_textInputs.SetName( "Text" );
	m_textInputs.LoadFromFont( THEME->GetPathF("Common","normal") );
	m_textInputs.SetText( "" );
	LOAD_ALL_COMMANDS_AND_SET_XY( m_textInputs );
	this->AddChild( &m_textInputs );
}

void ScreenTestLights::BeginScreen()
{
	LIGHTSMAN->SetLightsMode( LIGHTSMODE_TEST_AUTO_CYCLE );
	ScreenWithMenuElements::BeginScreen();
}

void ScreenTestLights::EndScreen()
{
	LIGHTSMAN->SetLightsMode( LIGHTSMODE_MENU_START_AND_DIRECTIONS );
	ScreenWithMenuElements::EndScreen();
}

static LocalizedString AUTO_CYCLE	( "ScreenTestLights", "Auto Cycle" );
static LocalizedString MANUAL_CYCLE	( "ScreenTestLights", "Manual Cycle" );
static LocalizedString CABINET_LIGHT( "ScreenTestLights", "cabinet light" );
static LocalizedString CONTROLLER_LIGHT( "ScreenTestLights", "controller light" );
void ScreenTestLights::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );


	if( m_timerBackToAutoCycle.Ago() > 20 )
	{
		m_timerBackToAutoCycle.Touch();
		LIGHTSMAN->SetLightsMode( LIGHTSMODE_TEST_AUTO_CYCLE );
	}


	CabinetLight cl = LIGHTSMAN->GetFirstLitCabinetLight();
	GameInput gi = LIGHTSMAN->GetFirstLitGameButtonLight();

	RString s;

	switch( LIGHTSMAN->GetLightsMode() )
	{
		case LIGHTSMODE_TEST_AUTO_CYCLE:
			s += AUTO_CYCLE.GetValue()+"\n";
			break;
		case LIGHTSMODE_TEST_MANUAL_CYCLE:
			s += MANUAL_CYCLE.GetValue()+"\n";
			break;
		default: break;
	}

	if( cl == CabinetLight_Invalid )
		s += CABINET_LIGHT.GetValue()+": -----\n";
	else
		s += ssprintf( "%s: %d %s\n", CABINET_LIGHT.GetValue().c_str(), cl, CabinetLightToString(cl).c_str() );

	if( !gi.IsValid() )
	{
		s += CONTROLLER_LIGHT.GetValue()+": -----\n";
	}
	else
	{
		RString sGameButton = GameButtonToLocalizedString( INPUTMAPPER->GetInputScheme(), gi.button );
		PlayerNumber pn = (PlayerNumber)(gi.controller);
		s += ssprintf( "%s: %s %d %s\n", CONTROLLER_LIGHT.GetValue().c_str(), PlayerNumberToString(pn).c_str(), gi.button, sGameButton.c_str() );
	}

	m_textInputs.SetText( s );
}


bool ScreenTestLights::Input( const InputEventPlus &input )
{
	if( input.type != IET_FIRST_PRESS && input.type != IET_REPEAT )
		return false;	// ignore

	return ScreenWithMenuElements::Input( input );	// default handler
}

bool ScreenTestLights::MenuLeft( const InputEventPlus &input )
{
	if( LIGHTSMAN->GetLightsMode() != LIGHTSMODE_TEST_MANUAL_CYCLE )
		LIGHTSMAN->SetLightsMode( LIGHTSMODE_TEST_MANUAL_CYCLE );
	if( input.pn == PLAYER_1 )
		LIGHTSMAN->PrevTestCabinetLight();
	else
		LIGHTSMAN->PrevTestGameButtonLight();
	m_timerBackToAutoCycle.Touch();
	return true;
}

bool ScreenTestLights::MenuRight( const InputEventPlus &input )
{
	if( LIGHTSMAN->GetLightsMode() != LIGHTSMODE_TEST_MANUAL_CYCLE )
		LIGHTSMAN->SetLightsMode( LIGHTSMODE_TEST_MANUAL_CYCLE );
	if( input.pn == PLAYER_1 )
		LIGHTSMAN->NextTestCabinetLight();
	else
		LIGHTSMAN->NextTestGameButtonLight();
	m_timerBackToAutoCycle.Touch();
	return true;
}

bool ScreenTestLights::MenuStart( const InputEventPlus &input )
{
	return MenuBack( input );
}

bool ScreenTestLights::MenuBack( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return false;
	SCREENMAN->PlayStartSound();
	StartTransitioningScreen( SM_GoToPrevScreen );		
	return true;
}

/*
 * (c) 2004 Chris Danford
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
