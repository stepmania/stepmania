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


#define SONGSEL_SCREEN			THEME->GetMetric("ScreenEvaluation","SongSelectScreen")
#define ENDGAME_SCREEN			THEME->GetMetric("ScreenEvaluation","EndGameScreen")
#define BANNER_X				THEME->GetMetricF("ScreenEvaluation","BannerX")
#define BANNER_Y				THEME->GetMetricF("ScreenEvaluation","BannerY")
#define STAGE_X					THEME->GetMetricF("ScreenEvaluation","StageX")
#define STAGE_Y					THEME->GetMetricF("ScreenEvaluation","StageY")
#define DIFFICULTY_ICON_X( p )	THEME->GetMetricF("ScreenEvaluation",ssprintf("DifficultyIconP%dX",p+1))
#define DIFFICULTY_ICON_Y( p )	THEME->GetMetricF("ScreenEvaluation",ssprintf("DifficultyIconP%dY",p+1))
#define GRADE_X( p )			THEME->GetMetricF("ScreenEvaluation",ssprintf("GradeP%dX",p+1))
#define GRADE_Y					THEME->GetMetricF("ScreenEvaluation","GradeY")
#define PERCENT_BASE_X( p )		THEME->GetMetricF("ScreenEvaluation",ssprintf("PercentBaseP%dX",p+1))
#define DANCE_POINT_X( p )		THEME->GetMetricF("ScreenEvaluation",ssprintf("DancePointP%dX",p+1))
#define DANCE_POINT_WIDTH		THEME->GetMetricF("ScreenEvaluation","DancePointDisplayWidth")
#define PERCENT_BASE_Y			THEME->GetMetricF("ScreenEvaluation","PercentBaseY")
#define JUDGE_LABELS_X			THEME->GetMetricF("ScreenEvaluation","JudgeLabelsX")
#define MARVELOUS_X(o,p)		THEME->GetMetricF("ScreenEvaluation",ssprintf("Marvelous%sP%dX",o?"Oni":"",p+1))
#define MARVELOUS_Y				THEME->GetMetricF("ScreenEvaluation","MarvelousY")
#define PERFECT_X(o,p)			THEME->GetMetricF("ScreenEvaluation",ssprintf("Perfect%sP%dX",o?"Oni":"",p+1))
#define PERFECT_Y				THEME->GetMetricF("ScreenEvaluation","PerfectY")
#define GREAT_X(o,p)			THEME->GetMetricF("ScreenEvaluation",ssprintf("Great%sP%dX",o?"Oni":"",p+1))
#define GREAT_Y					THEME->GetMetricF("ScreenEvaluation","GreatY")
#define GOOD_X(o,p)				THEME->GetMetricF("ScreenEvaluation",ssprintf("Good%sP%dX",o?"Oni":"",p+1))
#define GOOD_Y					THEME->GetMetricF("ScreenEvaluation","GoodY")
#define BOO_X(o,p)				THEME->GetMetricF("ScreenEvaluation",ssprintf("Boo%sP%dX",o?"Oni":"",p+1))
#define BOO_Y					THEME->GetMetricF("ScreenEvaluation","BooY")
#define MISS_X(o,p)				THEME->GetMetricF("ScreenEvaluation",ssprintf("Miss%sP%dX",o?"Oni":"",p+1))
#define MISS_Y					THEME->GetMetricF("ScreenEvaluation","MissY")
#define OK_X(o,p)				THEME->GetMetricF("ScreenEvaluation",ssprintf("OK%sP%dX",o?"Oni":"",p+1))
#define OK_Y					THEME->GetMetricF("ScreenEvaluation","OKY")
#define MAX_COMBO_X(o,p)		THEME->GetMetricF("ScreenEvaluation",ssprintf("MaxCombo%sP%dX",o?"Oni":"",p+1))
#define MAX_COMBO_Y				THEME->GetMetricF("ScreenEvaluation","MaxComboY")
#define SCORE_LABELS_X			THEME->GetMetricF("ScreenEvaluation","ScoreLabelsX")
#define SCORE_NUMBERS_X( p )	THEME->GetMetricF("ScreenEvaluation",ssprintf("ScoreNumbersP%dX",p+1))
#define SCORE_Y					THEME->GetMetricF("ScreenEvaluation","ScoreY")
#define BONUS_X( p )			THEME->GetMetricF("ScreenEvaluation",ssprintf("BonusP%dX",p+1))
#define BONUS_Y					THEME->GetMetricF("ScreenEvaluation","BonusY")
#define BAR_BASE_X( p )			THEME->GetMetricF("ScreenEvaluation",ssprintf("BarP%dBaseX",p+1))
#define BAR_ROTATION( p )		THEME->GetMetricF("ScreenEvaluation",ssprintf("BarP%dRotation",p+1))
#define BAR_START_Y				THEME->GetMetricF("ScreenEvaluation","BarStartY")
#define BAR_SPACING_Y			THEME->GetMetricF("ScreenEvaluation","BarSpacingY")
#define BAR_WIDTH				THEME->GetMetricF("ScreenEvaluation","BarWidth")
#define BAR_HEIGHT				THEME->GetMetricF("ScreenEvaluation","BarHeight")
#define SONGS_SURVIVED_X( p )	THEME->GetMetricF("ScreenEvaluation",ssprintf("SongsSurvivedP%dX",p+1))
#define SONGS_SURVIVED_Y		THEME->GetMetricF("ScreenEvaluation","SongsSurvivedY")
#define NEW_RECORD_X( p )		THEME->GetMetricF("ScreenEvaluation",ssprintf("NewRecordP%dX",p+1))
#define NEW_RECORD_Y			THEME->GetMetricF("ScreenEvaluation","NewRecordY")
#define TRY_EXTRA_STAGE_X		THEME->GetMetricF("ScreenEvaluation","TryExtraStageX")
#define TRY_EXTRA_STAGE_Y		THEME->GetMetricF("ScreenEvaluation","TryExtraStageY")
#define HELP_TEXT				THEME->GetMetric("ScreenEvaluation","HelpText")
#define TIMER_SECONDS			THEME->GetMetricI("ScreenEvaluation","TimerSeconds")
#define SPIN_GRADES				THEME->GetMetricB("ScreenEvaluation","SpinGrades")
#define GRADES_GLOW_COLOR_1		THEME->GetMetricC("ScreenEvaluation","GradesGlowColor1")
#define GRADES_GLOW_COLOR_2		THEME->GetMetricC("ScreenEvaluation","GradesGlowColor2")

