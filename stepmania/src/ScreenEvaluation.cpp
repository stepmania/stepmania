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
#include "LightsManager.h"
#include "ProfileManager.h"
#include "song.h"
#include "StageStats.h"

const int NUM_SCORE_DIGITS	=	9;


// metrics that are common to all ScreenEvaluation classes
#define BANNER_WIDTH						THEME->GetMetricF(m_sName,"BannerWidth")
#define BANNER_HEIGHT						THEME->GetMetricF(m_sName,"BannerHeight")
const char* JUDGE_STRING[NUM_JUDGE_LINES] =
{
	"Marvelous", "Perfect", "Great", "Good", "Boo", "Miss", "OK", "MaxCombo", "TotalError"
};
const char* STATS_STRING[NUM_STATS_LINES] =
{
	"Jumps", "Holds", "Mines", "Hands"
};

#define SPIN_GRADES							THEME->GetMetricB(m_sName,"SpinGrades")
#define CHEER_DELAY_SECONDS					THEME->GetMetricF(m_sName,"CheerDelaySeconds")
#define BAR_ACTUAL_MAX_COMMAND				THEME->GetMetric (m_sName,"BarActualMaxCommand")

// metrics that are specific to classes derived from ScreenEvaluation
#define FAILED_SCREEN						THEME->GetMetric (m_sName, "FailedScreen")
#define NEXT_SCREEN							THEME->GetMetric (m_sName,"NextScreen")
#define END_SCREEN							THEME->GetMetric (m_sName,"EndScreen")
#define SHOW_BANNER_AREA					THEME->GetMetricB(m_sName,"ShowBannerArea")
#define SHOW_GRADE_AREA						THEME->GetMetricB(m_sName,"ShowGradeArea")
#define SHOW_POINTS_AREA					THEME->GetMetricB(m_sName,"ShowPointsArea")
#define SHOW_BONUS_AREA						THEME->GetMetricB(m_sName,"ShowBonusArea")
#define SHOW_SURVIVED_AREA					THEME->GetMetricB(m_sName,"ShowSurvivedArea")
#define SHOW_WIN_AREA						THEME->GetMetricB(m_sName,"ShowWinArea")
#define SHOW_JUDGMENT( l )					THEME->GetMetricB(m_sName,ssprintf("Show%s",JUDGE_STRING[l]))
#define SHOW_STAT( s )						THEME->GetMetricB(m_sName,ssprintf("Show%s",STATS_STRING[l]))
#define SHOW_SCORE_AREA						THEME->GetMetricB(m_sName,"ShowScoreArea")
#define SHOW_TOTAL_SCORE_AREA				THEME->GetMetricB(m_sName,"ShowTotalScoreArea")
#define SHOW_TIME_AREA						THEME->GetMetricB(m_sName,"ShowTimeArea")
#define SHOW_GRAPH_AREA						THEME->GetMetricB(m_sName,"ShowGraphArea")
#define SHOW_COMBO_AREA						THEME->GetMetricB(m_sName,"ShowComboArea")
#define SHOW_FULL_COMBO						THEME->GetMetricB(m_sName,"ShowFullCombo")
#define GRAPH_START_HEIGHT					THEME->GetMetricF(m_sName,"GraphStartHeight")
#define TYPE								THEME->GetMetric (m_sName,"Type")
#define PASSED_SOUND_TIME					THEME->GetMetricF(m_sName,"PassedSoundTime")
#define FAILED_SOUND_TIME					THEME->GetMetricF(m_sName,"FailedSoundTime")
#define NUM_SEQUENCE_SOUNDS					THEME->GetMetricI(m_sName,"NumSequenceSounds")
#define SOUNDSEQ_TIME( i )					THEME->GetMetricF(m_sName,ssprintf("SoundSeqTime%d", i+1))
#define SOUNDSEQ_NAME( i )					THEME->GetMetric (m_sName,ssprintf("SoundSeqName%d", i+1))

static const int NUM_SHOWN_RADAR_CATEGORIES = 5;

const ScreenMessage SM_GoToSelectCourse			=	ScreenMessage(SM_User+3);
//const ScreenMessage SM_GoToEvaluationSummary	=	ScreenMessage(SM_User+4);
const ScreenMessage SM_GoToEndScreen			=	ScreenMessage(SM_User+5);
const ScreenMessage SM_PlayCheer				=	ScreenMessage(SM_User+6);


