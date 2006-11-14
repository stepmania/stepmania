#include "global.h"

#include "ScreenJukebox.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "SongManager.h"
#include "ScreenManager.h"
#include "GameSoundManager.h"
#include "Steps.h"
#include "ScreenAttract.h"
#include "RageUtil.h"
#include "UnlockManager.h"
#include "Course.h"
#include "ThemeManager.h"
#include "Style.h"
#include "PlayerState.h"
#include "StatsManager.h"
#include "CommonMetrics.h"
#include "PrefsManager.h"
#include "InputEventPlus.h"
#include "AdjustSync.h"
#include "SongUtil.h"
#include "song.h"

#define SHOW_COURSE_MODIFIERS_PROBABILITY	THEME->GetMetricF(m_sName,"ShowCourseModifiersProbability")

REGISTER_SCREEN_CLASS( ScreenJukebox );
void ScreenJukebox::SetSong()
{
	ThemeMetric<bool>				ALLOW_ADVANCED_MODIFIERS(m_sName,"AllowAdvancedModifiers");

	vector<Song*> vSongs;

	//Check to see if there is a theme-course
	//I.E. If there is a course called exactly the theme name, 
	//then we pick a song from this course.
	Course *pCourse = SONGMAN->GetCourseFromName( THEME->GetCurThemeName() );
	if( pCourse != NULL )
		for ( unsigned i = 0; i < pCourse->m_vEntries.size(); i++ )
			if( pCourse->m_vEntries[i].IsFixedSong() )
				vSongs.push_back( pCourse->m_vEntries[i].pSong );

	if ( vSongs.size() == 0 )
		SONGMAN->GetSongs( vSongs, GAMESTATE->m_sPreferredSongGroup );


	//
	// Calculate what difficulties to show
	//
	vector<Difficulty> vDifficultiesToShow;
	if( m_bDemonstration )
	{
		// HACK: This belongs in ScreenDemonstration
		ThemeMetricDifficultiesToShow	DIFFICULTIES_TO_SHOW_HERE(m_sName,"DifficultiesToShow");
		vDifficultiesToShow = DIFFICULTIES_TO_SHOW_HERE.GetValue();
	}
	else
	{
		if( GAMESTATE->m_PreferredDifficulty[PLAYER_1] != Difficulty_Invalid )
		{
			vDifficultiesToShow.push_back( GAMESTATE->m_PreferredDifficulty[PLAYER_1] );
		}
		else
		{
			FOREACH_Difficulty( dc )
				vDifficultiesToShow.push_back( dc );
		}
	}

	ASSERT( !vDifficultiesToShow.empty() );

	//
	// Search for a Song and Steps to play during the demo
	//
	for( int i=0; i<1000; i++ )
	{
		if( vSongs.size() == 0 )
			return;

		Song* pSong = vSongs[RandomInt(vSongs.size())];

		ASSERT( pSong != NULL );
		if( !pSong->HasMusic() )
			continue;	// skip
		if( UNLOCKMAN->SongIsLocked(pSong) )
			continue;
		if( !pSong->ShowInDemonstrationAndRanking() )
			continue;	// skip

		Difficulty dc = vDifficultiesToShow[ RandomInt(vDifficultiesToShow.size()) ];
		Steps* pSteps = SongUtil::GetStepsByDifficulty( pSong, GAMESTATE->GetCurrentStyle()->m_StepsType, dc );

		if( pSteps == NULL )
			continue;	// skip

		if( !PREFSMAN->m_bAutogenSteps && pSteps->IsAutogen())
			continue;	// skip

		// Found something we can use!
		GAMESTATE->m_pCurSong.Set( pSong );
		// We just changed the song. Reset the original sync data.
		AdjustSync::ResetOriginalSyncData();
		FOREACH_PlayerNumber( p )
			GAMESTATE->m_pCurSteps[p].Set( pSteps );
		

		bool bShowModifiers = randomf(0,1) <= SHOW_COURSE_MODIFIERS_PROBABILITY;
		if( bShowModifiers )
		{
			/* If we have a modifier course containing this song, apply its modifiers.  Only check
			 * fixed course entries. */
			vector<Course*> apCourses;
			SONGMAN->GetAllCourses( apCourses, false );
			vector<const CourseEntry *> apOptions;
			vector<Course*> apPossibleCourses;
			for( unsigned i = 0; i < apCourses.size(); ++i )
			{
				Course *pCourse = apCourses[i];
				const CourseEntry *pEntry = pCourse->FindFixedSong( pSong );
				if( pEntry == NULL || pEntry->attacks.size() == 0 )
					continue;
				

				if( !ALLOW_ADVANCED_MODIFIERS )
				{
					// There are some confusing mods that we don't want to show in demonstration.
					bool bModsAreOkToShow = true;
					AttackArray aAttacks = pEntry->attacks;
					if( !pEntry->sModifiers.empty() )
						aAttacks.push_back( Attack::FromGlobalCourseModifier( pEntry->sModifiers ) );
					FOREACH_CONST( Attack, aAttacks, a )
					{
						RString s = a->sModifiers;
						s.MakeLower();
						if( s.find("dark") != string::npos ||
							s.find("stealth") != string::npos )
						{
							bModsAreOkToShow = false;
							break;
						}
					}
					if( !bModsAreOkToShow )
						continue;	// skip
				}


				apOptions.push_back( pEntry );
				apPossibleCourses.push_back( pCourse );
			}

			if( !apOptions.empty() )
			{
				int iIndex = RandomInt( apOptions.size() );
				m_pCourseEntry = apOptions[iIndex];
				Course *pCourse = apPossibleCourses[iIndex]; 
			
				PlayMode pm = CourseTypeToPlayMode( pCourse->GetCourseType() );
				GAMESTATE->m_PlayMode.Set( pm );
				GAMESTATE->m_pCurCourse.Set( pCourse );
				FOREACH_PlayerNumber( p )
				{
					GAMESTATE->m_pCurTrail[p].Set( pCourse->GetTrail( GAMESTATE->GetCurrentStyle()->m_StepsType ) );
					ASSERT( GAMESTATE->m_pCurTrail[p] );
				}
			}
		}		
		
		return;	// done looking
	}

	return;	// didn't find a song
}

