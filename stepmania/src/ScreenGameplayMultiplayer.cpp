#include "global.h"
#include "ScreenGameplayMultiplayer.h"
#include "ThemeMetric.h"
#include "GameSoundManager.h"
#include "GameState.h"
#include "Foreach.h"
#include "Course.h"
#include "StatsManager.h"
#include "ScoreKeeperMAX2.h"
#include "ScoreKeeperRave.h"
#include "ScreenDimensions.h"
#include "ScreenManager.h"
#include "ScoreDisplayPercentage.h"
#include "ScoreDisplayNormal.h"
#include "ScoreDisplayOni.h"
#include "song.h"
#include "Steps.h"
#include "NoteDataUtil.h"
#include "RageLog.h"
#include "Style.h"
#include "PlayerState.h"
#include "InputMapper.h"
#include "ActorUtil.h"
#include "PrefsManager.h"

// received while STATE_DANCING
AutoScreenMessage( SM_NotesEnded )
AutoScreenMessage( SM_Ready )
AutoScreenMessage( SM_GoToStateAfterCleared )
AutoScreenMessage( SM_GoToScreenAfterBack )


REGISTER_SCREEN_CLASS( ScreenGameplayMultiplayer );
ScreenGameplayMultiplayer::ScreenGameplayMultiplayer( CString sName, bool bDemonstration ) : Screen(sName)
{
}

