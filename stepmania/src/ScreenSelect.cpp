#include "global.h"
#include "ScreenSelect.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "SongManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "GameCommand.h"
#include "RageDisplay.h"
#include "arch/ArchHooks/ArchHooks.h"
#include "LightsManager.h"


#define CHOICE_NAMES			THEME->GetMetric (m_sName,"ChoiceNames")
#define CHOICE( sChoiceName )	THEME->GetMetricA(m_sName,ssprintf("Choice%s",sChoiceName.c_str()))
#define CODE_NAMES				THEME->GetMetric (m_sName,"CodeNames")
#define CODE( c )				THEME->GetMetric (m_sName,ssprintf("Code%d",c+1))
#define CODE_ACTION( c )		THEME->GetMetricA(m_sName,ssprintf("Code%dAction",c+1))
#define NEXT_SCREEN( c )		THEME->GetMetric (m_sName,ssprintf("NextScreen%d",c+1))
#define IDLE_TIMEOUT_SCREEN		THEME->GetMetric (m_sName,"IdleTimeoutScreen")
#define ALLOW_DISABLED_PLAYER_INPUT		THEME->GetMetricB(m_sName,"AllowDisabledPlayerInput")

ScreenSelect::ScreenSelect( CString sClassName ) : 
	ScreenWithMenuElements(sClassName),
	IDLE_COMMENT_SECONDS(m_sName,"IdleCommentSeconds"),
	IDLE_TIMEOUT_SECONDS(m_sName,"IdleTimeoutSeconds")
{
	LOG->Trace( "ScreenSelect::ScreenSelect()" );

	//
	// Load choices
	//
	{
		// Instead of using NUM_CHOICES, use a comma-separated list of choices.  Each
		// element in the list is a choice name.  This level of indirection 
		// makes it easier to add or remove items without having to change a bunch
		// of indices.
		CStringArray asChoiceNames;
		split( CHOICE_NAMES, ",", asChoiceNames, true );

		for( unsigned c=0; c<asChoiceNames.size(); c++ )
		{
			CString sChoiceName = asChoiceNames[c];

			GameCommand mc;
			mc.m_sName = sChoiceName;
			mc.Load( c, CHOICE(sChoiceName) );
			m_aGameCommands.push_back( mc );
		
			CString sBGAnimationDir = THEME->GetPath(BGAnimations, m_sName, mc.m_sName, true);	// true="optional"
			if( sBGAnimationDir == "" )
				sBGAnimationDir = THEME->GetPathToB(m_sName+" background");
			BGAnimation *pBGA = new BGAnimation;
			m_vpBGAnimations.push_back( pBGA );
		}
	}

	//
	// Load codes
	//
	{
		CStringArray vsCodeNames;
		split( CODE_NAMES, ",", vsCodeNames, true );

		for( unsigned c=0; c<vsCodeNames.size(); c++ )
		{
			CString sCodeName = vsCodeNames[c];

			CodeItem code;
			if( !code.Load( CODE(c) ) )
				continue;

			m_aCodes.push_back( code );
			GameCommand mc;
			mc.Load( c, CODE_ACTION(c) );
			m_aCodeChoices.push_back( mc );
		}
	}

	if( !m_aGameCommands.size() )
		RageException::Throw( "Screen \"%s\" does not set any choices", m_sName.c_str() );

	// derived classes can override if they want
	LIGHTSMAN->SetLightsMode( LIGHTSMODE_MENU );
}


ScreenSelect::~ScreenSelect()
{
	LOG->Trace( "ScreenSelect::~ScreenSelect()" );
	for( unsigned i=0; i<m_vpBGAnimations.size(); i++ )
		SAFE_DELETE( m_vpBGAnimations[i] );
	m_vpBGAnimations.clear();
}