ScreenEvaluation::ScreenEvaluation( CString sClassName ) : Screen(sClassName)
{
	//
	// debugging
	//
	
	if( PREFSMAN->m_bScreenTestMode )
	{
		GAMESTATE->m_PlayMode = PLAY_MODE_RAVE;
		GAMESTATE->m_CurStyle = STYLE_DANCE_VERSUS;
		GAMESTATE->m_bSideIsJoined[PLAYER_1] = true;
		GAMESTATE->m_bSideIsJoined[PLAYER_2] = true;
		GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
		GAMESTATE->m_pCurSong = SONGMAN->GetAllSongs()[0];
//		GAMESTATE->m_pCurCourse = SONGMAN->m_pCourses[0];
		GAMESTATE->m_pCurNotes[PLAYER_1] = GAMESTATE->m_pCurSong->m_apNotes[0];
		GAMESTATE->m_pCurNotes[PLAYER_2] = GAMESTATE->m_pCurSong->m_apNotes[0];
		GAMESTATE->m_PlayerOptions[PLAYER_1].m_fScrollSpeed = 2;
		GAMESTATE->m_PlayerOptions[PLAYER_2].m_fScrollSpeed = 2;
		GAMESTATE->m_iCurrentStageIndex = 2;

		for( float f = 0; f < 100.0f; f += 1.0f )
		{
			float fP1 = fmodf(f/100*4+.3f,1);
			g_CurStageStats.SetLifeRecord( PLAYER_1, fP1, f );
			g_CurStageStats.SetLifeRecord( PLAYER_2, 1-fP1, f );
		}
	
		g_CurStageStats.iCurCombo[PLAYER_1] = 0;
		g_CurStageStats.UpdateComboList( PLAYER_1, 0, false );
		g_CurStageStats.iCurCombo[PLAYER_1] = 1;
		g_CurStageStats.UpdateComboList( PLAYER_1, 1, false );
		g_CurStageStats.iCurCombo[PLAYER_1] = 50;
		g_CurStageStats.UpdateComboList( PLAYER_1, 25, false );
		g_CurStageStats.iCurCombo[PLAYER_1] = 250;
		g_CurStageStats.UpdateComboList( PLAYER_1, 100, false );
		g_vPlayedStageStats.clear();
	}

	LOG->Trace( "ScreenEvaluation::ScreenEvaluation()" );

	LIGHTSMAN->SetLightMode( LIGHTMODE_MENU );

	m_bFailed = false; // the evaluation is not showing failed results by default

	m_sName = sClassName;
	if( !TYPE.CompareNoCase("stage") )
		m_Type = stage;
	else if( !TYPE.CompareNoCase("summary") )
		m_Type = summary;
	else if( !TYPE.CompareNoCase("course") )
		m_Type = course;
	else
		RageException::Throw("Unknown evaluation type \"%s\"", TYPE.c_str() );
		
	int p;
	

	//
	// Figure out which statistics and songs we're going to display
	//
	StageStats stageStats;

	if(stageStats.AllFailed() ) // if everyone failed
		m_bFailed = true; // flag it for this screen

	vector<Song*> vSongsToShow;
	switch( m_Type )
	{
	case summary:
		GAMESTATE->GetFinalEvalStatsAndSongs( stageStats, vSongsToShow );
		break;
	case stage:
	case course:
		stageStats = g_CurStageStats;
		break;
	default:
		ASSERT(0);
	}

	LOG->Trace( "total error: %i, %i", stageStats.iTotalError[0], stageStats.iTotalError[1] );

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
			grade[p] = GRADE_FAILED;

		if (PREFSMAN->m_iScoringType == PrefsManager::SCORING_5TH)
		{
			int ScoreBonuses[9] = {0, 0, 100, 1000, 10000, 100000, 1000000, 10000000, 10000000};
			g_CurStageStats.iBonus[p] += ScoreBonuses[(int)grade[p] ];
			stageStats.iBonus[p] += ScoreBonuses[(int)grade[p] ];
		}
	}

	Grade best_grade = GRADE_NO_DATA;
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		best_grade = min( best_grade, grade[p] ); 

		// if its extra stage, update # passed stages
		if (PREFSMAN->m_bUseUnlockSystem &&
			(GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() ) &&
			grade[p] > GRADE_FAILED &&
			m_Type != summary)
					UNLOCKSYS->UnlockClearExtraStage();
	}


	if ( PREFSMAN->m_bUseUnlockSystem )
	{
		for( p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
				continue;	// skip

			switch( m_Type )
			{
			case stage:
				// update unlock data
				UNLOCKSYS->UnlockClearStage();
				UNLOCKSYS->UnlockAddAP( grade[p] );
				UNLOCKSYS->UnlockAddSP( grade[p] );
				UNLOCKSYS->UnlockAddDP( (float)stageStats.iActualDancePoints[p] );
				break;

			case course:
				UNLOCKSYS->UnlockAddDP( (float) stageStats.iActualDancePoints[p] );
				UNLOCKSYS->UnlockAddAP( (float) stageStats.iSongsPassed[p] );
				UNLOCKSYS->UnlockAddSP( (float) stageStats.iSongsPassed[p] );
				break;
			}
		}
	}

	//
	// update persistent statistics
	//
	int iPersonalHighScoreIndex[NUM_PLAYERS] = { -1, -1 };
	int iMachineHighScoreIndex[NUM_PLAYERS] = { -1, -1 };
	RankingCategory rc[NUM_PLAYERS] = { NUM_RANKING_CATEGORIES, NUM_RANKING_CATEGORIES };
	CommitScores( stageStats, iPersonalHighScoreIndex, iMachineHighScoreIndex, rc );

	m_bTryExtraStage = 
		GAMESTATE->HasEarnedExtraStage()  && 
		m_Type==stage;
 


	m_Menu.Load( m_sName );
	this->AddChild( &m_Menu );

	//
	// load pass/fail sound
	//
	if(m_Type==stage)
	{
		int snd=0;
		for(snd=0;snd<NUM_SEQUENCE_SOUNDS;snd++) // grab in any sound sequence the user may want to throw onto this screen
		{
			EvalSoundSequence temp;
			temp.fTime = SOUNDSEQ_TIME(snd);
			temp.sSound.Load(THEME->GetPathToS(SOUNDSEQ_NAME(snd),false));
			m_SoundSequences.push_back(temp);
		}
	}
	m_bPassFailTriggered = false; // the sound hasn't been triggered yet
	if(m_bFailed && m_Type==stage)
	{
		m_bgFailedBack.LoadFromAniDir( THEME->GetPathToB("ScreenEvaluationStage Failed Background") );
		m_bgFailedOverlay.LoadFromAniDir( THEME->GetPathToB("ScreenEvaluationStage Failed Overlay") );
		m_sndPassFail.Load(THEME->GetPathToS("ScreenEvaluationStage Failed",false));
	//	m_sndPassFail.Play();
	}
	else if( m_Type == stage )
	{
		// the themer can use the regular background for passed background
		m_bgPassedOverlay.LoadFromAniDir( THEME->GetPathToB("ScreenEvaluationStage Passed Overlay") );
		m_sndPassFail.Load(THEME->GetPathToS("ScreenEvaluationStage Passed",false));
	//	m_sndPassFail.Play();
	}

	
	//
	// load other sounds
	//
	m_soundStart.Load( THEME->GetPathToS("ScreenEvaluation start") );

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
				SET_XY_AND_ON_COMMAND( m_LargeBanner );
				this->AddChild( &m_LargeBanner );

				m_sprLargeBannerFrame.Load( THEME->GetPathToG("ScreenEvaluation banner frame") );
				m_sprLargeBannerFrame.SetName( "LargeBannerFrame" );
				SET_XY_AND_ON_COMMAND( m_sprLargeBannerFrame );
				this->AddChild( &m_sprLargeBannerFrame );

				m_sprStage.Load( THEME->GetPathToG("ScreenEvaluation stage "+GAMESTATE->GetStageText()) );
				m_sprStage.SetName( "Stage" );
				SET_XY_AND_ON_COMMAND( m_sprStage );
				this->AddChild( &m_sprStage );

				for( int p=0; p<NUM_PLAYERS; p++ )
				{
					if( !GAMESTATE->IsPlayerEnabled(p) )
						continue;	// skip

#ifdef _XBOX
					//shorten filenames for FATX
					m_DifficultyIcon[p].Load( THEME->GetPathToG("ScreenEvaluation diff icons 1x5") );
#else
					m_DifficultyIcon[p].Load( THEME->GetPathToG("ScreenEvaluation difficulty icons 1x5") );
#endif

					m_DifficultyIcon[p].SetFromNotes( (PlayerNumber)p, GAMESTATE->m_pCurNotes[p] );
					m_DifficultyIcon[p].SetName( ssprintf("DifficultyIconP%d",p+1) );
					SET_XY_AND_ON_COMMAND( m_DifficultyIcon[p] );
					this->AddChild( &m_DifficultyIcon[p] );
					
					m_textPlayerOptions[p].LoadFromFont( THEME->GetPathToF("Common normal") );
					CString sPO = GAMESTATE->m_PlayerOptions[p].GetString();
					sPO.Replace( ", ", "\n" );
					m_textPlayerOptions[p].SetName( ssprintf("PlayerOptionsP%d",p+1) );
					SET_XY_AND_ON_COMMAND( m_textPlayerOptions[p] );
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
					SET_XY_AND_ON_COMMAND( m_SmallBanner[i] );
					this->AddChild( &m_SmallBanner[i] );

					m_sprSmallBannerFrame[i].Load( THEME->GetPathToG("ScreenEvaluation banner frame") );
					m_sprSmallBannerFrame[i].SetName( ssprintf("SmallBanner%d",i+1) );
					SET_XY_AND_ON_COMMAND( m_sprSmallBannerFrame[i] );
					this->AddChild( &m_sprSmallBannerFrame[i] );
				}
			}
			break;
		case course:
			{
				m_LargeBanner.LoadFromCourse( GAMESTATE->m_pCurCourse );
				m_LargeBanner.ScaleToClipped( BANNER_WIDTH, BANNER_HEIGHT );
				m_LargeBanner.SetName( "LargeBanner" );
				SET_XY_AND_ON_COMMAND( m_LargeBanner );
				this->AddChild( &m_LargeBanner );

				m_sprLargeBannerFrame.Load( THEME->GetPathToG("ScreenEvaluation banner frame") );
				m_sprLargeBannerFrame.SetName( "LargeBannerFrame" );
				SET_XY_AND_ON_COMMAND( m_sprLargeBannerFrame );
				this->AddChild( &m_sprLargeBannerFrame );
			}
			break;
		default:
			ASSERT(0);
		}
	}

	//
	// init graph area
	//
	if( SHOW_GRAPH_AREA )
	{
		for( p=0; p<NUM_PLAYERS; p++ ) 
		{
			if( !GAMESTATE->IsPlayerEnabled(p) )
				continue;	// skip

			m_sprGraphFrame[p].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation graph frame p%d",p+1)) );
			m_sprGraphFrame[p].SetName( ssprintf("GraphFrameP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_sprGraphFrame[p] );
			this->AddChild( &m_sprGraphFrame[p] );

			m_Graph[p].SetName( ssprintf("GraphP%i",p+1) );
			m_Graph[p].Load( THEME->GetPathToG(ssprintf("%s graph p%i",m_sName.c_str(), p+1)), GRAPH_START_HEIGHT );
			m_Graph[p].LoadFromStageStats( stageStats, (PlayerNumber)p );
			SET_XY_AND_ON_COMMAND( m_Graph[p] );
			this->AddChild( &m_Graph[p] );
		}
	}

	if( SHOW_COMBO_AREA )
	{
		for( p=0; p<NUM_PLAYERS; p++ ) 
		{
			if( !GAMESTATE->IsPlayerEnabled(p) )
				continue;	// skip

			m_Combo[p].SetName( m_sName, ssprintf("ComboP%i",p+1) );
			m_Combo[p].Load( ssprintf("ScreenEvaluationStage combo p%i", p+1), stageStats, (PlayerNumber)p );
			SET_XY_AND_ON_COMMAND( m_Combo[p] );

			this->AddChild( &m_Combo[p] );
		}
	}

	//
	// init grade area
	//
	if( SHOW_GRADE_AREA )
	{
		// ugly: Touch all possible grade files to make sure their all present
		for( int g=GRADE_TIER_1; g<NUM_GRADES; g++ )
			THEME->GetPathToG( "ScreenEvaluation grade "+ GradeToString((Grade)g) );

		for( p=0; p<NUM_PLAYERS; p++ ) 
		{
			if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
				continue;	// skip

			m_sprGradeFrame[p].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation grade frame p%d",p+1)) );
			m_sprGradeFrame[p].SetName( ssprintf("GradeFrameP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_sprGradeFrame[p] );
			this->AddChild( &m_sprGradeFrame[p] );

			m_Grades[p].Load( THEME->GetPathToG("ScreenEvaluation grades") );
			m_Grades[p].SetGrade( (PlayerNumber)p, grade[p] );
			m_Grades[p].SetName( ssprintf("GradeP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_Grades[p] );
			if( SPIN_GRADES )
				m_Grades[p].Spin();
			this->AddChild( &m_Grades[p] );

			m_sprGrade[p].Load( THEME->GetPathToG( "ScreenEvaluation grade "+ GradeToString(grade[p]) ) );
			m_sprGrade[p]->SetName( ssprintf("GradeP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_sprGrade[p] );
			this->AddChild( m_sprGrade[p] );
		}
	}

	//
	// init points area
	//
	if( SHOW_POINTS_AREA )
	{
		for( p=0; p<NUM_PLAYERS; p++ ) 
		{
			if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
				continue;	// skip

			m_sprPercentFrame[p].SetName( ssprintf("PercentFrameP%d",p+1) );
			m_sprPercentFrame[p].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation percent frame p%d",p+1)) );
			SET_XY_AND_ON_COMMAND( m_sprPercentFrame[p] );
			this->AddChild( &m_sprPercentFrame[p] );

			/* Use "ScreenEvaluation Percent" for the [metric set], but position and
			 * tween it with "PercentP1X", etc. */
			m_Percent[p].SetName( "ScreenEvaluation Percent" );
			m_Percent[p].Load( (PlayerNumber) p );
			m_Percent[p].SetXY( THEME->GetMetricF(m_sName, ssprintf("PercentP%dX",p+1)),
				THEME->GetMetricF(m_sName,ssprintf("PercentP%dY",p+1)) );
			m_Percent[p].Command( THEME->GetMetric(m_sName,ssprintf("PercentP%dOnCommand",p+1)) );
			this->AddChild( &m_Percent[p] );
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
			SET_XY_AND_ON_COMMAND( m_sprBonusFrame[p] );
			this->AddChild( &m_sprBonusFrame[p] );

			for( int r=0; r<NUM_SHOWN_RADAR_CATEGORIES; r++ )	// foreach line
			{
				m_sprPossibleBar[p][r].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation bar possible p%d",p+1)) );
				m_sprPossibleBar[p][r].SetWidth( m_sprPossibleBar[p][r].GetUnzoomedWidth() * stageStats.fRadarPossible[p][r] );
				m_sprPossibleBar[p][r].SetName( ssprintf("BarPossible%dP%d",r+1,p+1) );
				SET_XY_AND_ON_COMMAND( m_sprPossibleBar[p][r] );
				this->AddChild( &m_sprPossibleBar[p][r] );

				m_sprActualBar[p][r].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation bar actual p%d",p+1)) );
				// should be out of the possible bar, not actual (whatever value that is at)
				m_sprActualBar[p][r].SetWidth( m_sprPossibleBar[p][r].GetUnzoomedWidth() * stageStats.fRadarActual[p][r] );
				
				float value = (float)100 * m_sprActualBar[p][r].GetUnzoomedWidth() / m_sprPossibleBar[p][r].GetUnzoomedWidth();
				LOG->Trace("Radar bar %d of 5 - %f percent", r,  value);
				
				m_sprActualBar[p][r].SetName( ssprintf("BarActual%dP%d",r+1,p+1) );
				SET_XY_AND_ON_COMMAND( m_sprActualBar[p][r] );
				
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
			SET_XY_AND_ON_COMMAND( m_sprSurvivedFrame[p] );
			this->AddChild( &m_sprSurvivedFrame[p] );

			m_textSurvivedNumber[p].LoadFromNumbers( THEME->GetPathToN("ScreenEvaluation stage") );
			m_textSurvivedNumber[p].EnableShadow( false );
			// curewater: edited the "# stages cleared" text so it deducts one if you failed.
			// Should be accurate, but I'm not sure if its "standard" that (bool)true = 1.  (assumption)
			m_textSurvivedNumber[p].SetText( ssprintf("%02d", stageStats.iSongsPlayed[p] - (int)stageStats.bFailed[p]) );
			m_textSurvivedNumber[p].SetName( ssprintf("SurvivedNumberP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_textSurvivedNumber[p] );
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
			SET_XY_AND_ON_COMMAND( m_sprWinFrame[p] );
			this->AddChild( &m_sprWinFrame[p] );

			m_sprWin[p].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation win p%d 1x3",p+1)) );
			m_sprWin[p].StopAnimating();
			int iFrame = GAMESTATE->GetStageResult( (PlayerNumber)p );
			m_sprWin[p].SetState( iFrame );
			m_sprWin[p].SetName( ssprintf("WinP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_sprWin[p] );
			this->AddChild( &m_sprWin[p] );
		}
	}
	
	//
	// init judgment area
	//
	int l;
	for( l=0; l<NUM_JUDGE_LINES; l++ ) 
	{
		if( l == 0  && !GAMESTATE->ShowMarvelous() )
			continue;	// skip

		if( SHOW_JUDGMENT(l) )
		{
			m_sprJudgeLabels[l].Load( THEME->GetPathToG( "ScreenEvaluation judge labels" ) );
			m_sprJudgeLabels[l].StopAnimating();
			m_sprJudgeLabels[l].SetState( l );
			m_sprJudgeLabels[l].SetName( ssprintf("%sLabel",JUDGE_STRING[l]) );
			SET_XY_AND_ON_COMMAND( m_sprJudgeLabels[l] );
			this->AddChild( &m_sprJudgeLabels[l] );

			for( p=0; p<NUM_PLAYERS; p++ )
			{
				if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
					continue;	// skip

				m_textJudgeNumbers[l][p].LoadFromNumbers( THEME->GetPathToN("ScreenEvaluation judge") );
				m_textJudgeNumbers[l][p].EnableShadow( false );
				m_textJudgeNumbers[l][p].SetDiffuse( PlayerToColor(p) );
				m_textJudgeNumbers[l][p].SetName( ssprintf("%sNumberP%d",JUDGE_STRING[l],p+1) );
				SET_XY_AND_ON_COMMAND( m_textJudgeNumbers[l][p] );
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
				case 8:	iValue = stageStats.iTotalError[p];						break;
				default:	iValue = 0;	ASSERT(0);
				}
				m_textJudgeNumbers[l][p].SetText( ssprintf("%4d",iValue) );
			}
		}
	}

	for( l=0; l<NUM_STATS_LINES; l++ ) 
	{
		if( !SHOW_STAT(l) )
			continue;

		m_sprStatsLabel[l].Load( THEME->GetPathToG( ssprintf("ScreenEvaluation label %s", STATS_STRING[l]) ) );
		m_sprStatsLabel[l]->StopAnimating();
		m_sprStatsLabel[l]->SetName( ssprintf("%sLabel",STATS_STRING[l]) );
		SET_XY_AND_ON_COMMAND( m_sprStatsLabel[l] );
		this->AddChild( m_sprStatsLabel[l] );

		for( p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
				continue;	// skip

			m_textStatsText[l][p].LoadFromFont( THEME->GetPathToF("ScreenEvaluation stats") );
			m_textStatsText[l][p].SetDiffuse( PlayerToColor(p) );
			m_textStatsText[l][p].SetName( ssprintf("%sTextP%d",STATS_STRING[l],p+1) );
			SET_XY_AND_ON_COMMAND( m_textStatsText[l][p] );
			this->AddChild( &m_textStatsText[l][p] );

			const int indeces[NUM_STATS_LINES] =
			{
				RADAR_NUM_JUMPS, RADAR_NUM_HOLDS, RADAR_NUM_MINES, RADAR_NUM_HANDS
			};
			const int ind = indeces[l];
			const int iActual = (int) roundf(stageStats.fRadarActual[p][ind]);
			const int iPossible = (int) roundf(stageStats.fRadarPossible[p][ind]);

			m_textStatsText[l][p].SetText( ssprintf("%i/%i",iActual, iPossible) );
		}
	}

	//
	// init score area
	//
	if( SHOW_SCORE_AREA )
	{
		m_sprScoreLabel.Load( THEME->GetPathToG("ScreenEvaluation score label") );
		m_sprScoreLabel.SetState( 0 );
		m_sprScoreLabel.StopAnimating();
		m_sprScoreLabel.SetName( "ScoreLabel" );
		SET_XY_AND_ON_COMMAND( m_sprScoreLabel );
		this->AddChild( &m_sprScoreLabel );

		for( p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
				continue;	// skip

			m_textScore[p].LoadFromNumbers( THEME->GetPathToN("ScreenEvaluation score") );
			m_textScore[p].EnableShadow( false );
			m_textScore[p].SetDiffuse( PlayerToColor(p) );
			m_textScore[p].SetName( ssprintf("ScoreNumberP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_textScore[p] );
			m_textScore[p].SetText( ssprintf("%*.0i", NUM_SCORE_DIGITS, stageStats.iScore[p]) );
			this->AddChild( &m_textScore[p] );
		}
	}

	if( SHOW_TOTAL_SCORE_AREA )
	{
		m_sprTotalScoreLabel.Load( THEME->GetPathToG("ScreenEvaluation totalscore label") );
		m_sprTotalScoreLabel.SetState( 0 );
		m_sprTotalScoreLabel.StopAnimating();
		m_sprTotalScoreLabel.SetName( "TotalScoreLabel" );
		SET_XY_AND_ON_COMMAND( m_sprTotalScoreLabel );
		this->AddChild( &m_sprTotalScoreLabel );

		for( p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
				continue;	// skip

			int iTotalScore=0;
			for( unsigned i=0; i<g_vPlayedStageStats.size(); i++ )
				iTotalScore += g_vPlayedStageStats[i].iScore[p];

			iTotalScore += stageStats.iScore[p];

			m_textTotalScore[p].LoadFromNumbers( THEME->GetPathToN("ScreenEvaluation totalscore") );
			m_textTotalScore[p].EnableShadow( false );
			m_textTotalScore[p].SetDiffuse( PlayerToColor(p) );
			m_textTotalScore[p].SetName( ssprintf("TotalScoreNumberP%d",p+1) );
			m_textTotalScore[p].SetText( ssprintf("%*.0i", NUM_SCORE_DIGITS+2, iTotalScore) );
			SET_XY_AND_ON_COMMAND( m_textTotalScore[p] );

			this->AddChild( &m_textTotalScore[p] );
		}
	}

	//
	// init time area
	//
	if( SHOW_TIME_AREA )
	{
		m_sprTimeLabel.Load( THEME->GetPathToG("ScreenEvaluation time label") );
		m_sprTimeLabel.SetState( 1 );
		m_sprTimeLabel.StopAnimating();
		m_sprTimeLabel.SetName( "TimeLabel" );
		SET_XY_AND_ON_COMMAND( m_sprTimeLabel );
		this->AddChild( &m_sprTimeLabel );

		for( p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
				continue;	// skip

			m_textTime[p].LoadFromNumbers( THEME->GetPathToN("ScreenEvaluation time") );
			m_textTime[p].EnableShadow( false );
			m_textTime[p].SetDiffuse( PlayerToColor(p) );
			m_textTime[p].SetName( ssprintf("TimeNumberP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_textTime[p] );
			m_textTime[p].SetText( SecondsToTime(stageStats.fAliveSeconds[p]) );
			this->AddChild( &m_textTime[p] );
		}
	}


	//
	// init extra area
	//
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( iMachineHighScoreIndex[p] != -1 )
		{
			m_sprMachineRecord[p].Load( THEME->GetPathToG("ScreenEvaluation machine record") );
			m_sprMachineRecord[p].SetName( ssprintf("MachineRecordP%d",p+1) );
			m_sprMachineRecord[p].SetState( iMachineHighScoreIndex[p] );
			m_sprMachineRecord[p].StopAnimating();
			SET_XY_AND_ON_COMMAND( m_sprMachineRecord[p] );
			this->AddChild( &m_sprMachineRecord[p] );
		}
		if( iPersonalHighScoreIndex[p] != -1 )
		{
			m_sprPersonalRecord[p].Load( THEME->GetPathToG("ScreenEvaluation personal record") );
			m_sprPersonalRecord[p].SetName( ssprintf("PersonalRecordP%d",p+1) );
			m_sprPersonalRecord[p].SetState( iPersonalHighScoreIndex[p] );
			m_sprPersonalRecord[p].StopAnimating();
			SET_XY_AND_ON_COMMAND( m_sprPersonalRecord[p] );
			this->AddChild( &m_sprPersonalRecord[p] );
		}

		if( SHOW_FULL_COMBO && stageStats.FullCombo( (PlayerNumber) p ) )
		{
			LOG->Trace( "Full combo for P%i", p+1 );
			m_FullCombo[p].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation full combo P%i",p+1)) );
			m_FullCombo[p]->SetName( ssprintf("FullComboP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_FullCombo[p] );
			this->AddChild( m_FullCombo[p] );
		}
	}

	bool bOneHasNewTopRecord = false;
	for( p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsPlayerEnabled(p) && (iMachineHighScoreIndex[p] != -1 || iPersonalHighScoreIndex[p] != -1) )
			bOneHasNewTopRecord = true;
	
	if( PREFSMAN->m_bAllowExtraStage && m_bTryExtraStage )
	{
		m_sprTryExtraStage.Load( THEME->GetPathToG(GAMESTATE->IsExtraStage()?"ScreenEvaluation try extra2":"ScreenEvaluation try extra1") );
		m_sprTryExtraStage.SetName( "TryExtraStage" );
		SET_XY_AND_ON_COMMAND( m_sprTryExtraStage );
		this->AddChild( &m_sprTryExtraStage );

		if( GAMESTATE->IsExtraStage() )
			SOUND->PlayOnce( THEME->GetPathToS("ScreenEvaluation try extra2") );
		else
			SOUND->PlayOnce( THEME->GetPathToS("ScreenEvaluation try extra1") );
	}
	else if( bOneHasNewTopRecord  &&  ANNOUNCER->HasSoundsFor("evaluation new record") )
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
			case PLAY_MODE_BATTLE:
				{
					bool bWon = GAMESTATE->GetStageResult(GAMESTATE->m_MasterPlayerNumber) == RESULT_WIN;
					if( bWon )
						SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation win") );
					else
						SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation lose") );
				}
				break;
			default:
				SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation "+GradeToString(best_grade)) );
				break;
			}
			break;
		case course:
		case summary:
			SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation final "+GradeToOldString(best_grade)) );
			break;
		default:
			ASSERT(0);
		}
	}

	switch( best_grade )
	{
	case GRADE_TIER_1:
	case GRADE_TIER_2:	
	case GRADE_TIER_3:	
		this->PostScreenMessage( SM_PlayCheer, CHEER_DELAY_SECONDS );	
		break;
	}

	this->SortByZ();


	SOUND->PlayMusic( THEME->GetPathToS("ScreenEvaluation music") );
	m_timerSoundSequences.SetZero(); // zero the sound sequence timer
	m_timerSoundSequences.Touch(); // set the timer going :]
	m_fScreenCreateTime = RageTimer::GetTimeSinceStart();
}


void ScreenEvaluation::CommitScores( const StageStats &stageStats, int iPersonalHighScoreIndex[NUM_PLAYERS], int iMachineHighScoreIndex[NUM_PLAYERS], RankingCategory rc[NUM_PLAYERS] )
{
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
		return; /* don't save scores in battle */
	}

	// don't save scores if the player chose not to
	if( !GAMESTATE->m_SongOptions.m_bSaveScore )
		return;

	int p;
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;

		// whether or not to save scores when the stage was failed
		// depends on if this is a course or not ... it's handled
		// below in the switch

		switch( m_Type )
		{
		case stage:
			{
				// don't save scores for a failed song
				if( stageStats.bFailed[p] )
					continue;

				ASSERT( GAMESTATE->m_pCurNotes[p] );

				Steps::MemCardData::HighScore hs;
				hs.grade = stageStats.GetGrade( (PlayerNumber)p );
				hs.iScore = stageStats.iScore[p] + stageStats.iBonus[p];
				hs.fPercentDP = stageStats.GetPercentDancePoints( (PlayerNumber)p );
				if( hs.fPercentDP > PREFSMAN->m_fMinPercentageForHighScore )
					GAMESTATE->m_pCurNotes[p]->AddHighScore( (PlayerNumber)p, hs, iPersonalHighScoreIndex[p], iMachineHighScoreIndex[p] );
			}
			break;

		case summary:
			{
				// don't save scores if any stage was failed
				if( stageStats.bFailed[p] ) 
					continue;

				StepsType nt = GAMESTATE->GetCurrentStyleDef()->m_StepsType;

				float fAverageMeter = stageStats.iMeter[p] / (float)PREFSMAN->m_iNumArcadeStages;
				rc[p] = AverageMeterToRankingCategory( fAverageMeter );

				ProfileManager::CategoryData::HighScore hs;
				hs.iScore = stageStats.iScore[p];
				hs.fPercentDP = stageStats.GetPercentDancePoints( (PlayerNumber)p );
				if( hs.fPercentDP > PREFSMAN->m_fMinPercentageForHighScore )
					PROFILEMAN->AddHighScore( nt, rc[p], (PlayerNumber)p, hs, iPersonalHighScoreIndex[p], iMachineHighScoreIndex[p] );
			}
			break;

		case course:
			{
				Course* pCourse = GAMESTATE->m_pCurCourse;
				if( pCourse )
				{
					// don't save scores for a failed Nonstop
					// DO save scores for a failed Oni/Endless
					if( stageStats.bFailed[p] && pCourse->IsNonstop() )
						continue;

					StepsType nt = GAMESTATE->GetCurrentStyleDef()->m_StepsType;
					Course::MemCardData::HighScore hs;
					hs.iScore = stageStats.iScore[p];
					hs.fPercentDP = stageStats.GetPercentDancePoints( (PlayerNumber)p );
					hs.fSurviveTime = stageStats.fAliveSeconds[p];
					if( hs.fPercentDP > PREFSMAN->m_fMinPercentageForHighScore )
						pCourse->AddHighScore( nt, (PlayerNumber)p, hs, iPersonalHighScoreIndex[p], iMachineHighScoreIndex[p] );
				}
			}
			break;
		default:
			ASSERT(0);
		}
	}

	// If both players get a machine high score, a player whose score is added later 
	// may bump the players who were added earlier.  Adjust for this.
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip
		if( iMachineHighScoreIndex[p] == -1 )	// no record
			continue;	// skip
		if( m_bFailed ) // both players failed
			continue; // skip

		for( int p2=0; p2<p; p2++ )
		{
			if( !GAMESTATE->IsHumanPlayer(p2) )
				continue;	// skip
			if( iMachineHighScoreIndex[p2] == -1 )	// no record
				continue;	// skip
			bool bPlayedSameSteps;
			switch( m_Type )
			{
			case stage:
				bPlayedSameSteps = GAMESTATE->m_pCurNotes[p]==GAMESTATE->m_pCurNotes[p2];
				break;
			case summary:
				bPlayedSameSteps = rc[p] == rc[p2];
				break;
			case course:
				bPlayedSameSteps = true;
				break;
			}
			if( iMachineHighScoreIndex[p] >= iMachineHighScoreIndex[p2] )
			{
				iMachineHighScoreIndex[p2]++;
				if( iMachineHighScoreIndex[p2] >= NUM_RANKING_LINES )
					iMachineHighScoreIndex[p2] = -1;
			}
		}
	}
}


void ScreenEvaluation::TweenOffScreen()
{
	int p;

	// large banner area
	OFF_COMMAND( m_LargeBanner );
	OFF_COMMAND( m_sprLargeBannerFrame );
	OFF_COMMAND( m_sprStage );
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;
		OFF_COMMAND( m_DifficultyIcon[p] );
		OFF_COMMAND( m_textPlayerOptions[p] );
	}

	// small banner area
	for( int i=0; i<MAX_SONGS_TO_SHOW; i++ )
	{
		OFF_COMMAND( m_SmallBanner[i] );
		OFF_COMMAND( m_sprSmallBannerFrame[i] );
	}

	// grade area
	for( p=0; p<NUM_PLAYERS; p++ ) 
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;
		OFF_COMMAND( m_sprGradeFrame[p] );
		OFF_COMMAND( m_Grades[p] );
		OFF_COMMAND( m_sprGrade[p] );
	}

	if( SHOW_GRAPH_AREA )
	{
		for( p=0; p<NUM_PLAYERS; p++ ) 
		{
			if( !GAMESTATE->IsPlayerEnabled(p) )
				continue;	// skip

			OFF_COMMAND( m_sprGraphFrame[p] );
			OFF_COMMAND( m_Graph[p] );
		}
	}

	if( SHOW_COMBO_AREA )
	{
		for( p=0; p<NUM_PLAYERS; p++ ) 
		{
			if( !GAMESTATE->IsPlayerEnabled(p) )
				continue;	// skip

			OFF_COMMAND( m_Combo[p] );
		}
	}

	// points area
	if( SHOW_POINTS_AREA )
	{
		for( p=0; p<NUM_PLAYERS; p++ ) 
		{
			if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
				continue;	// skip

			OFF_COMMAND( m_sprPercentFrame[p] );
			m_Percent[p].Command( THEME->GetMetric(m_sName,ssprintf("PercentP%dOffCommand",p+1)) );
			m_Percent[p].TweenOffScreen();
		}
	}

	// bonus area
	if( SHOW_BONUS_AREA )
	{
		for( p=0; p<NUM_PLAYERS; p++ ) 
		{
			if( !GAMESTATE->IsPlayerEnabled(p) )
				continue;
			OFF_COMMAND( m_sprBonusFrame[p] );
			for( int r=0; r<NUM_SHOWN_RADAR_CATEGORIES; r++ )	// foreach line
			{
				OFF_COMMAND( m_sprPossibleBar[p][r] );
				OFF_COMMAND( m_sprActualBar[p][r] );
			}
		}
	}

	// survived area
	if( SHOW_SURVIVED_AREA )
	{
		for( p=0; p<NUM_PLAYERS; p++ ) 
		{
			if( !GAMESTATE->IsPlayerEnabled(p) )
				continue;
			OFF_COMMAND( m_sprSurvivedFrame[p] );
			OFF_COMMAND( m_textSurvivedNumber[p] );
		}
	}

	// win area
	if( SHOW_WIN_AREA )
	{
		for( p=0; p<NUM_PLAYERS; p++ ) 
		{
			if( !GAMESTATE->IsPlayerEnabled(p) )
				continue;
			OFF_COMMAND( m_sprWinFrame[p] );
			OFF_COMMAND( m_sprWin[p] );
		}
	}

	// judgement area
	int l;
	for( l=0; l<NUM_JUDGE_LINES; l++ ) 
	{
		if( !SHOW_JUDGMENT(l) )
			continue;

		OFF_COMMAND( m_sprJudgeLabels[l] );

		for( p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsPlayerEnabled(p) )
				continue;
			OFF_COMMAND( m_textJudgeNumbers[l][p] );
		}
	}

	// stats area
	for( l=0; l<NUM_STATS_LINES; l++ ) 
	{
		if( !SHOW_STAT(l) )
			continue;

		OFF_COMMAND( m_sprStatsLabel[l] );

		for( p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsPlayerEnabled(p) )
				continue;
			OFF_COMMAND( m_textStatsText[l][p] );
		}
	}

	// score area
	if( SHOW_SCORE_AREA )
	{
		OFF_COMMAND( m_sprScoreLabel );
		for( p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsPlayerEnabled(p) )
				continue;
			OFF_COMMAND( m_textScore[p] );
		}
	}

	// total score area
	if( SHOW_TOTAL_SCORE_AREA )
	{
		OFF_COMMAND( m_sprTotalScoreLabel );
		for( p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsPlayerEnabled(p) )
				continue;
			OFF_COMMAND( m_textTotalScore[p] );
		}
	}

	// time area
	if( SHOW_TIME_AREA )
	{
		OFF_COMMAND( m_sprTimeLabel );
		for( p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsPlayerEnabled(p) )
				continue;
			OFF_COMMAND( m_textTime[p] );
		}
	}

	// extra area
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;
		OFF_COMMAND( m_sprMachineRecord[p] );
		OFF_COMMAND( m_sprPersonalRecord[p] );
		if( m_FullCombo[p].IsLoaded() )
			OFF_COMMAND( m_FullCombo[p] );
	}
	OFF_COMMAND( m_sprTryExtraStage );
}

void ScreenEvaluation::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );

	if(m_bFailed && m_Type == stage)
	{
		if(	m_timerSoundSequences.Ago() > FAILED_SOUND_TIME && !m_bPassFailTriggered )
		{
			if(!m_sndPassFail.IsPlaying()) m_sndPassFail.Play();
			m_bPassFailTriggered = true;
		}
		m_bgFailedBack.Update( fDeltaTime );
		m_bgFailedOverlay.Update( fDeltaTime );
	}
	else if (m_Type == stage) // STAGE eval AND passed
	{
		if(	m_timerSoundSequences.Ago() > PASSED_SOUND_TIME && !m_bPassFailTriggered )
		{
			if(!m_sndPassFail.IsPlaying()) m_sndPassFail.Play();
			m_bPassFailTriggered = true;
		}

		m_bgPassedOverlay.Update( fDeltaTime );

	}
	if(m_Type == stage) // stage eval ... pass / fail / whatever
	{
		for( unsigned snd=0; snd<m_SoundSequences.size(); snd++ )
		{
			if(m_SoundSequences[snd].fTime != -1) // already played? skip...
			{
				if(m_timerSoundSequences.Ago() > m_SoundSequences[snd].fTime )
				{
					m_SoundSequences[snd].fTime = -1; // -1 indicates already started playing
					m_SoundSequences[snd].sSound.Play();
				}
			}
		}
	}

	for( int p=0; p<NUM_PLAYERS; p++)
	{
		if (g_CurStageStats.iBonus[p] == 0)
			continue;

		if (GAMESTATE->IsCourseMode())
			continue;

		// wait a few seconds before adding bonus
		if ( RageTimer::GetTimeSinceStart() - m_fScreenCreateTime  < 1.5f)
			continue;

		int increment = g_CurStageStats.iBonus[p]/10;
		if (increment < 1) 
			increment = min(1024, g_CurStageStats.iBonus[p]);

		g_CurStageStats.iBonus[p] -= increment;
		g_CurStageStats.iScore[p] += increment;

		if( SHOW_SCORE_AREA )
			m_textScore[p].SetText( ssprintf("%*.0i", NUM_SCORE_DIGITS, g_CurStageStats.iScore[p]) );
	}
}

