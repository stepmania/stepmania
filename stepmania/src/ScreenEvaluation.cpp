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

// If a metric isn't present in the category m_sClassName, fall back to "ScreenEvaluation"
#define GET_THEME_METRIC_FALLBACK(metric)		(THEME->HasMetric(m_sClassName,metric)?THEME->GetMetric(m_sClassName,metric):THEME->GetMetric("ScreenEvaluation",metric))
#define GET_THEME_ELEMENT_FALLBACK(cat,suffix)	(THEME->GetPathToOptional(cat,m_sClassName+(suffix))!="" ? THEME->GetPathTo(cat,m_sClassName+(suffix)) : THEME->GetPathTo(cat,CString("ScreenEvaluation")+(suffix)))


#define SONGSEL_SCREEN						GET_THEME_METRIC_FALLBACK("SongSelectScreen")
#define ENDGAME_SCREEN						GET_THEME_METRIC_FALLBACK("EndGameScreen")
#define BANNER_ON_COMMAND( i )				GET_THEME_METRIC_FALLBACK(ssprintf("Banner%dOnCommand",i+1))
#define BANNER_OFF_COMMAND( i )				GET_THEME_METRIC_FALLBACK(ssprintf("Banner%dOffCommand",i+1))
#define BANNER_WIDTH						float(atof(GET_THEME_METRIC_FALLBACK("BannerWidth")))
#define BANNER_HEIGHT						float(atof(GET_THEME_METRIC_FALLBACK("BannerHeight")))
#define STAGE_ON_COMMAND					GET_THEME_METRIC_FALLBACK("StageOnCommand")
#define STAGE_OFF_COMMAND					GET_THEME_METRIC_FALLBACK("StageOffCommand")
#define DIFFICULTY_ICON_ON_COMMAND( p )		GET_THEME_METRIC_FALLBACK(ssprintf("DifficultyIconP%dOnCommand",p+1))
#define DIFFICULTY_ICON_OFF_COMMAND( p )	GET_THEME_METRIC_FALLBACK(ssprintf("DifficultyIconP%dOffCommand",p+1))
#define PLAYER_OPTIONS_ON_COMMAND( p )		GET_THEME_METRIC_FALLBACK(ssprintf("PlayerOptionsP%dOnCommand",p+1))
#define PLAYER_OPTIONS_OFF_COMMAND( p )		GET_THEME_METRIC_FALLBACK(ssprintf("PlayerOptionsP%dOffCommand",p+1))
#define GRADE_FRAME_ON_COMMAND( p )			GET_THEME_METRIC_FALLBACK(ssprintf("GradeFrameP%dOnCommand",p+1))
#define GRADE_FRAME_OFF_COMMAND( p )		GET_THEME_METRIC_FALLBACK(ssprintf("GradeFrameP%dOffCommand",p+1))
#define GRADE_ON_COMMAND( p )				GET_THEME_METRIC_FALLBACK(ssprintf("GradeP%dOnCommand",p+1))
#define GRADE_OFF_COMMAND( p )				GET_THEME_METRIC_FALLBACK(ssprintf("GradeP%dOffCommand",p+1))
#define PERCENT_WHOLE_ON_COMMAND( p )		GET_THEME_METRIC_FALLBACK(ssprintf("PercentWholeP%dOnCommand",p+1))
#define PERCENT_WHOLE_OFF_COMMAND( p )		GET_THEME_METRIC_FALLBACK(ssprintf("PercentWholeP%dOffCommand",p+1))
#define PERCENT_REMAINDER_ON_COMMAND( p )	GET_THEME_METRIC_FALLBACK(ssprintf("PercentRemainderP%dOnCommand",p+1))
#define PERCENT_REMAINDER_OFF_COMMAND( p )	GET_THEME_METRIC_FALLBACK(ssprintf("PercentRemainderP%dOffCommand",p+1))
#define DANCE_POINTS_ON_COMMAND( p )		GET_THEME_METRIC_FALLBACK(ssprintf("DancePointsP%dOnCommand",p+1))
#define DANCE_POINTS_OFF_COMMAND( p )		GET_THEME_METRIC_FALLBACK(ssprintf("DancePointsP%dOffCommand",p+1))
const char* JUDGE_STRING[NUM_JUDGE_LINES] = { "Marvelous", "Perfect", "Great", "Good", "Boo", "Miss", "OK", "MaxCombo" };
#define JUDGE_LABEL_ON_COMMAND( l )			GET_THEME_METRIC_FALLBACK(ssprintf("%sLabelOnCommand",JUDGE_STRING[l]))
#define JUDGE_LABEL_OFF_COMMAND( l )		GET_THEME_METRIC_FALLBACK(ssprintf("%sLabelOffCommand",JUDGE_STRING[l]))
#define JUDGE_NUMBER_ON_COMMAND( l, p )		GET_THEME_METRIC_FALLBACK(ssprintf("%sNumberP%dOnCommand",JUDGE_STRING[l],p+1))
#define JUDGE_NUMBER_OFF_COMMAND( l, p )	GET_THEME_METRIC_FALLBACK(ssprintf("%sNumberP%dOffCommand",JUDGE_STRING[l],p+1))
const char* SCORE_STRING[NUM_SCORE_LINES] = { "Score", "Time" };
#define SCORE_LABEL_ON_COMMAND( l )			GET_THEME_METRIC_FALLBACK(ssprintf("%sLabelOnCommand",SCORE_STRING[l]))
#define SCORE_LABEL_OFF_COMMAND( l )		GET_THEME_METRIC_FALLBACK(ssprintf("%sLabelOffCommand",SCORE_STRING[l]))
#define SCORE_NUMBER_ON_COMMAND( l, p )		GET_THEME_METRIC_FALLBACK(ssprintf("%sNumberP%dOnCommand",SCORE_STRING[l],p+1))
#define SCORE_NUMBER_OFF_COMMAND( l, p )	GET_THEME_METRIC_FALLBACK(ssprintf("%sNumberP%dOffCommand",SCORE_STRING[l],p+1))
#define BONUS_FRAME_ON_COMMAND( p )			GET_THEME_METRIC_FALLBACK(ssprintf("BonusFrameP%dOnCommand",p+1))
#define BONUS_FRAME_OFF_COMMAND( p )		GET_THEME_METRIC_FALLBACK(ssprintf("BonusFrameP%dOffCommand",p+1))
#define BAR_POSSIBLE_ON_COMMAND( i, p )		GET_THEME_METRIC_FALLBACK(ssprintf("BarPossible%dP%dOnCommand",i+1,p+1))
#define BAR_POSSIBLE_OFF_COMMAND( i, p )	GET_THEME_METRIC_FALLBACK(ssprintf("BarPossible%dP%dOffCommand",i+1,p+1))
#define BAR_ACTUAL_ON_COMMAND( i, p )		GET_THEME_METRIC_FALLBACK(ssprintf("BarActual%dP%dOnCommand",i+1,p+1))
#define BAR_ACTUAL_OFF_COMMAND( i, p )		GET_THEME_METRIC_FALLBACK(ssprintf("BarActual%dP%dOffCommand",i+1,p+1))
#define BAR_ACTUAL_MAX_COMMAND				GET_THEME_METRIC_FALLBACK("BarActualMaxCommand")
#define SONGS_SURVIVED_ON_COMMAND( p )		GET_THEME_METRIC_FALLBACK(ssprintf("SongsSurvivedP%dOnCommand",p+1))
#define SONGS_SURVIVED_OFF_COMMAND( p )		GET_THEME_METRIC_FALLBACK(ssprintf("SongsSurvivedP%dOffCommand",p+1))
#define NEW_RECORD_ON_COMMAND( p )			GET_THEME_METRIC_FALLBACK(ssprintf("NewRecordP%dOnCommand",p+1))
#define NEW_RECORD_OFF_COMMAND( p )			GET_THEME_METRIC_FALLBACK(ssprintf("NewRecordP%dOffCommand",p+1))
#define TRY_EXTRA_STAGE_ON_COMMAND			GET_THEME_METRIC_FALLBACK(ssprintf("TryExtraStageOnCommand"))
#define TRY_EXTRA_STAGE_OFF_COMMAND			GET_THEME_METRIC_FALLBACK(ssprintf("TryExtraStageOffCommand"))
#define SPIN_GRADES							0!=atoi(GET_THEME_METRIC_FALLBACK("SpinGrades"))


