#include "global.h"
#include "ScreenStyleSplash.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameSoundManager.h"
#include "ScreenManager.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "GameManager.h"
#include "RageLog.h"
#include "Game.h"
#include "Style.h"

#define NEXT_SCREEN				THEME->GetMetric("ScreenStyleSplash","NextScreen")
#define PREV_SCREEN				THEME->GetMetric("ScreenStyleSplash","PrevScreen")
#define NONSTOP_SCREEN				THEME->GetMetric("ScreenStyleSplash","NonstopScreen")
#define ONI_SCREEN				THEME->GetMetric("ScreenStyleSplash","OniScreen")

const AutoScreenMessage SM_DoneOpening;
const AutoScreenMessage SM_StartClosing;


REGISTER_SCREEN_CLASS( ScreenStyleSplash );
ScreenStyleSplash::ScreenStyleSplash( CString sName ) : ScreenWithMenuElements( sName )
{
}

void ScreenStyleSplash::Init()
{
	ScreenWithMenuElements::Init();

	SOUND->StopMusic();

	CString sGameName = GAMESTATE->GetCurrentGame()->m_szName;	
	CString sStyleName = GAMESTATE->GetCurrentStyle()->m_szName;

	LOG->Trace( ssprintf("ScreenStyleSplash: Displaying Splash for style: %s", sStyleName.c_str()));

	int iDifficulty = GAMESTATE->m_PreferredDifficulty[0];
	if( GAMESTATE->m_bSideIsJoined[PLAYER_1] )
	{
		iDifficulty = GAMESTATE->m_PreferredDifficulty[PLAYER_1];
	}
	else
	{
		iDifficulty = GAMESTATE->m_PreferredDifficulty[PLAYER_2];
	}

	this->PostScreenMessage( SM_StartClosing, 2 );
}


void ScreenStyleSplash::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( IsTransitioning() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}


void ScreenStyleSplash::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_StartClosing )
	{
		if( !IsTransitioning() )
			StartTransitioning( SM_GoToNextScreen );
	}
	else if( SM == SM_DoneOpening )
	{
	}
	else if( SM == SM_GoToPrevScreen )
	{
		SCREENMAN->SetNewScreen( PREV_SCREEN );
	}
	else if( SM == SM_GoToNextScreen )
	{
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
	}
}

void ScreenStyleSplash::MenuStart( PlayerNumber pn )
{
	StartTransitioning( SM_GoToNextScreen );
}

void ScreenStyleSplash::MenuBack( PlayerNumber pn )
{
	if(IsTransitioning())
		return;
	this->ClearMessageQueue();
	Cancel( SM_GoToPrevScreen );
	SOUND->PlayOnce( THEME->GetPathS("menu","cancel") );
}

void ScreenStyleSplash::DrawPrimitives()
{
	Screen::DrawPrimitives();
}

/*
 * (c) 2001-2004 Chris Danford, "Frieza"
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
