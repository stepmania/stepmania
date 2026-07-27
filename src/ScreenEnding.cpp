#include "global.h"
#include "ScreenEnding.h"
#include "ScreenManager.h"
#include "SongManager.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "AnnouncerManager.h"
#include "Song.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "ActorUtil.h"
#include "GameState.h"
#include "MemoryCardManager.h"
#include "RageLog.h"
#include "Style.h"
#include "GameManager.h"
#include "PrefsManager.h"
#include "StatsManager.h"
#include "PlayerState.h"
#include "CommonMetrics.h"
#include "InputEventPlus.h"

#include <cmath>


REGISTER_SCREEN_CLASS( ScreenEnding );
ScreenEnding::ScreenEnding()
{
	if( PREFSMAN->m_sTestInitialScreen.Get() != "" )
	{
		PROFILEMAN->LoadFirstAvailableProfile(PLAYER_1);
		PROFILEMAN->LoadFirstAvailableProfile(PLAYER_2);

		GAMESTATE->m_PlayMode.Set( PLAY_MODE_REGULAR );
		GAMESTATE->SetCurrentStyle( GAMEMAN->GameAndStringToStyle( GAMEMAN->GetDefaultGame(),"versus"), PLAYER_INVALID );
		GAMESTATE->JoinPlayer( PLAYER_1 );
		GAMESTATE->JoinPlayer( PLAYER_2 );
		GAMESTATE->m_pCurSong.Set( SONGMAN->GetRandomSong() );
		GAMESTATE->m_pCurCourse.Set( SONGMAN->GetRandomCourse() );
		GAMESTATE->m_pCurSteps[PLAYER_1].Set( GAMESTATE->m_pCurSong->GetAllSteps()[0] );
		GAMESTATE->m_pCurSteps[PLAYER_2].Set( GAMESTATE->m_pCurSong->GetAllSteps()[0] );
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].m_vpPossibleSteps.push_back( GAMESTATE->m_pCurSteps[PLAYER_1] );
		STATSMAN->m_CurStageStats.m_player[PLAYER_2].m_vpPossibleSteps.push_back( GAMESTATE->m_pCurSteps[PLAYER_2] );
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].m_iStepsPlayed = 1;
		STATSMAN->m_CurStageStats.m_player[PLAYER_2].m_iStepsPlayed = 1;
		PO_GROUP_ASSIGN( GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions, ModsLevel_Stage, m_fScrollSpeed, 2.0f );
		PO_GROUP_ASSIGN( GAMESTATE->m_pPlayerState[PLAYER_2]->m_PlayerOptions, ModsLevel_Stage, m_fScrollSpeed, 2.0f );
		GAMESTATE->m_iCurrentStageIndex = 0;
		FOREACH_ENUM( PlayerNumber, p )
			GAMESTATE->m_iPlayerStageTokens[p] = 1;
		PO_GROUP_CALL( GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions, ModsLevel_Stage, ChooseRandomModifiers );
		PO_GROUP_CALL( GAMESTATE->m_pPlayerState[PLAYER_2]->m_PlayerOptions, ModsLevel_Stage, ChooseRandomModifiers );

		for( float f = 0; f < 100.0f; f += 1.0f )
		{
			float fP1 = std::fmod(f/100*4+.3f,1);
			STATSMAN->m_CurStageStats.m_player[PLAYER_1].SetLifeRecordAt( fP1, f );
			STATSMAN->m_CurStageStats.m_player[PLAYER_2].SetLifeRecordAt( 1-fP1, f );
		}

		STATSMAN->m_CurStageStats.m_player[PLAYER_1].m_iActualDancePoints = RandomInt( 3 );
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].m_iPossibleDancePoints = 2;
		STATSMAN->m_CurStageStats.m_player[PLAYER_2].m_iActualDancePoints = RandomInt( 2 );
		STATSMAN->m_CurStageStats.m_player[PLAYER_2].m_iPossibleDancePoints = 1;
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].m_iCurCombo = 0;
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].UpdateComboList( 0, false );
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].m_iCurCombo = 1;
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].UpdateComboList( 1, false );
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].m_iCurCombo = 50;
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].UpdateComboList( 25, false );
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].m_iCurCombo = 250;
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].UpdateComboList( 100, false );

		STATSMAN->m_CurStageStats.m_player[PLAYER_1].m_iTapNoteScores[TNS_W1] = RandomInt( 2 );
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].m_iTapNoteScores[TNS_W2] = RandomInt( 2 );
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].m_iTapNoteScores[TNS_W3] = RandomInt( 2 );
		STATSMAN->m_CurStageStats.m_player[PLAYER_2].m_iTapNoteScores[TNS_W1] = RandomInt( 2 );
		STATSMAN->m_CurStageStats.m_player[PLAYER_2].m_iTapNoteScores[TNS_W2] = RandomInt( 2 );
		STATSMAN->m_CurStageStats.m_player[PLAYER_2].m_iTapNoteScores[TNS_W3] = RandomInt( 2 );

		STATSMAN->m_vPlayedStageStats.clear();
	}
}

void ScreenEnding::Init()
{
	ScreenAttract::Init();

	FOREACH_HumanPlayer( p )
	{
		if( !PROFILEMAN->IsPersistentProfile(p) )
			continue;

		m_sprRemoveMemoryCard[p].SetName( ssprintf("RemoveCardP%d",p+1) );
		m_sprRemoveMemoryCard[p].Load( THEME->GetPathG("ScreenEnding",ssprintf("remove card P%d",p+1)) );
		switch( MEMCARDMAN->GetCardState(p) )
		{
			case MemoryCardState_Removed:
			case MemoryCardState_NoCard:
				m_sprRemoveMemoryCard[p].SetVisible( false );
			default:
				break;
		}
		LOAD_ALL_COMMANDS_AND_SET_XY_AND_ON_COMMAND( m_sprRemoveMemoryCard[p] );
		this->AddChild( &m_sprRemoveMemoryCard[p] );
	}

	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("music scroll") );

	// Now that we've read the data from the profile, it's ok to Reset()
	GAMESTATE->Reset();
}

bool ScreenEnding::Input( const InputEventPlus &input )
{
	bool handled = false;
	if( !IsTransitioning() )
	{
		switch( input.MenuI )
		{
			case GAME_BUTTON_START:
				SCREENMAN->PostMessageToTopScreen( SM_BeginFadingOut, 0 );
				handled = true;
			default:
				break;
		}
	}

	return ScreenAttract::Input( input ) || handled;
}

/*
 * (c) 2004 Chris Danford
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
