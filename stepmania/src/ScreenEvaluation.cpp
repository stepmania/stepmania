#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenEvaluation

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenEvaluation.h"
#include "SongManager.h"
#include "ScreenManager.h"
#include "GameManager.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "Steps.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "GrooveRadar.h"
#include "ThemeManager.h"
#include "RageSounds.h"
#include "ActorUtil.h"
#include "RageTimer.h"
#include "UnlockSystem.h"
#include "Course.h"

const int NUM_SCORE_DIGITS	=	9;

// metrics that are common to all ScreenEvaluation classes
#define BANNER_WIDTH						THEME->GetMetricF("ScreenEvaluation","BannerWidth")
#define BANNER_HEIGHT						THEME->GetMetricF("ScreenEvaluation","BannerHeight")
const char* JUDGE_STRING[NUM_JUDGE_LINES] = { "Marvelous", "Perfect", "Great", "Good", "Boo", "Miss", "OK", "MaxCombo" };
#define SPIN_GRADES							THEME->GetMetricB("ScreenEvaluation","SpinGrades")
#define CHEER_DELAY_SECONDS					THEME->GetMetricF("ScreenEvaluation","CheerDelaySeconds")
#define BAR_ACTUAL_MAX_COMMAND				THEME->GetMetric ("ScreenEvaluation","BarActualMaxCommand")

// #define JUDGE_SOUND_TIME( name )	THEME->GetMetricF( m_sName, ssprintf("JudgeSoundTime%s", name) );
// #define SOUND_ON_FULL_ALPHA			THEME->GetMetricB( m_sName, "JudgeSoundIfJudgeGraphicsFullAlpha" )
// metrics that are specific to classes derived from ScreenEvaluation
#define NEXT_SCREEN							THEME->GetMetric (m_sName,"NextScreen")
#define END_SCREEN							THEME->GetMetric (m_sName,"EndScreen")
#define SHOW_BANNER_AREA					THEME->GetMetricB(m_sName,"ShowBannerArea")
#define SHOW_GRADE_AREA						THEME->GetMetricB(m_sName,"ShowGradeArea")
#define SHOW_POINTS_AREA					THEME->GetMetricB(m_sName,"ShowPointsArea")
#define SHOW_BONUS_AREA						THEME->GetMetricB(m_sName,"ShowBonusArea")
#define SHOW_SURVIVED_AREA					THEME->GetMetricB(m_sName,"ShowSurvivedArea")
#define SHOW_WIN_AREA						THEME->GetMetricB(m_sName,"ShowWinArea")
#define SHOW_JUDGMENT( l )					THEME->GetMetricB(m_sName,ssprintf("Show%s",JUDGE_STRING[l]))
#define SHOW_SCORE_AREA						THEME->GetMetricB(m_sName,"ShowScoreArea")
#define SHOW_TIME_AREA						THEME->GetMetricB(m_sName,"ShowTimeArea")


const ScreenMessage SM_GoToSelectCourse			=	ScreenMessage(SM_User+3);
const ScreenMessage SM_GoToEvaluationSummary	=	ScreenMessage(SM_User+4);
const ScreenMessage SM_GoToEndScreen			=	ScreenMessage(SM_User+5);
const ScreenMessage SM_PlayCheer				=	ScreenMessage(SM_User+6);


