#include "global.h"
#include "ScreenSelect.h"
#include "ScreenManager.h"
#include "GameSoundManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "GameCommand.h"
#include "InputEventPlus.h"

#include <cstddef>
#include <vector>

#define CHOICE_NAMES		THEME->GetMetric (m_sName,"ChoiceNames")
#define CHOICE( s )		THEME->GetMetric (m_sName,ssprintf("Choice%s",s.c_str()))
#define IDLE_TIMEOUT_SCREEN	THEME->GetMetric (m_sName,"IdleTimeoutScreen")
#define UPDATE_ON_MESSAGE	THEME->GetMetric (m_sName,"UpdateOnMessage")

void ScreenSelect::Init()
{
	IDLE_COMMENT_SECONDS.Load( m_sName, "IdleCommentSeconds" );
	IDLE_TIMEOUT_SECONDS.Load( m_sName, "IdleTimeoutSeconds" );
	ALLOW_DISABLED_PLAYER_INPUT.Load( m_sName, "AllowDisabledPlayerInput" );

	ScreenWithMenuElements::Init();

	// Load messages to update on
	split( UPDATE_ON_MESSAGE, ",", m_asSubscribedMessages );
	for( unsigned i = 0; i < m_asSubscribedMessages.size(); ++i )
		MESSAGEMAN->Subscribe( this, m_asSubscribedMessages[i] );
	// Subscribe to PlayerJoined, if not already.
	if( !MESSAGEMAN->IsSubscribedToMessage(this, Message_PlayerJoined) )
		this->SubscribeToMessage( Message_PlayerJoined );

	// Load choices
	// Allow lua as an alternative to metrics.
	RString choice_names= CHOICE_NAMES;
	if(choice_names.Left(4) == "lua,")
	{
		RString command= choice_names.Right(choice_names.size()-4);
		Lua* L= LUA->Get();
		if(LuaHelpers::RunExpression(L, command, m_sName + "::ChoiceNames"))
		{
			if(!lua_istable(L, 1))
			{
				LuaHelpers::ReportScriptError(m_sName + "::ChoiceNames expression did not return a table of gamecommands.");
			}
			else
			{
				std::size_t len= lua_objlen(L, 1);
				for(std::size_t i= 1; i <= len; ++i)
				{
					lua_rawgeti(L, 1, i);
					if(!lua_isstring(L, -1))
					{
						LuaHelpers::ReportScriptErrorFmt(m_sName + "::ChoiceNames element %zu is not a string.", i);
					}
					else
					{
						RString com= SArg(-1);
						GameCommand mc;
						mc.ApplyCommitsScreens(false);
						mc.m_sName = ssprintf("%zu", i);
						Commands cmd= ParseCommands(com);
						mc.Load(i, cmd);
						m_aGameCommands.push_back(mc);
					}
					lua_pop(L, 1);
				}
			}
		}
		lua_settop(L, 0);
		LUA->Release(L);
	}
	else
	{
		// Instead of using NUM_CHOICES, use a comma-separated list of choices.
		// Each element in the list is a choice name. This level of indirection
		// makes it easier to add or remove items without having to change a
		// bunch of indices.
		std::vector<RString> asChoiceNames;
		split( CHOICE_NAMES, ",", asChoiceNames, true );

		for( unsigned c=0; c<asChoiceNames.size(); c++ )
		{
			RString sChoiceName = asChoiceNames[c];

			GameCommand mc;
			mc.ApplyCommitsScreens( false );
			mc.m_sName = sChoiceName;
			Commands cmd = ParseCommands( CHOICE(sChoiceName) );
			mc.Load( c, cmd );
			m_aGameCommands.push_back( mc );
		}
	}

	if(m_aGameCommands.empty())
	{
		LuaHelpers::ReportScriptErrorFmt("Screen \"%s\" does not set any choices.", m_sName.c_str());
	}
}

void ScreenSelect::BeginScreen()
{
	ScreenWithMenuElements::BeginScreen();

	m_timerIdleComment.GetDeltaTime();
	m_timerIdleTimeout.GetDeltaTime();
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
		if( IDLE_COMMENT_SECONDS > 0  &&  m_timerIdleComment.PeekDeltaTime() >= IDLE_COMMENT_SECONDS )
		{
			SOUND->PlayOnceFromAnnouncer( m_sName+" IdleComment" );
			m_timerIdleComment.GetDeltaTime();
		}

		if( IDLE_TIMEOUT_SECONDS > 0  &&  m_timerIdleTimeout.PeekDeltaTime() >= IDLE_TIMEOUT_SECONDS )
		{
			SCREENMAN->SetNewScreen( IDLE_TIMEOUT_SCREEN );
			m_timerIdleTimeout.GetDeltaTime();
			return;
		}
	}

	ScreenWithMenuElements::Update( fDelta );
}

