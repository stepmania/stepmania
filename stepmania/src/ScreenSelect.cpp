#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSelect

 Desc: See Header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ScreenSelect.h"
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
#include "ModeChoice.h"


#define CHOICES					THEME->GetMetric (m_sName,"Choices")
#define HELP_TEXT				THEME->GetMetric (m_sName,"HelpText")
#define TIMER_SECONDS			THEME->GetMetricI(m_sName,"TimerSeconds")
#define NEXT_SCREEN( choice )	THEME->GetMetric (m_sName,ssprintf("NextScreen%d",choice+1))


ScreenSelect::ScreenSelect( CString sClassName ) : Screen(sClassName)
{
	LOG->Trace( "ScreenSelect::ScreenSelect()" );

	m_sName = sClassName;

	GAMESTATE->m_bPlayersCanJoin = false;	
	// Set this true later if we discover a choice that chooses the Style


	CStringArray asChoices;
	split( CHOICES, ",", asChoices );

	for( unsigned c=0; c<asChoices.size(); c++ )
	{
		CString sChoice = asChoices[c];

		ModeChoice mc = {	// fill this in below
			GAME_INVALID,
			STYLE_INVALID,
			PLAY_MODE_INVALID,
			DIFFICULTY_INVALID,
			"",
			1 };

		strncpy( mc.name, sChoice, sizeof(mc.name) );

		bool bChoiceIsInvalid = false;

		CStringArray asBits;
		split( sChoice, "-", asBits );
		for( unsigned b=0; b<asBits.size(); b++ )
		{
			CString sBit = asBits[b];

			Game game = GAMEMAN->StringToGameType( sBit );
			if( game != GAME_INVALID )
			{
				mc.game = game;
				continue;
			}

			Style style = GAMEMAN->GameAndStringToStyle( GAMESTATE->m_CurGame, sBit );
			if( style != STYLE_INVALID )
			{
				mc.style = style;
				// There is a choices that allows players to choose a style.  Allow joining.
				GAMESTATE->m_bPlayersCanJoin = true;
				continue;
			}

			PlayMode pm = StringToPlayMode( sBit );
			if( pm != PLAY_MODE_INVALID )
			{
				mc.pm = pm;
				continue;
			}

			Difficulty dc = StringToDifficulty( sBit );
			if( dc != DIFFICULTY_INVALID )
			{
				mc.dc = dc;
				continue;
			}

			LOG->Warn( "The choice token '%s' is not recognized as a Game, Style, PlayMode, or Difficulty.  The choice containing this token will be ignored.", sBit.c_str() );
			bChoiceIsInvalid |= true;
		}

		if( mc.style != STYLE_INVALID )
		{
			const StyleDef* pStyleDef = GAMEMAN->GetStyleDefForStyle(mc.style);
			switch( pStyleDef->m_StyleType )
			{
			case StyleDef::ONE_PLAYER_ONE_CREDIT:
				mc.numSidesJoinedToPlay = 1;
				break;
			case StyleDef::TWO_PLAYERS_TWO_CREDITS:
			case StyleDef::ONE_PLAYER_TWO_CREDITS:
				mc.numSidesJoinedToPlay = 2;
				break;
			default:
				ASSERT(0);
			}
		}

		if( ! bChoiceIsInvalid )
			m_aModeChoices.push_back( mc );
		
		
		CString sBGAnimationDir = THEME->GetPathTo(BGAnimations, ssprintf("%s %s",m_sName.c_str(),mc.name), true);	// true="optional"
		if( sBGAnimationDir == "" )
			sBGAnimationDir = THEME->GetPathToB(m_sName+" background");
		m_BGAnimations[c].LoadFromAniDir( sBGAnimationDir );
	}

	m_Menu.Load( sClassName );
	this->AddChild( &m_Menu );
}


ScreenSelect::~ScreenSelect()
{
	LOG->Trace( "ScreenSelect::~ScreenSelect()" );
}

void ScreenSelect::Update( float fDelta )
{
	if(m_bFirstUpdate)
	{
		/* Don't play sounds during the ctor, since derived classes havn't loaded yet. */
		SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo(m_sName+" intro") );
		SOUNDMAN->PlayMusic( THEME->GetPathToS(m_sName+" music") );
	}

	Screen::Update( fDelta );
	m_BGAnimations[ this->GetSelectionIndex(GAMESTATE->m_MasterPlayerNumber) ].Update( fDelta );
}

void ScreenSelect::DrawPrimitives()
{
	m_BGAnimations[ this->GetSelectionIndex(GAMESTATE->m_MasterPlayerNumber) ].Draw();
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenSelect::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
//	LOG->Trace( "ScreenSelect::Input()" );

	if( MenuI.IsValid() && MenuI.button==MENU_BUTTON_START )
	{
		PlayerNumber pn = MenuI.player;
		if( GAMESTATE->m_bPlayersCanJoin )
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
				
				/* Joint premium on a DDR machine does allow two player modes with a single
				 * credit.		-Chris 
				*/

				/* Indeed, I can see three different premium settings here:
				AnyPlayersTwoCoins (2 coins for doubles and 2 coins for versus)
				OnePlayerOneCoin (1 coin for doubles, but 2 coins for versus (like pump and ez2))
				TwoPlayerOneCoin (1 coin for doubles / versus play)
				perhaps we should change the joint premium system to work this way?
				That way we can support all gametypes.
				*/

				if( PREFSMAN->m_iCoinMode == COIN_PAY )
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
				SOUNDMAN->PlayOnce( THEME->GetPathToS("Common start") );
				GAMESTATE->m_bSideIsJoined[pn] = true;
				SCREENMAN->RefreshCreditsMessages();
				this->UpdateSelectableChoices();
				return;
			}
		}
	}

// For some reason the menu likes to take 10 seconds to transition O_o
// quite noticeable pause... and in the meantime the player is sitting there
// wondering why their keys aren't working... even ESC... @_@

	if( m_Menu.IsTransitioning() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

void ScreenSelect::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_AllDoneChoosing:		
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsHumanPlayer(p) )
					GAMESTATE->ApplyModeChoice( m_aModeChoices[this->GetSelectionIndex((PlayerNumber)p)], (PlayerNumber)p );

			GAMESTATE->m_bPlayersCanJoin = false;
			SCREENMAN->RefreshCreditsMessages();

			if( !m_Menu.IsTransitioning() )
				m_Menu.StartTransitioning( SM_GoToNextScreen );
		}
		break;
	case SM_MenuTimer:
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsHumanPlayer(p) )
					MenuStart( (PlayerNumber)p );
		}
		break;
	case SM_GoToPrevScreen:
		SOUNDMAN->StopMusic();
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	case SM_GoToNextScreen:
		{
			int iSelectionIndex = GetSelectionIndex(GAMESTATE->m_MasterPlayerNumber);
			SCREENMAN->SetNewScreen( NEXT_SCREEN(iSelectionIndex) );
		}
		break;
	}
}

void ScreenSelect::MenuBack( PlayerNumber pn )
{
	SOUNDMAN->StopMusic();

	m_Menu.Back( SM_GoToPrevScreen );
}