ScreenEvaluation::ScreenEvaluation( CString sClassName, Type type ) : Screen(sClassName)
{
	//
	// debugging
	//
/*	GAMESTATE->m_PlayMode = PLAY_MODE_NONSTOP;
	GAMESTATE->m_CurStyle = STYLE_DANCE_VERSUS;
	GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
	GAMESTATE->m_pCurSong = SONGMAN->GetAllSongs()[0];
	GAMESTATE->m_pCurCourse = SONGMAN->m_pCourses[0];
	GAMESTATE->m_pCurNotes[PLAYER_1] = GAMESTATE->m_pCurSong->GetNotes( STEPS_TYPE_DANCE_SINGLE, DIFFICULTY_HARD );
	GAMESTATE->m_pCurNotes[PLAYER_2] = GAMESTATE->m_pCurSong->GetNotes( STEPS_TYPE_DANCE_SINGLE, DIFFICULTY_HARD );
	GAMESTATE->m_PlayerOptions[PLAYER_1].m_bHoldNotes = false;
	GAMESTATE->m_PlayerOptions[PLAYER_2].m_bHoldNotes = false;
	GAMESTATE->m_PlayerOptions[PLAYER_1].m_fScrollSpeed = 2;
	GAMESTATE->m_PlayerOptions[PLAYER_2].m_fScrollSpeed = 2;
	GAMESTATE->m_iCurrentStageIndex = 2;
*/

	LOG->Trace( "ScreenEvaluation::ScreenEvaluation()" );

	m_sName = sClassName;
	m_Type = type;

	int p;

	//
	// Figure out which statistics and songs we're going to display
	//
	StageStats stageStats;
	vector<Song*> vSongsToShow;
	switch( m_Type )
	{
	case summary:
		GAMESTATE->GetFinalEvalStatsAndSongs( stageStats, vSongsToShow );
		break;
	case stage:
	case course:
		LOG->Trace("We made it to stage");
		stageStats = GAMESTATE->m_CurStageStats;
		break;
	default:
		ASSERT(0);
	}
	LOG->Trace( "total error %i", stageStats.iTotalError[0], stageStats.iTotalError[1] );

/*
	//
	// Debugging
	//
	{
		for( int p=0; p<NUM_PLAYERS; p++ )	// foreach line
			for( int r=0; r<NUM_RADAR_CATEGORIES; r++ )	// foreach line
			{
				stageStats.fRadarPossible[p][r] = 0.5f + r/10.0f;
				stageStats.fRadarActual[p][r] = 0.5f + r/10.0f;
			}
	}
*/

	//
	// Calculate grades
	//
	Grade grade[NUM_PLAYERS];
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( GAMESTATE->IsPlayerEnabled(p) )
			grade[p] = stageStats.GetGrade( (PlayerNumber)p );
		else
			grade[p] = GRADE_E;
	}

	Grade max_grade = GRADE_NO_DATA;
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		max_grade = max( max_grade, grade[p] ); 

		// if its extra stage, update # passed stages
		if (PREFSMAN->m_bUseUnlockSystem &&
			(GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() ) &&
			grade[p] > GRADE_E &&
			m_Type != summary)
					GAMESTATE->m_pUnlockingSys->UnlockClearExtraStage();
	}


	//
	// update persistent statistics
	//
	bool bNewRecord[NUM_PLAYERS];
	ZERO( bNewRecord );


	switch( m_Type )
	{
	case stage:
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( GAMESTATE->IsHumanPlayer(p) )
				{
					GAMESTATE->m_pCurNotes[p]->AddScore( (PlayerNumber)p, grade[p], stageStats.iScore[p], bNewRecord[p] );
					
					// update unlock data if unlocks are on
					if ( PREFSMAN->m_bUseUnlockSystem )
					{
						GAMESTATE->m_pUnlockingSys->UnlockClearStage();
						GAMESTATE->m_pUnlockingSys->UnlockAddAP( grade[p] );
						GAMESTATE->m_pUnlockingSys->UnlockAddSP( grade[p] );
					}
				}
			}
		}
		break;
	case summary:
		{
			StepsType nt = GAMESTATE->GetCurrentStyleDef()->m_StepsType;
			bool bIsHumanPlayer[NUM_PLAYERS];
			for( p=0; p<NUM_PLAYERS; p++ )
				bIsHumanPlayer[p] = GAMESTATE->IsHumanPlayer(p);

			RankingCategory cat[NUM_PLAYERS];
			int iRankingIndex[NUM_PLAYERS];
			float	fTotalDP = 0;
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				float fAverageMeter = stageStats.iMeter[p] / (float)PREFSMAN->m_iNumArcadeStages;
				cat[p] = AverageMeterToRankingCategory( fAverageMeter );
				fTotalDP += stageStats.iActualDancePoints[p];
			}

			SONGMAN->AddScores( nt, bIsHumanPlayer, cat, stageStats.iScore, iRankingIndex );

			COPY( GAMESTATE->m_RankingCategory, cat );
			COPY( GAMESTATE->m_iRankingIndex, iRankingIndex );
			GAMESTATE->m_RankingNotesType = nt;

			// If unlocking is enabled, save the dance points
			if( PREFSMAN->m_bUseUnlockSystem )
				GAMESTATE->m_pUnlockingSys->UnlockAddDP( fTotalDP );
		}
		break;
	case course:
		{
			StepsType nt = GAMESTATE->GetCurrentStyleDef()->m_StepsType;
			bool bIsHumanPlayer[NUM_PLAYERS];
			for( p=0; p<NUM_PLAYERS; p++ )
				bIsHumanPlayer[p] = GAMESTATE->IsHumanPlayer(p);

			int iRankingIndex[NUM_PLAYERS];

			Course* pCourse = GAMESTATE->m_pCurCourse;

			int *score;
			if( pCourse->IsOni() )
				score = stageStats.iActualDancePoints;
			else
				score = stageStats.iScore;
			pCourse->AddScores( nt, bIsHumanPlayer, score, stageStats.fAliveSeconds, iRankingIndex, bNewRecord );
			COPY( GAMESTATE->m_iRankingIndex, iRankingIndex );
			GAMESTATE->m_pRankingCourse = pCourse;
			GAMESTATE->m_RankingNotesType = nt;

			// If unlocking is enabled, save the dance points
			// (for courses)
			if( PREFSMAN->m_bUseUnlockSystem )
			{
				for(p=0; p < NUM_PLAYERS; p++)
				{
					if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
						continue;	// skip

					GAMESTATE->m_pUnlockingSys->UnlockAddDP( (float) stageStats.iActualDancePoints[p] );
					GAMESTATE->m_pUnlockingSys->UnlockAddAP( (float) stageStats.iSongsPassed[p] );
					GAMESTATE->m_pUnlockingSys->UnlockAddSP( (float) stageStats.iSongsPassed[p] );
				}
			}
			// cannot just use score since it may be nonstop mode


		}
		break;
	default:
		ASSERT(0);
	}

	m_bTryExtraStage = 
		GAMESTATE->HasEarnedExtraStage()  && 
		m_Type==stage;
 


	m_Menu.Load( m_sName );
	this->AddChild( &m_Menu );



	//
	// init banner area
	//
	if( SHOW_BANNER_AREA )
	{
		switch( m_Type )
		{
		case stage:
			{
				m_LargeBanner.LoadFromSong( GAMESTATE->m_pCurSong );
				m_LargeBanner.ScaleToClipped( BANNER_WIDTH, BANNER_HEIGHT );
				m_LargeBanner.SetName( "LargeBanner" );
				UtilSetXYAndOnCommand( m_LargeBanner, "ScreenEvaluation" );
				this->AddChild( &m_LargeBanner );

				m_sprLargeBannerFrame.Load( THEME->GetPathToG("ScreenEvaluation banner frame") );
				m_sprLargeBannerFrame.SetName( "LargeBanner" );
				UtilSetXYAndOnCommand( m_sprLargeBannerFrame, "ScreenEvaluation" );
				this->AddChild( &m_sprLargeBannerFrame );

				m_sprStage.Load( THEME->GetPathToG("ScreenEvaluation stage "+GAMESTATE->GetStageText()) );
				m_sprStage.SetName( "Stage" );
				UtilSetXYAndOnCommand( m_sprStage, "ScreenEvaluation" );
				this->AddChild( &m_sprStage );

				for( int p=0; p<NUM_PLAYERS; p++ )
				{
					if( !GAMESTATE->IsPlayerEnabled(p) )
						continue;	// skip

					m_DifficultyIcon[p].Load( THEME->GetPathToG("ScreenEvaluation difficulty icons 1x5") );
					m_DifficultyIcon[p].SetFromNotes( (PlayerNumber)p, GAMESTATE->m_pCurNotes[p] );
					m_DifficultyIcon[p].SetName( ssprintf("DifficultyIconP%d",p+1) );
					UtilSetXYAndOnCommand( m_DifficultyIcon[p], "ScreenEvaluation" );
					this->AddChild( &m_DifficultyIcon[p] );
					
					m_textPlayerOptions[p].LoadFromFont( THEME->GetPathToF("Common normal") );
					CString sPO = GAMESTATE->m_PlayerOptions[p].GetString();
					sPO.Replace( ", ", "\n" );
					m_textPlayerOptions[p].SetName( ssprintf("PlayerOptionsP%d",p+1) );
					UtilSetXYAndOnCommand( m_textPlayerOptions[p], "ScreenEvaluation" );
					m_textPlayerOptions[p].SetText( sPO );
					this->AddChild( &m_textPlayerOptions[p] );
				}
			}
			break;
		case summary:
			{
				for( unsigned i=0; i<vSongsToShow.size(); i++ )
				{
					m_SmallBanner[i].LoadFromSong( vSongsToShow[i] );
					m_SmallBanner[i].ScaleToClipped( BANNER_WIDTH, BANNER_HEIGHT );
					m_SmallBanner[i].SetName( ssprintf("SmallBanner%d",i+1) );
					UtilSetXYAndOnCommand( m_SmallBanner[i], "ScreenEvaluation" );
					this->AddChild( &m_SmallBanner[i] );

					m_sprSmallBannerFrame[i].Load( THEME->GetPathToG("ScreenEvaluation banner frame") );
					m_sprSmallBannerFrame[i].SetName( ssprintf("SmallBanner%d",i+1) );
					UtilSetXYAndOnCommand( m_sprSmallBannerFrame[i], "ScreenEvaluation" );
					this->AddChild( &m_sprSmallBannerFrame[i] );
				}
			}
			break;
		case course:
			{
				m_LargeBanner.LoadFromCourse( GAMESTATE->m_pCurCourse );
				m_LargeBanner.ScaleToClipped( BANNER_WIDTH, BANNER_HEIGHT );
				m_LargeBanner.SetName( "LargeBanner" );
				UtilSetXYAndOnCommand( m_LargeBanner, "ScreenEvaluation" );
				this->AddChild( &m_LargeBanner );

				m_sprLargeBannerFrame.Load( THEME->GetPathToG("ScreenEvaluation banner frame") );
				m_sprLargeBannerFrame.SetName( "LargeBanner" );
				UtilSetXYAndOnCommand( m_sprLargeBannerFrame, "ScreenEvaluation" );
				this->AddChild( &m_sprLargeBannerFrame );
			}
			break;
		default:
			ASSERT(0);
		}
	}

	//
	// init grade area
	//
	if( SHOW_GRADE_AREA )
	{
		for( p=0; p<NUM_PLAYERS; p++ ) 
		{
			if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
				continue;	// skip

			m_sprGradeFrame[p].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation grade frame p%d",p+1)) );
			m_sprGradeFrame[p].SetName( ssprintf("GradeFrameP%d",p+1) );
			UtilSetXYAndOnCommand( m_sprGradeFrame[p], "ScreenEvaluation" );
			this->AddChild( &m_sprGradeFrame[p] );

			m_Grades[p].Load( THEME->GetPathToG("ScreenEvaluation grades") );
			m_Grades[p].SetGrade( (PlayerNumber)p, grade[p] );
			m_Grades[p].SetName( ssprintf("GradeP%d",p+1) );
			UtilSetXYAndOnCommand( m_Grades[p], "ScreenEvaluation" );
			if( SPIN_GRADES )
				m_Grades[p].Spin();
			this->AddChild( &m_Grades[p] );
		}
	}

	//
	// init points area
	//
	if( SHOW_POINTS_AREA )
	{
		for( p=0; p<NUM_PLAYERS; p++ ) 
		{
			m_sprPercentFrame[p].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation percent frame p%d",p+1)) );
			m_sprPercentFrame[p].SetName( ssprintf("PercentFrameP%d",p+1) );
			UtilSetXYAndOnCommand( m_sprPercentFrame[p], "ScreenEvaluation" );
			this->AddChild( &m_sprPercentFrame[p] );

			if( !PREFSMAN->m_bDancePointsForOni )
			{
				m_textPercentWhole[p].LoadFromNumbers( THEME->GetPathToN("ScreenEvaluation percent") );
				m_textPercentWhole[p].EnableShadow( false );
				m_textPercentWhole[p].SetName( ssprintf("PercentWholeP%d",p+1) );
				UtilSetXYAndOnCommand( m_textPercentWhole[p], "ScreenEvaluation" );
				this->AddChild( &m_textPercentWhole[p] );

				m_textPercentRemainder[p].LoadFromNumbers( THEME->GetPathToN("ScreenEvaluation percent") );
				m_textPercentRemainder[p].EnableShadow( false );
				m_textPercentRemainder[p].SetName( ssprintf("PercentRemainderP%d",p+1) );
				UtilSetXYAndOnCommand( m_textPercentRemainder[p], "ScreenEvaluation" );
				this->AddChild( &m_textPercentRemainder[p] );

				stageStats.iPossibleDancePoints[p] = max( 1, stageStats.iPossibleDancePoints[p] );
				float fPercentDancePoints =  stageStats.iActualDancePoints[p] / (float)stageStats.iPossibleDancePoints[p] + 0.0001f;	// correct for rounding errors
				fPercentDancePoints = max( fPercentDancePoints, 0 );
				int iPercentWhole = int(fPercentDancePoints*100);
				int iPercentRemainder = int( (fPercentDancePoints*100 - int(fPercentDancePoints*100)) * 10 );
				m_textPercentWhole[p].SetText( ssprintf("%02d", iPercentWhole) );
				m_textPercentRemainder[p].SetText( ssprintf(".%01d%%", iPercentRemainder) );
			}
			else	// PREFSMAN->m_bDancePointsForOni
			{
				m_textDancePoints[p].LoadFromNumbers( THEME->GetPathToN("ScreenEvaluation percent") );
				m_textDancePoints[p].EnableShadow( false );
				/* We don't have a hyphen. */
				if( stageStats.iActualDancePoints[p] < 0 )
					m_textDancePoints[p].SetText( ssprintf("0") );
				else
					m_textDancePoints[p].SetText( ssprintf("%d", stageStats.iActualDancePoints[p]) );
				m_textDancePoints[p].SetName( ssprintf("DancePointsP%d",p+1) );
				UtilSetXYAndOnCommand( m_textDancePoints[p], "ScreenEvaluation" );
				this->AddChild( &m_textDancePoints[p] );
			}
		}
	}

	//
	// init bonus area
	//
	if( SHOW_BONUS_AREA )
	{
		for( p=0; p<NUM_PLAYERS; p++ ) 
		{
			if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
				continue;	// skip

			m_sprBonusFrame[p].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation bonus frame p%d",p+1)) );
			m_sprBonusFrame[p].SetName( ssprintf("BonusFrameP%d",p+1) );
			UtilSetXYAndOnCommand( m_sprBonusFrame[p], "ScreenEvaluation" );
			this->AddChild( &m_sprBonusFrame[p] );

			for( int r=0; r<NUM_RADAR_CATEGORIES; r++ )	// foreach line
			{
				m_sprPossibleBar[p][r].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation bar possible p%d",p+1)) );
				m_sprPossibleBar[p][r].SetWidth( m_sprPossibleBar[p][r].GetUnzoomedWidth() * stageStats.fRadarPossible[p][r] );
				m_sprPossibleBar[p][r].SetName( ssprintf("BarPossible%dP%d",r+1,p+1) );
				UtilSetXYAndOnCommand( m_sprPossibleBar[p][r], "ScreenEvaluation" );
				this->AddChild( &m_sprPossibleBar[p][r] );

				m_sprActualBar[p][r].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation bar actual p%d",p+1)) );
				// should be out of the possible bar, not actual (whatever value that is at)
				m_sprActualBar[p][r].SetWidth( m_sprPossibleBar[p][r].GetUnzoomedWidth() * stageStats.fRadarActual[p][r] );
				
				float value = (float)100 * m_sprActualBar[p][r].GetUnzoomedWidth() / m_sprPossibleBar[p][r].GetUnzoomedWidth();
				LOG->Trace("Radar bar %d of 5 - %f percent", r,  value);
				
				m_sprActualBar[p][r].SetName( ssprintf("BarActual%dP%d",r+1,p+1) );
				UtilSetXYAndOnCommand( m_sprActualBar[p][r], "ScreenEvaluation" );
				
				// .99999 is fairly close to 1.00, so we use that 
				if( stageStats.fRadarActual[p][r] > 0.99999f )
					m_sprActualBar[p][r].Command( BAR_ACTUAL_MAX_COMMAND );
				this->AddChild( &m_sprActualBar[p][r] );
			}
		}
	}

	//
	// init survived area
	//
	if( SHOW_SURVIVED_AREA )
	{
		for( p=0; p<NUM_PLAYERS; p++ ) 
		{
			if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
				continue;	// skip

			m_sprSurvivedFrame[p].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation survived frame p%d",p+1)) );
			m_sprSurvivedFrame[p].SetName( ssprintf("SurvivedFrameP%d",p+1) );
			UtilSetXYAndOnCommand( m_sprSurvivedFrame[p], "ScreenEvaluation" );
			this->AddChild( &m_sprSurvivedFrame[p] );

			m_textSurvivedNumber[p].LoadFromNumbers( THEME->GetPathToN("ScreenEvaluation stage") );
			m_textSurvivedNumber[p].EnableShadow( false );
			// curewater: edited the "# stages cleared" text so it deducts one if you failed.
			// Should be accurate, but I'm not sure if its "standard" that (bool)true = 1.  (assumption)
			m_textSurvivedNumber[p].SetText( ssprintf("%02d", stageStats.iSongsPlayed[p] - (int)stageStats.bFailed[p]) );
			m_textSurvivedNumber[p].SetName( ssprintf("SurvivedNumberP%d",p+1) );
			UtilSetXYAndOnCommand( m_textSurvivedNumber[p], "ScreenEvaluation" );
			this->AddChild( &m_textSurvivedNumber[p] );
		}
	}
	
	//
	// init win area
	//
	if( SHOW_WIN_AREA )
	{
		for( p=0; p<NUM_PLAYERS; p++ ) 
		{
			if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
				continue;	// skip

			m_sprWinFrame[p].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation win frame p%d",p+1)) );
			m_sprWinFrame[p].SetName( ssprintf("WinFrameP%d",p+1) );
			UtilSetXYAndOnCommand( m_sprWinFrame[p], "ScreenEvaluation" );
			this->AddChild( &m_sprWinFrame[p] );

			m_sprWin[p].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation win p%d 1x3",p+1)) );
			m_sprWin[p].StopAnimating();
			int iFrame = GAMESTATE->GetStageResult( (PlayerNumber)p );
			m_sprWin[p].SetState( iFrame );
			m_sprWin[p].SetName( ssprintf("WinP%d",p+1) );
			UtilSetXYAndOnCommand( m_sprWin[p], "ScreenEvaluation" );
			this->AddChild( &m_sprWin[p] );
		}
	}
	
	//
	// init judgment area
	//
	int l;
	for( l=0; l<NUM_JUDGE_LINES; l++ ) 
	{
//		m_TimeToPlayJudgeSound[l] = -1;
		if( l == 0  && !GAMESTATE->ShowMarvelous() )
			continue;	// skip

		if( SHOW_JUDGMENT(l) )
		{
			m_sprJudgeLabels[l].Load( THEME->GetPathToG("ScreenEvaluation judge labels 1x8") );
			m_sprJudgeLabels[l].StopAnimating();
			m_sprJudgeLabels[l].SetState( l );
			m_sprJudgeLabels[l].SetName( ssprintf("%sLabel",JUDGE_STRING[l]) );
			UtilSetXYAndOnCommand( m_sprJudgeLabels[l], "ScreenEvaluation" );
			this->AddChild( &m_sprJudgeLabels[l] );

	//		m_soundJudgeSound[l].Load( THEME->GetPathToS( ssprintf("ScreenEvaluation JudgeSound %s", JUDGE_STRING[l]) ) );
	//		m_TimeToPlayJudgeSound[l] = JUDGE_SOUND_TIME( JUDGE_STRING[l] );

			for( p=0; p<NUM_PLAYERS; p++ )
			{
				if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
					continue;	// skip

				m_textJudgeNumbers[l][p].LoadFromNumbers( THEME->GetPathToN("ScreenEvaluation judge") );
				m_textJudgeNumbers[l][p].EnableShadow( false );
				m_textJudgeNumbers[l][p].SetDiffuse( PlayerToColor(p) );
				m_textJudgeNumbers[l][p].SetName( ssprintf("%sNumberP%d",JUDGE_STRING[l],p+1) );
				UtilSetXYAndOnCommand( m_textJudgeNumbers[l][p], "ScreenEvaluation" );
				this->AddChild( &m_textJudgeNumbers[l][p] );

				int iValue;
				switch( l )
				{
				case 0:	iValue = stageStats.iTapNoteScores[p][TNS_MARVELOUS];	break;
				case 1:	iValue = stageStats.iTapNoteScores[p][TNS_PERFECT];		break;
				case 2:	iValue = stageStats.iTapNoteScores[p][TNS_GREAT];		break;
				case 3:	iValue = stageStats.iTapNoteScores[p][TNS_GOOD];		break;
				case 4:	iValue = stageStats.iTapNoteScores[p][TNS_BOO];			break;
				case 5:	iValue = stageStats.iTapNoteScores[p][TNS_MISS];		break;
				case 6:	iValue = stageStats.iHoldNoteScores[p][HNS_OK];			break;
				case 7:	iValue = stageStats.iMaxCombo[p];						break;
				default:	iValue = 0;	ASSERT(0);
				}
				m_textJudgeNumbers[l][p].SetText( ssprintf("%4d",iValue) );
			}
		}
	}

	//
	// init score area
	//
	if( SHOW_SCORE_AREA )
	{
		m_sprScoreLabel.Load( THEME->GetPathToG("ScreenEvaluation score labels 1x2") );
		m_sprScoreLabel.SetState( 0 );
		m_sprScoreLabel.StopAnimating();
		m_sprScoreLabel.SetName( "ScoreLabel" );
		UtilSetXYAndOnCommand( m_sprScoreLabel, "ScreenEvaluation" );
		this->AddChild( &m_sprScoreLabel );

		for( p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
				continue;	// skip

			m_textScore[p].LoadFromNumbers( THEME->GetPathToN("ScreenEvaluation score") );
			m_textScore[p].EnableShadow( false );
			m_textScore[p].SetDiffuse( PlayerToColor(p) );
			m_textScore[p].SetName( ssprintf("ScoreNumberP%d",p+1) );
			UtilSetXYAndOnCommand( m_textScore[p], "ScreenEvaluation" );
			m_textScore[p].SetText( ssprintf("%*.0i", NUM_SCORE_DIGITS, stageStats.iScore[p]) );
			this->AddChild( &m_textScore[p] );
		}
	}

	//
	// init time area
	//
	if( SHOW_TIME_AREA )
	{
		m_sprTimeLabel.Load( THEME->GetPathToG("ScreenEvaluation score labels 1x2") );
		m_sprTimeLabel.SetState( 1 );
		m_sprTimeLabel.StopAnimating();
		m_sprTimeLabel.SetName( "TimeLabel" );
		UtilSetXYAndOnCommand( m_sprTimeLabel, "ScreenEvaluation" );
		this->AddChild( &m_sprTimeLabel );

		for( p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
				continue;	// skip

			m_textTime[p].LoadFromNumbers( THEME->GetPathToN("ScreenEvaluation score") );
			m_textTime[p].EnableShadow( false );
			m_textTime[p].SetDiffuse( PlayerToColor(p) );
			m_textTime[p].SetName( ssprintf("ScoreNumberP%d",p+1) );
			UtilSetXYAndOnCommand( m_textTime[p], "ScreenEvaluation" );
			m_textTime[p].SetText( SecondsToTime(stageStats.fAliveSeconds[p]) );
			this->AddChild( &m_textTime[p] );
		}
	}


	//
	// init extra area
	//
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( bNewRecord[p] )
		{
			m_sprNewRecord[p].Load( THEME->GetPathToG("ScreenEvaluation new record") );
			m_sprNewRecord[p].SetName( ssprintf("NewRecordP%d",p+1) );
			UtilSetXYAndOnCommand( m_sprNewRecord[p], "ScreenEvaluation" );
			this->AddChild( &m_sprNewRecord[p] );
		}
	}

	bool bOneHasNewRecord = false;
	for( p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsPlayerEnabled(p) && bNewRecord[p] )
			bOneHasNewRecord = true;
	
	if( m_bTryExtraStage )
	{
		m_sprTryExtraStage.Load( THEME->GetPathToG(GAMESTATE->IsExtraStage()?"ScreenEvaluation try extra2":"ScreenEvaluation try extra1") );
		m_sprTryExtraStage.SetName( "TryExtraStage" );
		UtilSetXYAndOnCommand( m_sprTryExtraStage, "ScreenEvaluation" );
		this->AddChild( &m_sprTryExtraStage );

		if( GAMESTATE->IsExtraStage() )
			SOUND->PlayOnce( THEME->GetPathToS("ScreenEvaluation try extra2") );
		else
			SOUND->PlayOnce( THEME->GetPathToS("ScreenEvaluation try extra1") );
	}
	else if( bOneHasNewRecord  &&  ANNOUNCER->HasSoundsFor("evaluation new record") )
	{
		SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation new record") );
	}
	else
	{	
		switch( m_Type )
		{
		case stage:
			switch( GAMESTATE->m_PlayMode )
			{
			case PLAY_MODE_HUMAN_BATTLE:
			case PLAY_MODE_CPU_BATTLE:
				{
					bool bWon = GAMESTATE->GetStageResult(GAMESTATE->m_MasterPlayerNumber) == RESULT_WIN;
					if( bWon )
						SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation win") );
					else
						SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation lose") );
				}
				break;
			default:
				SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation "+GradeToString(max_grade)) );
				break;
			}
			break;
		case course:
		case summary:
			SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation final "+GradeToString(max_grade)) );
			break;
		default:
			ASSERT(0);
		}
	}

	switch( max_grade )
	{
	case GRADE_AA:
	case GRADE_AAA:	
	case GRADE_AAAA:	
		this->PostScreenMessage( SM_PlayCheer, CHEER_DELAY_SECONDS );	
		break;
	}


	SOUND->PlayMusic( THEME->GetPathToS("ScreenEvaluation music") );
	m_fScreenCreateTime = RageTimer::GetTimeSinceStart();
}