void ScreenEvaluation::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();

	// draw the failed background here if the player(s) failed
	if(m_bFailed && m_Type == stage)
		m_bgFailedBack.Draw();

	Screen::DrawPrimitives();
	
	// draw the pass / failed overlays here respectively
	if(m_bFailed && m_Type == stage)
		m_bgFailedOverlay.Draw();
	else if (m_Type == stage)
		m_bgPassedOverlay.Draw();

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
		EndScreen();
		break;
	case SM_GoToNextScreen:
		if(m_bFailed && !PREFSMAN->m_bEventMode && !(GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2()) && GAMESTATE->m_PlayMode == PLAY_MODE_ARCADE) // if failed and not in event mode go to gameover screen
			SCREENMAN->SetNewScreen( FAILED_SCREEN );
		else
			SCREENMAN->SetNewScreen( NEXT_SCREEN );

		if(	m_sndPassFail.IsPlaying() ) m_sndPassFail.Stop();
		break;
	case SM_GoToSelectCourse:
		SCREENMAN->SetNewScreen( "ScreenSelectCourse" );
		break;
	case SM_GoToEndScreen:
		if(m_bFailed && !PREFSMAN->m_bEventMode && !(GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2()) && GAMESTATE->m_PlayMode == PLAY_MODE_ARCADE) // if failed and not in event mode go to gameover screen
			SCREENMAN->SetNewScreen( FAILED_SCREEN );
		else
			SCREENMAN->SetNewScreen( END_SCREEN );
		break;
//	case SM_GoToEvaluationSummary:
//		SCREENMAN->SetNewScreen( "ScreenEvaluationSummary" );
//		break;
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
	m_soundStart.Play();

	EndScreen();
}

void ScreenEvaluation::EndScreen()
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
				m_Menu.StartTransitioning( SM_GoToEndScreen );
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

	if( m_Type == stage )
		GAMESTATE->IncrementStageIndex();
}

