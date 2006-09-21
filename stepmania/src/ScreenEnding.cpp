#include "global.h"
#include "ScreenEnding.h"
#include "ScreenManager.h"
#include "SongManager.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "AnnouncerManager.h"
#include "song.h"
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


REGISTER_SCREEN_CLASS( ScreenEnding );
ScreenEnding::ScreenEnding() : ScreenAttract( false/*dont reset GAMESTATE*/ )
{
	if( PREFSMAN->m_bScreenTestMode )
	{
		PROFILEMAN->LoadFirstAvailableProfile(PLAYER_1);
		PROFILEMAN->LoadFirstAvailableProfile(PLAYER_2);

		GAMESTATE->m_PlayMode.Set( PLAY_MODE_REGULAR );
		GAMESTATE->m_pCurStyle.Set( GAMEMAN->GameAndStringToStyle( GAMEMAN->GetDefaultGame(),"versus") );
		GAMESTATE->JoinPlayer( PLAYER_1 );
		GAMESTATE->JoinPlayer( PLAYER_2 );
		GAMESTATE->m_pCurSong.Set( SONGMAN->GetRandomSong() );
		GAMESTATE->m_pCurCourse.Set( SONGMAN->GetRandomCourse() );
		GAMESTATE->m_pCurSteps[PLAYER_1].Set( GAMESTATE->m_pCurSong->GetAllSteps()[0] );
		GAMESTATE->m_pCurSteps[PLAYER_2].Set( GAMESTATE->m_pCurSong->GetAllSteps()[0] );
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].vpPlayedSteps.push_back( GAMESTATE->m_pCurSteps[PLAYER_1] );
		STATSMAN->m_CurStageStats.m_player[PLAYER_2].vpPlayedSteps.push_back( GAMESTATE->m_pCurSteps[PLAYER_2] );
		PO_GROUP_ASSIGN( GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions, ModsLevel_Stage, m_fScrollSpeed, 2.0f );
		PO_GROUP_ASSIGN( GAMESTATE->m_pPlayerState[PLAYER_2]->m_PlayerOptions, ModsLevel_Stage, m_fScrollSpeed, 2.0f );
		GAMESTATE->m_iCurrentStageIndex = 0;
		PO_GROUP_CALL( GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions, ModsLevel_Stage, ChooseRandomModifiers );
		PO_GROUP_CALL( GAMESTATE->m_pPlayerState[PLAYER_2]->m_PlayerOptions, ModsLevel_Stage, ChooseRandomModifiers );

		for( float f = 0; f < 100.0f; f += 1.0f )
		{
			float fP1 = fmodf(f/100*4+.3f,1);
			STATSMAN->m_CurStageStats.m_player[PLAYER_1].SetLifeRecordAt( fP1, f );
			STATSMAN->m_CurStageStats.m_player[PLAYER_2].SetLifeRecordAt( 1-fP1, f );
		}
	
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].iActualDancePoints = RandomInt( 3 );
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].iPossibleDancePoints = 2;
		STATSMAN->m_CurStageStats.m_player[PLAYER_2].iActualDancePoints = RandomInt( 2 );
		STATSMAN->m_CurStageStats.m_player[PLAYER_2].iPossibleDancePoints = 1;
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].iCurCombo = 0;
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].UpdateComboList( 0, false );
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].iCurCombo = 1;
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].UpdateComboList( 1, false );
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].iCurCombo = 50;
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].UpdateComboList( 25, false );
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].iCurCombo = 250;
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].UpdateComboList( 100, false );

		STATSMAN->m_CurStageStats.m_player[PLAYER_1].iTapNoteScores[TNS_W1] = RandomInt( 2 );
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].iTapNoteScores[TNS_W2] = RandomInt( 2 );
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].iTapNoteScores[TNS_W3] = RandomInt( 2 );
		STATSMAN->m_CurStageStats.m_player[PLAYER_2].iTapNoteScores[TNS_W1] = RandomInt( 2 );
		STATSMAN->m_CurStageStats.m_player[PLAYER_2].iTapNoteScores[TNS_W2] = RandomInt( 2 );
		STATSMAN->m_CurStageStats.m_player[PLAYER_2].iTapNoteScores[TNS_W3] = RandomInt( 2 );

		STATSMAN->m_vPlayedStageStats.clear();
	}


	// Update final profile stats before we load them for display.
	GAMESTATE->FinishStage();
}

void ScreenEnding::Init()
{
	ScreenAttract::Init();

	vector<Song*> arraySongs;
	SONGMAN->GetSongs( arraySongs );
	SongUtil::SortSongPointerArrayByTitle( arraySongs );

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
			m_sprRemoveMemoryCard[p].SetHidden( true );
			break;
		}
		SET_XY_AND_ON_COMMAND( m_sprRemoveMemoryCard[p] );
		m_sprRemoveMemoryCard[p].AddCommand( ssprintf("CardRemovedP%dMessage",p+1), ActorCommands("hidden,1") );
		this->AddChild( &m_sprRemoveMemoryCard[p] );
	}

	
	this->SortByDrawOrder();

	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("music scroll") );

	// Now that we've read the data from the profile, it's ok to Reset()
	GAMESTATE->Reset();
}

void ScreenEnding::Input( const InputEventPlus &input )
{
	if( !IsTransitioning() )
	{
		switch( input.MenuI )
		{
		case MENU_BUTTON_START:
			SCREENMAN->PostMessageToTopScreen( SM_BeginFadingOut, 0 );
			break;
		}
	}

	ScreenAttract::Input( input );
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