ScreenJukebox::ScreenJukebox()
{
	m_bDemonstration = false;

	m_pCourseEntry = NULL;
}

void ScreenJukebox::Init()
{
	// ScreeJukeboxMenu must set this
	ASSERT( GAMESTATE->GetCurrentStyle() );
	GAMESTATE->m_PlayMode.Set( PLAY_MODE_REGULAR );

	SetSong();

//	ASSERT( GAMESTATE->m_pCurSong );

	GAMESTATE->m_MasterPlayerNumber = PLAYER_1;

	// choose some cool options
	int Benchmark = 0;
	if( Benchmark )
	{
		/* Note that you also need to make sure you benchmark with the
		 * same notes.  I use a copy of MaxU with only heavy notes included. */
		FOREACH_EnabledPlayer( p )
		{
			/* Lots and lots of arrows.  This might even bias to arrows a little
			 * too much. */
			PO_GROUP_CALL( GAMESTATE->m_pPlayerState[p]->m_PlayerOptions, ModsLevel_Stage, Init );
			PO_GROUP_ASSIGN( GAMESTATE->m_pPlayerState[p]->m_PlayerOptions, ModsLevel_Stage, m_fScrollSpeed, .25f );
			PO_GROUP_ASSIGN( GAMESTATE->m_pPlayerState[p]->m_PlayerOptions, ModsLevel_Stage, m_fPerspectiveTilt, -1.0f );
			PO_GROUP_ASSIGN_N( GAMESTATE->m_pPlayerState[p]->m_PlayerOptions, ModsLevel_Stage, m_fEffects, PlayerOptions::EFFECT_MINI, 1.0f );
		}
		SO_GROUP_ASSIGN( GAMESTATE->m_SongOptions, ModsLevel_Stage, m_LifeType, SongOptions::LIFE_BATTERY );
		SO_GROUP_ASSIGN( GAMESTATE->m_SongOptions, ModsLevel_Stage, m_FailType, SongOptions::FAIL_OFF );
	}

	FOREACH_EnabledPlayer( p )
	{
		/* Reset the combo, in case ComboContinuesBetweenSongs is enabled. */
		STATSMAN->m_CurStageStats.m_player[p].m_iCurCombo = 0;

		if( GAMESTATE->m_bJukeboxUsesModifiers )
		{
			PlayerOptions po;
			GAMESTATE->GetDefaultPlayerOptions( po );
			po.ChooseRandomModifiers();
			GAMESTATE->m_pPlayerState[p]->m_PlayerOptions.Assign( ModsLevel_Stage, po );
		}
	}

	SongOptions so;
	GAMESTATE->GetDefaultSongOptions( so );
	GAMESTATE->m_SongOptions.Assign( ModsLevel_Stage, so );

	SO_GROUP_ASSIGN( GAMESTATE->m_SongOptions, ModsLevel_Stage, m_FailType, SongOptions::FAIL_OFF );

	GAMESTATE->m_bDemonstrationOrJukebox = true;

	/* Now that we've set up, init the base class. */
	ScreenGameplay::Init();

	if( GAMESTATE->m_pCurSong == NULL )	// we didn't find a song.
	{
		this->PostScreenMessage( SM_GoToNextScreen, 0 );	// Abort demonstration.
		return;
	}

	ClearMessageQueue();	// remove all of the messages set in ScreenGameplay that animate "ready", "here we go", etc.

	GAMESTATE->m_bGameplayLeadIn.Set( false );

	m_DancingState = STATE_DANCING;
}

