#include "global.h"
#include "ScreenTitleMenu.h"
#include "ScreenManager.h"
#include "RageUtil.h"
#include "StepMania.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "GameManager.h"
#include "ThemeManager.h"
#include "GameSoundManager.h"
#include "CodeDetector.h"
#include "RageTextureManager.h"
#include "LightsManager.h"
#include "Game.h"
#include "InputMapper.h"
#include "ProfileManager.h"
#include "CharacterManager.h"

#define COIN_MODE_CHANGE_SCREEN		THEME->GetMetric (m_sName,"CoinModeChangeScreen")

REGISTER_SCREEN_CLASS( ScreenTitleMenu );
ScreenTitleMenu::ScreenTitleMenu( CString sScreenName ) : 
	ScreenSelectMaster( sScreenName )
{
	LOG->Trace( "ScreenTitleMenu::ScreenTitleMenu()" );

	/* XXX We really need two common calls: 1, something run when exiting from gameplay
	 * (to do this reset), and 2, something run when entering gameplay, to apply default
	 * options.  Having special cases in attract screens and the title menu to reset
	 * things stinks ... */
	GAMESTATE->Reset();

	// TRICKY: Do this after GameState::Reset.
	LIGHTSMAN->SetLightsMode( LIGHTSMODE_JOINING );

	this->SubscribeToMessage( PREFSMAN->m_CoinMode.GetName()+"Changed" );
}

void ScreenTitleMenu::Init()
{
	m_bSelectIsDown = false; // used by LoadHelpText which is called by ScreenWithMenuElements::Init()

	ScreenSelectMaster::Init();

	m_soundSelectPressed.Load( THEME->GetPathS(m_sName,"select down"), true );
	m_soundProfileChange.Load( THEME->GetPathS(m_sName,"profile change"), true );
	m_soundProfileSwap.Load( THEME->GetPathS(m_sName,"profile swap"), true );

	this->SortByDrawOrder();

	CHARMAN->DemandGraphics();

	SOUND->PlayOnceFromAnnouncer( "title menu game name" );
}

ScreenTitleMenu::~ScreenTitleMenu()
{
	LOG->Trace( "ScreenTitleMenu::~ScreenTitleMenu()" );
	
	CHARMAN->UndemandGraphics();
}

void ScreenTitleMenu::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenTitleMenu::Input( %d-%d )", DeviceI.device, DeviceI.button );	// debugging gameport joystick problem

	if( m_In.IsTransitioning() || m_Cancel.IsTransitioning() ) /* not m_Out */
		return;

	if( type == IET_FIRST_PRESS )
	{
		//
		// detect codes
		//
		if( CodeDetector::EnteredCode(GameI.controller,CODE_NEXT_THEME) ||
			CodeDetector::EnteredCode(GameI.controller,CODE_NEXT_THEME2) )
		{
			THEME->NextTheme();
			ApplyGraphicOptions();	// update window title and icon
			SCREENMAN->SystemMessage( "Theme: "+THEME->GetCurThemeName() );
			SCREENMAN->SetNewScreen( m_sName );
			TEXTUREMAN->DoDelayedDelete();
		}
		if( CodeDetector::EnteredCode(GameI.controller,CODE_NEXT_ANNOUNCER) ||
			CodeDetector::EnteredCode(GameI.controller,CODE_NEXT_ANNOUNCER2) )
		{
			ANNOUNCER->NextAnnouncer();
			CString sName = ANNOUNCER->GetCurAnnouncerName();
			if( sName=="" ) sName = "(none)";
			SCREENMAN->SystemMessage( "Announcer: "+sName );
			SCREENMAN->SetNewScreen( m_sName );
		}
		if( CodeDetector::EnteredCode(GameI.controller,CODE_NEXT_GAME) ||
			CodeDetector::EnteredCode(GameI.controller,CODE_NEXT_GAME2) )
		{
			vector<const Game*> vGames;
			GAMEMAN->GetEnabledGames( vGames );
			ASSERT( !vGames.empty() );
			vector<const Game*>::iterator iter = find(vGames.begin(),vGames.end(),GAMESTATE->m_pCurGame);
			ASSERT( iter != vGames.end() );

			iter++;	// move to the next game

			// wrap
			if( iter == vGames.end() )
				iter = vGames.begin();

			GAMESTATE->m_pCurGame = *iter;

			/* Reload the theme if it's changed, but don't back to the initial screen. */
			ResetGame();

			SCREENMAN->SystemMessage( CString("Game: ") + GAMESTATE->GetCurrentGame()->m_szName );
			SCREENMAN->SetNewScreen( m_sName );
		}
	}


	if( MenuI.IsValid() )
	{
		LoadHelpText();

		PlayerNumber pn = MenuI.player;

		bool bSelectIsDown = false;
		FOREACH_PlayerNumber( p )
			bSelectIsDown |= INPUTMAPPER->IsButtonDown( MenuInput(p, MENU_BUTTON_SELECT) );
		if( bSelectIsDown )
		{
			if( type == IET_FIRST_PRESS )
			{
				switch( MenuI.button )
				{
				case MENU_BUTTON_LEFT:
					ChangeDefaultLocalProfile( pn, -1 );
					MESSAGEMAN->Broadcast( ssprintf("MenuLeftP%d",pn+1) );
					break;
				case MENU_BUTTON_RIGHT:
					ChangeDefaultLocalProfile( pn, +1 );
					MESSAGEMAN->Broadcast( ssprintf("MenuRightP%d",pn+1) );
					break;
				case MENU_BUTTON_DOWN:
					SwapDefaultLocalProfiles();
					break;
				}
			}
			return;
		}
	}


	ScreenSelectMaster::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenTitleMenu::HandleMessage( const CString& sMessage )
{
	if( sMessage == PREFSMAN->m_CoinMode.GetName()+"Changed" )
	{
		/* If the CoinMode was changed, we need to reload this screen
		 * so that the right m_aGameCommands will show */
		SCREENMAN->SetNewScreen( COIN_MODE_CHANGE_SCREEN );
	}
}