void ScreenEvaluation::TweenOffScreen()
{
	int p;

	// large banner area
	UtilOffCommand( m_LargeBanner, "ScreenEvaluation" );
	UtilOffCommand( m_sprLargeBannerFrame, "ScreenEvaluation" );
	UtilOffCommand( m_sprStage, "ScreenEvaluation" );
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;
		UtilOffCommand( m_DifficultyIcon[p], "ScreenEvaluation" );
		UtilOffCommand( m_textPlayerOptions[p], "ScreenEvaluation" );
	}

	// small banner area
	for( int i=0; i<MAX_SONGS_TO_SHOW; i++ )
	{
		UtilOffCommand( m_SmallBanner[i], "ScreenEvaluation" );
		UtilOffCommand( m_sprSmallBannerFrame[i], "ScreenEvaluation" );
	}

	// grade area
	for( p=0; p<NUM_PLAYERS; p++ ) 
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;
		UtilOffCommand( m_sprGradeFrame[p], "ScreenEvaluation" );
		UtilOffCommand( m_Grades[p], "ScreenEvaluation" );
	}

	// points area
	for( p=0; p<NUM_PLAYERS; p++ ) 
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;
		UtilOffCommand( m_sprPercentFrame[p], "ScreenEvaluation" );
		UtilOffCommand( m_textPercentWhole[p], "ScreenEvaluation" );
		UtilOffCommand( m_textPercentRemainder[p], "ScreenEvaluation" );
		UtilOffCommand( m_textDancePoints[p], "ScreenEvaluation" );
	}

	// bonus area
	for( p=0; p<NUM_PLAYERS; p++ ) 
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;
		UtilOffCommand( m_sprBonusFrame[p], "ScreenEvaluation" );
		for( int r=0; r<NUM_RADAR_CATEGORIES; r++ )	// foreach line
		{
			UtilOffCommand( m_sprPossibleBar[p][r], "ScreenEvaluation" );
			UtilOffCommand( m_sprActualBar[p][r], "ScreenEvaluation" );
		}
	}

	// survived area
	for( p=0; p<NUM_PLAYERS; p++ ) 
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;
		UtilOffCommand( m_sprSurvivedFrame[p], "ScreenEvaluation" );
		UtilOffCommand( m_textSurvivedNumber[p], "ScreenEvaluation" );
	}
		
	// win area
	for( p=0; p<NUM_PLAYERS; p++ ) 
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;
		UtilOffCommand( m_sprWinFrame[p], "ScreenEvaluation" );
		UtilOffCommand( m_sprWin[p], "ScreenEvaluation" );
	}
		
	// judgement area
	int l;
	for( l=0; l<NUM_JUDGE_LINES; l++ ) 
		UtilOffCommand( m_sprJudgeLabels[l], "ScreenEvaluation" );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;
		for( l=0; l<NUM_JUDGE_LINES; l++ ) 
			UtilOffCommand( m_textJudgeNumbers[l][p], "ScreenEvaluation" );
	}

	// score area
	UtilOffCommand( m_sprScoreLabel, "ScreenEvaluation" );
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;
		UtilOffCommand( m_textScore[p], "ScreenEvaluation" );
	}

	// time area
	UtilOffCommand( m_sprTimeLabel, "ScreenEvaluation" );
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;
		UtilOffCommand( m_textTime[p], "ScreenEvaluation" );
	}

	// extra area
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;
		UtilOffCommand( m_sprNewRecord[p], "ScreenEvaluation" );
	}
	UtilOffCommand( m_sprTryExtraStage, "ScreenEvaluation" );
}

