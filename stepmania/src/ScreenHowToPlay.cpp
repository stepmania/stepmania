#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenHowToPlay

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenHowToPlay.h"
#include <assert.h>
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageMusic.h"
#include "ScreenManager.h"
#include "ScreenSelectMusic.h"
#include "ScreenSelectCourse.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "GameState.h"


const ScreenMessage SM_GoToNextState	=	ScreenMessage(SM_User + 1);


ScreenHowToPlay::ScreenHowToPlay()
{
	LOG->Trace( "ScreenHowToPlay::ScreenHowToPlay()" );

	m_Menu.Load(
		THEME->GetPathTo("Graphics","How To Play Background"), 
		THEME->GetPathTo("Graphics","How To Play Top Edge"), 
		ssprintf("%s %s to change line   %s %s to select between options      then press START", CString(char(3)), CString(char(4)), CString(char(1)), CString(char(2)) ),
		false, true, 20
		);
	this->AddSubActor( &m_Menu );

	CString sHowToPlayPath;
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
	case PLAY_MODE_ENDLESS:
		sHowToPlayPath = THEME->GetPathTo("Graphics","How To Play Arcade");
		break;
	case PLAY_MODE_ONI:
		sHowToPlayPath = THEME->GetPathTo("Graphics","How To Play Arcade");
		break;
	default:
		ASSERT(0);
	}

	m_sprHowToPlay.Load( sHowToPlayPath );
	m_sprHowToPlay.SetXY( CENTER_X, CENTER_Y );
	this->AddSubActor( &m_sprHowToPlay );

	m_sprHowToPlay.SetX( SCREEN_LEFT-SCREEN_WIDTH );
	m_sprHowToPlay.BeginTweening( 0.3f, Actor::TWEEN_BIAS_BEGIN );
	m_sprHowToPlay.SetTweenX( 0 );
}

ScreenHowToPlay::~ScreenHowToPlay()
{
	LOG->Trace( "ScreenHowToPlay::~ScreenHowToPlay()" );

}

void ScreenHowToPlay::Update( float fDeltaTime )
{
	//LOG->Trace( "ScreenHowToPlay::Update(%f)", fDeltaTime );

	Screen::Update( fDeltaTime );
}

void ScreenHowToPlay::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenHowToPlay::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( m_Menu.IsClosing() )
		return;

	// default input handler
	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenHowToPlay::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_MenuTimer:
		this->MenuStart(PLAYER_1);
		break;
	case SM_GoToNextState:
		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_ARCADE:
			SCREENMAN->SetNewScreen( new ScreenSelectMusic );
			break;
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:
			SCREENMAN->SetNewScreen( new ScreenSelectCourse );
			break;
		default:
			ASSERT(0);
		}
		break;
	}
}

void ScreenHowToPlay::MenuBack( const PlayerNumber p )
{
}

void ScreenHowToPlay::MenuStart( const PlayerNumber p )
{
	m_Menu.TweenOffScreenToMenu( SM_GoToNextState );

	m_sprHowToPlay.BeginTweening( 0.3f, Actor::TWEEN_BIAS_END );
	m_sprHowToPlay.SetTweenX( SCREEN_RIGHT );
}