void ScreenGameplayMultiplayer::Init()
{
	Screen::Init();

	m_pSoundMusic = NULL;

	/* We do this ourself. */
	SOUND->HandleSongTimer( false );

	//need to initialize these before checking for demonstration mode
	//otherwise destructor will try to delete possibly invalid pointers

    FOREACH_MultiPlayer(p)
	{
		m_pPrimaryScoreDisplay[p] = NULL;
		m_pPrimaryScoreKeeper[p] = NULL;

		m_PlayerState[p].m_PlayerNumber = GAMESTATE->m_MasterPlayerNumber;
		m_PlayerState[p].m_PlayerController = PC_HUMAN;
	}

	if( GAMESTATE->m_pCurSong == NULL && GAMESTATE->m_pCurCourse == NULL )
		return;	// ScreenDemonstration will move us to the next scren.  We just need to survive for one update without crashing.

	/* Save selected options before we change them. */
	GAMESTATE->StoreSelectedOptions();

	/* Save settings to the profile now.  Don't do this on extra stages, since the
	 * user doesn't have full control; saving would force profiles to DIFFICULTY_HARD
	 * and save over their default modifiers every time someone got an extra stage.
	 * Do this before course modifiers are set up. */
	if( !GAMESTATE->IsExtraStage() && !GAMESTATE->IsExtraStage2() )
	{
		FOREACH_HumanPlayer( pn )
			GAMESTATE->SaveCurrentSettingsToProfile(pn);
	}

	GAMESTATE->ResetStageStatistics();




	// fill in difficulty of CPU players with that of the first human player
    FOREACH_PotentialCpuPlayer(p)
        GAMESTATE->m_pCurSteps[p].Set( GAMESTATE->m_pCurSteps[ GAMESTATE->GetFirstHumanPlayer() ] );



	//
	// fill in m_vpSongsQueue, m_vpStepsQueue, m_asModifiersQueue
	//
	if( GAMESTATE->IsCourseMode() )
	{
		Course* pCourse = GAMESTATE->m_pCurCourse;
		ASSERT( pCourse );


		m_vpSongsQueue.clear();
		PlayerNumber pnMaster = GAMESTATE->m_MasterPlayerNumber;
		Trail *pTrail = GAMESTATE->m_pCurTrail[pnMaster];
		FOREACH_CONST( TrailEntry, pTrail->m_vEntries, e )
		{
			m_vpSongsQueue.push_back( e->pSong );
		}

		m_vpStepsQueue.clear();
		FOREACH_CONST( TrailEntry, pTrail->m_vEntries, e )
		{
			m_vpStepsQueue.push_back( e->pSteps );
			m_vModifiersQueue.push_back( AttackArray() );
		}
	}
	else
	{
		m_vpSongsQueue.push_back( GAMESTATE->m_pCurSong );

		{
			m_vpStepsQueue.push_back( GAMESTATE->m_pCurSteps[GAMESTATE->m_MasterPlayerNumber] );
			m_vModifiersQueue.push_back( AttackArray() );
		}
	}

	/* Called once per stage (single song or single course). */
	GAMESTATE->BeginStage();

	STATSMAN->m_CurStageStats.playMode = GAMESTATE->m_PlayMode;
	STATSMAN->m_CurStageStats.pStyle = GAMESTATE->m_pCurStyle;

	{
		ASSERT( !m_vpStepsQueue.empty() );
	}

	if( GAMESTATE->IsExtraStage() )
		STATSMAN->m_CurStageStats.StageType = StageStats::STAGE_EXTRA;
	else if( GAMESTATE->IsExtraStage2() )
		STATSMAN->m_CurStageStats.StageType = StageStats::STAGE_EXTRA2;
	else
		STATSMAN->m_CurStageStats.StageType = StageStats::STAGE_NORMAL;
	
	//
	// Init ScoreKeepers
	//

	FOREACH_MultiPlayer( p )
	{
        switch( PREFSMAN->m_ScoringType )
		{
		case PrefsManager::SCORING_MAX2:
		case PrefsManager::SCORING_5TH:
			m_pPrimaryScoreKeeper[p] = new ScoreKeeperMAX2( 
				&m_PlayerState[p],
				&m_PlayerStageStats[p] );
			m_pPrimaryScoreKeeper[p]->Load( 
				m_vpSongsQueue, 
				m_vpStepsQueue, 
				m_vModifiersQueue );
			break;
		default: ASSERT(0);
		}
	}

	m_Background.SetDrawOrder( DRAW_ORDER_BEFORE_EVERYTHING );
	this->AddChild( &m_Background );

	m_Foreground.SetDrawOrder( DRAW_ORDER_AFTER_EVERYTHING );	// on top of everything else, including transitions
	this->AddChild( &m_Foreground );

	
	{
		float fPlayerX = SCREEN_CENTER_X;

		/* Perhaps this should be handled better by defining a new
		 * StyleType for ONE_PLAYER_ONE_CREDIT_AND_ONE_COMPUTER,
		 * but for now just ignore SoloSingles when it's Battle or Rave
		 * Mode.  This doesn't begin to address two-player solo (6 arrows) */
		if( PREFSMAN->m_bSoloSingle && 
			GAMESTATE->m_PlayMode != PLAY_MODE_BATTLE &&
			GAMESTATE->m_PlayMode != PLAY_MODE_RAVE &&
			GAMESTATE->GetCurrentStyle()->m_StyleType == ONE_PLAYER_ONE_SIDE )
			fPlayerX = SCREEN_CENTER_X;

		m_AutoPlayer.SetName( "Player" );
		m_AutoPlayer.SetX( fPlayerX );
		m_AutoPlayer.SetY( SCREEN_CENTER_Y );
		this->AddChild( &m_AutoPlayer );
	}


	FOREACH_MultiPlayer( p )
	{
		this->AddChild( &m_HumanPlayer[p] );
	}


	FOREACH_MultiPlayer( p )
	{
		//
		// primary score display
		//
		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_REGULAR:
		case PLAY_MODE_NONSTOP:
		case PLAY_MODE_BATTLE:
		case PLAY_MODE_RAVE:
			if( PREFSMAN->m_bPercentageScoring )
				m_pPrimaryScoreDisplay[p] = new ScoreDisplayPercentage;
			else
				m_pPrimaryScoreDisplay[p] = new ScoreDisplayNormal;
			break;
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:
			m_pPrimaryScoreDisplay[p] = new ScoreDisplayOni;
			break;
		default:
			ASSERT(0);
		}

		m_pPrimaryScoreDisplay[p]->Init( &m_PlayerState[p] );
		m_pPrimaryScoreDisplay[p]->SetName( ssprintf("ScoreP%d",p+1) );
		SET_XY( *m_pPrimaryScoreDisplay[p] );
		this->AddChild( m_pPrimaryScoreDisplay[p] );
	}
	

	m_In.Load( THEME->GetPathB(m_sName,"in") );
	m_In.SetDrawOrder( DRAW_ORDER_TRANSITIONS );
	this->AddChild( &m_In );

	m_Out.Load( THEME->GetPathB(m_sName,"out") );
	m_Out.SetDrawOrder( DRAW_ORDER_TRANSITIONS );
	this->AddChild( &m_Out );

	m_Cancel.Load( THEME->GetPathB(m_sName,"cancel") );
	m_Cancel.SetDrawOrder( DRAW_ORDER_TRANSITIONS ); // on top of everything else
	this->AddChild( &m_Cancel );



	/* LoadNextSong first, since that positions some elements which need to be
	 * positioned before we TweenOursOnScreen. */
	LoadNextSong();

	TweenOursOnScreen();

	this->SortByDrawOrder();

	// Get the transitions rolling on the first update.
	// We can't do this in the constructor because ScreenGameplayMultiplayer is constructed 
	// in the middle of ScreenStage.
}

