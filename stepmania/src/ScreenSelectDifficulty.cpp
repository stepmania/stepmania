#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectDifficulty

 Desc: Testing the Screen class.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenSelectDifficulty.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "RageSoundManager.h"
#include "ThemeManager.h"
#include "ModeSelector.h"


#define CHOICES					THEME->GetMetric ("ScreenSelectDifficulty","Choices")
#define SELECTOR				THEME->GetMetric ("ScreenSelectDifficulty","Selector")
#define HELP_TEXT				THEME->GetMetric("ScreenSelectDifficulty","HelpText")
#define TIMER_SECONDS			THEME->GetMetricI("ScreenSelectDifficulty","TimerSeconds")
#define NEXT_SCREEN_ARCADE		THEME->GetMetric ("ScreenSelectDifficulty","NextScreenArcade")
#define NEXT_SCREEN_ONI			THEME->GetMetric ("ScreenSelectDifficulty","NextScreenOni")
#define NEXT_SCREEN_BATTLE		THEME->GetMetric ("ScreenSelectDifficulty","NextScreenBattle")


const ScreenMessage SM_StartTweeningOffScreen	= ScreenMessage(SM_User + 3);
const ScreenMessage SM_StartFadingOut			= ScreenMessage(SM_User + 4);


ScreenSelectDifficulty::ScreenSelectDifficulty()
{
	LOG->Trace( "ScreenSelectDifficulty::ScreenSelectDifficulty()" );

	// Reset the current PlayMode
	GAMESTATE->m_PlayMode = PLAY_MODE_INVALID;
	


	vector<ModeChoice> aModeChoices;

	CStringArray asBits;
	split( CHOICES, ",", asBits );

	unsigned choice;

	for( choice=0; choice<asBits.size(); choice++ )
	{
		Difficulty dc = StringToDifficulty(asBits[choice]);
		PlayMode pm = StringToPlayMode(asBits[choice]);
		
		if( dc!=DIFFICULTY_INVALID )	// valid difficulty
		{
			ModeChoice mc = {
				GAMESTATE->m_CurGame,
				PLAY_MODE_ARCADE,
				GAMESTATE->m_CurStyle,
				dc,
				"",
				GAMESTATE->GetNumSidesJoined() };
			strcpy( mc.name, DifficultyToString(dc) );
			aModeChoices.push_back( mc );
		}
		else if( pm!=PLAY_MODE_INVALID )	// valid play mode
		{
			ModeChoice mc = {
				GAMESTATE->m_CurGame,
				pm,
				GAMESTATE->m_CurStyle,
				DIFFICULTY_MEDIUM,
				"",
				GAMESTATE->GetNumSidesJoined() };
			strcpy( mc.name, PlayModeToString(pm) );
			aModeChoices.push_back( mc );
		}
		else
			RageException::Throw( "Invalid Choice%d value '%s'.", choice+1, asBits[choice].GetString() );
	}


	m_pSelector = ModeSelector::Create( SELECTOR );
	m_pSelector->Init( aModeChoices, "ScreenSelectDifficulty", "select difficulty" );
	this->AddChild( m_pSelector );

	m_Menu.Load(
		THEME->GetPathTo("BGAnimations","select difficulty"), 
		THEME->GetPathTo("Graphics","select difficulty top edge"),
		HELP_TEXT, true, true, TIMER_SECONDS
		);
	this->AddChild( &m_Menu );


	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select difficulty intro") );

	m_Menu.TweenOnScreenFromMenu( SM_None );
	m_pSelector->TweenOnScreen();

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","select difficulty music") );
}


ScreenSelectDifficulty::~ScreenSelectDifficulty()
{
	LOG->Trace( "ScreenSelectDifficulty::~ScreenSelectDifficulty()" );

}


void ScreenSelectDifficulty::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenSelectDifficulty::Input()" );

	if( m_Menu.IsClosing() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

void ScreenSelectDifficulty::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );
}

void ScreenSelectDifficulty::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenSelectDifficulty::HandleScreenMessage( const ScreenMessage SM )
{
	int p;

	switch( SM )
	{
	case SM_MenuTimer:
		for( p=0; p<NUM_PLAYERS; p++ )
			if( GAMESTATE->IsPlayerEnabled(p) )
				MenuStart( (PlayerNumber)p );
		break;
	case SM_BeginFadingOut:		// sent by our ModeSelector
		{
			ModeChoice mc;
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsPlayerEnabled(p) )
				{
					m_pSelector->GetSelectedModeChoice( (PlayerNumber)p, &mc );
					GAMESTATE->ApplyModeChoice( &mc, (PlayerNumber)p );
				}

			m_Menu.StopTimer();
			m_pSelector->TweenOffScreen();

			m_Menu.TweenOffScreenToMenu( SM_GoToNextScreen );
		}
		break;
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	case SM_GoToNextScreen:
		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_ARCADE:	SCREENMAN->SetNewScreen( NEXT_SCREEN_ARCADE );	break;
		case PLAY_MODE_NONSTOP:
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:	SCREENMAN->SetNewScreen( NEXT_SCREEN_ONI );		break;
		case PLAY_MODE_BATTLE:	SCREENMAN->SetNewScreen( NEXT_SCREEN_BATTLE );	break;
		default:	ASSERT(0);
		}
		break;
	case SM_StartTweeningOffScreen:
		m_pSelector->TweenOffScreen();
		this->SendScreenMessage( SM_StartFadingOut, 0.8f );
		break;
	case SM_StartFadingOut:
		m_Menu.TweenOffScreenToMenu( SM_GoToNextScreen );
		break;
	}
}

void ScreenSelectDifficulty::MenuLeft( PlayerNumber pn )
{
	m_pSelector->MenuLeft( pn );
}

void ScreenSelectDifficulty::MenuRight( PlayerNumber pn )
{
	m_pSelector->MenuRight( pn );
}

void ScreenSelectDifficulty::MenuUp( PlayerNumber pn )
{
	m_pSelector->MenuUp( pn );
}

void ScreenSelectDifficulty::MenuDown( PlayerNumber pn )
{
	m_pSelector->MenuDown( pn );
}

void ScreenSelectDifficulty::MenuStart( PlayerNumber pn )
{
	m_pSelector->MenuStart( pn );
}

void ScreenSelectDifficulty::MenuBack( PlayerNumber pn )
{
	SOUNDMAN->StopMusic();

	m_Menu.TweenOffScreenToBlack( SM_GoToPrevScreen, true );
}
