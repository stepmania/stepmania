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


#define NUM_CHOICES				THEME->GetMetricI(m_sName,"NumChoices")
#define CHOICE( choice )		THEME->GetMetric (m_sName,ssprintf("Choice%d",choice+1))
#define HELP_TEXT				THEME->GetMetric (m_sName,"HelpText")
#define NEXT_SCREEN( choice )	THEME->GetMetric (m_sName,ssprintf("NextScreen%d",choice+1))

ScreenSelect::ScreenSelect( CString sClassName ) : Screen(sClassName)
{
	LOG->Trace( "ScreenSelect::ScreenSelect()" );

	m_sName = sClassName;

	for( int c=0; c<NUM_CHOICES; c++ )
	{
		CString sChoice = CHOICE(c);

		ModeChoice mc;
		mc.Load( c, sChoice );
		m_aModeChoices.push_back( mc );
	
		CString sBGAnimationDir = THEME->GetPathTo(BGAnimations, m_sName, mc.m_sName, true);	// true="optional"
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
	
	// GAMESTATE->m_MasterPlayerNumber is set to PLAYER_INVALID when going Back to 
	// the title screen and this screen is updated after.  TODO: find out why
	if( GAMESTATE->m_MasterPlayerNumber != PLAYER_INVALID )	
	{
		int iSelection = this->GetSelectionIndex(GAMESTATE->m_MasterPlayerNumber);
		m_BGAnimations[iSelection].Update( fDelta );
	}
}

void ScreenSelect::DrawPrimitives()
{
	// GAMESTATE->m_MasterPlayerNumber is set to PLAYER_INVALID when going Back to 
	// the title screen and this screen is updated after.  TODO: find out why
	if( GAMESTATE->m_MasterPlayerNumber != PLAYER_INVALID )	
	{
		int iSelection = this->GetSelectionIndex(GAMESTATE->m_MasterPlayerNumber);
		m_BGAnimations[iSelection].Draw();
	}
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenSelect::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
//	LOG->Trace( "ScreenSelect::Input()" );

	if( Screen::JoinInput(DeviceI, type, GameI, MenuI, StyleI) )
	{
		this->UpdateSelectableChoices();
		return;	// don't let the screen handle the MENU_START press
	}

	if( m_Menu.IsTransitioning() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

void ScreenSelect::FinalizeChoices()
{
	/* At this point, we're tweening out; we can't change the selection.
	 * We don't want to allow players to join if the style will be set,
	 * since that can change the available selection and is likely to
	 * invalidate the choice we've already made.  Hack: apply the style.
	 * (Applying the style may have other side-effects, so it'll be re-applied
	 * in SM_GoToNextScreen.) */
	for( int p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsHumanPlayer(p) )
		{
			const int sel = GetSelectionIndex( (PlayerNumber)p );
			
			if( m_aModeChoices[sel].m_style != STYLE_INVALID )
				GAMESTATE->m_CurStyle = m_aModeChoices[sel].m_style;
		}
	SCREENMAN->RefreshCreditsMessages();
}

void ScreenSelect::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	/* Screen is starting to tween out. */
	case SM_BeginFadingOut:
		FinalizeChoices();
		break;

	/* It's our turn to tween out. */
	case SM_AllDoneChoosing:		
		FinalizeChoices();
		if( !m_Menu.IsTransitioning() )
			m_Menu.StartTransitioning( SM_GoToNextScreen );
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
			/* Apply here, not in SM_AllDoneChoosing, because applying can take a very
			 * long time (200+ms), and at SM_AllDoneChoosing, we're still tweening stuff
			 * off-screen. */
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsHumanPlayer(p) )
					m_aModeChoices[this->GetSelectionIndex((PlayerNumber)p)].Apply( (PlayerNumber)p );

			const int iSelectionIndex = GetSelectionIndex(GAMESTATE->m_MasterPlayerNumber);
			if( m_aModeChoices[iSelectionIndex].m_sScreen != "" )
				SCREENMAN->SetNewScreen( m_aModeChoices[iSelectionIndex ].m_sScreen );
			else
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
