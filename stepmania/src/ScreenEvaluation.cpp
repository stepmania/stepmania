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
#include "Notes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "GrooveRadar.h"
#include "ThemeManager.h"
#include "RageSoundManager.h"

// metrics that are common to all ScreenEvaluation classes
#define BANNER_WIDTH						THEME->GetMetricF("ScreenEvaluation","BannerWidth")
#define BANNER_HEIGHT						THEME->GetMetricF("ScreenEvaluation","BannerHeight")
#define LARGE_BANNER_ON_COMMAND				THEME->GetMetric ("ScreenEvaluation","LargeBannerOnCommand")
#define LARGE_BANNER_OFF_COMMAND			THEME->GetMetric ("ScreenEvaluation","LargeBannerOffCommand")
#define STAGE_ON_COMMAND					THEME->GetMetric ("ScreenEvaluation","StageOnCommand")
#define STAGE_OFF_COMMAND					THEME->GetMetric ("ScreenEvaluation","StageOffCommand")
#define DIFFICULTY_ICON_ON_COMMAND( p )		THEME->GetMetric ("ScreenEvaluation",ssprintf("DifficultyIconP%dOnCommand",p+1))
#define DIFFICULTY_ICON_OFF_COMMAND( p )	THEME->GetMetric ("ScreenEvaluation",ssprintf("DifficultyIconP%dOffCommand",p+1))
#define PLAYER_OPTIONS_ON_COMMAND( p )		THEME->GetMetric ("ScreenEvaluation",ssprintf("PlayerOptionsP%dOnCommand",p+1))
#define PLAYER_OPTIONS_OFF_COMMAND( p )		THEME->GetMetric ("ScreenEvaluation",ssprintf("PlayerOptionsP%dOffCommand",p+1))
#define SMALL_BANNER_ON_COMMAND( i )		THEME->GetMetric ("ScreenEvaluation",ssprintf("SmallBanner%dOnCommand",i+1))
#define SMALL_BANNER_OFF_COMMAND( i )		THEME->GetMetric ("ScreenEvaluation",ssprintf("SmallBanner%dOffCommand",i+1))
#define GRADE_FRAME_ON_COMMAND( p )			THEME->GetMetric ("ScreenEvaluation",ssprintf("GradeFrameP%dOnCommand",p+1))
#define GRADE_FRAME_OFF_COMMAND( p )		THEME->GetMetric ("ScreenEvaluation",ssprintf("GradeFrameP%dOffCommand",p+1))
#define GRADE_ON_COMMAND( p )				THEME->GetMetric ("ScreenEvaluation",ssprintf("GradeP%dOnCommand",p+1))
#define GRADE_OFF_COMMAND( p )				THEME->GetMetric ("ScreenEvaluation",ssprintf("GradeP%dOffCommand",p+1))
#define PERCENT_FRAME_ON_COMMAND( p )		THEME->GetMetric ("ScreenEvaluation",ssprintf("PercentFrameP%dOnCommand",p+1))
#define PERCENT_FRAME_OFF_COMMAND( p )		THEME->GetMetric ("ScreenEvaluation",ssprintf("PercentFrameP%dOffCommand",p+1))
#define PERCENT_WHOLE_ON_COMMAND( p )		THEME->GetMetric ("ScreenEvaluation",ssprintf("PercentWholeP%dOnCommand",p+1))
#define PERCENT_WHOLE_OFF_COMMAND( p )		THEME->GetMetric ("ScreenEvaluation",ssprintf("PercentWholeP%dOffCommand",p+1))
#define PERCENT_REMAINDER_ON_COMMAND( p )	THEME->GetMetric ("ScreenEvaluation",ssprintf("PercentRemainderP%dOnCommand",p+1))
#define PERCENT_REMAINDER_OFF_COMMAND( p )	THEME->GetMetric ("ScreenEvaluation",ssprintf("PercentRemainderP%dOffCommand",p+1))
#define DANCE_POINTS_ON_COMMAND( p )		THEME->GetMetric ("ScreenEvaluation",ssprintf("DancePointsP%dOnCommand",p+1))
#define DANCE_POINTS_OFF_COMMAND( p )		THEME->GetMetric ("ScreenEvaluation",ssprintf("DancePointsP%dOffCommand",p+1))
const char* JUDGE_STRING[NUM_JUDGE_LINES] = { "Marvelous", "Perfect", "Great", "Good", "Boo", "Miss", "OK", "MaxCombo" };
#define JUDGE_LABEL_ON_COMMAND( l )			THEME->GetMetric ("ScreenEvaluation",ssprintf("%sLabelOnCommand",JUDGE_STRING[l]))
#define JUDGE_LABEL_OFF_COMMAND( l )		THEME->GetMetric ("ScreenEvaluation",ssprintf("%sLabelOffCommand",JUDGE_STRING[l]))
#define JUDGE_NUMBER_ON_COMMAND( l, p )		THEME->GetMetric ("ScreenEvaluation",ssprintf("%sNumberP%dOnCommand",JUDGE_STRING[l],p+1))
#define JUDGE_NUMBER_OFF_COMMAND( l, p )	THEME->GetMetric ("ScreenEvaluation",ssprintf("%sNumberP%dOffCommand",JUDGE_STRING[l],p+1))
#define SCORE_LABEL_ON_COMMAND				THEME->GetMetric ("ScreenEvaluation","ScoreLabelOnCommand")
#define SCORE_LABEL_OFF_COMMAND				THEME->GetMetric ("ScreenEvaluation","ScoreLabelOffCommand")
#define SCORE_NUMBER_ON_COMMAND( p )		THEME->GetMetric ("ScreenEvaluation",ssprintf("ScoreNumberP%dOnCommand",p+1))
#define SCORE_NUMBER_OFF_COMMAND( p )		THEME->GetMetric ("ScreenEvaluation",ssprintf("ScoreNumberP%dOffCommand",p+1))
#define TIME_LABEL_ON_COMMAND				THEME->GetMetric ("ScreenEvaluation","TimeLabelOnCommand")
#define TIME_LABEL_OFF_COMMAND				THEME->GetMetric ("ScreenEvaluation","TimeLabelOffCommand")
#define TIME_NUMBER_ON_COMMAND( p )			THEME->GetMetric ("ScreenEvaluation",ssprintf("TimeNumberP%dOnCommand",p+1))
#define TIME_NUMBER_OFF_COMMAND( p )		THEME->GetMetric ("ScreenEvaluation",ssprintf("TimeNumberP%dOffCommand",p+1))
#define BONUS_FRAME_ON_COMMAND( p )			THEME->GetMetric ("ScreenEvaluation",ssprintf("BonusFrameP%dOnCommand",p+1))
#define BONUS_FRAME_OFF_COMMAND( p )		THEME->GetMetric ("ScreenEvaluation",ssprintf("BonusFrameP%dOffCommand",p+1))
#define BAR_POSSIBLE_ON_COMMAND( i, p )		THEME->GetMetric ("ScreenEvaluation",ssprintf("BarPossible%dP%dOnCommand",i+1,p+1))
#define BAR_POSSIBLE_OFF_COMMAND( i, p )	THEME->GetMetric ("ScreenEvaluation",ssprintf("BarPossible%dP%dOffCommand",i+1,p+1))
#define BAR_ACTUAL_ON_COMMAND( i, p )		THEME->GetMetric ("ScreenEvaluation",ssprintf("BarActual%dP%dOnCommand",i+1,p+1))
#define BAR_ACTUAL_OFF_COMMAND( i, p )		THEME->GetMetric ("ScreenEvaluation",ssprintf("BarActual%dP%dOffCommand",i+1,p+1))
#define BAR_ACTUAL_MAX_COMMAND				THEME->GetMetric ("ScreenEvaluation","BarActualMaxCommand")
#define SURVIVED_FRAME_ON_COMMAND( p )		THEME->GetMetric ("ScreenEvaluation",ssprintf("SurvivedFrameP%dOnCommand",p+1))
#define SURVIVED_FRAME_OFF_COMMAND( p )		THEME->GetMetric ("ScreenEvaluation",ssprintf("SurvivedFrameP%dOffCommand",p+1))
#define SURVIVED_NUMBER_ON_COMMAND( p )		THEME->GetMetric ("ScreenEvaluation",ssprintf("SurvivedNumberP%dOnCommand",p+1))
#define SURVIVED_NUMBER_OFF_COMMAND( p )	THEME->GetMetric ("ScreenEvaluation",ssprintf("SurvivedNumberP%dOffCommand",p+1))
#define NEW_RECORD_ON_COMMAND( p )			THEME->GetMetric ("ScreenEvaluation",ssprintf("NewRecordP%dOnCommand",p+1))
#define NEW_RECORD_OFF_COMMAND( p )			THEME->GetMetric ("ScreenEvaluation",ssprintf("NewRecordP%dOffCommand",p+1))
#define TRY_EXTRA_STAGE_ON_COMMAND			THEME->GetMetric ("ScreenEvaluation",ssprintf("TryExtraStageOnCommand"))
#define TRY_EXTRA_STAGE_OFF_COMMAND			THEME->GetMetric ("ScreenEvaluation",ssprintf("TryExtraStageOffCommand"))
#define SPIN_GRADES							THEME->GetMetricB("ScreenEvaluation","SpinGrades")