void ScreenEvaluation::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );

/*	for( int l=0; l<NUM_JUDGE_LINES; l++ ) 
	{
		if(!SHOW_JUDGMENT(l))
			continue;

		if(m_TimeToPlayJudgeSound[l] == -1)
			continue;

		if(RageTimer::GetTimeSinceStart() < m_fScreenCreateTime + m_TimeToPlayJudgeSound[l])
			continue;

		if(SOUND_ON_FULL_ALPHA)
		{
			if( m_sprJudgeLabels[l].GetDiffuse().a != 1.0f &&
				m_textJudgeNumbers[l][PLAYER_1].GetDiffuse().a != 1.0f &&
				m_textJudgeNumbers[l][PLAYER_2].GetDiffuse().a != 1.0f )
				continue;
		}

		m_soundJudgeSound[l].Play();
		m_TimeToPlayJudgeSound[l] = -1;
	}*/
}

void ScreenEvaluation::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenEvaluation::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
//	LOG->Trace( "ScreenEvaluation::Input()" );

	if( m_Menu.IsTransitioning() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenEvaluation::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_MenuTimer:
		MenuStart( PLAYER_INVALID );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		break;
	case SM_GoToSelectCourse:
		SCREENMAN->SetNewScreen( "ScreenSelectCourse" );
		break;
	case SM_GoToEndScreen:
		SCREENMAN->SetNewScreen( END_SCREEN );
		break;
	case SM_GoToEvaluationSummary:
		SCREENMAN->SetNewScreen( "ScreenEvaluationSummary" );
		break;
	case SM_PlayCheer:
		SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation cheer") );
		break;
	}
}

