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


CString GetStatsLineTitle( PlayerNumber pn, EndingStatsLine line )
{
	switch( line )
	{
	case CALORIES_TODAY:	return "Calories Today";
	case CURRENT_COMBO:		return "Current Combo";
	case PERCENT_COMPLETE:
		{
			StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;
			CString sStepsType = GAMEMAN->StepsTypeToThemedString(st);
			CString sType = GAMESTATE->IsCourseMode() ? "Courses" : "Songs";
			return ssprintf( "%s %s %%", sStepsType.c_str(), sType.c_str() );
		}
	case PERCENT_COMPLETE_EASY:
	case PERCENT_COMPLETE_MEDIUM:
	case PERCENT_COMPLETE_HARD:
	case PERCENT_COMPLETE_CHALLENGE:
		{
			if( GAMESTATE->IsCourseMode() )
			{
				CourseDifficulty cd = (CourseDifficulty)(DIFFICULTY_EASY+line-PERCENT_COMPLETE_EASY);
				ASSERT( cd >= 0 && cd < NUM_COURSE_DIFFICULTIES );
				if( !GAMESTATE->IsCourseDifficultyShown(cd) )
					return CString();
				return CourseDifficultyToThemedString(cd);
			}
			else
			{
				Difficulty dc = (Difficulty)(DIFFICULTY_EASY+line-PERCENT_COMPLETE_EASY);
				ASSERT( dc >= 0 && dc < NUM_DIFFICULTIES );
				return DifficultyToThemedString(dc);
			}
		}
	default:	ASSERT(0);	return CString();
	}
}

CString GetStatsLineValue( PlayerNumber pn, EndingStatsLine line )
{
	CHECKPOINT_M( ssprintf("GetStatsLineValue(%d,%d)",pn,line) );

	Profile* pProfile = PROFILEMAN->GetProfile( pn );
	ASSERT( pProfile );

	StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;

	switch( line )
	{
	case CALORIES_TODAY:		return pProfile->GetDisplayTotalCaloriesBurnedToday();
	case CURRENT_COMBO:			return Commify( pProfile->m_iCurrentCombo );
	case PERCENT_COMPLETE:
		{
			float fActual = 0;
			float fPossible = 0;

			if( GAMESTATE->IsCourseMode() )
			{
				FOREACH_CONST( CourseDifficulty, COURSE_DIFFICULTIES_TO_SHOW.GetValue(), iter )
				{
					fActual += pProfile->GetCoursesActual(st,*iter);
					fPossible += pProfile->GetCoursesPossible(st,*iter);
				}
			}
			else
			{
				FOREACH_CONST( Difficulty, DIFFICULTIES_TO_SHOW.GetValue(), iter )
				{
					fActual += pProfile->GetSongsActual(st,*iter);
					fPossible += pProfile->GetSongsPossible(st,*iter);
				}
			}

			return ssprintf( "%05.2f%%", fActual/fPossible*100 );
		}
	case PERCENT_COMPLETE_EASY:
	case PERCENT_COMPLETE_MEDIUM:
	case PERCENT_COMPLETE_HARD:
	case PERCENT_COMPLETE_CHALLENGE:
		// Ugly...
		{
			CString sStepsType = GAMEMAN->StepsTypeToThemedString(st);
			float fPercent = 0;
			if( GAMESTATE->IsCourseMode() )
			{
				CourseDifficulty cd = (CourseDifficulty)(DIFFICULTY_EASY+line-PERCENT_COMPLETE_EASY);
				ASSERT( cd >= 0 && cd < NUM_COURSE_DIFFICULTIES );
				if( !GAMESTATE->IsCourseDifficultyShown(cd) )
					return CString();
				CString sDifficulty = CourseDifficultyToThemedString(cd);
				fPercent = pProfile->GetCoursesPercentComplete(st,cd);
			}
			else
			{
				Difficulty dc = (Difficulty)(DIFFICULTY_EASY+line-PERCENT_COMPLETE_EASY);
				ASSERT( dc >= 0 && dc < NUM_DIFFICULTIES );
				CString sDifficulty = DifficultyToThemedString(dc);
				fPercent = pProfile->GetSongsPercentComplete(st,dc);
			}
			return ssprintf( "%05.2f%%", fPercent*100 );
		}
	default:	ASSERT(0);	return CString();
	}
}