// metrics that are specific to classes derived from ScreenEvaluation
#define NEXT_SCREEN							THEME->GetMetric (m_sClassName,"NextScreen")
#define END_SCREEN							THEME->GetMetric (m_sClassName,"EndScreen")
#define SHOW_BANNER_AREA					THEME->GetMetricB(m_sClassName,"ShowBannerArea")
#define SHOW_GRADE_AREA						THEME->GetMetricB(m_sClassName,"ShowGradeArea")
#define SHOW_POINTS_AREA					THEME->GetMetricB(m_sClassName,"ShowPointsArea")
#define SHOW_BONUS_AREA						THEME->GetMetricB(m_sClassName,"ShowBonusArea")
#define SHOW_SURVIVED_AREA					THEME->GetMetricB(m_sClassName,"ShowSurvivedArea")
#define SHOW_JUDGMENT( l )					THEME->GetMetricB(m_sClassName,ssprintf("Show%s",JUDGE_STRING[l]))
#define SHOW_SCORE_AREA						THEME->GetMetricB(m_sClassName,"ShowScoreArea")
#define SHOW_TIME_AREA						THEME->GetMetricB(m_sClassName,"ShowTimeArea")


const ScreenMessage SM_GoToSelectCourse			=	ScreenMessage(SM_User+3);
const ScreenMessage SM_GoToEvaluationSummary	=	ScreenMessage(SM_User+4);
const ScreenMessage SM_GoToEndScreen			=	ScreenMessage(SM_User+5);
const ScreenMessage SM_PlayCheer				=	ScreenMessage(SM_User+6);