const ScreenMessage SM_GoToSelectMusic		=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToSelectCourse		=	ScreenMessage(SM_User+3);
const ScreenMessage SM_GoToEvaluationSummary	=	ScreenMessage(SM_User+4);
const ScreenMessage SM_GoToGameFinished		=	ScreenMessage(SM_User+5);
const ScreenMessage SM_PlayCheer			=	ScreenMessage(SM_User+6);


ScreenEvaluation::ScreenEvaluation( CString sClassName, Type type )
{
	/*
	//
	// debugging
	//
	GAMESTATE->m_PlayMode = PLAY_MODE_NONSTOP;
	GAMESTATE->m_CurStyle = STYLE_DANCE_VERSUS;
	GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
	GAMESTATE->m_pCurCourse = SONGMAN->m_pCourses[0];
//	GAMESTATE->m_pCurNotes[PLAYER_1] = GAMESTATE->m_pCurSong->GetNotes( NOTES_TYPE_DANCE_SINGLE, DIFFICULTY_HARD );
//	GAMESTATE->m_pCurNotes[PLAYER_2] = GAMESTATE->m_pCurSong->GetNotes( NOTES_TYPE_DANCE_SINGLE, DIFFICULTY_HARD );
	GAMESTATE->m_PlayerOptions[PLAYER_1].m_bHoldNotes = false;
	GAMESTATE->m_PlayerOptions[PLAYER_2].m_bHoldNotes = false;
	GAMESTATE->m_PlayerOptions[PLAYER_1].m_fScrollSpeed = 2;
	GAMESTATE->m_PlayerOptions[PLAYER_2].m_fScrollSpeed = 2;
//	GAMESTATE->m_iCurrentStageIndex = 2;
*/

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
	switch( m_Type )
	{
	case stage:
		{
			m_Banner[0].SetCroppedSize( BANNER_WIDTH, BANNER_HEIGHT );
			m_Banner[0].LoadFromSong( GAMESTATE->m_pCurSong );
			m_Banner[0].Command( BANNER_ON_COMMAND(0) );
			this->AddChild( &m_Banner[0] );

			m_sprBannerFrame[0].Load( GET_THEME_ELEMENT_FALLBACK("Graphics"," banner frame") );
			m_sprBannerFrame[0].Command( BANNER_ON_COMMAND(0) );
			this->AddChild( &m_sprBannerFrame[0] );

			m_sprStage.Load( GET_THEME_ELEMENT_FALLBACK("Graphics"," stage "+GAMESTATE->GetStageText()) );
			m_sprStage.Command( STAGE_ON_COMMAND );
			this->AddChild( &m_sprStage );

			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( !GAMESTATE->IsPlayerEnabled(p) )
					continue;	// skip

				m_DifficultyIcon[p].Load( GET_THEME_ELEMENT_FALLBACK("Graphics"," difficulty icons 1x5") );
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
				m_Banner[i].SetCroppedSize( BANNER_WIDTH, BANNER_HEIGHT );
				m_Banner[i].LoadFromSong( vSongsToShow[i] );
				m_Banner[i].Command( BANNER_ON_COMMAND(i) );
				this->AddChild( &m_Banner[i] );

				m_sprBannerFrame[i].Load( GET_THEME_ELEMENT_FALLBACK("Graphics"," banner frame") );
				m_sprBannerFrame[i].Command( BANNER_ON_COMMAND(i) );
				this->AddChild( &m_sprBannerFrame[i] );
			}
		}
		break;
	case course:
		{
			m_Banner[0].SetCroppedSize( BANNER_WIDTH, BANNER_HEIGHT );
			m_Banner[0].LoadFromCourse( GAMESTATE->m_pCurCourse );
			m_Banner[0].Command( BANNER_ON_COMMAND(0) );
			this->AddChild( &m_Banner[0] );

			m_sprBannerFrame[0].Load( GET_THEME_ELEMENT_FALLBACK("Graphics"," banner frame") );
			m_sprBannerFrame[0].Command( BANNER_ON_COMMAND(0) );
			this->AddChild( &m_sprBannerFrame[0] );
		}
		break;
	default:
		ASSERT(0);
	}


	//
	// init grade area
	//
	for( p=0; p<NUM_PLAYERS; p++ ) 
	{
		if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
			continue;	// skip

		m_sprGradeFrame[p].Load( GET_THEME_ELEMENT_FALLBACK("Graphics"," grade frame 1x2") );
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

		if( !PREFSMAN->m_bDancePointsForOni )
		{
			m_textPercentWhole[p].LoadFromNumbers( GET_THEME_ELEMENT_FALLBACK("Numbers"," percent numbers") );
			m_textPercentWhole[p].EnableShadow( false );
			m_textPercentWhole[p].Command( PERCENT_WHOLE_ON_COMMAND(p) );
			this->AddChild( &m_textPercentWhole[p] );

			m_textPercentRemainder[p].LoadFromNumbers( GET_THEME_ELEMENT_FALLBACK("Numbers"," percent numbers") );
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
			m_textDancePoints[p].LoadFromNumbers( GET_THEME_ELEMENT_FALLBACK("Numbers"," percent numbers") );
			m_textDancePoints[p].EnableShadow( false );
			m_textDancePoints[p].SetText( ssprintf("%d", stageStats.iActualDancePoints[p]) );
			m_textDancePoints[p].Command( DANCE_POINTS_ON_COMMAND(p) );
			this->AddChild( &m_textDancePoints[p] );
		}
	}


	//
	// init bonus area
	//
	for( p=0; p<NUM_PLAYERS; p++ ) 
	{
		if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
			continue;	// skip

		m_sprBonusFrame[p].Load( GET_THEME_ELEMENT_FALLBACK("Graphics"," bonus frame 2x1") );
		m_sprBonusFrame[p].StopAnimating();
		m_sprBonusFrame[p].SetState( p );
		m_sprBonusFrame[p].Command( BONUS_FRAME_ON_COMMAND(p) );
		this->AddChild( &m_sprBonusFrame[p] );

		for( int r=0; r<NUM_RADAR_CATEGORIES; r++ )	// foreach line
		{
			m_sprPossibleBar[p][r].Load( GET_THEME_ELEMENT_FALLBACK("Graphics"," bars possible 1x2") );
			m_sprPossibleBar[p][r].StopAnimating();
			m_sprPossibleBar[p][r].SetState( p );
			m_sprPossibleBar[p][r].SetWidth( m_sprPossibleBar[p][r].GetUnzoomedWidth() * stageStats.fRadarPossible[p][r] );
			m_sprPossibleBar[p][r].Command( BAR_POSSIBLE_ON_COMMAND(r,p) );
			this->AddChild( &m_sprPossibleBar[p][r] );

			m_sprActualBar[p][r].Load( GET_THEME_ELEMENT_FALLBACK("Graphics"," bars actual 1x2") );
			m_sprActualBar[p][r].StopAnimating();
			m_sprActualBar[p][r].SetState( p );
			m_sprActualBar[p][r].SetWidth( m_sprActualBar[p][r].GetUnzoomedWidth() * stageStats.fRadarActual[p][r] );
			m_sprActualBar[p][r].Command( BAR_ACTUAL_ON_COMMAND(r,p) );
			if( stageStats.fRadarActual[p][r] == stageStats.fRadarPossible[p][r] )
				m_sprActualBar[p][r].Command( BAR_ACTUAL_MAX_COMMAND );
			this->AddChild( &m_sprActualBar[p][r] );
		}

		m_textSongsSurvived[p].LoadFromNumbers( GET_THEME_ELEMENT_FALLBACK("Numbers"," stage numbers") );
		m_textSongsSurvived[p].EnableShadow( false );
		m_textSongsSurvived[p].SetText( ssprintf("%02d", stageStats.iSongsPlayed[p]) );
		m_textSongsSurvived[p].Command( SONGS_SURVIVED_ON_COMMAND(p) );
		this->AddChild( &m_textSongsSurvived[p] );
	}
		
	//
	// init judgement area
	//
	int l;
	for( l=0; l<NUM_JUDGE_LINES; l++ ) 
	{
		m_sprJudgeLabels[l].Load( GET_THEME_ELEMENT_FALLBACK("Graphics"," judge labels 1x8") );
		m_sprJudgeLabels[l].StopAnimating();
		m_sprJudgeLabels[l].SetState( l );
		m_sprJudgeLabels[l].Command( JUDGE_LABEL_ON_COMMAND(l) );
		this->AddChild( &m_sprJudgeLabels[l] );
	}

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
			continue;	// skip

		for( l=0; l<NUM_JUDGE_LINES; l++ ) 
		{
			m_textJudgeNumbers[l][p].LoadFromNumbers( GET_THEME_ELEMENT_FALLBACK("Numbers"," judge numbers") );
			m_textJudgeNumbers[l][p].EnableShadow( false );
			m_textJudgeNumbers[l][p].SetDiffuse( PlayerToColor(p) );
			m_textJudgeNumbers[l][p].Command( JUDGE_NUMBER_ON_COMMAND(l,p) );
			this->AddChild( &m_textJudgeNumbers[l][p] );
		}

		m_textJudgeNumbers[0][p].SetText( ssprintf("%4d", stageStats.iTapNoteScores[p][TNS_MARVELOUS]) );
		m_textJudgeNumbers[1][p].SetText( ssprintf("%4d", stageStats.iTapNoteScores[p][TNS_PERFECT]) );
		m_textJudgeNumbers[2][p].SetText( ssprintf("%4d", stageStats.iTapNoteScores[p][TNS_GREAT]) );
		m_textJudgeNumbers[3][p].SetText( ssprintf("%4d", stageStats.iTapNoteScores[p][TNS_GOOD]) );
		m_textJudgeNumbers[4][p].SetText( ssprintf("%4d", stageStats.iTapNoteScores[p][TNS_BOO]) );
		m_textJudgeNumbers[5][p].SetText( ssprintf("%4d", stageStats.iTapNoteScores[p][TNS_MISS]) );
		m_textJudgeNumbers[6][p].SetText( ssprintf("%4d", stageStats.iHoldNoteScores[p][HNS_OK]) );
		m_textJudgeNumbers[7][p].SetText( ssprintf("%4d", stageStats.iMaxCombo[p]) );
	}


	//
	// init score area
	//
	int s;
	for( s=0; s<NUM_SCORE_LINES; s++ )
	{
		m_sprScoreLabel[s].Load( GET_THEME_ELEMENT_FALLBACK("Graphics"," score labels 1x2") );
		m_sprScoreLabel[s].SetState( s );
		m_sprScoreLabel[s].StopAnimating();
		m_sprScoreLabel[s].Command( SCORE_LABEL_ON_COMMAND(s) );
		this->AddChild( &m_sprScoreLabel[s] );
	}

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
			continue;	// skip

		for( int s=0; s<NUM_SCORE_LINES; s++ )
		{
			m_ScoreDisplay[s][p].Init( (PlayerNumber)p );
			m_ScoreDisplay[s][p].Command( SCORE_NUMBER_ON_COMMAND(s,p) );
			this->AddChild( &m_ScoreDisplay[s][p] );
		}

		m_ScoreDisplay[0][p].SetScore( stageStats.fScore[p] );
		m_ScoreDisplay[1][p].SetText( SecondsToTime(stageStats.fAliveSeconds[p]) );

		if( bNewRecord[p] )
		{
			m_sprNewRecord[p].Load( GET_THEME_ELEMENT_FALLBACK("Graphics"," new record") );
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
		m_sprTryExtraStage.Load( GET_THEME_ELEMENT_FALLBACK("Graphics",GAMESTATE->IsExtraStage()?" try extra2":" try extra1") );
		m_sprTryExtraStage.Command( TRY_EXTRA_STAGE_ON_COMMAND );
		this->AddChild( &m_sprTryExtraStage );

		if( GAMESTATE->IsExtraStage() )
			SOUNDMAN->PlayOnce( GET_THEME_ELEMENT_FALLBACK("Sounds"," try extra2") );
		else
			SOUNDMAN->PlayOnce( GET_THEME_ELEMENT_FALLBACK("Sounds"," try extra1") );
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


	SOUNDMAN->PlayMusic( GET_THEME_ELEMENT_FALLBACK("Sounds"," music") );
}