float JUDGE_X( bool oni, int p, int l ) {
	switch( l ) {
		case 0:		return MARVELOUS_X(oni,p);
		case 1:		return PERFECT_X(oni,p);
		case 2:		return GREAT_X(oni,p);
		case 3:		return GOOD_X(oni,p);
		case 4:		return BOO_X(oni,p);
		case 5:		return MISS_X(oni,p);
		case 6:		return OK_X(oni,p);
		case 7:		return MAX_COMBO_X(oni,p);
		default:	ASSERT(0);	return 0;
	}
}
float JUDGE_Y( int l ) {
	switch( l ) {
		case 0:		return MARVELOUS_Y;
		case 1:		return PERFECT_Y;
		case 2:		return GREAT_Y;
		case 3:		return GOOD_Y;
		case 4:		return BOO_Y;
		case 5:		return MISS_Y;
		case 6:		return OK_Y;
		case 7:		return MAX_COMBO_Y;
		default:	ASSERT(0);	return 0;
	}
}

const ScreenMessage SM_GoToSelectMusic		=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToSelectCourse		=	ScreenMessage(SM_User+3);
const ScreenMessage SM_GoToFinalEvaluation	=	ScreenMessage(SM_User+4);
const ScreenMessage SM_GoToGameFinished		=	ScreenMessage(SM_User+5);
const ScreenMessage SM_PlayCheer			=	ScreenMessage(SM_User+6);


