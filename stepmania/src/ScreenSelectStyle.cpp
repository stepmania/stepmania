#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectStyle

 Desc: See Header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ScreenSelectStyle.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "RageSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "ModeSelector.h"


#define SELECTOR				THEME->GetMetric ("ScreenSelectStyle","Selector")
#define HELP_TEXT				THEME->GetMetric ("ScreenSelectStyle","HelpText")
#define TIMER_SECONDS			THEME->GetMetricI("ScreenSelectStyle","TimerSeconds")
#define NEXT_SCREEN				THEME->GetMetric ("ScreenSelectStyle","NextScreen")


ScreenSelectStyle::ScreenSelectStyle()
{
	LOG->Trace( "ScreenSelectStyle::ScreenSelectStyle()" );


	GAMESTATE->m_bPlayersCanJoin = true;


	vector<ModeChoice> aModeChoices;
	
	vector<Style> aStyles;
	GAMEMAN->GetStylesForGame( GAMESTATE->m_CurGame, aStyles );
	ASSERT( !aStyles.empty() );	// every game should have at least one Style, or else why have the Game? :-)

	for( unsigned s=0; s<aStyles.size(); s++ )
	{
		Style style = aStyles[s];
		const StyleDef* pStyleDef = GAMEMAN->GetStyleDefForStyle(aStyles[s]);

		int iNumSidesJoinedToPlay;
		switch( pStyleDef->m_StyleType )
		{
		case StyleDef::ONE_PLAYER_ONE_CREDIT:	iNumSidesJoinedToPlay = 1;	break;
		case StyleDef::TWO_PLAYERS_TWO_CREDITS:	iNumSidesJoinedToPlay = 2;	break;
		case StyleDef::ONE_PLAYER_TWO_CREDITS:	iNumSidesJoinedToPlay = 2;	break;
		default:	ASSERT(0);	iNumSidesJoinedToPlay = 1;	
		}
		
		ModeChoice mc = {
			GAMESTATE->m_CurGame,
			PLAY_MODE_INVALID,
			style,
			DIFFICULTY_INVALID,
			"",
			iNumSidesJoinedToPlay			
		};
		strcpy( mc.name, pStyleDef->m_szName );
		aModeChoices.push_back( mc );
	}

	m_pSelector = ModeSelector::Create( SELECTOR );
	m_pSelector->Init( aModeChoices, "ScreenSelectStyle", "select style" );
	this->AddChild( m_pSelector );

	m_Menu.Load( 	
		THEME->GetPathTo("BGAnimations","select style"), 
		THEME->GetPathTo("Graphics","select style top edge"),
		HELP_TEXT, false, true, TIMER_SECONDS
		);
	this->AddChild( &m_Menu );

	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select style intro") );

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","select style music") );

	m_Menu.TweenOnScreenFromBlack( SM_None );
}


ScreenSelectStyle::~ScreenSelectStyle()
{
	LOG->Trace( "ScreenSelectStyle::~ScreenSelectStyle()" );
}

void ScreenSelectStyle::Update( float fDelta )
{
	Screen::Update( fDelta );
}

void ScreenSelectStyle::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenSelectStyle::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenSelectStyle::Input()" );

	if( m_Menu.IsClosing() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

void ScreenSelectStyle::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_BeginFadingOut:		// sent by our ModeSelector
		ModeChoice mc;
		m_pSelector->GetSelectedModeChoice( GAMESTATE->m_MasterPlayerNumber, &mc );
		GAMESTATE->ApplyModeChoice( &mc, GAMESTATE->m_MasterPlayerNumber );

		GAMESTATE->m_bPlayersCanJoin = false;
		SCREENMAN->RefreshCreditsMessages();

		m_Menu.StopTimer();
		m_pSelector->TweenOffScreen();

		m_Menu.TweenOffScreenToMenu( SM_GoToNextScreen );
		break;
	case SM_MenuTimer:
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsPlayerEnabled(p) )
					MenuStart( (PlayerNumber)p );
		}
		break;
	case SM_GoToPrevScreen:
		SOUNDMAN->StopMusic();
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		break;
	}
}


void ScreenSelectStyle::MenuLeft( PlayerNumber pn )
{
	m_pSelector->MenuLeft( pn );
}

void ScreenSelectStyle::MenuRight( PlayerNumber pn )
{
	m_pSelector->MenuRight( pn );
}

void ScreenSelectStyle::MenuUp( PlayerNumber pn )
{
	m_pSelector->MenuUp( pn );
}

void ScreenSelectStyle::MenuDown( PlayerNumber pn )
{
	m_pSelector->MenuDown( pn );
}

void ScreenSelectStyle::MenuStart( PlayerNumber pn )
{
	if( pn!=PLAYER_INVALID  && !GAMESTATE->m_bSideIsJoined[pn] )
	{
		/* I think JP should allow playing two-pad singleplayer modes (doubles),
		 * but not two-pad two-player modes (battle).  (Battle mode isn't "joint".)
		 * That means we should leave player-entry logic alone and simply enable
		 * couples mode if JP is on and only one person has clicked in.  (However,
		 * that means we'll display couples even if we don't really know if we have
		 * a second pad, which is a little annoying.) 
		 *
		 * Also, credit deduction should be handled in StepMania.cpp (along with
		 * the coin logic) using GAMESTATE->m_bPlayersCanJoin, since there
		 * are other screens you can join (eg ScreenCaution). -glenn */
		if( PREFSMAN->m_CoinMode == PrefsManager::COIN_PAY )
		{
			if( !PREFSMAN->m_bJointPremium )
			{
				if( GAMESTATE->m_iCoins < PREFSMAN->m_iCoinsPerCredit )
				{
					/* Joint Premium is NOT enabled, and we do not have enough credits */
					return;
				}

				/* Joint Premium is NOT enabled, but we have enough credits. Pay up! */
				GAMESTATE->m_iCoins -= PREFSMAN->m_iCoinsPerCredit;
			}
		}

		/* If credits had to be used, it's already taken care of.. add the player */
		SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","menu start") );
		GAMESTATE->m_bSideIsJoined[pn] = true;
		SCREENMAN->RefreshCreditsMessages();
		m_pSelector->UpdateSelectableChoices();
	}
	else
		m_pSelector->MenuStart( pn );
}

void ScreenSelectStyle::MenuBack( PlayerNumber pn )
{
	SOUNDMAN->StopMusic();

	m_Menu.TweenOffScreenToBlack( SM_GoToPrevScreen, true );
}