/*
void ScreenEvaluation::TweenOnScreen()
{
	int i, p;

//	m_Menu.TweenOnScreenFromBlack( SM_None );

	float fOriginalX, fOriginalY;

	for( i=0; i<MAX_SONGS_TO_SHOW; i++ )
	{
		fOriginalY = m_BannerWithFrame[i].GetY();
		m_BannerWithFrame[i].SetY( fOriginalY + SCREEN_HEIGHT );
		m_BannerWithFrame[i].BeginTweening( 0.0f );
		m_BannerWithFrame[i].BeginTweening( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_ACCELERATE );
		m_BannerWithFrame[i].SetTweenY( fOriginalY );
	}
	
	fOriginalY = m_sprStage.GetY();
	m_sprStage.BeginTweening( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_ACCELERATE );
	m_sprStage.SetTweenY( fOriginalY );

	for( p=0; p<NUM_PLAYERS; p++ ) 
		m_DifficultyIcon[p].FadeOn( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );

	for( i=0; i<NUM_JUDGE_LINES; i++ ) 
	{
		fOriginalY = m_sprJudgeLabels[i].GetY();
		m_sprJudgeLabels[i].SetY( fOriginalY + SCREEN_HEIGHT );
		m_sprJudgeLabels[i].BeginTweening( 0.2f + 0.1f*i );
		m_sprJudgeLabels[i].BeginTweening( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_ACCELERATE );
		m_sprJudgeLabels[i].SetTweenY( fOriginalY );

		for( int p=0; p<NUM_PLAYERS; p++ ) 
		{
			fOriginalX = m_textJudgeNumbers[i][p].GetX();
			m_textJudgeNumbers[i][p].SetX( fOriginalX + SCREEN_WIDTH/2*(p==PLAYER_1 ? 1 : -1) );
			m_textJudgeNumbers[i][p].BeginTweening( 0.2f + 0.1f*i );
			m_textJudgeNumbers[i][p].BeginTweening( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_ACCELERATE );
			m_textJudgeNumbers[i][p].SetTweenX( fOriginalX );
		}
	}

	//  Chris:  If EZ2 wants to hide these things, position them off screen using theme metrics

	fOriginalY = m_sprScoreLabel.GetY();
	m_sprScoreLabel.SetY( fOriginalY + SCREEN_HEIGHT );
	m_sprScoreLabel.BeginTweening( 0.8f + 0.1f*i );
	m_sprScoreLabel.BeginTweening( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_ACCELERATE );
	m_sprScoreLabel.SetTweenY( fOriginalY );
	
	for( p=0; p<NUM_PLAYERS; p++ ) 
	{
		fOriginalX = m_ScoreDisplay[p].GetX();
		m_ScoreDisplay[p].SetX( fOriginalX + SCREEN_WIDTH/2*(p==PLAYER_1 ? 1 : -1) );
		m_ScoreDisplay[p].BeginTweening( 0.8f + 0.1f*i );
		m_ScoreDisplay[p].BeginTweening( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_ACCELERATE );
		m_ScoreDisplay[p].SetTweenX( fOriginalX );
	}

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		unsigned i;

		vector<Actor*> apActorsInBonusOrStageInfo;
		apActorsInBonusOrStageInfo.push_back( &m_sprBonusFrame[p] );
		for( i=0; i<NUM_RADAR_CATEGORIES; i++ )
		{
			apActorsInBonusOrStageInfo.push_back( &m_sprPossibleBar[p][i] );
			apActorsInBonusOrStageInfo.push_back( &m_sprActualBar[p][i] );
		}
		apActorsInBonusOrStageInfo.push_back( &m_sprCourseFrame[p] );
		apActorsInBonusOrStageInfo.push_back( &m_textTime[p] );
		apActorsInBonusOrStageInfo.push_back( &m_textSongsSurvived[p] );
		for( i=0; i<apActorsInBonusOrStageInfo.size(); i++ )
		{
			fOriginalX = apActorsInBonusOrStageInfo[i]->GetX();
			apActorsInBonusOrStageInfo[i]->SetX( fOriginalX + SCREEN_WIDTH/2*(p==PLAYER_1 ? 1 : -1) );
			apActorsInBonusOrStageInfo[i]->BeginTweening( 0.2f );
			apActorsInBonusOrStageInfo[i]->BeginTweening( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_ACCELERATE );
			apActorsInBonusOrStageInfo[i]->SetTweenX( fOriginalX );
		}

		vector<Actor*> apActorsInGradeOrPercentFrame;
		apActorsInGradeOrPercentFrame.push_back( &m_sprBonusFrame[p] );
		apActorsInGradeOrPercentFrame.push_back( &m_sprGradeFrame[p] );
		apActorsInGradeOrPercentFrame.push_back( &m_Grades[p] );
		apActorsInGradeOrPercentFrame.push_back( &m_sprPercentFrame[p] );
		apActorsInGradeOrPercentFrame.push_back( &m_textPercentWhole[p] );
		apActorsInGradeOrPercentFrame.push_back( &m_textPercentRemainder[p] );
		apActorsInGradeOrPercentFrame.push_back( &m_sprNewRecord[p] );
		for( i=0; i<apActorsInGradeOrPercentFrame.size(); i++ )
		{
			float fOriginalZoomY = apActorsInGradeOrPercentFrame[i]->GetZoomY();
			apActorsInGradeOrPercentFrame[i]->SetZoomY( 0 );
			apActorsInGradeOrPercentFrame[i]->BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
			apActorsInGradeOrPercentFrame[i]->SetTweenZoomY( fOriginalZoomY );
		}
	}
	
	float fOriginalZoomY = m_sprTryExtraStage.GetZoomY();
	m_sprTryExtraStage.SetZoomY( 0 );
	m_sprTryExtraStage.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_sprTryExtraStage.SetTweenZoomY( fOriginalZoomY );
}
*/