void ScreenEvaluation::MenuLeft( PlayerNumber pn )
{
	// What is the purpose of this?  I keep my feet on the pads and 
	// was wondering why the grades weren't spinning. -Chris
	// To be able to see the grade without having to wait for it to
	// stop spinning.  (I was hitting left repeatedly and wondering
	// why it kept spinning ...)
	//m_Grades[pn].SettleQuickly();
}

void ScreenEvaluation::MenuRight( PlayerNumber pn )
{
	// What is the purpose of this?  I keep my feet on the pads and 
	// was wondering why the grades weren't spinning. -Chris
	//m_Grades[pn].SettleQuickly();
}

void ScreenEvaluation::MenuBack( PlayerNumber pn )
{
	MenuStart( pn );
}

void ScreenEvaluation::MenuStart( PlayerNumber pn )
{
	TweenOffScreen();

	for( int p=0; p<NUM_PLAYERS; p++ )
		m_Grades[p].SettleImmediately();

	GAMESTATE->m_iRoundSeed = rand();

	if( PREFSMAN->m_bEventMode )
	{
		m_Menu.StartTransitioning( SM_GoToNextScreen );
	}
	else	// not event mode
	{
		switch( m_Type )
		{
		case stage:
			if( m_bTryExtraStage )
			{
				m_Menu.StartTransitioning( SM_GoToNextScreen );
			}
			else if( GAMESTATE->IsFinalStage() || GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
			{
				/* Tween the screen out, but leave the MenuElements where they are.
					* Play the "swoosh" sound manually (would normally be played by the ME
					* tween out). */
				TweenOffScreen();
				m_Menu.StartTransitioning( SM_GoToEvaluationSummary );
			}
			else
			{
				m_Menu.StartTransitioning( SM_GoToNextScreen );
			}
			break;
		case summary:
			m_Menu.StartTransitioning( SM_GoToEndScreen );
			break;
		case course:
			m_Menu.StartTransitioning( SM_GoToEndScreen );
			break;
		}
	}

	switch( m_Type )
	{
	case stage:
		// Increment the stage counter.
		int iNumStagesOfLastSong;
		iNumStagesOfLastSong = SongManager::GetNumStagesForSong( GAMESTATE->m_pCurSong );
		GAMESTATE->m_iCurrentStageIndex += iNumStagesOfLastSong;

		// add current stage stats to accumulated total only if this song was passed
		{
			bool bOnePassed = false;
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsHumanPlayer(p) )
					bOnePassed |= !GAMESTATE->m_CurStageStats.bFailed[p];

			if( bOnePassed )
				GAMESTATE->m_vPassedStageStats.push_back( GAMESTATE->m_CurStageStats );	// Save this stage's stats
		}
		break;
	}
}