REGISTER_SCREEN_CLASS( ScreenEnding );
ScreenEnding::ScreenEnding( CString sClassName ) : ScreenAttract( sClassName, false/*dont reset GAMESTATE*/ )
{
	if( PREFSMAN->m_bScreenTestMode )
	{
		PROFILEMAN->LoadFirstAvailableProfile(PLAYER_1);
		PROFILEMAN->LoadFirstAvailableProfile(PLAYER_2);

		GAMESTATE->m_PlayMode.Set( PLAY_MODE_REGULAR );
		GAMESTATE->m_pCurStyle.Set( GAMEMAN->GameAndStringToStyle( GAMEMAN->GetDefaultGame(),"versus") );
		GAMESTATE->m_bSideIsJoined[PLAYER_1] = true;
		GAMESTATE->m_bSideIsJoined[PLAYER_2] = true;
		GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
		GAMESTATE->m_pCurSong.Set( SONGMAN->GetRandomSong() );
		GAMESTATE->m_pCurCourse.Set( SONGMAN->GetRandomCourse() );
		GAMESTATE->m_pCurSteps[PLAYER_1].Set( GAMESTATE->m_pCurSong->GetAllSteps()[0] );
		GAMESTATE->m_pCurSteps[PLAYER_2].Set( GAMESTATE->m_pCurSong->GetAllSteps()[0] );
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].vpPlayedSteps.push_back( GAMESTATE->m_pCurSteps[PLAYER_1] );
		STATSMAN->m_CurStageStats.m_player[PLAYER_2].vpPlayedSteps.push_back( GAMESTATE->m_pCurSteps[PLAYER_2] );
		GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions.m_fScrollSpeed = 2;
		GAMESTATE->m_pPlayerState[PLAYER_2]->m_PlayerOptions.m_fScrollSpeed = 2;
		GAMESTATE->m_iCurrentStageIndex = 0;
		GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions.ChooseRandomMofifiers();
		GAMESTATE->m_pPlayerState[PLAYER_2]->m_PlayerOptions.ChooseRandomMofifiers();

		for( float f = 0; f < 100.0f; f += 1.0f )
		{
			float fP1 = fmodf(f/100*4+.3f,1);
			STATSMAN->m_CurStageStats.m_player[PLAYER_1].SetLifeRecordAt( fP1, f );
			STATSMAN->m_CurStageStats.m_player[PLAYER_2].SetLifeRecordAt( 1-fP1, f );
		}
	
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].iActualDancePoints = rand()%3;
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].iPossibleDancePoints = 2;
		STATSMAN->m_CurStageStats.m_player[PLAYER_2].iActualDancePoints = rand()%2;
		STATSMAN->m_CurStageStats.m_player[PLAYER_2].iPossibleDancePoints = 1;
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].iCurCombo = 0;
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].UpdateComboList( 0, false );
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].iCurCombo = 1;
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].UpdateComboList( 1, false );
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].iCurCombo = 50;
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].UpdateComboList( 25, false );
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].iCurCombo = 250;
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].UpdateComboList( 100, false );

		STATSMAN->m_CurStageStats.m_player[PLAYER_1].iTapNoteScores[TNS_MARVELOUS] = rand()%2;
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].iTapNoteScores[TNS_PERFECT] = rand()%2;
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].iTapNoteScores[TNS_GREAT] = rand()%2;
		STATSMAN->m_CurStageStats.m_player[PLAYER_2].iTapNoteScores[TNS_MARVELOUS] = rand()%2;
		STATSMAN->m_CurStageStats.m_player[PLAYER_2].iTapNoteScores[TNS_PERFECT] = rand()%2;
		STATSMAN->m_CurStageStats.m_player[PLAYER_2].iTapNoteScores[TNS_GREAT] = rand()%2;

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
		// don't show stats if not using a persistent profile
		if( !PROFILEMAN->IsPersistentProfile(p) )
			continue;
	
		FOREACH_EndingStatsLine( i )
		{
			m_Lines[i][p].title.LoadFromFont( THEME->GetPathF("ScreenEnding","stats title") );
			m_Lines[i][p].title.SetText( GetStatsLineTitle(p, i) );
			m_Lines[i][p].title.SetName( ssprintf("StatsTitleP%dLine%d",p+1,i+1) );
			SET_XY_AND_ON_COMMAND( m_Lines[i][p].title );
			this->AddChild( &m_Lines[i][p].title );
		
			m_Lines[i][p].value.LoadFromFont( THEME->GetPathF("ScreenEnding","stats value") );
			m_Lines[i][p].value.SetText( GetStatsLineValue(p, i) );
			m_Lines[i][p].value.SetName( ssprintf("StatsValueP%dLine%d",p+1,i+1) );
			SET_XY_AND_ON_COMMAND( m_Lines[i][p].value );
			this->AddChild( &m_Lines[i][p].value );
		}

		m_sprRemoveMemoryCard[p].SetName( ssprintf("RemoveCardP%d",p+1) );
		m_sprRemoveMemoryCard[p].Load( THEME->GetPathG("ScreenEnding",ssprintf("remove card P%d",p+1)) );
		switch( MEMCARDMAN->GetCardState(p) )
		{
		case MEMORY_CARD_STATE_REMOVED:
		case MEMORY_CARD_STATE_NO_CARD:
			m_sprRemoveMemoryCard[p].SetHidden( true );
			break;
		}
		SET_XY_AND_ON_COMMAND( m_sprRemoveMemoryCard[p] );
		m_sprRemoveMemoryCard[p].AddCommand( ssprintf("CardRemovedP%dMessage",p+1), apActorCommands(new ActorCommands("hidden,1")) );
		this->AddChild( &m_sprRemoveMemoryCard[p] );
	}

	
	this->SortByDrawOrder();

	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("music scroll") );

	// Now that we've read the data from the profile, it's ok to Reset()
	GAMESTATE->Reset();
}

void ScreenEnding::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	bool bIsTransitioning = m_In.IsTransitioning() || m_Out.IsTransitioning();
	if( MenuI.IsValid() && !bIsTransitioning )
	{
		switch( MenuI.button )
		{
		case MENU_BUTTON_START:
			SCREENMAN->PostMessageToTopScreen( SM_BeginFadingOut, 0 );
			break;
		}
	}

	ScreenAttract::Input( DeviceI, type, GameI, MenuI, StyleI );
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
