#include "global.h"

#include "ScreenAttract.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "StepMania.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "SongManager.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "GameManager.h"
#include "InputMapper.h"
#include "ThemeManager.h"
#include "GameSoundManager.h"
#include "CommonMetrics.h"

#define NEXT_SCREEN				THEME->GetMetric (m_sName,"NextScreen")


ScreenAttract::ScreenAttract( CString sName, bool bResetGameState ) : Screen( sName )
{
	LOG->Trace( "ScreenAttract::ScreenAttract(%s)", m_sName.c_str() );

	// increment times through attract count
	if( m_sName == INITIAL_SCREEN )
		GAMESTATE->m_iNumTimesThroughAttract++;

	if( bResetGameState )
		GAMESTATE->Reset();

	// We have to do initialization in the first update because this->GetElementName() won't
	// work until the object has been fully constructed.
	m_Background.LoadFromAniDir( THEME->GetPathToB(m_sName+" background") );
	this->AddChild( &m_Background );

	m_In.Load( THEME->GetPathToB(m_sName+" in") );
	m_In.StartTransitioning();
	m_In.SetDrawOrder( DRAW_ORDER_TRANSITIONS );
	this->AddChild( &m_In );

	m_Out.Load( THEME->GetPathToB(m_sName+" out") );
	m_Out.SetDrawOrder( DRAW_ORDER_TRANSITIONS );
	this->AddChild( &m_Out );

	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo(m_sName) );

	float fTimeUntilBeginFadingOut = m_Background.GetLengthSeconds() - m_Out.GetLengthSeconds();
	if( fTimeUntilBeginFadingOut < 0 )
	{
		LOG->Warn( "Screen '%s' Out BGAnimation (%f seconds) is longer than Background BGAnimation (%f seconds); background BGA will be truncated",
			m_sName.c_str(), m_Out.GetLengthSeconds(), m_Background.GetLengthSeconds() );
		fTimeUntilBeginFadingOut = 0;
	}

	this->PostScreenMessage( SM_BeginFadingOut, fTimeUntilBeginFadingOut );
}


ScreenAttract::~ScreenAttract()
{
	LOG->Trace( "ScreenAttract::~ScreenAttract()" );
}


void ScreenAttract::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
//	LOG->Trace( "ScreenAttract::Input()" );

	AttractInput( DeviceI, type, GameI, MenuI, StyleI, m_In.IsTransitioning() || m_Out.IsTransitioning() );
}

void ScreenAttract::AttractInput( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI, bool bTransitioning )
{
	if(type != IET_FIRST_PRESS) 
		return; // don't care

	ChangeCoinModeInput( DeviceI, type, GameI, MenuI, StyleI );

	if( MenuI.IsValid() )
	{
		switch( MenuI.button )
		{
		case MENU_BUTTON_START:
		case MENU_BUTTON_BACK:
		case MENU_BUTTON_COIN:
			switch( PREFSMAN->GetCoinMode() )
			{
			case COIN_PAY:
				LOG->Trace("ScreenAttract::AttractInput: COIN_PAY (%i/%i)", GAMESTATE->m_iCoins, PREFSMAN->m_iCoinsPerCredit );
				if( GAMESTATE->m_iCoins < PREFSMAN->m_iCoinsPerCredit )
					break;	// don't fall through
				// fall through
			case COIN_HOME:
			case COIN_FREE:
				SOUND->StopMusic();
				/* We already played the coin sound.  Don't play it again. */
				if( MenuI.button != MENU_BUTTON_COIN )
					SCREENMAN->PlayCoinSound();
				SCREENMAN->SendMessageToTopScreen( SM_StopMusic );
				usleep( (int)(JOIN_PAUSE_SECONDS*1000000) );	// do a little pause, like the arcade does
				LOG->Trace("ScreenAttract::AttractInput: go to ScreenTitleMenu" );
				SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
				break;
			default:
				ASSERT(0);
			}
			break;
		}
	}

	if( bTransitioning )
		return;

	if( MenuI.IsValid() )
	{
		switch( MenuI.button )
		{
		case MENU_BUTTON_LEFT:
		case MENU_BUTTON_RIGHT:
			SCREENMAN->PostMessageToTopScreen( SM_BeginFadingOut, 0 );
			break;
		}
	}

//	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenAttract::Update( float fDelta )
{
	if( IsFirstUpdate() )
	{
		if( GAMESTATE->IsTimeToPlayAttractSounds() )
			SOUND->PlayMusic( THEME->GetPathToS(m_sName + " music") );
		else
			SOUND->PlayMusic( "" );	// stop music
	}
	Screen::Update(fDelta);
}

void ScreenAttract::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_BeginFadingOut:
		if( !m_Out.IsTransitioning() )
			m_Out.StartTransitioning( SM_GoToNextScreen );
		break;
	case SM_GoToNextScreen:
		/* Look at the def of the screen we're going to; if it has a music theme element
		 * and it's the same as the one we're playing now, don't stop.  However, if we're
		 * going to interrupt it when we fade in, stop the old music before we fade out. */
		bool bGoingToPlayTheSameMusic =
			THEME->GetPathS( NEXT_SCREEN, "music", false) == THEME->GetPathS( m_sName, "music", false);
		if( bGoingToPlayTheSameMusic )
			; // do nothing
		else
			SOUND->PlayMusic( "" );	// stop the music

		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		break;
	}
}

/*
 * (c) 2003-2004 Chris Danford
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
