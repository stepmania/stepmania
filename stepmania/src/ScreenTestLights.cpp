#include "global.h"
#include "ScreenTestLights.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "InputMapper.h"
#include "GameManager.h"
#include "GameState.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "RageDisplay.h"
#include "LightsManager.h"
#include "Game.h"


ScreenTestLights::ScreenTestLights( CString sClassName ) : ScreenWithMenuElements( sClassName )
{
	LOG->Trace( "ScreenTestLights::ScreenTestLights()" );
	

	m_textInputs.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textInputs.SetText( "" );
	m_textInputs.SetXY( CENTER_X, CENTER_Y );
	m_textInputs.SetDiffuse( RageColor(1,1,1,1) );
	m_textInputs.SetZoom( 0.8f );
	this->AddChild( &m_textInputs );

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

	CabinetLight cl = (CabinetLight)(iSec%NUM_CABINET_LIGHTS);
	int iNumGameButtonsToShow = GAMESTATE->GetCurrentGame()->GetNumGameplayButtons();
	GameButton gb = (GameButton)(iSec%iNumGameButtonsToShow);
	CString sCabLight = CabinetLightToString(cl);
	CString sGameButton = GAMESTATE->GetCurrentGame()->m_szButtonNames[gb];

	CString s;
	s += ssprintf("cabinet light %d: %s\n", cl, sCabLight.c_str());
	FOREACH_GameController( gc )
		s += ssprintf("controller %d light %d: %s\n", gc+1, gb, sGameButton.c_str());

	m_textInputs.SetText( s );
}


void ScreenTestLights::DrawPrimitives()
{
	Screen::DrawPrimitives();
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
	if(!IsTransitioning())
	{
		SCREENMAN->PlayStartSound();
		StartTransitioning( SM_GoToPrevScreen );		
	}
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
