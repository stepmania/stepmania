#include "global.h"

#include "ScreenJukebox.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "SongManager.h"
#include "StepMania.h"
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
#include "ProfileManager.h"
#include "StatsManager.h"

// HACK: This belongs in ScreenDemonstration
#define DIFFICULTIES_TO_SHOW		THEME->GetMetric ("ScreenDemonstration","DifficultiesToShow")

#define SHOW_COURSE_MODIFIERS		THEME->GetMetricB("ScreenJukebox","ShowCourseModifiers")


REGISTER_SCREEN_CLASS( ScreenJukebox );
bool ScreenJukebox::SetSong( bool bDemonstration )
{
	vector<Song*> vSongs;

	//Check to see if there is a theme-course
	//I.E. If there is a course called exactly the theme name, 
	//then we pick a song from this course.
	Course *pCourse = SONGMAN->GetCourseFromName( THEME->GetCurThemeName() );
	if( pCourse != NULL )
		for ( unsigned i = 0; i < pCourse->m_entries.size(); i++ )
			vSongs.push_back( pCourse->m_entries[i].pSong );

	if ( vSongs.size() == 0 )
		SONGMAN->GetSongs( vSongs, GAMESTATE->m_sPreferredSongGroup );

	//
	// Calculate what difficulties to show
	//
	CStringArray asBits;
	vector<Difficulty> vDifficultiesToShow;
	if( bDemonstration )
	{
		split( DIFFICULTIES_TO_SHOW, ",", asBits );
		for( unsigned i=0; i<asBits.size(); i++ )
		{
			Difficulty dc = StringToDifficulty( asBits[i] );
			if( dc != DIFFICULTY_INVALID )
				vDifficultiesToShow.push_back( dc );
		}
	}
	else
	{
		if( GAMESTATE->m_PreferredDifficulty[PLAYER_1] != DIFFICULTY_INVALID )
		{
			vDifficultiesToShow.push_back( GAMESTATE->m_PreferredDifficulty[PLAYER_1] );
		}
		else
		{
			FOREACH_Difficulty( dc )
				vDifficultiesToShow.push_back( dc );
		}
	}

	if( vDifficultiesToShow.empty() )
		vDifficultiesToShow.push_back( DIFFICULTY_EASY );

	//
	// Search for a Song and Steps to play during the demo
	//
	for( int i=0; i<1000; i++ )
	{
		if( vSongs.size() == 0 )
			return true;

		Song* pSong = vSongs[rand()%vSongs.size()];

		if( !pSong->HasMusic() )
			continue;	// skip
		if( UNLOCKMAN->SongIsLocked(pSong) )
			continue;
		if( !pSong->ShowInDemonstrationAndRanking() )
			continue;	// skip

		Difficulty dc = vDifficultiesToShow[ rand()%vDifficultiesToShow.size() ];
		Steps* pSteps = pSong->GetStepsByDifficulty( GAMESTATE->GetCurrentStyle()->m_StepsType, dc );

		if( pSteps == NULL )
			continue;	// skip

		if( !PREFSMAN->m_bAutogenSteps && pSteps->IsAutogen())
			continue;	// skip

		// Found something we can use!
		GAMESTATE->m_pCurSong.Set( pSong );
		FOREACH_PlayerNumber( p )
			GAMESTATE->m_pCurSteps[p].Set( pSteps );
		
		return true;	// done looking
	}

	return false;
}

ScreenJukebox::ScreenJukebox( CString sName ) : ScreenGameplay( sName )
{
	LOG->Trace( "ScreenJukebox::ScreenJukebox()" );
	m_bDemonstration = false;
}

