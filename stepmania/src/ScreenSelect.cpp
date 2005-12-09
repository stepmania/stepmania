#include "global.h"
#include "ScreenSelect.h"
#include "ScreenManager.h"
#include "GameSoundManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "GameCommand.h"
#include "LightsManager.h"
#include "Command.h"
#include "InputEventPlus.h"

#define CHOICE_NAMES			THEME->GetMetric (m_sName,"ChoiceNames")
#define CHOICE( s )				THEME->GetMetricM(m_sName,ssprintf("Choice%s",s.c_str()))
#define CODE_NAMES				THEME->GetMetric (m_sName,"CodeNames")
#define CODE( s )				THEME->GetMetric (m_sName,ssprintf("Code%s",s.c_str()))
#define CODE_ACTION( s )		THEME->GetMetricM(m_sName,ssprintf("Code%sAction",s.c_str()))
#define IDLE_TIMEOUT_SCREEN		THEME->GetMetric (m_sName,"IdleTimeoutScreen")
#define UPDATE_ON_MESSAGE		THEME->GetMetric (m_sName,"UpdateOnMessage")

ScreenSelect::ScreenSelect( CString sClassName ) : 
	ScreenWithMenuElements(sClassName),
	IDLE_COMMENT_SECONDS(m_sName,"IdleCommentSeconds"),
	IDLE_TIMEOUT_SECONDS(m_sName,"IdleTimeoutSeconds"),
	ALLOW_DISABLED_PLAYER_INPUT(m_sName,"AllowDisabledPlayerInput")
{
	LOG->Trace( "ScreenSelect::ScreenSelect()" );
}

void ScreenSelect::Init()
{
	ScreenWithMenuElements::Init();

	//
	// Load messages to update on
	//
	split( UPDATE_ON_MESSAGE, ",", m_asSubscribedMessages );
	for( unsigned i = 0; i < m_asSubscribedMessages.size(); ++i )
		MESSAGEMAN->Subscribe( this, m_asSubscribedMessages[i] );

	//
	// Load choices
	//
	{
		// Instead of using NUM_CHOICES, use a comma-separated list of choices.  Each
		// element in the list is a choice name.  This level of indirection 
		// makes it easier to add or remove items without having to change a bunch
		// of indices.
		vector<CString> asChoiceNames;
		split( CHOICE_NAMES, ",", asChoiceNames, true );

		for( unsigned c=0; c<asChoiceNames.size(); c++ )
		{
			CString sChoiceName = asChoiceNames[c];

			GameCommand mc;
			mc.m_sName = sChoiceName;
			mc.Load( c, CHOICE(sChoiceName) );
			m_aGameCommands.push_back( mc );
		}
	}

	//
	// Load codes
	//
	{
		vector<CString> vsCodeNames;
		split( CODE_NAMES, ",", vsCodeNames, true );

		for( unsigned c=0; c<vsCodeNames.size(); c++ )
		{
			CString sCodeName = vsCodeNames[c];

			CodeItem code;
			if( !code.Load( CODE(sCodeName) ) )
				continue;

			m_aCodes.push_back( code );
			GameCommand mc;
			mc.Load( c, CODE_ACTION(sCodeName) );
			m_aCodeChoices.push_back( mc );
		}
	}

	if( !m_aGameCommands.size() )
		RageException::Throw( "Screen \"%s\" does not set any choices", m_sName.c_str() );
}

void ScreenSelect::BeginScreen()
{
	ScreenWithMenuElements::BeginScreen();

	m_bTimeToFinalizePlayers = false;

	// derived classes can override if they want
	LIGHTSMAN->SetLightsMode( LIGHTSMODE_MENU );
}


ScreenSelect::~ScreenSelect()
{
	LOG->Trace( "ScreenSelect::~ScreenSelect()" );
	for( unsigned i = 0; i < m_asSubscribedMessages.size(); ++i )
		MESSAGEMAN->Unsubscribe( this, m_asSubscribedMessages[i] );
}

void ScreenSelect::Update( float fDelta )
{
	if( !IsTransitioning() )
	{
		if( IDLE_COMMENT_SECONDS > 0 && m_timerIdleComment.PeekDeltaTime() >= IDLE_COMMENT_SECONDS )
		{
			SOUND->PlayOnceFromAnnouncer( m_sName+" IdleComment" );
			m_timerIdleComment.GetDeltaTime();
		}
		// don't time out on this screen is coin mode is pay.  
		// If we're here, then there's a credit in the machine.
		if( GAMESTATE->GetCoinMode() != COIN_MODE_PAY )
		{
			if( IDLE_TIMEOUT_SECONDS > 0 && m_timerIdleTimeout.PeekDeltaTime() >= IDLE_TIMEOUT_SECONDS )
			{
				SCREENMAN->SetNewScreen( IDLE_TIMEOUT_SCREEN );
				m_timerIdleTimeout.GetDeltaTime();
				return;
			}
		}
	}

	ScreenWithMenuElements::Update( fDelta );
}