ScreenGameplayMultiplayer::~ScreenGameplayMultiplayer()
{
	if( this->IsFirstUpdate() )
	{
		/* We never received any updates.  That means we were deleted without being
		 * used, and never actually played.  (This can happen when backing out of
		 * ScreenStage.)  Cancel the stage. */
		GAMESTATE->CancelStage();
	}

	FOREACH_MultiPlayer( p )
	{
		SAFE_DELETE( m_pPrimaryScoreDisplay[p] );
		SAFE_DELETE( m_pPrimaryScoreKeeper[p] );
	}

	if( m_pSoundMusic )
		m_pSoundMusic->StopPlaying();
}

bool ScreenGameplayMultiplayer::IsLastSong()
{
	if( GAMESTATE->m_pCurCourse  &&  GAMESTATE->m_pCurCourse->m_bRepeat )
		return false;
	return GAMESTATE->GetCourseSongIndex()+1 == (int)m_vpSongsQueue.size(); // GetCourseSongIndex() is 0-based but size() is not
}

void ScreenGameplayMultiplayer::SetupSong( MultiPlayer p, int iSongIndex )
{
	/* This is the first beat that can be changed without it being visible.  Until
	 * we draw for the first time, any beat can be changed. */
	GAMESTATE->m_pPlayerState[GAMESTATE->m_MasterPlayerNumber]->m_fLastDrawnBeat = -100;
	GAMESTATE->m_pCurSteps[GAMESTATE->m_MasterPlayerNumber].Set( m_vpStepsQueue[iSongIndex] );

	/* Load new NoteData into Player.  Do this before 
	 * RebuildPlayerOptionsFromActiveAttacks or else transform mods will get
	 * propogated to GAMESTATE->m_AutoPlayerOptions too early and be double-applied
	 * to the NoteData:
	 * once in Player::Load, then again in Player::ApplyActiveAttacks.  This 
	 * is very bad for transforms like AddMines.
	 */
	NoteData originalNoteData;
	GAMESTATE->m_pCurSteps[GAMESTATE->m_MasterPlayerNumber]->GetNoteData( originalNoteData );
	
	const Style* pStyle = GAMESTATE->GetCurrentStyle();
	NoteData ndTransformed;
	pStyle->GetTransformedNoteDataForStyle( GAMESTATE->m_MasterPlayerNumber, originalNoteData, ndTransformed );

	// load player
	{
		NoteData nd = ndTransformed;
		NoteDataUtil::RemoveAllTapsOfType( nd, TapNote::autoKeysound );
		m_AutoPlayer.Init( 
			"Player",
			GAMESTATE->m_pPlayerState[ GAMESTATE->m_MasterPlayerNumber ], 
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			NULL );
		m_AutoPlayer.Load( nd );
		
		m_HumanPlayer[p].Init( 
			"Player",
			&m_PlayerState[p], 
			&m_PlayerStageStats[p],
			NULL, 
			NULL, 
			m_pPrimaryScoreDisplay[p], 
			NULL, 
			NULL, 
			m_pPrimaryScoreKeeper[p], 
			NULL );
		m_HumanPlayer[p].Load( nd );
	}

	// load auto keysounds
	{
		NoteData nd = ndTransformed;
		NoteDataUtil::RemoveAllTapsExceptForType( nd, TapNote::autoKeysound );
		m_AutoKeysounds.Load( GAMESTATE->m_MasterPlayerNumber, nd );
	}
}