void ScreenJukebox::Init()
{
	// ScreeJukeboxMenu must set this
	ASSERT( GAMESTATE->m_pCurStyle );
	GAMESTATE->m_PlayMode = PLAY_MODE_REGULAR;

	SetSong( m_bDemonstration );

//	ASSERT( GAMESTATE->m_pCurSong );

	GAMESTATE->m_MasterPlayerNumber = PLAYER_1;

	// choose some cool options
	int Benchmark = 0;
	if(Benchmark)
	{
		/* Note that you also need to make sure you benchmark with the
		 * same notes.  I use a copy of MaxU with only heavy notes included. */
		FOREACH_PlayerNumber( p )
		{
			if( !GAMESTATE->IsPlayerEnabled(p) )
				continue;

			/* Lots and lots of arrows.  This might even bias to arrows a little
			 * too much. */
			GAMESTATE->m_pPlayerState[p]->m_PlayerOptions = PlayerOptions();
			GAMESTATE->m_pPlayerState[p]->m_PlayerOptions.m_fScrollSpeed = .25f;
			GAMESTATE->m_pPlayerState[p]->m_PlayerOptions.m_fPerspectiveTilt = -1;
			GAMESTATE->m_pPlayerState[p]->m_PlayerOptions.m_fEffects[ PlayerOptions::EFFECT_MINI ] = 1;
		}
		GAMESTATE->m_SongOptions.m_LifeType = SongOptions::LIFE_BATTERY;
		GAMESTATE->m_SongOptions.m_FailType = SongOptions::FAIL_OFF;
	}

	FOREACH_PlayerNumber( p )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;

		/* Reset the combo, in case ComboContinuesBetweenSongs is enabled. */
		Profile* pProfile = PROFILEMAN->GetProfile(p);
		if( pProfile )
			STATSMAN->m_CurStageStats.m_player[p].iCurCombo = 0;

		if( GAMESTATE->m_bJukeboxUsesModifiers )
		{
			GAMESTATE->m_pPlayerState[p]->m_PlayerOptions.Init();
			GAMESTATE->m_pPlayerState[p]->m_PlayerOptions.FromString( PREFSMAN->m_sDefaultModifiers );
			GAMESTATE->m_pPlayerState[p]->m_PlayerOptions.ChooseRandomMofifiers();
		}
	}

	GAMESTATE->m_SongOptions.Init();
	GAMESTATE->m_SongOptions.FromString( PREFSMAN->m_sDefaultModifiers );

	GAMESTATE->m_SongOptions.m_FailType = SongOptions::FAIL_OFF;

	GAMESTATE->m_bDemonstrationOrJukebox = true;

	/* Now that we've set up, init the base class. */
	ScreenGameplay::Init();

	if( GAMESTATE->m_pCurSong == NULL )	// we didn't find a song.
	{
		this->PostScreenMessage( SM_GoToNextScreen, 0 );	// Abort demonstration.
		return;
	}

	m_In.Load( THEME->GetPathB("ScreenDemonstration","in") );
	this->AddChild( &m_In );
	m_In.StartTransitioning();

	m_Out.Load( THEME->GetPathB("ScreenDemonstration","out") );
	this->AddChild( &m_Out );

	ClearMessageQueue();	// remove all of the messages set in ScreenGameplay that animate "ready", "here we go", etc.

	GAMESTATE->m_bPastHereWeGo = true;

	m_DancingState = STATE_DANCING;
}

void ScreenJukebox::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	//LOG->Trace( "ScreenJukebox::Input()" );

	if( type != IET_FIRST_PRESS )
		return; /* ignore */

	if( MenuI.IsValid() )
	{
		switch( MenuI.button )
		{
		case MENU_BUTTON_LEFT:
		case MENU_BUTTON_RIGHT:
			SCREENMAN->PostMessageToTopScreen( SM_NotesEnded, 0 );
			return;
		}
	}

	ScreenAttract::AttractInput( DeviceI, type, GameI, MenuI, StyleI, m_Out.IsTransitioning(), m_sName );
}

void ScreenJukebox::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_NotesEnded:
		if( m_Out.IsTransitioning() || m_Out.IsFinished() )
			return;	// ignore - we're already fading or faded
		m_Out.StartTransitioning( SM_GoToNextScreen );
		return;
	case SM_GoToNextScreen:
		if( m_pSoundMusic )
			m_pSoundMusic->Stop();
		SCREENMAN->SetNewScreen( "ScreenJukebox" );
		return;
	}

	ScreenGameplay::HandleScreenMessage( SM );
}

void ScreenJukebox::InitSongQueues()
{
	ScreenGameplay::InitSongQueues();

	ASSERT_M( m_apSongsQueue.size() == 1, ssprintf("%i", (int) m_apSongsQueue.size()) );
	FOREACH_PlayerNumber(p)
		ASSERT_M( m_asModifiersQueue[p].size() == 1, ssprintf("%i", (int) m_asModifiersQueue[p].size()) );

	if( !SHOW_COURSE_MODIFIERS )
		return;

	/* If we have a modifier course containing this song, apply its modifiers.  Only check
	 * fixed course entries. */
	vector<Course*> apCourses;
	SONGMAN->GetAllCourses( apCourses, false );
	const Song *pSong = m_apSongsQueue[0];
	vector<const CourseEntry *> apOptions;
	for( unsigned i = 0; i < apCourses.size(); ++i )
	{
		const Course *pCourse = apCourses[i];
		const CourseEntry *pEntry = pCourse->FindFixedSong( pSong );
		if( pEntry == NULL || pEntry->attacks.size() == 0 )
			continue;
		apOptions.push_back( pEntry );
	}

	if( apOptions.size() == 0 )
		return;

	const CourseEntry *pEntry = apOptions[ rand()%apOptions.size() ];
	AttackArray aAttacks = pEntry->attacks;
	if( !pEntry->modifiers.empty() )
		aAttacks.push_back( Attack::FromGlobalCourseModifier( pEntry->modifiers ) );

	FOREACH_PlayerNumber(pn)
		m_asModifiersQueue[pn][0] = aAttacks;
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
