#include "global.h"
#include "Screen.h"
#include "GameManager.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "RageSound.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "ScreenManager.h"
#include "GameSoundManager.h"
#include "ActorUtil.h"
#include "InputEventPlus.h"

#define NEXT_SCREEN					THEME->GetMetric (m_sName,"NextScreen")
#define PREV_SCREEN					THEME->GetMetric (m_sName,"PrevScreen")
#define PREPARE_SCREENS				THEME->GetMetric (m_sName,"PrepareScreens")
#define PERSIST_SCREENS				THEME->GetMetric (m_sName,"PersistScreens")
#define GROUPED_SCREENS				THEME->GetMetric (m_sName,"GroupedScreens")

void Screen::InitScreen( Screen *pScreen )
{
	/* Set the name of the loading screen. */
	LuaReference Old;
	{
		Lua *L = LUA->Get();
		LuaHelpers::Push( pScreen->GetName(), L );
		ActorUtil::SetParamFromStack( L, "LoadingScreen", &Old );
		LUA->Release( L );
	}

	pScreen->Init();

	/* Restore the old value. */
	{
		Lua *L = LUA->Get();
		Old.PushSelf( L );
		ActorUtil::SetParamFromStack( L, "LoadingScreen" );
		LUA->Release( L );
	}
}

Screen::Screen( CString sName )
{
	SetName( sName );

	ALLOW_OPERATOR_MENU_BUTTON.Load( sName, "AllowOperatorMenuButton" );
}

Screen::~Screen()
{

}

bool Screen::SortMessagesByDelayRemaining(const Screen::QueuedScreenMessage &m1,
										 const Screen::QueuedScreenMessage &m2)
{
	return m1.fDelayRemaining < m2.fDelayRemaining;
}

void Screen::Init()
{
	SetFOV( 0 );

	m_smSendOnPop = SM_None;

	ActorUtil::LoadAllCommandsFromName( *this, m_sName, "Screen" );

	this->PlayCommand( "Init" );

	vector<CString> asList;
	split( PREPARE_SCREENS, ",", asList );
	for( unsigned i = 0; i < asList.size(); ++i )
	{
		LOG->Trace( "Screen \"%s\" preparing \"%s\"", m_sName.c_str(), asList[i].c_str() );
		SCREENMAN->PrepareScreen( asList[i] );
	}

	asList.clear();
	split( GROUPED_SCREENS, ",", asList );
	for( unsigned i = 0; i < asList.size(); ++i )
		SCREENMAN->GroupScreen( asList[i] );

	asList.clear();
	split( PERSIST_SCREENS, ",", asList );
	for( unsigned i = 0; i < asList.size(); ++i )
		SCREENMAN->PersistantScreen( asList[i] );
}

void Screen::BeginScreen()
{
	this->RunCommands( THEME->GetMetricA(m_sName, "ScreenOnCommand") );
}