void ScreenTitleMenu::LoadHelpText()
{
	ScreenWithMenuElements::LoadHelpText();

	bool bSelectIsDown = false;
	FOREACH_PlayerNumber( p )
		bSelectIsDown |= INPUTMAPPER->IsButtonDown( MenuInput(p, MENU_BUTTON_SELECT) );

	if( bSelectIsDown )
		LOG->Trace( "bSelectIsDown" );

	/* If m_soundSelectPressed isn't loaded yet, wait until it is before we do this. */
	if( m_bSelectIsDown != bSelectIsDown && m_soundSelectPressed.IsLoaded() )
	{
		if( bSelectIsDown )
			m_soundSelectPressed.Play();

		m_bSelectIsDown = bSelectIsDown;
		if( bSelectIsDown )
			MESSAGEMAN->Broadcast( "SelectMenuOn" );
		else
			MESSAGEMAN->Broadcast( "SelectMenuOff" );
	}
}

void ScreenTitleMenu::ChangeDefaultLocalProfile( PlayerNumber pn, int iDir )
{
	CString sCurrent = PREFSMAN->GetDefaultLocalProfileID(pn);
	vector<CString> vsProfileID;
	PROFILEMAN->GetLocalProfileIDs( vsProfileID );
	if( vsProfileID.empty() )
		return;

	int iIndex = 0;
	vector<CString>::const_iterator iter = find( vsProfileID.begin(), vsProfileID.end(), sCurrent );
	if( iter != vsProfileID.end() )
		iIndex = iter - vsProfileID.begin();

	for( int i=0; i<PROFILEMAN->GetNumLocalProfiles(); i++ )
	{
		iIndex += iDir;
		wrap( iIndex, vsProfileID.size() );
		sCurrent = vsProfileID[iIndex];

		bool bAnyOtherIsUsingThisProfile = false;
		FOREACH_PlayerNumber( p )
		{
			if( p!=pn  &&  PREFSMAN->GetDefaultLocalProfileID(p).Get() == sCurrent )
			{
				bAnyOtherIsUsingThisProfile = true;
				break;
			}
		}
		if( !bAnyOtherIsUsingThisProfile )
			break;
	}

	m_soundProfileChange.Play();

	PREFSMAN->GetDefaultLocalProfileID(pn).Set( sCurrent );
}

void ScreenTitleMenu::SwapDefaultLocalProfiles()
{
	CString s1 = PREFSMAN->GetDefaultLocalProfileID(PLAYER_1);
	CString s2 = PREFSMAN->GetDefaultLocalProfileID(PLAYER_2);
	PREFSMAN->GetDefaultLocalProfileID(PLAYER_1).Set( s2 );
	PREFSMAN->GetDefaultLocalProfileID(PLAYER_2).Set( s1 );

	m_soundProfileSwap.Play();
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