void ScreenGameplayMultiplayer::LoadNextSong()
{
	GAMESTATE->ResetMusicStatistics();

	int iPlaySongIndex = GAMESTATE->GetCourseSongIndex();
	iPlaySongIndex %= m_vpSongsQueue.size();
	GAMESTATE->m_pCurSong.Set( m_vpSongsQueue[iPlaySongIndex] );
	STATSMAN->m_CurStageStats.vpPlayedSongs.push_back( GAMESTATE->m_pCurSong );

	// No need to do this here.  We do it in SongFinished().
	//GAMESTATE->RemoveAllActiveAttacks();

	// Restore the player's originally selected options.
	GAMESTATE->RestoreSelectedOptions();

	/* If we're in battery mode, force FailImmediate.  We assume in PlayerMinus::Step that
	 * failed players can't step. */
	if( GAMESTATE->m_SongOptions.m_LifeType == SongOptions::LIFE_BATTERY )
		GAMESTATE->m_SongOptions.m_FailType = SongOptions::FAIL_IMMEDIATE;

	Steps* pSteps = GAMESTATE->m_pCurSteps[GAMESTATE->m_MasterPlayerNumber];

	STATSMAN->m_CurStageStats.m_player[GAMESTATE->m_MasterPlayerNumber].vpPlayedSteps.push_back( pSteps );

	FOREACH_MultiPlayer( p )
	{
		SetupSong( p, iPlaySongIndex );

		ASSERT( GAMESTATE->m_pCurSteps[GAMESTATE->m_MasterPlayerNumber] );

		/* The actual note data for scoring is the base class of Player.  This includes
		 * transforms, like Wide.  Otherwise, the scoring will operate on the wrong data. */
		m_pPrimaryScoreKeeper[p]->OnNextSong( 
			GAMESTATE->GetCourseSongIndex(), 
			GAMESTATE->m_pCurSteps[GAMESTATE->m_MasterPlayerNumber], 
			&m_HumanPlayer[p].m_NoteData );
	}

	GAMESTATE->m_pPlayerState[GAMESTATE->m_MasterPlayerNumber]->m_PlayerController = PC_AUTOPLAY;

	m_AutoKeysounds.FinishLoading();
	m_pSoundMusic = m_AutoKeysounds.GetSound();

	{
		/* BeginnerHelper disabled/failed to load. */
		m_Background.LoadFromSong( GAMESTATE->m_pCurSong );
		/* This will fade from a preset brightness to the actual brightness (based
		 * on prefs and "cover").  The preset brightness may be 0 (to fade from
		 * black), or it might be 1, if the stage screen has the song BG and we're
		 * coming from it (like Pump).  This used to be done in SM_PlayReady, but
		 * that means it's impossible to snap to the new brightness immediately. */
		m_Background.FadeToActualBrightness();
	}

	m_Foreground.LoadFromSong( GAMESTATE->m_pCurSong );
}

float ScreenGameplayMultiplayer::StartPlayingSong(float MinTimeToNotes, float MinTimeToMusic)
{
	ASSERT(MinTimeToNotes >= 0);
	ASSERT(MinTimeToMusic >= 0);

	/* XXX: We want the first beat *in use*, so we don't delay needlessly. */
	const float fFirstBeat = GAMESTATE->m_pCurSong->m_fFirstBeat;
	const float fFirstSecond = GAMESTATE->m_pCurSong->GetElapsedTimeFromBeat( fFirstBeat );
	float fStartSecond = fFirstSecond - MinTimeToNotes;

	fStartSecond = min(fStartSecond, -MinTimeToMusic);
	
	RageSoundParams p;
	p.AccurateSync = true;
	p.SetPlaybackRate( GAMESTATE->m_SongOptions.m_fMusicRate );
	p.StopMode = RageSoundParams::M_CONTINUE;
	p.m_StartSecond = fStartSecond;


	m_pSoundMusic->Play( &p );

	/* Make sure GAMESTATE->m_fMusicSeconds is set up. */
	GAMESTATE->m_fMusicSeconds = -5000;
	UpdateSongPosition(0);

	ASSERT( GAMESTATE->m_fMusicSeconds > -4000 ); /* make sure the "fake timer" code doesn't trigger */

	/* Return the amount of time until the first beat. */
	return fFirstSecond - fStartSecond;
}

