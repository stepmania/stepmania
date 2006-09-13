#include "global.h"

#if !defined(WITHOUT_NETWORKING)
#include "ScreenNetSelectBase.h"
#include "ScreenManager.h"
#include "ThemeManager.h"
#include "RageTimer.h"
#include "ActorUtil.h"
#include "Actor.h"
#include "GameSoundManager.h"
#include "MenuTimer.h"
#include "NetworkSyncManager.h"
#include "RageUtil.h"
#include "GameState.h"
#include "InputEventPlus.h"
#include "RageInput.h"

#define CHATINPUT_WIDTH				THEME->GetMetricF(m_sName,"ChatInputBoxWidth")
#define CHATINPUT_HEIGHT			THEME->GetMetricF(m_sName,"ChatInputBoxHeight")
#define CHATOUTPUT_WIDTH			THEME->GetMetricF(m_sName,"ChatOutputBoxWidth")
#define CHATOUTPUT_HEIGHT			THEME->GetMetricF(m_sName,"ChatOutputBoxHeight")
#define SHOW_CHAT_LINES				THEME->GetMetricI(m_sName,"ChatOutputLines")

#define CHAT_TEXT_OUTPUT_WIDTH		THEME->GetMetricF(m_sName,"ChatTextOutputWidth")
#define CHAT_TEXT_INPUT_WIDTH		THEME->GetMetricF(m_sName,"ChatTextInputWidth")

#define USERSALT_Y					THEME->GetMetricF(m_sName,"UsersAY")
#define USERSDELT_X					THEME->GetMetricF(m_sName,"UsersDX")
#define USERS_Y						THEME->GetMetricF(m_sName,"UsersY")
#define USERS_X						THEME->GetMetricF(m_sName,"UsersX")

AutoScreenMessage( SM_AddToChat )
AutoScreenMessage( SM_UsersUpdate )
AutoScreenMessage( SM_SMOnlinePack )

REGISTER_SCREEN_CLASS( ScreenNetSelectBase );

void ScreenNetSelectBase::Init()
{
	ScreenWithMenuElements::Init();

	//ChatBox
	m_sprChatInputBox.SetName( "ChatInputBox" );
	m_sprChatInputBox.Load( THEME->GetPathG( m_sName, "ChatInputBox" ) );
	m_sprChatInputBox.SetWidth( CHATINPUT_WIDTH );
	m_sprChatInputBox.SetHeight( CHATINPUT_HEIGHT );
	SET_XY_AND_ON_COMMAND( m_sprChatInputBox );
	this->AddChild( &m_sprChatInputBox );

	m_sprChatOutputBox.SetName( "ChatOutputBox" );
	m_sprChatOutputBox.Load( THEME->GetPathG( m_sName, "ChatOutputBox" ) );
	m_sprChatOutputBox.SetWidth( CHATOUTPUT_WIDTH );
	m_sprChatOutputBox.SetHeight( CHATOUTPUT_HEIGHT );
	SET_XY_AND_ON_COMMAND( m_sprChatOutputBox );
	this->AddChild( &m_sprChatOutputBox );

	m_textChatInput.LoadFromFont( THEME->GetPathF(m_sName,"chat") );
	m_textChatInput.SetHorizAlign( align_left );
	m_textChatInput.SetVertAlign( align_top );
	m_textChatInput.SetShadowLength( 0 );
	m_textChatInput.SetName( "ChatInput" );
	m_textChatInput.SetWrapWidthPixels( (int)(CHAT_TEXT_INPUT_WIDTH) );
	SET_XY_AND_ON_COMMAND( m_textChatInput );
	this->AddChild( &m_textChatInput );

	m_textChatOutput.LoadFromFont( THEME->GetPathF(m_sName,"chat") );
	m_textChatOutput.SetWrapWidthPixels( (int)(CHAT_TEXT_OUTPUT_WIDTH) );
	m_textChatOutput.SetHorizAlign( align_left );
	m_textChatOutput.SetVertAlign( align_bottom );
	m_textChatOutput.SetShadowLength( 0 );
	m_textChatOutput.SetName( "ChatOutput" );
	SET_XY_AND_ON_COMMAND( m_textChatOutput );
	this->AddChild( &m_textChatOutput );

	m_textChatOutput.SetText( NSMAN->m_sChatText );
	m_textChatOutput.SetMaxLines( SHOW_CHAT_LINES, 1 );

	//Users' Background
	
	m_rectUsersBG.SetName( "UsersBG" );
	SET_QUAD_INIT( m_rectUsersBG );
	this->AddChild( &m_rectUsersBG );

	//Display users list
	UpdateUsers();

	return;
}