ScreenEvaluation::ScreenEvaluation( CString sClassName, Type type )
{
	//
	// debugging
	//
	GAMESTATE->m_PlayMode = PLAY_MODE_NONSTOP;
	GAMESTATE->m_CurStyle = STYLE_DANCE_VERSUS;
	GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
	GAMESTATE->m_pCurSong = SONGMAN->GetAllSongs()[0];
	GAMESTATE->m_pCurCourse = SONGMAN->m_pCourses[0];
	GAMESTATE->m_pCurNotes[PLAYER_1] = GAMESTATE->m_pCurSong->GetNotes( NOTES_TYPE_DANCE_SINGLE, DIFFICULTY_HARD );
	GAMESTATE->m_pCurNotes[PLAYER_2] = GAMESTATE->m_pCurSong->GetNotes( NOTES_TYPE_DANCE_SINGLE, DIFFICULTY_HARD );
	GAMESTATE->m_PlayerOptions[PLAYER_1].m_bHoldNotes = false;
	GAMESTATE->m_PlayerOptions[PLAYER_2].m_bHoldNotes = false;
	GAMESTATE->m_PlayerOptions[PLAYER_1].m_fScrollSpeed = 2;
	GAMESTATE->m_PlayerOptions[PLAYER_2].m_fScrollSpeed = 2;
	GAMESTATE->m_iCurrentStageIndex = 2;


	LOG->Trace( "ScreenEvaluation::ScreenEvaluation()" );

	m_sClassName = sClassName;
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
		stageStats = GAMESTATE->m_CurStageStats;
		break;
	default:
		ASSERT(0);
	}

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
		max_grade = max( max_grade, grade[p] ); 


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
				if( GAMESTATE->IsPlayerEnabled(p) )
					GAMESTATE->m_pCurNotes[p]->AddScore( (PlayerNumber)p, grade[p], stageStats.fScore[p], bNewRecord[p] );
		}
		break;
	case summary:
		{
			NotesType nt = GAMESTATE->GetCurrentStyleDef()->m_NotesType;
			bool bIsPlayerEnabled[NUM_PLAYERS];
			for( p=0; p<NUM_PLAYERS; p++ )
				bIsPlayerEnabled[p] = GAMESTATE->IsPlayerEnabled(p);

			RankingCategory cat[NUM_PLAYERS];
			int iRankingIndex[NUM_PLAYERS];
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				float fAverageMeter = stageStats.iMeter[p] / (float)PREFSMAN->m_iNumArcadeStages;
				cat[p] = AverageMeterToRankingCategory( fAverageMeter );
			}

			SONGMAN->AddScores( nt, bIsPlayerEnabled, cat, stageStats.fScore, iRankingIndex );

			COPY( GAMESTATE->m_RankingCategory, cat );
			COPY( GAMESTATE->m_iRankingIndex, iRankingIndex );
			GAMESTATE->m_RankingNotesType = nt;
		}
		break;
	case course:
		{
			NotesType nt = GAMESTATE->GetCurrentStyleDef()->m_NotesType;
			bool bIsPlayerEnabled[NUM_PLAYERS];
			for( p=0; p<NUM_PLAYERS; p++ )
				bIsPlayerEnabled[p] = GAMESTATE->IsPlayerEnabled(p);

			int iRankingIndex[NUM_PLAYERS];

			Course* pCourse = GAMESTATE->m_pCurCourse;
			pCourse->AddScores( nt, bIsPlayerEnabled, stageStats.iActualDancePoints, stageStats.fAliveSeconds, iRankingIndex, bNewRecord );
			COPY( GAMESTATE->m_iRankingIndex, iRankingIndex );
			GAMESTATE->m_pRankingCourse = pCourse;
			GAMESTATE->m_RankingNotesType = nt;
		}
		break;
	default:
		ASSERT(0);
	}

	m_bTryExtraStage = 
		GAMESTATE->HasEarnedExtraStage()  && 
		m_Type==stage;
 


	m_Menu.Load( m_sClassName );
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
				m_LargeBanner.SetCroppedSize( BANNER_WIDTH, BANNER_HEIGHT );
				m_LargeBanner.LoadFromSong( GAMESTATE->m_pCurSong );
				m_LargeBanner.Command( LARGE_BANNER_ON_COMMAND );
				this->AddChild( &m_LargeBanner );

				m_sprLargeBannerFrame.Load( THEME->GetPathTo("Graphics","ScreenEvaluation banner frame") );
				m_sprLargeBannerFrame.Command( LARGE_BANNER_ON_COMMAND );
				this->AddChild( &m_sprLargeBannerFrame );

				m_sprStage.Load( THEME->GetPathTo("Graphics","ScreenEvaluation stage "+GAMESTATE->GetStageText()) );
				m_sprStage.Command( STAGE_ON_COMMAND );
				this->AddChild( &m_sprStage );

				for( int p=0; p<NUM_PLAYERS; p++ )
				{
					if( !GAMESTATE->IsPlayerEnabled(p) )
						continue;	// skip

					m_DifficultyIcon[p].Load( THEME->GetPathTo("Graphics","ScreenEvaluation difficulty icons 1x5") );
					m_DifficultyIcon[p].SetFromNotes( (PlayerNumber)p, GAMESTATE->m_pCurNotes[p] );
					m_DifficultyIcon[p].Command( DIFFICULTY_ICON_ON_COMMAND(p) );
					this->AddChild( &m_DifficultyIcon[p] );
					
					m_textPlayerOptions[p].LoadFromFont( THEME->GetPathTo("Fonts","normal") );
					CString sPO = GAMESTATE->m_PlayerOptions[p].GetString();
					sPO.Replace( ", ", "\n" );
					m_textPlayerOptions[p].Command( PLAYER_OPTIONS_ON_COMMAND(p) );
					m_textPlayerOptions[p].SetText( sPO );
					this->AddChild( &m_textPlayerOptions[p] );
				}
			}
			break;
		case summary:
			{
				for( unsigned i=0; i<vSongsToShow.size(); i++ )
				{
					m_SmallBanner[i].SetCroppedSize( BANNER_WIDTH, BANNER_HEIGHT );
					m_SmallBanner[i].LoadFromSong( vSongsToShow[i] );
					m_SmallBanner[i].Command( SMALL_BANNER_ON_COMMAND(i) );
					this->AddChild( &m_SmallBanner[i] );

					m_sprSmallBannerFrame[i].Load( THEME->GetPathTo("Graphics","ScreenEvaluation banner frame") );
					m_sprSmallBannerFrame[i].Command( SMALL_BANNER_ON_COMMAND(i) );
					this->AddChild( &m_sprSmallBannerFrame[i] );
				}
			}
			break;
		case course:
			{
				m_LargeBanner.SetCroppedSize( BANNER_WIDTH, BANNER_HEIGHT );
				m_LargeBanner.LoadFromCourse( GAMESTATE->m_pCurCourse );
				m_LargeBanner.Command( LARGE_BANNER_ON_COMMAND );
				this->AddChild( &m_LargeBanner );

				m_sprLargeBannerFrame.Load( THEME->GetPathTo("Graphics","ScreenEvaluation banner frame") );
				m_sprLargeBannerFrame.Command( LARGE_BANNER_ON_COMMAND );
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

			m_sprGradeFrame[p].Load( THEME->GetPathTo("Graphics","ScreenEvaluation grade frame 1x2") );
			m_sprGradeFrame[p].StopAnimating();
			m_sprGradeFrame[p].SetState( p );
			m_sprGradeFrame[p].Command( GRADE_FRAME_ON_COMMAND(p) );
			this->AddChild( &m_sprGradeFrame[p] );

			if( SPIN_GRADES )
				m_Grades[p].SpinAndSettleOn( grade[p] );
			else
				m_Grades[p].SetGrade( (PlayerNumber)p, grade[p] );
			m_Grades[p].Command( GRADE_ON_COMMAND(p) );
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
			m_sprPercentFrame[p].Load( THEME->GetPathTo("Graphics","ScreenEvaluation percent frame 1x2") );
			m_sprPercentFrame[p].StopAnimating();
			m_sprPercentFrame[p].SetState( p );
			m_sprPercentFrame[p].Command( PERCENT_FRAME_ON_COMMAND(p) );
			this->AddChild( &m_sprPercentFrame[p] );

			if( !PREFSMAN->m_bDancePointsForOni )
			{
				m_textPercentWhole[p].LoadFromNumbers( THEME->GetPathTo("Numbers","ScreenEvaluation percent numbers") );
				m_textPercentWhole[p].EnableShadow( false );
				m_textPercentWhole[p].Command( PERCENT_WHOLE_ON_COMMAND(p) );
				this->AddChild( &m_textPercentWhole[p] );

				m_textPercentRemainder[p].LoadFromNumbers( THEME->GetPathTo("Numbers","ScreenEvaluation percent numbers") );
				m_textPercentRemainder[p].EnableShadow( false );
				m_textPercentRemainder[p].Command( PERCENT_REMAINDER_ON_COMMAND(p) );
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
				m_textDancePoints[p].LoadFromNumbers( THEME->GetPathTo("Numbers","ScreenEvaluation percent numbers") );
				m_textDancePoints[p].EnableShadow( false );
				m_textDancePoints[p].SetText( ssprintf("%d", stageStats.iActualDancePoints[p]) );
				m_textDancePoints[p].Command( DANCE_POINTS_ON_COMMAND(p) );
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

			m_sprBonusFrame[p].Load( THEME->GetPathTo("Graphics","ScreenEvaluation bonus frame 2x1") );
			m_sprBonusFrame[p].StopAnimating();
			m_sprBonusFrame[p].SetState( p );
			m_sprBonusFrame[p].Command( BONUS_FRAME_ON_COMMAND(p) );
			this->AddChild( &m_sprBonusFrame[p] );

			for( int r=0; r<NUM_RADAR_CATEGORIES; r++ )	// foreach line
			{
				m_sprPossibleBar[p][r].Load( THEME->GetPathTo("Graphics","ScreenEvaluation bars possible 1x2") );
				m_sprPossibleBar[p][r].StopAnimating();
				m_sprPossibleBar[p][r].SetState( p );
				m_sprPossibleBar[p][r].SetWidth( m_sprPossibleBar[p][r].GetUnzoomedWidth() * stageStats.fRadarPossible[p][r] );
				m_sprPossibleBar[p][r].Command( BAR_POSSIBLE_ON_COMMAND(r,p) );
				this->AddChild( &m_sprPossibleBar[p][r] );

				m_sprActualBar[p][r].Load( THEME->GetPathTo("Graphics","ScreenEvaluation bars actual 1x2") );
				m_sprActualBar[p][r].StopAnimating();
				m_sprActualBar[p][r].SetState( p );
				m_sprActualBar[p][r].SetWidth( m_sprActualBar[p][r].GetUnzoomedWidth() * stageStats.fRadarActual[p][r] );
				m_sprActualBar[p][r].Command( BAR_ACTUAL_ON_COMMAND(r,p) );
				if( stageStats.fRadarActual[p][r] == stageStats.fRadarPossible[p][r] )
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

			m_sprSurvivedFrame[p].Load( THEME->GetPathTo("Graphics","ScreenEvaluation survived frame 2x1") );
			m_sprSurvivedFrame[p].StopAnimating();
			m_sprSurvivedFrame[p].SetState( p );
			m_sprSurvivedFrame[p].Command( SURVIVED_FRAME_ON_COMMAND(p) );
			this->AddChild( &m_sprSurvivedFrame[p] );

			m_textSurvivedNumber[p].LoadFromNumbers( THEME->GetPathTo("Numbers","ScreenEvaluation stage numbers") );
			m_textSurvivedNumber[p].EnableShadow( false );
			m_textSurvivedNumber[p].SetText( ssprintf("%02d", stageStats.iSongsPlayed[p]) );
			m_textSurvivedNumber[p].Command( SURVIVED_NUMBER_ON_COMMAND(p) );
			this->AddChild( &m_textSurvivedNumber[p] );
		}
	}
	
	//
	// init judgment area
	//
	int l;
	for( l=0; l<NUM_JUDGE_LINES; l++ ) 
	{
		if( SHOW_JUDGMENT(l) )
		{
			m_sprJudgeLabels[l].Load( THEME->GetPathTo("Graphics","ScreenEvaluation judge labels 1x8") );
			m_sprJudgeLabels[l].StopAnimating();
			m_sprJudgeLabels[l].SetState( l );
			m_sprJudgeLabels[l].Command( JUDGE_LABEL_ON_COMMAND(l) );
			this->AddChild( &m_sprJudgeLabels[l] );

			for( p=0; p<NUM_PLAYERS; p++ )
			{
				if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
					continue;	// skip

				m_textJudgeNumbers[l][p].LoadFromNumbers( THEME->GetPathTo("Numbers","ScreenEvaluation judge numbers") );
				m_textJudgeNumbers[l][p].EnableShadow( false );
				m_textJudgeNumbers[l][p].SetDiffuse( PlayerToColor(p) );
				m_textJudgeNumbers[l][p].Command( JUDGE_NUMBER_ON_COMMAND(l,p) );
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
				case 6:	iValue = stageStats.iTapNoteScores[p][HNS_OK];			break;
				case 7:	iValue = stageStats.iMaxCombo[p];						break;
				default:	ASSERT(0);
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
		int s;
		for( s=0; s<NUM_SCORE_LINES; s++ )
		{
			m_sprScoreLabel.Load( THEME->GetPathTo("Graphics","ScreenEvaluation score labels 1x2") );
			m_sprScoreLabel.SetState( s );
			m_sprScoreLabel.StopAnimating();
			m_sprScoreLabel.Command( SCORE_LABEL_ON_COMMAND );
			this->AddChild( &m_sprScoreLabel );
		}

		for( p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
				continue;	// skip

			m_ScoreDisplay[p].Init( (PlayerNumber)p );
			m_ScoreDisplay[p].Command( SCORE_NUMBER_ON_COMMAND(p) );
			m_ScoreDisplay[p].SetScore( stageStats.fScore[p] );
			this->AddChild( &m_ScoreDisplay[p] );

		}
	}

	//
	// init time area
	//
	if( SHOW_TIME_AREA )
	{
		m_sprTimeLabel.Load( THEME->GetPathTo("Graphics","ScreenEvaluation score labels 1x2") );
		m_sprTimeLabel.SetState( 1 );
		m_sprTimeLabel.StopAnimating();
		m_sprTimeLabel.Command( TIME_LABEL_ON_COMMAND );
		this->AddChild( &m_sprTimeLabel );

		for( p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
				continue;	// skip

			m_TimeDisplay[p].Init( (PlayerNumber)p );
			m_TimeDisplay[p].Command( TIME_NUMBER_ON_COMMAND(p) );
			m_TimeDisplay[p].SetText( SecondsToTime(stageStats.fAliveSeconds[p]) );
			this->AddChild( &m_TimeDisplay[p] );
		}
	}


	//
	// init extra area
	//
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( bNewRecord[p] )
		{
			m_sprNewRecord[p].Load( THEME->GetPathTo("Graphics","ScreenEvaluation new record") );
			m_sprNewRecord[p].Command( NEW_RECORD_ON_COMMAND(p) );
			this->AddChild( &m_sprNewRecord[p] );
		}
	}

	bool bOneHasNewRecord = false;
	for( p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsPlayerEnabled(p) && bNewRecord[p] )
			bOneHasNewRecord = true;
	
	if( m_bTryExtraStage )
	{
		m_sprTryExtraStage.Load( THEME->GetPathTo("Graphics",GAMESTATE->IsExtraStage()?" try extra2":" try extra1") );
		m_sprTryExtraStage.Command( TRY_EXTRA_STAGE_ON_COMMAND );
		this->AddChild( &m_sprTryExtraStage );

		if( GAMESTATE->IsExtraStage() )
			SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","ScreenEvaluation try extra2") );
		else
			SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","ScreenEvaluation try extra1") );
	}
	else if( bOneHasNewRecord  &&  ANNOUNCER->HasSoundsFor("evaluation new record") )
	{
		SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation new record") );
	}
	else
	{	
		switch( m_Type )
		{
		case stage:
			SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation "+GradeToString(max_grade)) );
			break;
		case course:
		case summary:
			SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation final "+GradeToString(max_grade)) );
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
		this->PostScreenMessage( SM_PlayCheer, 2.5f );	
		break;
	}


	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","ScreenEvaluation music") );
}


void ScreenEvaluation::TweenOffScreen()
{
	int p;

	// large banner area
	m_LargeBanner.Command( LARGE_BANNER_OFF_COMMAND );
	m_sprLargeBannerFrame.Command( LARGE_BANNER_OFF_COMMAND );
	m_sprStage.Command( STAGE_OFF_COMMAND );
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_DifficultyIcon[p].Command( DIFFICULTY_ICON_OFF_COMMAND(p) );
		m_textPlayerOptions[p].Command( PLAYER_OPTIONS_OFF_COMMAND(p) );
	}

	// small banner area
	for( unsigned i=0; i<MAX_SONGS_TO_SHOW; i++ )
	{
		m_SmallBanner[i].Command( SMALL_BANNER_OFF_COMMAND(i) );
		m_sprSmallBannerFrame[i].Command( SMALL_BANNER_OFF_COMMAND(i) );
	}

	// grade area
	for( p=0; p<NUM_PLAYERS; p++ ) 
	{
		m_sprGradeFrame[p].Command( GRADE_FRAME_OFF_COMMAND(p) );
		m_Grades[p].Command( GRADE_OFF_COMMAND(p) );
	}

	// points area
	for( p=0; p<NUM_PLAYERS; p++ ) 
	{
		m_sprPercentFrame[p].Command( PERCENT_FRAME_OFF_COMMAND(p) );
		m_textPercentWhole[p].Command( PERCENT_WHOLE_OFF_COMMAND(p) );
		m_textPercentRemainder[p].Command( PERCENT_REMAINDER_OFF_COMMAND(p) );
		m_textDancePoints[p].Command( DANCE_POINTS_OFF_COMMAND(p) );
	}

	// bonus area
	for( p=0; p<NUM_PLAYERS; p++ ) 
	{
		m_sprBonusFrame[p].Command( BONUS_FRAME_OFF_COMMAND(p) );
		for( int r=0; r<NUM_RADAR_CATEGORIES; r++ )	// foreach line
		{
			m_sprPossibleBar[p][r].Command( BAR_POSSIBLE_OFF_COMMAND(r,p) );
			m_sprActualBar[p][r].Command( BAR_ACTUAL_OFF_COMMAND(r,p) );
		}
	}

	// survived area
	for( p=0; p<NUM_PLAYERS; p++ ) 
	{
		m_sprSurvivedFrame[p].Command( SURVIVED_FRAME_OFF_COMMAND(p) );
		m_textSurvivedNumber[p].Command( SURVIVED_NUMBER_OFF_COMMAND(p) );
	}
		
	// judgement area
	int l;
	for( l=0; l<NUM_JUDGE_LINES; l++ ) 
		m_sprJudgeLabels[l].Command( JUDGE_LABEL_OFF_COMMAND(l) );

	for( p=0; p<NUM_PLAYERS; p++ )
		for( l=0; l<NUM_JUDGE_LINES; l++ ) 
			m_textJudgeNumbers[l][p].Command( JUDGE_NUMBER_OFF_COMMAND(l,p) );

	// score area
	m_sprScoreLabel.Command( SCORE_LABEL_OFF_COMMAND );
	for( p=0; p<NUM_PLAYERS; p++ )
		m_ScoreDisplay[p].Command( SCORE_NUMBER_OFF_COMMAND(p) );

	// time area
	m_sprTimeLabel.Command( TIME_LABEL_OFF_COMMAND );
	for( p=0; p<NUM_PLAYERS; p++ )
		m_TimeDisplay[p].Command( TIME_NUMBER_OFF_COMMAND(p) );

	// extra area
	for( p=0; p<NUM_PLAYERS; p++ )
		m_sprNewRecord[p].Command( NEW_RECORD_OFF_COMMAND(p) );
	m_sprTryExtraStage.Command( TRY_EXTRA_STAGE_OFF_COMMAND );
}


void ScreenEvaluation::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );
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
		SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation cheer") );
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


	if( PREFSMAN->m_bEventMode )
	{
		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_ARCADE:
			m_Menu.StartTransitioning( SM_GoToNextScreen );
			break;
		case PLAY_MODE_NONSTOP:
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:
			m_Menu.StartTransitioning( SM_GoToSelectCourse );
			break;
		default:
			ASSERT(0);
		}
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
				if( GAMESTATE->IsPlayerEnabled(p) )
					bOnePassed |= !GAMESTATE->m_CurStageStats.bFailed[p];

			if( bOnePassed )
				GAMESTATE->m_vPassedStageStats.push_back( GAMESTATE->m_CurStageStats );	// Save this stage's stats
		}
		break;
	}
}