void ScreenSelect::DrawPrimitives()
{
	Screen::DrawPrimitives();
}

void ScreenSelect::Input( const InputEventPlus &input )
{
//	LOG->Trace( "ScreenSelect::Input()" );

	/* Reset the demonstration timer when a key is pressed. */
	m_timerIdleComment.GetDeltaTime();
	m_timerIdleTimeout.GetDeltaTime();


	// HACK: Don't process JoinInput on the TitleMenu.  Otherwise, if the user presses start
	// on a choice that doesn't move to a new screen, the player will be joined and the
	// user will still be on the title menu.
	bool bAllowJoinInput = !ALLOW_DISABLED_PLAYER_INPUT;

	if( input.MenuI.button == MENU_BUTTON_COIN ||
		(bAllowJoinInput && Screen::JoinInput(input.MenuI)) )
	{
		if( input.type == IET_FIRST_PRESS )
			this->UpdateSelectableChoices();
	
		if( !ALLOW_DISABLED_PLAYER_INPUT )
			return;	// don't let the screen handle the MENU_START press
	}

	// block input of disabled players
	if( !ALLOW_DISABLED_PLAYER_INPUT && !GAMESTATE->IsPlayerEnabled(input.MenuI.player) )
		return;

	if( IsTransitioning() )
		return;

	for( unsigned i = 0; i < m_aCodes.size(); ++i )
	{
		if( !m_aCodes[i].EnteredCode( input.GameI.controller ) )
			continue;

		LOG->Trace("entered code for index %d", i);
		m_aCodeChoices[i].Apply( input.MenuI.player );
	}
	Screen::Input( input );	// default input handler
}

void ScreenSelect::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_BeginFadingOut )	/* Screen is starting to tween out. */
	{
		/*
		 * Don't call GameCommand::Apply once per player on screens that 
		 * have a shared selection.  This can cause change messages to be broadcast
		 * multiple times.  Detect whether all players have the same choice, and 
		 * if so, call ApplyToAll instead.
		 * TODO: Think of a better way to handle this.
		 */
		int iMastersIndex = this->GetSelectionIndex( GAMESTATE->m_MasterPlayerNumber );
		bool bAllPlayersChoseTheSame = true;
		FOREACH_HumanPlayer( p )
		{
			if( this->GetSelectionIndex(p) != iMastersIndex )
			{
				bAllPlayersChoseTheSame = false;
				break;
			}
		}

		/* When applying, do make a copy of the GameCommand.  That way,
		 * GetAndClearScreen doesn't mangle m_sScreen on our real copy. */
		if( bAllPlayersChoseTheSame )
		{
			GameCommand gc = m_aGameCommands[iMastersIndex];
			CString sThisScreen = gc.GetAndClearScreen();
			if( m_sNextScreen == "" )
				m_sNextScreen = sThisScreen;
			gc.ApplyToAllPlayers();
		}
		else
		{
			FOREACH_HumanPlayer( p )
			{
				int iIndex = this->GetSelectionIndex(p);
				GameCommand gc = m_aGameCommands[iIndex];
				CString sThisScreen = gc.GetAndClearScreen();
				if( m_sNextScreen == "" )
					m_sNextScreen = sThisScreen;
				gc.Apply( p );
			}
		}

		StopTimer();

		SCREENMAN->RefreshCreditsMessages();

		//
		// Finalize players if we set a style on this screen.
		//
		FOREACH_HumanPlayer( p )
		{
			const int sel = GetSelectionIndex( p );
			if( m_aGameCommands[sel].m_pStyle )
			{
				m_bTimeToFinalizePlayers = true;
				break;
			}
		}

		if( !IsTransitioning() )
			StartTransitioningScreen( SM_GoToNextScreen );

		SCREENMAN->ConcurrentlyPrepareScreen( m_sNextScreen );
	}
	else if( SM == SM_GoToNextScreen )
	{
		/* Finalizing players can take a long time, since it reads profile data.  Do it
		 * here.(XXX: this should be done in a separate screen.) */
		if( m_bTimeToFinalizePlayers )
			GAMESTATE->PlayersFinalized();
	}

	Screen::HandleScreenMessage( SM );
}

void ScreenSelect::HandleMessage( const CString &sMessage )
{
	this->UpdateSelectableChoices();

	Screen::HandleMessage( sMessage );
}

void ScreenSelect::MenuBack( PlayerNumber pn )
{
	SOUND->StopMusic();

	Cancel( SM_GoToPrevScreen );
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