void ScreenGameplayMultiplayer::UpdateSongPosition( float fDeltaTime )
{
	if( !m_pSoundMusic->IsPlaying() )
		return;

	RageTimer tm;
	const float fSeconds = m_pSoundMusic->GetPositionSeconds( NULL, &tm );
	const float fAdjust = SOUND->GetFrameTimingAdjustment( fDeltaTime );
	GAMESTATE->UpdateSongPosition( fSeconds+fAdjust, GAMESTATE->m_pCurSong->m_Timing, tm+fAdjust );
}

void ScreenGameplayMultiplayer::Update( float fDeltaTime )
{
	if( GAMESTATE->m_pCurSong == NULL  )
	{
		/* ScreenDemonstration will move us to the next screen.  We just need to
		 * survive for one update without crashing.  We need to call Screen::Update
		 * to make sure we receive the next-screen message. */
		Screen::Update( fDeltaTime );
		return;
	}

	if( m_bFirstUpdate )
	{
		SOUND->PlayOnceFromAnnouncer( "gameplay intro" );	// crowd cheer

		//
		// Get the transitions rolling
		//
		StartPlayingSong( 0, 0 );	// *kick* (no transitions)
	}


	UpdateSongPosition( fDeltaTime );

	Screen::Update( fDeltaTime );

	/* This happens if ScreenDemonstration::HandleScreenMessage sets a new screen when
	 * PREFSMAN->m_bDelayedScreenLoad. */
	if( GAMESTATE->m_pCurSong == NULL )
		return;
	/* This can happen if ScreenDemonstration::HandleScreenMessage sets a new screen when
	 * !PREFSMAN->m_bDelayedScreenLoad.  (The new screen was loaded when we called Screen::Update,
	 * and the ctor might set a new GAMESTATE->m_pCurSong, so the above check can fail.) */
	if( SCREENMAN->GetTopScreen() != this )
		return;

	m_AutoKeysounds.Update(fDeltaTime);


	//
	// Check for end of song
	//
	float fSecondsToStop = GAMESTATE->m_pCurSong->GetElapsedTimeFromBeat( GAMESTATE->m_pCurSong->m_fLastBeat );

	/* Make sure we keep going long enough to register a miss for the last note. */
	fSecondsToStop += m_AutoPlayer.GetMaxStepDistanceSeconds();

	if( GAMESTATE->m_fMusicSeconds > fSecondsToStop && !m_Out.IsTransitioning() )
		this->HandleScreenMessage( SM_NotesEnded );
}


