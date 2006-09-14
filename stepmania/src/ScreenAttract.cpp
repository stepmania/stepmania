#include "global.h"
#include "ScreenAttract.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "StepMania.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "SongManager.h"
#include "GameState.h"
#include "GameManager.h"
#include "InputMapper.h"
#include "ThemeManager.h"
#include "GameSoundManager.h"
#include "CommonMetrics.h"
#include "InputEventPlus.h"
#include "RageSoundManager.h"

#define START_SCREEN(sScreenName)	THEME->GetMetric (sScreenName,"StartScreen")

ThemeMetric<bool>	BACK_GOES_TO_START_SCREEN( "ScreenAttract", "BackGoesToStartScreen" );

REGISTER_SCREEN_CLASS( ScreenAttract );
ScreenAttract::ScreenAttract( bool bResetGameState )
{
	if( bResetGameState )
		GAMESTATE->Reset();
}


void ScreenAttract::Init()
{
	SOUNDMAN->SetPlayOnlyCriticalSounds( !GAMESTATE->IsTimeToPlayAttractSounds() );	// mute attract sounds

	GAMESTATE->VisitAttractScreen( m_sName );

	ScreenWithMenuElements::Init();
}

void ScreenAttract::Input( const InputEventPlus &input )
{
//	LOG->Trace( "ScreenAttract::Input()" );

	AttractInput( input, this );
}

void ScreenAttract::AttractInput( const InputEventPlus &input, ScreenWithMenuElements *pScreen )
{
	if( input.type != IET_FIRST_PRESS ) 
		return; // don't care

	if( input.MenuI.IsValid() )
	{
		switch( input.MenuI.button )
		{
		case MENU_BUTTON_BACK:
			if( !(bool)BACK_GOES_TO_START_SCREEN )
				break;
			// fall through
		case MENU_BUTTON_START:
		case MENU_BUTTON_COIN:
			switch( GAMESTATE->GetCoinMode() )
			{
			case COIN_MODE_PAY:
				LOG->Trace("ScreenAttract::AttractInput: COIN_PAY (%i/%i)", GAMESTATE->m_iCoins, PREFSMAN->m_iCoinsPerCredit.Get() );
				if( GAMESTATE->m_iCoins < PREFSMAN->m_iCoinsPerCredit )
					break;	// don't fall through
				// fall through
			case COIN_MODE_HOME:
			case COIN_MODE_FREE:
				if( pScreen->IsTransitioning() )
					return;

				LOG->Trace("ScreenAttract::AttractInput: begin fading to START_SCREEN" );

				/* HandleGlobalInputs() already played the coin sound.  Don't play it again. */
				if( input.MenuI != MENU_BUTTON_COIN )
					SCREENMAN->PlayCoinSound();
				
				SOUNDMAN->SetPlayOnlyCriticalSounds( false );	// unmute attract sounds
				pScreen->Cancel( SM_GoToStartScreen );
				break;
			default:
				ASSERT(0);
			}
			break;
		}
	}

	if( pScreen->IsTransitioning() )
		return;

	if( input.MenuI.IsValid() )
	{
		switch( input.MenuI.button )
		{
		case MENU_BUTTON_LEFT:
		case MENU_BUTTON_RIGHT:
			SCREENMAN->PostMessageToTopScreen( SM_BeginFadingOut, 0 );
			break;
		}
	}	

//	Screen::Input( input );
}

void ScreenAttract::StartPlayingMusic()
{
	ScreenWithMenuElements::StartPlayingMusic();
}

void ScreenAttract::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_MenuTimer ||
		SM == SM_BeginFadingOut )
	{
		if( !IsTransitioning() )
			StartTransitioningScreen( SM_GoToNextScreen );
	}
	else if( SM == SM_GoToStartScreen )
	{
		GoToStartScreen( m_sName );
	}
	else if( SM == SM_GoToNextScreen )
	{
		/* Look at the def of the screen we're going to; if it has a music theme element
		 * and it's the same as the one we're playing now, don't stop.  However, if we're
		 * going to interrupt it when we fade in, stop the old music before we fade out. */
		bool bMusicChanging = false;
		if( PLAY_MUSIC )
			bMusicChanging = THEME->GetPathS(m_sName,"music") != THEME->GetPathS(GetNextScreen(),"music",true);	// GetPath optional on the next screen because it may not have music.

		if( bMusicChanging )
			SOUND->StopMusic();
	}

	ScreenWithMenuElements::HandleScreenMessage( SM );
}

void ScreenAttract::GoToStartScreen( RString sScreenName )
{
	SCREENMAN->SetNewScreen( START_SCREEN(sScreenName) );
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