void Screen::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	/*
	 * We need to ensure two things:
	 * 1. Messages must be sent in the order of delay.  If two messages are sent
	 *    simultaneously, one with a .001 delay and another with a .002 delay, the
	 *    .001 delay message must be sent first.
	 * 2. Messages to be delivered simultaneously must be sent in the order queued.
	 * 
	 * Sort by time to ensure #1; use a stable sort to ensure #2.
	 */
	stable_sort(m_QueuedMessages.begin(), m_QueuedMessages.end(), SortMessagesByDelayRemaining);

	/* Update the times of queued ScreenMessages. */
	for( unsigned i=0; i<m_QueuedMessages.size(); i++ )
	{
		/* Hack:
		 *
		 * If we simply subtract time and then send messages, we have a problem.
		 * Messages are queued to arrive at specific times, and those times line
		 * up with things like tweens finishing.  If we send the message at the
		 * exact time given, then it'll be on the same cycle that would be rendering
		 * the last frame of a tween (such as an object going off the screen).  However,
		 * when we send the message, we're likely to set up a new screen, which
		 * causes everything to stop in place; this results in actors occasionally
		 * not quite finishing their tweens.
		 *
		 * Let's delay all messages that have a non-zero time an extra frame. 
		 */
		if( m_QueuedMessages[i].fDelayRemaining > 0.0001f )
		{
			m_QueuedMessages[i].fDelayRemaining -= fDeltaTime;
			m_QueuedMessages[i].fDelayRemaining = max( m_QueuedMessages[i].fDelayRemaining, 0.0001f );
		}
		else
		{
			m_QueuedMessages[i].fDelayRemaining -= fDeltaTime;
		}
	}

	/* Now dispatch messages.  If the number of messages on the queue changes
	 * within HandleScreenMessage, someone cleared messages on the queue.  This
	 * means we have no idea where 'i' is, so start over. Since we applied time
	 * already, this won't cause messages to be mistimed. */
	for( unsigned i=0; i<m_QueuedMessages.size(); i++ )
	{
		if( m_QueuedMessages[i].fDelayRemaining > 0.0f )
			continue; /* not yet */

		/* Remove the message from the list. */
		const ScreenMessage SM = m_QueuedMessages[i].SM;
		m_QueuedMessages.erase( m_QueuedMessages.begin()+i );
		i--;

		unsigned iSize = m_QueuedMessages.size();

		// send this sucker!
		CHECKPOINT_M( ssprintf("ScreenMessage(%i)", SM) );
		this->HandleScreenMessage( SM );

		/* If the size changed, start over. */
		if( iSize != m_QueuedMessages.size() )
			i = 0;
	}
}

void Screen::MenuUp(	const InputEventPlus &input )	{ if(input.type==IET_FIRST_PRESS) MenuUp(input.MenuI.player); }
void Screen::MenuDown(	const InputEventPlus &input )	{ if(input.type==IET_FIRST_PRESS) MenuDown(input.MenuI.player); }
void Screen::MenuLeft(	const InputEventPlus &input )	{ if(input.type==IET_FIRST_PRESS) MenuLeft(input.MenuI.player); }
void Screen::MenuRight( const InputEventPlus &input )	{ if(input.type==IET_FIRST_PRESS) MenuRight(input.MenuI.player); }
void Screen::MenuStart( const InputEventPlus &input )	{ if(input.type==IET_FIRST_PRESS) MenuStart(input.MenuI.player); }
void Screen::MenuSelect( const InputEventPlus &input )	{ if(input.type==IET_FIRST_PRESS) MenuSelect(input.MenuI.player); }

void Screen::MenuBack( const InputEventPlus &input )
{
	if(!PREFSMAN->m_bDelayedBack || input.type==IET_SLOW_REPEAT || input.type==IET_FAST_REPEAT)
		MenuBack( input.MenuI.player) ; 
}

void Screen::MenuCoin(	const InputEventPlus &input )	{ if(input.type==IET_FIRST_PRESS) MenuCoin(input.MenuI.player); }

/* ScreenManager sends input here first.  Overlay screens can use it to get a first
 * pass at input.  Return true if the input was handled and should not be passed
 * to lower screens, or false if not handled.  If true is returned, Input() will
 * not be called, either.  Normal screens should not overload this function. */
bool Screen::OverlayInput( const InputEventPlus &input )
{
	return false;
}

void Screen::Input( const InputEventPlus &input )
{
	/* Don't send release messages with the default handler. */
	switch( input.type )
	{
	case IET_FIRST_PRESS:
	case IET_SLOW_REPEAT:
	case IET_FAST_REPEAT:
		break; /* OK */
	default:
		return; // don't care
	}

	/* Don't make the user hold the back button if they're pressing escape and escape is the back button. */
	if( input.MenuI.button == MENU_BUTTON_BACK && input.DeviceI.device == DEVICE_KEYBOARD  &&  input.DeviceI.button == KEY_ESC )
	{
		this->MenuBack( input.MenuI.player );
		return;
	}

	// default input handler used by most menus
	if( !input.MenuI.IsValid() )
		return;

	switch( input.MenuI.button )
	{
	case MENU_BUTTON_UP:	this->MenuUp	( input );	return;
	case MENU_BUTTON_DOWN:	this->MenuDown	( input );	return;
	case MENU_BUTTON_LEFT:	this->MenuLeft	( input );	return;
	case MENU_BUTTON_RIGHT:	this->MenuRight	( input );	return;
	case MENU_BUTTON_BACK:	this->MenuBack	( input );	return;
	case MENU_BUTTON_START:	this->MenuStart	( input );	return;
	case MENU_BUTTON_SELECT:this->MenuSelect( input );	return;
	case MENU_BUTTON_COIN:	this->MenuCoin	( input );	return;
	}
}

