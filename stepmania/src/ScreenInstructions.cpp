#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenInstructions

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenInstructions.h"
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageSoundManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "PrefsManager.h"


#define HELP_TEXT				THEME->GetMetric("ScreenInstructions","HelpText")
#define TIMER_SECONDS			THEME->GetMetricI("ScreenInstructions","TimerSeconds")
#define NEXT_SCREEN_ARCADE		THEME->GetMetric("ScreenInstructions","NextScreenArcade")
#define NEXT_SCREEN_ONI			THEME->GetMetric("ScreenInstructions","NextScreenOni")


const ScreenMessage SM_GoToPrevScreen	=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToNextScreen	=	ScreenMessage(SM_User+2);


ScreenInstructions::ScreenInstructions()
{
	LOG->Trace( "ScreenInstructions::ScreenInstructions()" );

	m_Menu.Load(
		THEME->GetPathTo("BGAnimations","How To Play"), 
		THEME->GetPathTo("Graphics","How To Play Top Edge"), 
		HELP_TEXT, false, true, TIMER_SECONDS
		);
	this->AddChild( &m_Menu );

	if(!PREFSMAN->m_bHowToPlay)
	{
		m_Menu.ImmedOffScreenToMenu();
		this->SendScreenMessage( SM_GoToNextScreen, 0.f );
		return;
	}

	m_Menu.TweenOnScreenFromMenu( SM_None, true );

	CString sHowToPlayPath;
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
		sHowToPlayPath = THEME->GetPathTo("Graphics","how to play arcade");
		break;
	case PLAY_MODE_ENDLESS:
		sHowToPlayPath = THEME->GetPathTo("Graphics","how to play endless");
		break;
	case PLAY_MODE_ONI:
		sHowToPlayPath = THEME->GetPathTo("Graphics","how to play oni");
		break;
	default:
		ASSERT(0);
	}

	m_sprHowToPlay.Load( sHowToPlayPath );
	m_sprHowToPlay.SetXY( CENTER_X, CENTER_Y );
	this->AddChild( &m_sprHowToPlay );

	m_sprHowToPlay.SetX( SCREEN_LEFT-SCREEN_WIDTH );
	m_sprHowToPlay.BeginTweening( 0.4f );		// sleep
	m_sprHowToPlay.BeginTweening( 0.6f, Actor::TWEEN_BIAS_BEGIN );
	m_sprHowToPlay.SetTweenX( CENTER_X );

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","how to play music") );
}

ScreenInstructions::~ScreenInstructions()
{
	LOG->Trace( "ScreenInstructions::~ScreenInstructions()" );
}

void ScreenInstructions::Update( float fDeltaTime )
{
	//LOG->Trace( "ScreenInstructions::Update(%f)", fDeltaTime );

	Screen::Update( fDeltaTime );
}

void ScreenInstructions::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	m_Menu.DrawTopLayer();
	Screen::DrawPrimitives();	// draw How To Play graphic on top of all the others!
}

void ScreenInstructions::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( m_Menu.IsClosing() )
		return;

	// default input handler
	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenInstructions::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_MenuTimer:
		this->MenuStart(PLAYER_1);
		break;
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );		
		break;
	case SM_GoToNextScreen:
		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_ARCADE:
			SCREENMAN->SetNewScreen( NEXT_SCREEN_ARCADE );
			break;
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:
			SCREENMAN->SetNewScreen( NEXT_SCREEN_ONI );
			break;
		default:
			ASSERT(0);
		}
		break;
	}
}

void ScreenInstructions::MenuBack( PlayerNumber pn )
{
	m_Menu.TweenOffScreenToBlack( SM_GoToPrevScreen, true );
}

void ScreenInstructions::MenuStart( PlayerNumber pn )
{
	m_Menu.TweenOffScreenToMenu( SM_GoToNextScreen );

	m_sprHowToPlay.StopTweening();
	m_sprHowToPlay.BeginTweening( 0.3f, Actor::TWEEN_BIAS_END );
	m_sprHowToPlay.SetTweenX( SCREEN_RIGHT+m_sprHowToPlay.GetUnzoomedWidth()/2 );
}