void ScreenSelect::Update( float fDelta )
{
	if(m_bFirstUpdate)
	{
		/* Don't play sounds during the ctor, since derived classes havn't loaded yet. */
		SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo(m_sName+" intro") );
		SOUND->PlayMusic( THEME->GetPathToS(m_sName+" music") );
	}

	if( !IsTransitioning() )
	{
		if( IDLE_COMMENT_SECONDS > 0 && m_timerIdleComment.PeekDeltaTime() >= IDLE_COMMENT_SECONDS )
		{
			SOUND->PlayOnceFromAnnouncer( m_sName+" IdleComment" );
			m_timerIdleComment.GetDeltaTime();
		}
		// don't time out on this screen is coin mode is pay.  
		// If we're here, then there's a credit in the machine.
		if( PREFSMAN->GetCoinMode() != COIN_PAY )
		{
			if( IDLE_TIMEOUT_SECONDS > 0 && m_timerIdleTimeout.PeekDeltaTime() >= IDLE_TIMEOUT_SECONDS )
			{
				SCREENMAN->SetNewScreen( IDLE_TIMEOUT_SCREEN );
				m_timerIdleTimeout.GetDeltaTime();
				return;
			}
		}
	}

	Screen::Update( fDelta );
	
	// GAMESTATE->m_MasterPlayerNumber is set to PLAYER_INVALID when going Back to 
	// the title screen and this screen is updated after.  TODO: find out why
	if( GAMESTATE->m_MasterPlayerNumber != PLAYER_INVALID )	
	{
		int iSelection = this->GetSelectionIndex(GAMESTATE->m_MasterPlayerNumber);
		m_vpBGAnimations[iSelection]->Update( fDelta );
	}
}

void ScreenSelect::DrawPrimitives()
{
	// GAMESTATE->m_MasterPlayerNumber is set to PLAYER_INVALID when going Back to 
	// the title screen and this screen is updated after.  TODO: find out why
	if( GAMESTATE->m_MasterPlayerNumber != PLAYER_INVALID )	
	{
		int iSelection = this->GetSelectionIndex(GAMESTATE->m_MasterPlayerNumber);
		m_vpBGAnimations[iSelection]->Draw();
	}
	Screen::DrawPrimitives();
}

void ScreenSelect::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
//	LOG->Trace( "ScreenSelect::Input()" );

	/* Reset the demonstration timer when a key is pressed. */
	m_timerIdleComment.GetDeltaTime();
	m_timerIdleTimeout.GetDeltaTime();

	if( MenuI.button == MENU_BUTTON_COIN ||
		Screen::JoinInput(DeviceI, type, GameI, MenuI, StyleI) )
	{
		if( type == IET_FIRST_PRESS )
			this->UpdateSelectableChoices();
	
		if( ALLOW_DISABLED_PLAYER_INPUT )
			;
		else
			return;	// don't let the screen handle the MENU_START press
	}

	// block input of disabled players
	if( !ALLOW_DISABLED_PLAYER_INPUT && !GAMESTATE->IsPlayerEnabled(MenuI.player) )
		return;

	if( IsTransitioning() )
		return;

	for( unsigned i = 0; i < m_aCodes.size(); ++i )
	{
		if( !m_aCodes[i].EnteredCode( GameI.controller ) )
			continue;

		LOG->Trace("entered code for index %d", i);
		m_aCodeChoices[i].Apply( MenuI.player );
	}
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
	FOREACH_HumanPlayer( p )
	{
		const int sel = GetSelectionIndex( p );
		if( m_aGameCommands[sel].m_pStyle )
			GAMESTATE->m_pCurStyle = m_aGameCommands[sel].m_pStyle;
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
		if( !IsTransitioning() )
			StartTransitioning( SM_GoToNextScreen );
		break;
	case SM_GoToNextScreen:
		{
			/* Apply here, not in SM_AllDoneChoosing, because applying can take a very
			 * long time (200+ms), and at SM_AllDoneChoosing, we're still tweening stuff
			 * off-screen. */
			FOREACH_HumanPlayer( p )
				m_aGameCommands[this->GetSelectionIndex(p)].Apply( p );

			//
			// Finalize players if we set a style on this screen.
			//
			FOREACH_HumanPlayer( p )
			{
				const int sel = GetSelectionIndex( p );
				if( m_aGameCommands[sel].m_pStyle )
				{
					GAMESTATE->PlayersFinalized();
					break;
				}
			}

			const int iSelectionIndex = GetSelectionIndex(GAMESTATE->m_MasterPlayerNumber);
			if( m_aGameCommands[iSelectionIndex].m_sScreen != "" )
				SCREENMAN->SetNewScreen( m_aGameCommands[iSelectionIndex].m_sScreen );
			else
				SCREENMAN->SetNewScreen( NEXT_SCREEN(iSelectionIndex) );
		}
		return;
	}

	Screen::HandleScreenMessage( SM );
}

void ScreenSelect::MenuBack( PlayerNumber pn )
{
	SOUND->StopMusic();

	Back( SM_GoToPrevScreen );
}

/*
 * (c) 2001-2004 Chris Danford
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