void ScreenJukebox::Input( const InputEventPlus &input )
{
	//LOG->Trace( "ScreenJukebox::Input()" );

	if( input.type != IET_FIRST_PRESS )
		return; /* ignore */

	switch( input.MenuI )
	{
	case MENU_BUTTON_LEFT:
	case MENU_BUTTON_RIGHT:
		SCREENMAN->PostMessageToTopScreen( SM_NotesEnded, 0 );
		return;
	}

	ScreenAttract::AttractInput( input, this );
}

void ScreenJukebox::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_NotesEnded )
	{
		if( m_Out.IsTransitioning() || m_Out.IsFinished() )
			return;	// ignore - we're already fading or faded
		m_Out.StartTransitioning( SM_GoToNextScreen );
		return;
	}
	else if( SM == SM_GoToNextScreen )
	{
		if( m_pSoundMusic )
			m_pSoundMusic->Stop();
	}
	else if( SM == SM_GoToStartScreen )
	{
		ScreenAttract::GoToStartScreen( m_sName );
	}

	ScreenGameplay::HandleScreenMessage( SM );
}

void ScreenJukebox::InitSongQueues()
{
	ScreenGameplay::InitSongQueues();

	// Pare down to just the song in the course that we want.

	int iIndexToKeep = -1;
	for( unsigned i=0; i<m_apSongsQueue.size(); i++ )
	{
		if( m_apSongsQueue[i] == GAMESTATE->m_pCurSong )
		{
			iIndexToKeep = i;
			break;
		}
	}

	ASSERT( iIndexToKeep != -1 );

	for( int i=(m_apSongsQueue.size())-1; i>=0; i-- )
	{
		if( i != iIndexToKeep )
		{
			m_apSongsQueue.erase( m_apSongsQueue.begin()+i );
			FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
			{
				pi->m_vpStepsQueue.erase( pi->m_vpStepsQueue.begin()+i );
				pi->m_asModifiersQueue.erase( pi->m_asModifiersQueue.begin()+i );
			}
		}
	}

	ASSERT_M( m_apSongsQueue.size() == 1, ssprintf("%i", (int) m_apSongsQueue.size()) );
	FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
	{
		ASSERT_M( pi->m_vpStepsQueue.size() == 1, ssprintf("%i", (int) pi->m_vpStepsQueue.size()) );
		ASSERT_M( pi->m_asModifiersQueue.size() == 1, ssprintf("%i", (int) pi->m_asModifiersQueue.size()) );
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
