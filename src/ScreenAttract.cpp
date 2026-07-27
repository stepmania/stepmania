#include "global.h"
#include "ScreenAttract.h"
#include "ScreenManager.h"
#include "RageUtil.h"
#include "StepMania.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "InputMapper.h"
#include "ThemeManager.h"
#include "GameSoundManager.h"
#include "InputEventPlus.h"
#include "RageSoundManager.h"

#define START_SCREEN(sScreenName)	THEME->GetMetric (sScreenName,"StartScreen")

ThemeMetric<bool>	BACK_GOES_TO_START_SCREEN( "ScreenAttract", "BackGoesToStartScreen" );
Preference<float>	g_fSoundVolumeAttract( "SoundVolumeAttract", 1.0f );

REGISTER_SCREEN_CLASS( ScreenAttract );
void ScreenAttract::Init()
{
	RESET_GAME_STATE.Load( m_sName, "ResetGameState" );
	ATTRACT_VOLUME.Load( m_sName, "AttractVolume" );
	ScreenWithMenuElements::Init();
}

void ScreenAttract::BeginScreen()
{
	if( RESET_GAME_STATE )
		GAMESTATE->Reset();

	GAMESTATE->VisitAttractScreen( m_sName );
	ScreenAttract::SetAttractVolume( ATTRACT_VOLUME );

	ScreenWithMenuElements::BeginScreen();
}

bool ScreenAttract::Input( const InputEventPlus &input )
{
	bool handled;
//	LOG->Trace( "ScreenAttract::Input()" );

	handled = AttractInput( input, this );

	// Always run both AttractInput and ScreenWithMenuElements::Input
	return ScreenWithMenuElements::Input( input ) || handled;
}

void ScreenAttract::SetAttractVolume( bool bInAttract )
{
	if( bInAttract )
	{
		if( GAMESTATE->IsTimeToPlayAttractSounds() )
			SOUNDMAN->SetVolumeOfNonCriticalSounds( g_fSoundVolumeAttract ); // unmute attract sounds
		else
			SOUNDMAN->SetVolumeOfNonCriticalSounds( 0.0f ); // mute attract sounds
	}
	else
	{
		SOUNDMAN->SetVolumeOfNonCriticalSounds( 1.0f ); // unmute all sounds
	}
}

void ScreenAttract::Cancel( ScreenMessage smSendWhenDone )
{
	LOG->Trace("ScreenAttract::AttractInput: begin fading to START_SCREEN" );

	SetAttractVolume( false ); // unmute attract sounds
	ScreenWithMenuElements::Cancel( smSendWhenDone );
}

bool ScreenAttract::AttractInput( const InputEventPlus &input, ScreenWithMenuElements *pScreen )
{
	if( input.type != IET_FIRST_PRESS )
		return false; // don't care

	switch( input.MenuI )
	{
		case GAME_BUTTON_BACK:
			if( !BACK_GOES_TO_START_SCREEN )
				break;
			[[fallthrough]];
		case GAME_BUTTON_START:
		case GAME_BUTTON_COIN:
			// If we're not in a game and there aren't enough credits to start,
			// eat the input and do nothing.
			if( GAMESTATE->GetCoinMode() == CoinMode_Pay &&
					GAMESTATE->m_iCoins < PREFSMAN->m_iCoinsPerCredit &&
					GAMESTATE->GetNumSidesJoined() == 0 )
				return true;
			if( pScreen->IsTransitioning() )
				return false;

			// HandleGlobalInputs() already played the coin sound. Don't play it again.
			if( input.MenuI != GAME_BUTTON_COIN )
				SCREENMAN->PlayStartSound();

			pScreen->Cancel( SM_GoToStartScreen );
			return true;
		default: break;
	}

	if( pScreen->IsTransitioning() )
		return false;

	switch( input.MenuI )
	{
		case GAME_BUTTON_LEFT:
		case GAME_BUTTON_RIGHT:
			SCREENMAN->PostMessageToTopScreen( SM_BeginFadingOut, 0 );
			return true;
		default:
			return false;
	}
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
		/* Look at the def of the screen we're going to; if it has a music theme
		 * element and it's the same as the one we're playing now, don't stop.
		 * However, if we're going to interrupt it when we fade in, stop the old
		 * music before we fade out. */
		bool bMusicChanging = false;
		if( PLAY_MUSIC )
			bMusicChanging = THEME->GetPathS(m_sName,"music") != THEME->GetPathS(GetNextScreenName(),"music",true);	// GetPath optional on the next screen because it may not have music.

		if( bMusicChanging )
			SOUND->StopMusic();
	}
	else if( SM == SM_LoseFocus )
	{
		ScreenAttract::SetAttractVolume( false );
	}

	ScreenWithMenuElements::HandleScreenMessage( SM );
}

void ScreenAttract::GoToStartScreen( RString sScreenName )
{
	SCREENMAN->SetNewScreen( START_SCREEN(sScreenName) );
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the ScreenAttract. */
class LunaScreenAttract: public Luna<ScreenAttract>
{
public:

	LunaScreenAttract()
	{
	}
};

LUA_REGISTER_DERIVED_CLASS( ScreenAttract, ScreenWithMenuElements )
// lua end

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
