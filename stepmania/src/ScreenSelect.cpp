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
#include "RageSounds.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "ModeChoice.h"
#include "RageDisplay.h"
#include "arch/ArchHooks/ArchHooks.h"


#define CHOICES					THEME->GetMetric (m_sName,"Choices")
#define HELP_TEXT				THEME->GetMetric (m_sName,"HelpText")
#define TIMER_SECONDS			THEME->GetMetricI(m_sName,"TimerSeconds")
#define NEXT_SCREEN( choice )	THEME->GetMetric (m_sName,ssprintf("NextScreen%d",choice+1))

// Temporary hack: specify announcer in selection
#define SPECIFY_ANNOUNCER		THEME->HasMetric(m_sName,"Announcer1")
#define ANNOUNCER( choice )		THEME->GetMetric (m_sName,ssprintf("Announcer%d",choice+1))


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
		ModeChoice mc;

		if( SPECIFY_ANNOUNCER )
			mc.sAnnouncer = ANNOUNCER( c );

		if( mc.FromString(asChoices[c]) )
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
		SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo(m_sName+" intro") );
		SOUND->PlayMusic( THEME->GetPathToS(m_sName+" music") );
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
				SOUND->PlayOnce( THEME->GetPathToS("Common start") );
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
					m_aModeChoices[this->GetSelectionIndex((PlayerNumber)p)].Apply( (PlayerNumber)p );

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
		SOUND->StopMusic();
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
	SOUND->StopMusic();

	m_Menu.Back( SM_GoToPrevScreen );
}