void ScreenNetSelectBase::Input( const InputEventPlus &input )
{
	if( m_In.IsTransitioning() || m_Out.IsTransitioning() )
		return;

	if( input.type != IET_FIRST_PRESS && input.type != IET_SLOW_REPEAT )
		return;

	bool bHoldingCtrl = 
		INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL)) ||
		INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL)) ||
		(!NSMAN->useSMserver);	//If we are disconnected, assume no chatting

	switch( input.DeviceI.button )
	{
	case KEY_ENTER:
	case KEY_KP_ENTER:
		if (!bHoldingCtrl)
		{
			if ( m_sTextInput != "" )
				NSMAN->SendChat( m_sTextInput );
			m_sTextInput="";
			UpdateTextInput();
			return;
		}
		break;
	case KEY_BACK:
		if(!m_sTextInput.empty())
			m_sTextInput = m_sTextInput.erase( m_sTextInput.size()-1 );
		UpdateTextInput();
		break;
	default:
		wchar_t c;
		c = INPUTMAN->DeviceInputToChar(input.DeviceI, true);

		if( (c >= ' ') && (!bHoldingCtrl) )
		{
			m_sTextInput += WStringToRString(wstring()+c);
			UpdateTextInput();
		}

		//Tricky: If both players are playing, allow the 2 button through to the keymapper
		if( c == '2' && GAMESTATE->IsPlayerEnabled( PLAYER_2 ) && GAMESTATE->IsPlayerEnabled( PLAYER_1 ) )
			break;

		if( c >= ' ' )
			return;
		break;
	}
	Screen::Input( input );	// default input handler
}

void ScreenNetSelectBase::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_GoToNextScreen )
		SOUND->StopMusic();
	else if( SM == SM_AddToChat )
	{
		m_textChatOutput.SetText( NSMAN->m_sChatText );
		m_textChatOutput.SetMaxLines( SHOW_CHAT_LINES, 1 );
	}
	else if( SM == SM_UsersUpdate )
	{
		UpdateUsers();
	}

	ScreenWithMenuElements::HandleScreenMessage( SM );
}

void ScreenNetSelectBase::TweenOffScreen()
{
	OFF_COMMAND( m_sprChatInputBox );
	OFF_COMMAND( m_sprChatOutputBox );
	OFF_COMMAND( m_textChatInput );
	OFF_COMMAND( m_textChatOutput );

	for( unsigned i=0; i<m_textUsers.size(); i++ )
		OFF_COMMAND( m_textUsers[i] );

	OFF_COMMAND( m_rectUsersBG );
}

void ScreenNetSelectBase::UpdateTextInput()
{
	m_textChatInput.SetText( m_sTextInput );  
}

void ScreenNetSelectBase::UpdateUsers()
{
	float tX = USERS_X - USERSDELT_X;
	float tY = USERS_Y;

	for( unsigned i=0; i< m_textUsers.size(); i++)
		this->RemoveChild( &m_textUsers[i] );

	unsigned oldUsers = m_textUsers.size();

	m_textUsers.clear();

	m_textUsers.resize( NSMAN->m_ActivePlayer.size() );

	for( unsigned i=0; i < NSMAN->m_ActivePlayer.size(); i++)
	{
		m_textUsers[i].LoadFromFont( THEME->GetPathF(m_sName,"chat") );
		m_textUsers[i].SetHorizAlign( align_center );
		m_textUsers[i].SetVertAlign( align_top );
		m_textUsers[i].SetShadowLength( 0 );
		m_textUsers[i].SetName( "Users" );
		
		tX += USERSDELT_X;

		if ( (i % 2) == 1)
			tY = USERSALT_Y + USERS_Y;
		else
			tY = USERS_Y;
		m_textUsers[i].SetXY( tX, tY );

		if ( i > oldUsers )
			ON_COMMAND( m_textUsers[i] );
	
		m_textUsers[i].SetText( NSMAN->m_PlayerNames[NSMAN->m_ActivePlayer[i]] );
		m_textUsers[i].RunCommands( THEME->GetMetricA( m_sName,
			ssprintf("Users%dCommand", NSMAN->m_PlayerStatus[NSMAN->m_ActivePlayer[i]] ) ) );

		this->AddChild( &m_textUsers[i] );
	}
}

void UtilSetQuadInit( Actor& actor, const RString &sClassName )
{
	ActorUtil::SetXYAndOnCommand( actor, sClassName );
	actor.RunCommands( THEME->GetMetricA( sClassName, actor.GetName() + "Command" ) );
	actor.SetWidth( THEME->GetMetricF( sClassName, actor.GetName() + "Width" ) );
	actor.SetHeight( THEME->GetMetricF( sClassName, actor.GetName() + "Height" ) );
}

#endif

/*
 * (c) 2004 Charles Lohr
 * All rights reserved.
 *      Elements from ScreenTextEntry
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