ScreenEvaluation::ScreenEvaluation( bool bSummary )
{
	LOG->Trace( "ScreenEvaluation::ScreenEvaluation()" );
	
	//
	// Set m_ResultMode.  This enum will make our life easier later when we init different pieces.
	//
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
	case PLAY_MODE_BATTLE:
		m_ResultMode = bSummary ? RM_ARCADE_SUMMARY : RM_ARCADE_STAGE;
		break;
	case PLAY_MODE_NONSTOP:
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		m_ResultMode = RM_COURSE;
		break;
	default:
		ASSERT(0);
	}

	//
	// Figure out which statistics and songs we're going to display
	//
	StageStats stageStats;
	vector<Song*> vSongsToShow;
	switch( m_ResultMode )
	{
	case RM_ARCADE_SUMMARY:
		GAMESTATE->GetFinalEvalStatsAndSongs( stageStats, vSongsToShow );
		break;
	case RM_ARCADE_STAGE:
	case RM_COURSE:
		stageStats = GAMESTATE->m_CurStageStats;
		break;
	default:
		ASSERT(0);
	}


	//
	// Init the song banners depending on m_ResultMode
	//
	switch( m_ResultMode )
	{
	case RM_ARCADE_STAGE:
		{
			m_BannerWithFrame[0].LoadFromSong( GAMESTATE->m_pCurSong );
			m_BannerWithFrame[0].SetXY( BANNER_X, BANNER_Y );
			this->AddChild( &m_BannerWithFrame[0] );

			m_sprStage.Load( THEME->GetPathTo("Graphics","ScreenEvaluation stage "+GAMESTATE->GetStageText()) );
			m_sprStage.SetXY( STAGE_X, STAGE_Y );
			this->AddChild( &m_sprStage );

			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( !GAMESTATE->IsPlayerEnabled(p) )
					continue;	// skip
				m_DifficultyIcon[p].Load( THEME->GetPathTo("graphics","ScreenEvaluation difficulty icons 1x5") );
				m_DifficultyIcon[p].SetFromNotes( (PlayerNumber)p, GAMESTATE->m_pCurNotes[p] );
				m_DifficultyIcon[p].SetXY( DIFFICULTY_ICON_X(p), DIFFICULTY_ICON_Y(p) );
				this->AddChild( &m_DifficultyIcon[p] );
			}
		}
		break;
	case RM_ARCADE_SUMMARY:
		{
			for( unsigned i=0; i<vSongsToShow.size(); i++ )
			{
				m_BannerWithFrame[i].LoadFromSong( vSongsToShow[i] );
				float fBannerOffset = i - (vSongsToShow.size()-1)/2.0f;
				m_BannerWithFrame[i].SetXY( BANNER_X + fBannerOffset*32, BANNER_Y + fBannerOffset*16 );
				m_BannerWithFrame[i].SetZoom( 0.70f );
				this->AddChild( &m_BannerWithFrame[i] );
			}
		}
		break;
	case RM_COURSE:
		m_BannerWithFrame[0].LoadFromCourse( GAMESTATE->m_pCurCourse );
		m_BannerWithFrame[0].SetXY( BANNER_X, BANNER_Y );
		this->AddChild( &m_BannerWithFrame[0] );
		break;
	default:
		ASSERT(0);
	}


	//////////////////////////
	// Init graphic elements
	//////////////////////////
	m_Menu.Load( "ScreenEvaluation" );