bool ScreenSelect::Input( const InputEventPlus &input )
{
//	LOG->Trace( "ScreenSelect::Input()" );

	/* Reset the announcer timers when a key is pressed. */
	m_timerIdleComment.GetDeltaTime();
	m_timerIdleTimeout.GetDeltaTime();

	/* Choices may change when more coins are inserted. */
	if( input.MenuI == GAME_BUTTON_COIN && input.type == IET_FIRST_PRESS )
		this->UpdateSelectableChoices();

	if( input.MenuI == GAME_BUTTON_START && input.type == IET_FIRST_PRESS && GAMESTATE->JoinInput(input.pn) )
	{
		// HACK: Only play start sound for the 2nd player who joins. The
		// start sound for the 1st player will be played by ScreenTitleMenu
		// when the player makes a selection on the screen.
		if( GAMESTATE->GetNumSidesJoined() > 1 )
			SCREENMAN->PlayStartSound();

		if( !ALLOW_DISABLED_PLAYER_INPUT )
			return false;	// don't let the screen handle the MENU_START press
	}

	if( !GAMESTATE->IsPlayerEnabled(input.pn) )
	{
		// block input of disabled players
		if( !ALLOW_DISABLED_PLAYER_INPUT )
			return false;

		/* Never allow a START press by a player that's still not joined, even if
		 * ALLOW_DISABLED_PLAYER_INPUT would allow other types of input. If we
		 * let a non-joined player start, we might start the game with no
		 * players joined (eg. if ScreenTitleJoin is started in pay with no
		 * credits). */
		if( input.MenuI == GAME_BUTTON_START )
			return false;
	}

	return ScreenWithMenuElements::Input( input ); // default input handler
}

void ScreenSelect::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_BeginFadingOut )	// Screen is starting to tween out.
	{
		/* Don't call GameCommand::Apply once per player on screens that
		 * have a shared selection. This can cause change messages to be
		 * broadcast multiple times. Detect whether all players have the
		 * same choice, and  if so, call ApplyToAll instead.
		 * TODO: Think of a better way to handle this.
		 */
		ASSERT( GAMESTATE->GetMasterPlayerNumber() != PlayerNumber_Invalid );
		int iMastersIndex = this->GetSelectionIndex( GAMESTATE->GetMasterPlayerNumber() );
		bool bAllPlayersChoseTheSame = true;
		FOREACH_HumanPlayer( p )
		{
			if( this->GetSelectionIndex(p) != iMastersIndex )
			{
				bAllPlayersChoseTheSame = false;
				break;
			}
		}

		if(!m_aGameCommands.empty())
		{
			if( bAllPlayersChoseTheSame )
			{
				const GameCommand &gc = m_aGameCommands[iMastersIndex];
				m_sNextScreen = gc.m_sScreen;
				if( !gc.m_bInvalid )
				gc.ApplyToAllPlayers();
			}
			else
			{
				FOREACH_HumanPlayer( p )
				{
					int iIndex = this->GetSelectionIndex(p);
					const GameCommand &gc = m_aGameCommands[iIndex];
					m_sNextScreen = gc.m_sScreen;
					if( !gc.m_bInvalid )
					gc.Apply( p );
				}
			}
		}
		StopTimer();

		SCREENMAN->RefreshCreditsMessages();

		ASSERT( !IsTransitioning() );
		StartTransitioningScreen( SM_GoToNextScreen );
	}

	ScreenWithMenuElements::HandleScreenMessage( SM );
}

void ScreenSelect::HandleMessage( const Message &msg )
{
	if( find(m_asSubscribedMessages.begin(), m_asSubscribedMessages.end(), msg.GetName()) != m_asSubscribedMessages.end() )
		this->UpdateSelectableChoices();

	ScreenWithMenuElements::HandleMessage( msg );
}

bool ScreenSelect::MenuBack( const InputEventPlus &input )
{
	Cancel( SM_GoToPrevScreen );
	return true;
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

