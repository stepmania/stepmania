#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenNetworkWaiting

 Desc: The main title screen and menu.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenNetworkWaiting.h"
#include "SongManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "RageMusic.h"
#include "ThemeManager.h"
#include "RageNetworkClient.h"
#include "NetGameState.h"
#include "InputMapper.h"


//
// Defines specific to ScreenNetworkWaiting
//
const int SERVER_WAIT_TIMEOUT_SECS	=	60;
const float REFRESH_SECONDS = 0.2f;

#define SERVER_INFO_X			THEME->GetMetricF("ScreenNetworkWaiting","ServerInfoX")
#define SERVER_INFO_Y			THEME->GetMetricF("ScreenNetworkWaiting","ServerInfoY")
#define PLAYER_LIST_X			THEME->GetMetricF("ScreenNetworkWaiting","PlayerListX")
#define PLAYER_LIST_Y			THEME->GetMetricF("ScreenNetworkWaiting","PlayerListY")
#define HELP_TEXT				THEME->GetMetric("ScreenNetworkWaiting","HelpText")

const ScreenMessage SM_ServerGoToNextScreen	=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToPrevScreen		=	ScreenMessage(SM_User+3);
const ScreenMessage SM_GoToNextScreen		=	ScreenMessage(SM_User+4);


ScreenNetworkWaiting::ScreenNetworkWaiting()
{
	LOG->Trace( "ScreenNetworkWaiting::ScreenNetworkWaiting()" );


	//
	// I think it's better to do all the initialization here rather than have it scattered
	// about in all the global singleton classes
	//
	GAMESTATE->Reset();
	PREFSMAN->ReadGamePrefsFromDisk();
	INPUTMAPPER->ReadMappingsFromDisk();
	GAMESTATE->m_bPlayersCanJoin = true;
	if( !GAMEMAN->DoesNoteSkinExist( GAMEMAN->GetCurNoteSkin() ) )
	{
		CStringArray asNoteSkinNames;
		GAMEMAN->GetNoteSkinNames( asNoteSkinNames );
		GAMEMAN->SwitchNoteSkin( asNoteSkinNames[0] );
	}
	if( !THEME->DoesThemeExist( THEME->GetCurThemeName() ) )
	{
		CString sGameName = GAMESTATE->GetCurrentGameDef()->m_szName;
		if( THEME->DoesThemeExist( sGameName ) )
			THEME->SwitchTheme( sGameName );
		else
			THEME->SwitchTheme( "default" );
	}
	PREFSMAN->SaveGamePrefsToDisk();
	
	

	m_Menu.Load( 
		THEME->GetPathTo("BGAnimations","network waiting"), 
		THEME->GetPathTo("Graphics","network waiting top edge"),
		HELP_TEXT, false, false, 99 
		);
	this->AddChild( &m_Menu );


	m_textServerInfo.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textServerInfo.SetXY( SERVER_INFO_X, SERVER_INFO_Y );
	m_textServerInfo.SetText( "Server Info" );
	this->AddChild( &m_textServerInfo );

	m_textPlayerList.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textPlayerList.SetXY( PLAYER_LIST_X, PLAYER_LIST_Y );
	this->AddChild( &m_textPlayerList );

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","network waiting music") );

	m_Menu.TweenOnScreenFromBlack( SM_None );

	m_bReady = false;
	NetPlayerState ps = { "billy", 100, 100, m_bReady };
	CLIENT->SendMyPlayerState( ps );

	m_fUpdateTimer = REFRESH_SECONDS;
}


ScreenNetworkWaiting::~ScreenNetworkWaiting()
{
	LOG->Trace( "ScreenNetworkWaiting::~ScreenNetworkWaiting()" );
}

void ScreenNetworkWaiting::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );
	
	m_fUpdateTimer -= fDeltaTime;
	if( m_fUpdateTimer < 0 )
	{
		CString s;
		NetGameState& gns = CLIENT->m_NetGameState;
		for( int i=0; i<gns.num_players; i++ )
		{
			s += gns.player[i].name;
			s += gns.player[i].bReady ? " (Ready)" : " (Not Ready)";
			s += '\n';
		}
		
		m_textPlayerList.SetText( s );
		
		m_fUpdateTimer = REFRESH_SECONDS;
	}
}

void ScreenNetworkWaiting::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenNetworkWaiting::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenNetworkWaiting::Input()" );

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenNetworkWaiting::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_ServerGoToNextScreen:
		SCREENMAN->SetNewScreen( "ScreenNetworkMenu" );
		break;
	}
}

void ScreenNetworkWaiting::MenuUp( PlayerNumber pn )
{
	m_bReady = true;
	NetPlayerState ps = { "billy", 100, 100, m_bReady };
	CLIENT->SendMyPlayerState( ps );
}

void ScreenNetworkWaiting::MenuDown( PlayerNumber pn )
{
	m_bReady = false;
	NetPlayerState ps = { "billy", 100, 100, m_bReady };
	CLIENT->SendMyPlayerState( ps );
}