// FIXME:
//	The header needs to be different depending on the result mode.  
//  We should really split these different ResultModes into separate derived classes
//	so that each can have its own Transitions, headers, and such.
//		THEME->GetPathTo("Graphics",m_ResultMode==RM_ARCADE_SUMMARY?"evaluation summary top edge":"evaluation top edge"),
	this->AddChild( &m_Menu );

	int p;
	for( p=0; p<NUM_PLAYERS; p++ ) 
	{
		if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )	// If EZ2 wants to hide this graphic, place it somewhere off screen using theme metrics
			continue;	// skip

		m_ScoreDisplay[p].SetXY( SCORE_NUMBERS_X(p), SCORE_Y );
		m_ScoreDisplay[p].Init( (PlayerNumber)p );
		m_ScoreDisplay[p].SetZoomY( 1.0 );
		this->AddChild( &m_ScoreDisplay[p] );
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
	bool bNewRecord[NUM_PLAYERS] = { false, false };


	switch( m_ResultMode )
	{
	case RM_ARCADE_STAGE:
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsPlayerEnabled(p) )
					GAMESTATE->m_pCurNotes[p]->AddScore( (PlayerNumber)p, grade[p], stageStats.fScore[p], bNewRecord[p] );
		}
		break;
	case RM_ARCADE_SUMMARY:
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
	case RM_COURSE:
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
		m_ResultMode==RM_ARCADE_STAGE;
 
	//
	// Init ResultMode-specific displays
	//
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
			continue;	// skip

		// If EZ2 shouldn't have a grade frame, then make a theme that has a 1x1 transparent graphic for the grade frame.

		switch( m_ResultMode )
		{
		case RM_COURSE:
			{
				m_sprPercentFrame[p].Load( THEME->GetPathTo("Graphics","ScreenEvaluation percent frame") );
				m_sprPercentFrame[p].StopAnimating();
				m_sprPercentFrame[p].SetState( p );
				m_sprPercentFrame[p].SetXY( GRADE_X(p), GRADE_Y );
				this->AddChild( &m_sprPercentFrame[p] );

				m_textOniPercentLarge[p].LoadFromNumbers( THEME->GetPathTo("Numbers","ScreenEvaluation percent numbers") );
				m_textOniPercentLarge[p].EnableShadow( false );
				m_textOniPercentLarge[p].SetXY( (PREFSMAN->m_bDancePointsForOni ? DANCE_POINT_X(p) : PERCENT_BASE_X(p) ), PERCENT_BASE_Y );
				m_textOniPercentLarge[p].SetHorizAlign( Actor::align_right );
				m_textOniPercentLarge[p].SetVertAlign( Actor::align_bottom );
				m_textOniPercentLarge[p].SetEffectGlowShift( 2.5f );
				this->AddChild( &m_textOniPercentLarge[p] );

				if(!PREFSMAN->m_bDancePointsForOni)
				{
					m_textOniPercentSmall[p].LoadFromNumbers( THEME->GetPathTo("Numbers","ScreenEvaluation percent numbers") );
					m_textOniPercentSmall[p].EnableShadow( false );
					m_textOniPercentSmall[p].SetZoom( 0.5f );
					m_textOniPercentSmall[p].SetXY( PERCENT_BASE_X(p), PERCENT_BASE_Y );
					m_textOniPercentSmall[p].SetHorizAlign( Actor::align_left );
					m_textOniPercentSmall[p].SetVertAlign( Actor::align_bottom );
					m_textOniPercentSmall[p].SetEffectGlowShift( 2.5f );
					this->AddChild( &m_textOniPercentSmall[p] );
				}

				if(PREFSMAN->m_bDancePointsForOni)
				{
					m_textOniPercentLarge[p].SetText( ssprintf("%d", stageStats.iActualDancePoints[p]) );

					const float fDancePointWidth = DANCE_POINT_WIDTH;
					const float WidestLineWidth = (float) m_textOniPercentLarge[p].GetWidestLineWidthInSourcePixels();
					if(WidestLineWidth > fDancePointWidth)
						m_textOniPercentLarge[p].SetZoomX( fDancePointWidth / WidestLineWidth );
				}
				else
				{
					stageStats.iPossibleDancePoints[p] = max( 1, stageStats.iPossibleDancePoints[p] );
					float fPercentDancePoints =  stageStats.iActualDancePoints[p] / (float)stageStats.iPossibleDancePoints[p] + 0.0001f;	// correct for rounding errors
					fPercentDancePoints = max( fPercentDancePoints, 0 );
					int iPercentDancePointsLarge = int(fPercentDancePoints*100);
					int iPercentDancePointsSmall = int( (fPercentDancePoints*100 - int(fPercentDancePoints*100)) * 10 );
					m_textOniPercentLarge[p].SetText( ssprintf("%02d", iPercentDancePointsLarge) );
					m_textOniPercentSmall[p].SetText( ssprintf(".%01d%%", iPercentDancePointsSmall) );
				}

				// StageInfo stuff
				m_sprCourseFrame[p].Load( THEME->GetPathTo("Graphics","ScreenEvaluation stage frame 2x1") );
				m_sprCourseFrame[p].StopAnimating();
				m_sprCourseFrame[p].SetState( p );
				m_sprCourseFrame[p].SetXY( BONUS_X(p), BONUS_Y );
				this->AddChild( &m_sprCourseFrame[p] );
		
				m_textSongsSurvived[p].LoadFromNumbers( THEME->GetPathTo("Numbers","ScreenEvaluation stage numbers") );
				m_textSongsSurvived[p].EnableShadow( false );
				m_textSongsSurvived[p].SetXY( SONGS_SURVIVED_X(p), SONGS_SURVIVED_Y );
				m_textSongsSurvived[p].SetText( ssprintf("%02d", stageStats.iSongsPlayed[p]) );
				this->AddChild( &m_textSongsSurvived[p] );
			}
			break;
		case RM_ARCADE_STAGE:
		case RM_ARCADE_SUMMARY:
			{
				m_sprGradeFrame[p].Load( THEME->GetPathTo("Graphics","ScreenEvaluation grade frame 1x2") );
				m_sprGradeFrame[p].StopAnimating();
				m_sprGradeFrame[p].SetState( p );
				m_sprGradeFrame[p].SetXY( GRADE_X(p), GRADE_Y );
				this->AddChild( &m_sprGradeFrame[p] );

				// Ez2dancer should control the grade tween using theme metrics
				m_Grades[p].SetXY( GRADE_X(p), GRADE_Y );
				m_Grades[p].SetZ( 2 );
				m_Grades[p].SetZoom( 1.0f );
				m_Grades[p].SetEffectGlowShift( 1.0f, GRADES_GLOW_COLOR_1, GRADES_GLOW_COLOR_2 );
				if( SPIN_GRADES )
					m_Grades[p].SpinAndSettleOn( grade[p] );
				else
					m_Grades[p].SetGrade( (PlayerNumber)p, grade[p] );
				this->AddChild( &m_Grades[p] );

				// Bonus info frame
				m_sprBonusFrame[p].Load( THEME->GetPathTo("Graphics","ScreenEvaluation bonus frame 2x1") );
				m_sprBonusFrame[p].StopAnimating();
				m_sprBonusFrame[p].SetState( p );
				m_sprBonusFrame[p].SetXY( BONUS_X(p), BONUS_Y );
				this->AddChild( &m_sprBonusFrame[p] );
	
				for( int r=0; r<NUM_RADAR_CATEGORIES; r++ )	// foreach line
				{
					m_sprPossibleBar[p][r].Load( THEME->GetPathTo("Graphics","ScreenEvaluation bars possible 1x2") );
					m_sprPossibleBar[p][r].StopAnimating();
					m_sprPossibleBar[p][r].SetState( p );
					m_sprPossibleBar[p][r].SetHorizAlign( Actor::align_left );
					m_sprPossibleBar[p][r].SetX( BAR_BASE_X(p) );
					m_sprPossibleBar[p][r].SetY( BAR_START_Y + BAR_SPACING_Y*r );
					m_sprPossibleBar[p][r].SetRotationZ( BAR_ROTATION(p) );
					m_sprPossibleBar[p][r].SetZoomX( 0 );
					m_sprPossibleBar[p][r].ZoomToHeight( BAR_HEIGHT );
					m_sprPossibleBar[p][r].BeginTweening( 0.2f + r*0.1f );	// sleep
					m_sprPossibleBar[p][r].BeginTweening( 0.5f );			// tween
					m_sprPossibleBar[p][r].SetTweenZoomToWidth( BAR_WIDTH*stageStats.fRadarPossible[p][r] );
					this->AddChild( &m_sprPossibleBar[p][r] );

					m_sprActualBar[p][r].Load( THEME->GetPathTo("Graphics","ScreenEvaluation bars actual 1x2") );
					m_sprActualBar[p][r].StopAnimating();
					m_sprActualBar[p][r].SetState( p );
					m_sprActualBar[p][r].SetHorizAlign( Actor::align_left );
					m_sprActualBar[p][r].SetX( BAR_BASE_X(p) );
					m_sprActualBar[p][r].SetY( BAR_START_Y + BAR_SPACING_Y*r );
					m_sprActualBar[p][r].SetRotationZ( BAR_ROTATION(p) );
					m_sprActualBar[p][r].SetZoomX( 0 );
					m_sprActualBar[p][r].ZoomToHeight( BAR_HEIGHT );
					m_sprActualBar[p][r].BeginTweening( 1.0f + r*0.2f );	// sleep
					m_sprActualBar[p][r].BeginTweening( 1.0f );				// tween
					m_sprActualBar[p][r].SetTweenZoomToWidth( BAR_WIDTH*stageStats.fRadarActual[p][r] );
					if( stageStats.fRadarActual[p][r] == stageStats.fRadarPossible[p][r] )
						m_sprActualBar[p][r].SetEffectGlowShift();
					this->AddChild( &m_sprActualBar[p][r] );
				}
				break;
			}
		}

		if( bNewRecord[p] )
		{
			m_sprNewRecord[p].Load( THEME->GetPathTo("Graphics","ScreenEvaluation new record") );
			m_sprNewRecord[p].SetXY( NEW_RECORD_X(p), NEW_RECORD_Y );
			m_sprNewRecord[p].SetEffectGlowShift( 2.5f );
			this->AddChild( &m_sprNewRecord[p] );
		}
	}
		
	//
	// Init non-ResultMode specific displays.
	// Do this after Result-specific displays so that the text will draw on top of 
	// the bonus frame.
	//
	int l;
	for( l=0; l<NUM_JUDGE_LINES; l++ ) 
	{
		// EZ2 should hide these things by placing them off screen with theme metrics
		m_sprJudgeLabels[l].Load( THEME->GetPathTo("Graphics","ScreenEvaluation judge labels 1x8") );
		m_sprJudgeLabels[l].StopAnimating();
		m_sprJudgeLabels[l].SetState( l );
		m_sprJudgeLabels[l].SetXY( JUDGE_LABELS_X, JUDGE_Y(l) );
		m_sprJudgeLabels[l].SetZoom( 1.0f );
		this->AddChild( &m_sprJudgeLabels[l] );
	}

	m_sprScoreLabel.Load( THEME->GetPathTo("Graphics","ScreenEvaluation score labels") );
	m_sprScoreLabel.SetState( m_ResultMode==RM_COURSE ? 1 : 0 );
	m_sprScoreLabel.StopAnimating();
	m_sprScoreLabel.SetXY( SCORE_LABELS_X, SCORE_Y );
	m_sprScoreLabel.SetZoom( 1.0f );
	this->AddChild( &m_sprScoreLabel );


	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
			continue;	// skip

		for( l=0; l<NUM_JUDGE_LINES; l++ ) 
		{
			m_textJudgeNumbers[l][p].LoadFromNumbers( THEME->GetPathTo("Numbers","ScreenEvaluation judge numbers") );
			m_textJudgeNumbers[l][p].EnableShadow( false );
			m_textJudgeNumbers[l][p].SetXY( JUDGE_X(m_ResultMode==RM_COURSE,p,l), JUDGE_Y(l) );
			m_textJudgeNumbers[l][p].SetZoom( 1.0f );
			m_textJudgeNumbers[l][p].SetDiffuse( PlayerToColor(p) );
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


		if( m_ResultMode==RM_COURSE )
			m_ScoreDisplay[p].SetText( SecondsToTime(stageStats.fAliveSeconds[p]) );
		else
			m_ScoreDisplay[p].SetScore( stageStats.fScore[p] );
	}



	bool bOneHasNewRecord = false;
	for( p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsPlayerEnabled(p) && bNewRecord[p] )
			bOneHasNewRecord = true;
	
	if( m_bTryExtraStage )
	{
		m_sprTryExtraStage.Load( THEME->GetPathTo("Graphics",GAMESTATE->IsExtraStage()?"ScreenEvaluation extra2":"ScreenEvaluation extra1") );
		m_sprTryExtraStage.SetXY( TRY_EXTRA_STAGE_X, TRY_EXTRA_STAGE_Y );
		m_sprTryExtraStage.SetEffectGlowShift( 2.5f );
		this->AddChild( &m_sprTryExtraStage );

		SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","ScreenEvaluation extra stage") );
	}
	else if( bOneHasNewRecord  &&  ANNOUNCER->HasSoundsFor("evaluation new record") )
	{
		SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation new record") );
	}
	else
	{
		
		switch( m_ResultMode )
		{
		case RM_ARCADE_STAGE:
			SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation "+GradeToString(max_grade)) );
			break;
		case RM_COURSE:
		case RM_ARCADE_SUMMARY:
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
		this->SendScreenMessage( SM_PlayCheer, 2.5f );	
		break;
	}

//	if( bSummary )
//		m_Menu.ImmedOnScreenFromMenu();
//	else
//		m_Menu.TweenOnScreenFromBlack( SM_None );

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","ScreenEvaluation music") );
}


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
		apActorsInGradeOrPercentFrame.push_back( &m_textOniPercentLarge[p] );
		apActorsInGradeOrPercentFrame.push_back( &m_textOniPercentSmall[p] );
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