void ScreenGameplayMultiplayer::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	//LOG->Trace( "ScreenGameplayMultiplayer::Input()" );

	if( type == IET_LEVEL_CHANGED )
		return;

	if( MenuI.IsValid()  &&  !m_Cancel.IsTransitioning() )
	{
		if( MenuI.button == MENU_BUTTON_BACK && 
			((!PREFSMAN->m_bDelayedBack && type==IET_FIRST_PRESS) ||
			(DeviceI.device==DEVICE_KEYBOARD && (type==IET_SLOW_REPEAT||type==IET_FAST_REPEAT)) ||
			(DeviceI.device!=DEVICE_KEYBOARD && type==IET_FAST_REPEAT)) )
		{
			/* I had battle mode back out on me mysteriously once. -glenn */
			LOG->Trace("Player %i went back", MenuI.player+1);

			/* Hmm.  There are a bunch of subtly different ways we can
			 * tween out: 
			 *   1. Keep rendering the song, and keep it moving.  This might
			 *      cause problems if the cancel and the end of the song overlap.
			 *   2. Stop the song completely, so all song motion under the tween
			 *      ceases.
			 *   3. Stop the song, but keep effects (eg. Drunk) running.
			 *   4. Don't display the song at all.
			 *
			 * We're doing #3.  I'm not sure which is best.
			 */
			
			m_pSoundMusic->StopPlaying();

			this->ClearMessageQueue();
			m_Cancel.StartTransitioning( SM_GoToScreenAfterBack );
			return;
		}
	}

	/* Nothing below cares about releases. */
	if(type == IET_RELEASE) return;

	// Handle special keys to adjust the offset
	if( DeviceI.device == DEVICE_KEYBOARD )
	{
		switch( DeviceI.button )
		{
		case KEY_F5:
			this->HandleScreenMessage( SM_NotesEnded );
			break;
		}
	}
	
	
	// Translate input and sent to the appropriate player.  Assume that all 
	// joystick devices are mapped the same as the master player.
	if( DeviceI.device >= DEVICE_JOY1  &&  
		DeviceI.device < DEVICE_JOY1 + NUM_JOYSTICKS )
	{
		DeviceInput _DeviceI = DeviceI;
		_DeviceI.device = DEVICE_JOY1;
		GameInput _GameI;
		INPUTMAPPER->DeviceToGame( _DeviceI, _GameI );

		if( GameI.IsValid() )
		{
			StyleInput _StyleI;
			INPUTMAPPER->GameToStyle( _GameI, _StyleI );

			MultiPlayer mp = (MultiPlayer)(DeviceI.device - DEVICE_JOY1);

			ASSERT( mp>=0 && mp<NUM_MultiPlayer );
			m_HumanPlayer[mp].Step( _StyleI.col, DeviceI.ts );
		}
	}
}


void ScreenGameplayMultiplayer::StageFinished( bool bBackedOut )
{
	if( GAMESTATE->IsCourseMode() && GAMESTATE->m_PlayMode != PLAY_MODE_ENDLESS )
	{
		LOG->Trace("Stage finished at index %i/%i", GAMESTATE->GetCourseSongIndex(), (int) m_vpSongsQueue.size() );
		/* +1 to skip the current song; that's done already. */
		for( unsigned iPlaySongIndex = GAMESTATE->GetCourseSongIndex()+1;
			 iPlaySongIndex < m_vpSongsQueue.size(); ++iPlaySongIndex )
		{
			LOG->Trace("Running stats for %i", iPlaySongIndex );
			FOREACH_MultiPlayer(p)
			{
				SetupSong( p, iPlaySongIndex );
			}
		}
	}

	// save current stage stats
	if( !bBackedOut )
		STATSMAN->m_vPlayedStageStats.push_back( STATSMAN->m_CurStageStats );

	/* Reset options. */
	GAMESTATE->RestoreSelectedOptions();
}

void ScreenGameplayMultiplayer::HandleScreenMessage( const ScreenMessage SM )
{
	CHECKPOINT_M( ssprintf("HandleScreenMessage(%i)", SM) );
	if( SM == SM_Ready )
	{
		GAMESTATE->m_bPastHereWeGo = true;
	}
	else if( SM == SM_NotesEnded )
	{
		m_Out.StartTransitioning( SM_GoToStateAfterCleared );
	}
	else if( SM == SM_GoToScreenAfterBack )
	{
		StageFinished( true );

		GAMESTATE->CancelStage();

		SCREENMAN->PostMessageToTopScreen( SM_GoToPrevScreen, 0 );
	}
	else if( SM == SM_GoToStateAfterCleared )
	{
		StageFinished( false );

		SCREENMAN->PostMessageToTopScreen( SM_GoToNextScreen, 0 );
	}

	Screen::HandleScreenMessage( SM );
}


void ScreenGameplayMultiplayer::TweenOursOnScreen()
{
    FOREACH_MultiPlayer(p)
	{
		if( m_pPrimaryScoreDisplay[p] )
			ON_COMMAND( *m_pPrimaryScoreDisplay[p] );
	}

	m_In.StartTransitioning( SM_Ready );
}

void ScreenGameplayMultiplayer::TweenOursOffScreen()
{
    FOREACH_MultiPlayer(p)
	{
		if( m_pPrimaryScoreDisplay[p] )
			OFF_COMMAND( *m_pPrimaryScoreDisplay[p] );
	}
}


/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