void ScreenEvaluation::TweenOffScreen()
{
	int p;

	// banner area
	for( unsigned i=0; i<MAX_SONGS_TO_SHOW; i++ )
	{
		m_Banner[i].Command( BANNER_OFF_COMMAND(i) );
		m_sprBannerFrame[i].Command( BANNER_OFF_COMMAND(i) );
	}
	m_sprStage.Command( STAGE_OFF_COMMAND );
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_DifficultyIcon[p].Command( DIFFICULTY_ICON_OFF_COMMAND(p) );
		m_textPlayerOptions[p].Command( PLAYER_OPTIONS_OFF_COMMAND(p) );
	}

	// grade area
	for( p=0; p<NUM_PLAYERS; p++ ) 
	{
		m_sprGradeFrame[p].Command( GRADE_FRAME_OFF_COMMAND(p) );
		m_Grades[p].Command( GRADE_OFF_COMMAND(p) );
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
		m_textSongsSurvived[p].Command( SONGS_SURVIVED_OFF_COMMAND(p) );
	}
		
	// judgement area
	int l;
	for( l=0; l<NUM_JUDGE_LINES; l++ ) 
		m_sprJudgeLabels[l].Command( JUDGE_LABEL_OFF_COMMAND(l) );

	for( p=0; p<NUM_PLAYERS; p++ )
		for( l=0; l<NUM_JUDGE_LINES; l++ ) 
			m_textJudgeNumbers[l][p].Command( JUDGE_NUMBER_OFF_COMMAND(l,p) );


	// score area
	for( int s=0; s<NUM_SCORE_LINES; s++ )
		m_sprScoreLabel[s].Command( SCORE_LABEL_OFF_COMMAND(s) );
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		for( int s=0; s<NUM_SCORE_LINES; s++ )
			m_ScoreDisplay[s][p].Command( SCORE_NUMBER_OFF_COMMAND(s,p) );
		m_sprNewRecord[p].Command( NEW_RECORD_OFF_COMMAND(p) );
	}
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
	case SM_GoToSelectMusic:
	//	SCREENMAN->SetNewScreen( "ScreenSelectMusic" );
		SCREENMAN->SetNewScreen( SONGSEL_SCREEN );
		break;
	case SM_GoToSelectCourse:
		SCREENMAN->SetNewScreen( "ScreenSelectCourse" );
		break;
	case SM_GoToGameFinished:
	//	SCREENMAN->SetNewScreen( "ScreenNameEntry" );
		SCREENMAN->SetNewScreen( ENDGAME_SCREEN );
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

	if( PREFSMAN->m_bEventMode )
	{
		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_ARCADE:
			m_Menu.StartTransitioning( SM_GoToSelectMusic );
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
				m_Menu.StartTransitioning( SM_GoToSelectMusic );
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
				m_Menu.StartTransitioning( SM_GoToSelectMusic );
			}
			break;
		case summary:
			m_Menu.StartTransitioning( SM_GoToGameFinished );
			break;
		case course:
			m_Menu.StartTransitioning( SM_GoToGameFinished );
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

