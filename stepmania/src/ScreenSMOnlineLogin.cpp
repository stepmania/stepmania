#include "global.h"

#if !defined(WITHOUT_NETWORKING)
#include "ScreenSMOnlineLogin.h"
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
#include "ScreenDimensions.h"
#include "crypto/CryptMD5.h"

const ScreenMessage SM_SMOnlinePack	= ScreenMessage(SM_User+8);

REGISTER_SCREEN_CLASS( ScreenSMOnlineLogin );
ScreenSMOnlineLogin::ScreenSMOnlineLogin( const CString& sName ) : ScreenWithMenuElements( sName )
{
	m_iPlayer = THEME->GetMetricI( sName, "PlayerNumber" ) - 1;

	if ( GAMESTATE->IsPlayerEnabled( (PlayerNumber)m_iPlayer ) )
		m_sUserName = GAMESTATE->GetPlayerDisplayName((PlayerNumber)m_iPlayer);
	else
		m_sUserName = "INVALID";

	m_sprPassword.SetName( "PasswordBG" );
	m_sprPassword.Load( THEME->GetPathG( m_sName, "PasswordBG" ) );
	m_sprPassword.SetHorizAlign( align_left );
	SET_XY_AND_ON_COMMAND( m_sprPassword );
	this->AddChild( &m_sprPassword );

	m_textTitle.LoadFromFont( THEME->GetPathF(m_sName,"text") );
	m_textTitle.SetName( "Title" );
	SET_XY_AND_ON_COMMAND( m_textTitle );
	this->AddChild( &m_textTitle );

	m_textTitle.SetText( NSMAN->GetServerName() );

	m_textUserName.LoadFromFont( THEME->GetPathF(m_sName,"text") );
	m_textUserName.SetName( "UserName" );
	m_textUserName.SetWrapWidthPixels( (int)SCREEN_WIDTH );
	SET_XY_AND_ON_COMMAND( m_textUserName );
	this->AddChild( &m_textUserName );

	//XXX: Yuck, this should be metric'ed out when we get a chance
	m_textUserName.SetText( "You are logging on as " + m_sUserName + " if this is incorrect, please change your profile." );	

	m_textPassword.LoadFromFont( THEME->GetPathF(m_sName,"text") );
	m_textPassword.SetName( "Password" );
	SET_XY_AND_ON_COMMAND( m_textPassword );
	this->AddChild( &m_textPassword );
	m_sPassword = "";
	
	m_textLoginMessage.LoadFromFont( THEME->GetPathF(m_sName,"text") );
	m_textLoginMessage.SetName( "LoginMessage" );
	SET_XY_AND_ON_COMMAND( m_textLoginMessage );
	this->AddChild( &m_textLoginMessage );

	NSMAN->isSMOLoggedIn[m_iPlayer] = false;
}

void ScreenSMOnlineLogin::Input( const DeviceInput& DeviceI, const InputEventType type,
								  const GameInput& GameI, const MenuInput& MenuI,
								  const StyleInput& StyleI )
{
	if( m_In.IsTransitioning() || m_Out.IsTransitioning() )
		return;

	if( (type != IET_FIRST_PRESS) && (type != IET_SLOW_REPEAT) && (type != IET_FAST_REPEAT ) )
		return;

	bool bHoldingShift = 
		INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT)) ||
		INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT));

	bool bHoldingCtrl = 
		INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL)) ||
		INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL)) ||
		(!NSMAN->useSMserver);	//If we are disconnected, assume no chatting

	switch( DeviceI.button )
	{
	case KEY_ENTER:
	case KEY_KP_ENTER:
		if ( m_sPassword != "" )
			SendLogin();
		return;
		break;
	case KEY_BACK:
		if(!m_sPassword.empty())
			m_sPassword = m_sPassword.erase( m_sPassword.size()-1 );
		UpdateTextInput();
		break;
	default:
		char c;
		c = DeviceI.ToChar();

		if( bHoldingShift && !bHoldingCtrl )
		{
			c = (char)toupper(c);

			switch( c )
			{
			case '`':	c='~';	break;
			case '1':	c='!';	break;
			case '2':	c='@';	break;
			case '3':	c='#';	break;
			case '4':	c='$';	break;
			case '5':	c='%';	break;
			case '6':	c='^';	break;
			case '7':	c='&';	break;
			case '8':	c='*';	break;
			case '9':	c='(';	break;
			case '0':	c=')';	break;
			case '-':	c='_';	break;
			case '=':	c='+';	break;
			case '[':	c='{';	break;
			case ']':	c='}';	break;
			case '\'':	c='"';	break;
			case '\\':	c='|';	break;
			case ';':	c=':';	break;
			case ',':	c='<';	break;
			case '.':	c='>';	break;
			case '/':	c='?';	break;
			}
		}
		if( (c >= ' ') && (!bHoldingCtrl) )
		{
			m_sPassword += c;
			UpdateTextInput();
		}
		break;
	}
	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