void Screen::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_MenuTimer:
		FOREACH_HumanPlayer(p)
			MenuStart( p );
		break;
	case SM_GoToNextScreen:
		if( SCREENMAN->IsStackedScreen(this) )
			SCREENMAN->PopTopScreen( m_smSendOnPop );
		else
			SCREENMAN->SetNewScreen( GetNextScreen() );
		break;
	case SM_GoToPrevScreen:
		if( SCREENMAN->IsStackedScreen(this) )
			SCREENMAN->PopTopScreen( m_smSendOnPop );
		else
			SCREENMAN->SetNewScreen( GetPrevScreen() );
		break;
	}
}

CString Screen::GetNextScreen() const
{
	if( !m_sNextScreen.empty() )
		return m_sNextScreen;
	return NEXT_SCREEN;
}

CString Screen::GetPrevScreen() const
{
	if( !m_sPrevScreen.empty() )
		return m_sPrevScreen;
	return PREV_SCREEN;
}


bool Screen::JoinInput( const MenuInput &MenuI )
{
	if( !GAMESTATE->PlayersCanJoin() )
		return false;

	if( MenuI.IsValid()  &&  MenuI.button==MENU_BUTTON_START )
	{
		/* If this side is already in, don't re-join (and re-pay!). */
		if(GAMESTATE->m_bSideIsJoined[MenuI.player])
			return false;

		/* subtract coins */
		int iCoinsNeededToJoin = GAMESTATE->GetCoinsNeededToJoin();

		if( GAMESTATE->m_iCoins < iCoinsNeededToJoin )
			return false;	// not enough coins
		else
			GAMESTATE->m_iCoins -= iCoinsNeededToJoin;

		// HACK: Only play start sound for the 2nd player who joins.  The 
		// start sound for the 1st player will be played by ScreenTitleMenu 
		// when the player makes a selection on the screen.
		if( GAMESTATE->GetNumSidesJoined() > 0 )
			SCREENMAN->PlayStartSound();

		GAMESTATE->JoinPlayer( MenuI.player );

		// don't load memory card profiles here.  It's slow and can cause a big skip.
		/* Don't load the local profile, either.  It causes a 150+ms skip on my A64 3000+,
		 * so it probably causes a skip for everyone.  We probably shouldn't load this here,
		 * anyway: leave it unloaded and display "INSERT CARD" until the normal time, and
		 * load the local profile at the time we would have loaded the memory card if none
		 * was inserted (via LoadFirstAvailableProfile). */
//		PROFILEMAN->LoadLocalProfileFromMachine( MenuI.player );
		SCREENMAN->RefreshCreditsMessages();

		return true;
	}

	return false;
}


void Screen::MenuCoin( PlayerNumber pn )
{
	// This is now handled globally by Stepmania.cpp  --  Miryokuteki
}

void Screen::PostScreenMessage( const ScreenMessage SM, float fDelay )
{
	ASSERT( fDelay >= 0.0 );

	QueuedScreenMessage QSM;
	QSM.SM = SM;
	QSM.fDelayRemaining = fDelay;
	m_QueuedMessages.push_back( QSM );
}

void Screen::ClearMessageQueue()
{
	m_QueuedMessages.clear(); 
}

void Screen::ClearMessageQueue( const ScreenMessage SM )
{
	for( int i=m_QueuedMessages.size()-1; i>=0; i-- )
		if( m_QueuedMessages[i].SM == SM )
			m_QueuedMessages.erase( m_QueuedMessages.begin()+i ); 
}

// lua start
#include "LuaBinding.h"

class LunaScreen: public Luna<Screen>
{
public:
	LunaScreen() { LUA->Register( Register ); }

	static void Register( Lua *L )
	{
		Luna<T>::Register( L );
	}
};

LUA_REGISTER_DERIVED_CLASS( Screen, ActorFrame )
// lua end

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