void ScreenEvaluation::TweenOffScreen()
{
	int i, p;

	for( i=0; i<MAX_SONGS_TO_SHOW; i++ )
		m_BannerWithFrame[i].FadeOff( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );

	m_sprStage.FadeOff( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );

	for( p=0; p<NUM_PLAYERS; p++ ) 
		m_DifficultyIcon[p].FadeOff( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );

	for( i=0; i<NUM_JUDGE_LINES; i++ ) 
	{
		m_sprJudgeLabels[i].FadeOff( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );

		for( int p=0; p<NUM_PLAYERS; p++ ) 
			m_textJudgeNumbers[i][p].FadeOff( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );
	}
	
	m_sprScoreLabel.FadeOff( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );


	for( p=0; p<NUM_PLAYERS; p++ ) 
		m_ScoreDisplay[p].FadeOff( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		int i;

		m_Grades[p].SettleImmediately();

		for( i=0; i<NUM_RADAR_CATEGORIES; i++ )
		{
			m_sprPossibleBar[p][i].FadeOff( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );
			m_sprActualBar[p][i].FadeOff( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );
		}

		m_sprCourseFrame[p].FadeOff( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );
		m_textTime[p].FadeOff( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );
		m_textSongsSurvived[p].FadeOff( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );


		m_sprBonusFrame[p].FadeOff( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );
		m_sprGradeFrame[p].FadeOff( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );
		m_Grades[p].FadeOff( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );
		m_sprPercentFrame[p].FadeOff( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );
		m_textOniPercentLarge[p].FadeOff( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );
		m_textOniPercentSmall[p].FadeOff( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );
		m_sprNewRecord[p].FadeOff( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );
	}
	
	m_sprTryExtraStage.FadeOff( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );
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
	case SM_GoToFinalEvaluation:
		SCREENMAN->SetNewScreen( "ScreenFinalEvaluation" );
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
		switch( m_ResultMode )
		{
		case RM_ARCADE_STAGE:
			if( m_bTryExtraStage )
			{
				m_Menu.StartTransitioning( SM_GoToSelectMusic );
			}
			else if( GAMESTATE->IsFinalStage() || GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
			{
				/* Tween the screen out, but leave the MenuElements where they are.
					* Play the "swoosh" sound manually (would normally be played by the ME
					* tween out). */
				SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","menu swoosh") );
				TweenOffScreen();
				SCREENMAN->SendMessageToTopScreen( SM_GoToFinalEvaluation, MENU_ELEMENTS_TWEEN_TIME );
			}
			else
			{
				m_Menu.StartTransitioning( SM_GoToSelectMusic );
			}
			break;
		case RM_ARCADE_SUMMARY:
			m_Menu.StartTransitioning( SM_GoToGameFinished );
			break;
		case RM_COURSE:
			m_Menu.StartTransitioning( SM_GoToGameFinished );
			break;
		}
	}

	switch( m_ResultMode )
	{
	case RM_ARCADE_STAGE:
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