void ScreenSMOnlineLogin::HandleScreenMessage( const ScreenMessage SM )
{
	Screen::HandleScreenMessage( SM );
	switch( SM )
	{
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( THEME->GetMetric (m_sName, "PrevScreen") );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( THEME->GetMetric (m_sName, "NextScreen") );
		break;
	case SM_SMOnlinePack:

		int ResponceCode = NSMAN->m_SMOnlinePacket.Read1();
		if ( ResponceCode == 0 )
		{
			int Status = NSMAN->m_SMOnlinePacket.Read1();
			if ( Status == 0 )
			{
				NSMAN->isSMOLoggedIn[m_iPlayer] = true;
				SCREENMAN->SendMessageToTopScreen( SM_GoToNextScreen );
			} 
			else 
			{
				CString Responce = NSMAN->m_SMOnlinePacket.ReadNT();
				m_textLoginMessage.SetText( Responce );
			}
		}
		//Else we ignore it.

		break;
	}
}

void ScreenSMOnlineLogin::MenuStart( PlayerNumber pn )
{
	SendLogin();
}

void ScreenSMOnlineLogin::MenuBack( PlayerNumber pn )
{
	SCREENMAN->SendMessageToTopScreen( SM_GoToPrevScreen );
}

void ScreenSMOnlineLogin::TweenOffScreen()
{
	OFF_COMMAND( m_textTitle );
	OFF_COMMAND( m_textUserName );
	OFF_COMMAND( m_textPassword );
	OFF_COMMAND( m_textLoginMessage );
	OFF_COMMAND( m_sprPassword );
}

void ScreenSMOnlineLogin::UpdateTextInput()
{
	CString PasswordText;

	for ( unsigned i = 0; i < m_sPassword.length(); ++i )
		PasswordText+='*';

	m_textPassword.SetText( PasswordText );  
}

void ScreenSMOnlineLogin::SendLogin()
{
	CString HashedName;
	CString PreHashedName;

	unsigned char Output[16];
	const unsigned char *Input = (unsigned char *)m_sPassword.c_str();

	MD5Context BASE;

	memset(&BASE,0,sizeof(MD5Context));
	memset(&Output,0,16);

	MD5Init( &BASE );


	MD5Update( &BASE, Input, m_sPassword.length());

	MD5Final( Output, &BASE );

	for (int i = 0; i < 16; i++)
		PreHashedName += ssprintf( "%2X", Output[i] );

	//XXX: Yuck. Convert spaces to 0's better. (will fix soon)
	for (int i = 0; i < 32; i++)
		if ( PreHashedName.c_str()[i] == ' ' )
			HashedName += '0';
		else
			if ( PreHashedName.c_str()[i]=='\0' )
				HashedName += ' ';
			else
				HashedName += PreHashedName.c_str()[i];

		NSMAN->m_SMOnlinePacket.ClearPacket();

	NSMAN->m_SMOnlinePacket.Write1((uint8_t)0); //Login command
	NSMAN->m_SMOnlinePacket.Write1((uint8_t)m_iPlayer); //Player
	NSMAN->m_SMOnlinePacket.Write1((uint8_t)0); //MD5 hash style
	NSMAN->m_SMOnlinePacket.WriteNT(m_sUserName);
	NSMAN->m_SMOnlinePacket.WriteNT(HashedName);

	NSMAN->SendSMOnline( );

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
