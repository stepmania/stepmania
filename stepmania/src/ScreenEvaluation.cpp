#include "stdafx.h"
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


#define SONGSEL_SCREEN				THEME->GetMetric("ScreenEvaluation","SongSelectScreen")
#define BANNER_X				THEME->GetMetricF("ScreenEvaluation","BannerX")
#define BANNER_Y				THEME->GetMetricF("ScreenEvaluation","BannerY")
#define STAGE_X					THEME->GetMetricF("ScreenEvaluation","StageX")
#define STAGE_Y					THEME->GetMetricF("ScreenEvaluation","StageY")
#define DIFFICULTY_ICON_X( p )	THEME->GetMetricF("ScreenEvaluation",ssprintf("DifficultyIconP%dX",p+1))
#define DIFFICULTY_ICON_Y( p )	THEME->GetMetricF("ScreenEvaluation",ssprintf("DifficultyIconP%dY",p+1))
#define GRADE_X( p )			THEME->GetMetricF("ScreenEvaluation",ssprintf("GradeP%dX",p+1))
#define GRADE_Y					THEME->GetMetricF("ScreenEvaluation","GradeY")
#define PERCENT_BASE_X( p )		THEME->GetMetricF("ScreenEvaluation",ssprintf("PercentBaseP%dX",p+1))
#define PERCENT_BASE_Y			THEME->GetMetricF("ScreenEvaluation","PercentBaseY")
#define JUDGE_LABELS_X			THEME->GetMetricF("ScreenEvaluation","JudgeLabelsX")
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
		case 0:		return PERFECT_X(oni,p);
		case 1:		return GREAT_X(oni,p);
		case 2:		return GOOD_X(oni,p);
		case 3:		return BOO_X(oni,p);
		case 4:		return MISS_X(oni,p);
		case 5:		return OK_X(oni,p);
		case 6:		return MAX_COMBO_X(oni,p);
		default:	ASSERT(0);	return 0;
	}
}
float JUDGE_Y( int l ) {
	switch( l ) {
		case 0:		return PERFECT_Y;
		case 1:		return GREAT_Y;
		case 2:		return GOOD_Y;
		case 3:		return BOO_Y;
		case 4:		return MISS_Y;
		case 5:		return OK_Y;
		case 6:		return MAX_COMBO_Y;
		default:	ASSERT(0);	return 0;
	}
}

const ScreenMessage SM_GoToSelectMusic		=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToSelectCourse		=	ScreenMessage(SM_User+2);
const ScreenMessage SM_GoToFinalEvaluation	=	ScreenMessage(SM_User+3);
const ScreenMessage SM_GoToMusicScroll		=	ScreenMessage(SM_User+4);
const ScreenMessage SM_PlayCheer			=	ScreenMessage(SM_User+5);


ScreenEvaluation::ScreenEvaluation( bool bSummary )
{
	LOG->Trace( "ScreenEvaluation::ScreenEvaluation()" );
	
	int l, p;	// for counting


	///////////////////////////
	// Set m_ResultMode.  This enum will make our life easier later when we init different pieces depending on context.
	///////////////////////////
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
		m_ResultMode = bSummary ? RM_ARCADE_SUMMARY : RM_ARCADE_STAGE;
		break;
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		m_ResultMode = RM_ONI;
		break;
	default:
		ASSERT(0);
	}

	///////////////////////////
	// Figure out which statistics we're going to display
	///////////////////////////
	int		iPossibleDancePoints[NUM_PLAYERS];
	int		iActualDancePoints[NUM_PLAYERS];
	int		iTapNoteScores[NUM_PLAYERS][NUM_TAP_NOTE_SCORES];
	int		iHoldNoteScores[NUM_PLAYERS][NUM_HOLD_NOTE_SCORES];
	int		iMaxCombo[NUM_PLAYERS];
	float	fScore[NUM_PLAYERS];
	float	fPossibleRadarValues[NUM_PLAYERS][NUM_RADAR_CATEGORIES];
	float	fActualRadarValues[NUM_PLAYERS][NUM_RADAR_CATEGORIES];

	switch( m_ResultMode )
	{
	case RM_ARCADE_SUMMARY:
		COPY( iPossibleDancePoints,	GAMESTATE->m_iAccumPossibleDancePoints );
		COPY( iActualDancePoints,	GAMESTATE->m_iAccumActualDancePoints );
		COPY( iTapNoteScores,		GAMESTATE->m_AccumTapNoteScores );
		COPY( iHoldNoteScores,		GAMESTATE->m_AccumHoldNoteScores );
		COPY( iMaxCombo,			GAMESTATE->m_iAccumMaxCombo );
		COPY( fScore,				GAMESTATE->m_fAccumScore );
		COPY( fPossibleRadarValues,	GAMESTATE->m_fAccumRadarPossible );
		COPY( fActualRadarValues,	GAMESTATE->m_fAccumRadarActual );
		break;
	case RM_ARCADE_STAGE:
	case RM_ONI:
		COPY( iPossibleDancePoints,	GAMESTATE->m_iPossibleDancePoints );
		COPY( iActualDancePoints,	GAMESTATE->m_iActualDancePoints );
		COPY( iTapNoteScores,		GAMESTATE->m_TapNoteScores );
		COPY( iHoldNoteScores,		GAMESTATE->m_HoldNoteScores );
		COPY( iMaxCombo,			GAMESTATE->m_iMaxCombo );
		COPY( fScore,				GAMESTATE->m_fScore );
		COPY( fPossibleRadarValues,	GAMESTATE->m_fRadarPossible );
		COPY( fActualRadarValues,	GAMESTATE->m_fRadarActual );
		break;
	}


	///////////////////////////
	// Andy:
	// Fake COOL! / GOOD / OOPS for Ez2dancer using the DDR Rankings.
/*	Todo:  Accomodate this using theme metrics

	if( GAMESTATE->m_CurGame == GAME_EZ2 ) 
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			iTapNoteScores[p][TNS_PERFECT] += iTapNoteScores[p][TNS_GREAT];
			iTapNoteScores[p][TNS_GREAT] = 0;
			iTapNoteScores[p][TNS_MISS] += iTapNoteScores[p][TNS_BOO];
			iTapNoteScores[p][TNS_BOO] = 0;
		}
	}
*/

	///////////////////////////
	// Init the song banners depending on m_ResultMode
	///////////////////////////
	// EZ2 should hide these things by placing them off screen with theme metrics
	switch( m_ResultMode )
	{
	case RM_ARCADE_STAGE:
		{
			m_BannerWithFrame[0].LoadFromSong( GAMESTATE->m_pCurSong );
			m_BannerWithFrame[0].SetXY( BANNER_X, BANNER_Y );
			this->AddChild( &m_BannerWithFrame[0] );

			m_textStage.LoadFromFont( THEME->GetPathTo("Fonts","evaluation stage") );
			m_textStage.TurnShadowOff();
			m_textStage.SetXY( STAGE_X, STAGE_Y );
			m_textStage.SetZoom( 0.5f );
			m_textStage.SetText( GAMESTATE->GetStageText() + " Stage" );
			this->AddChild( &m_textStage );

			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( !GAMESTATE->IsPlayerEnabled(p) )
					continue;	// skip
				m_DifficultyIcon[p].Load( THEME->GetPathTo("graphics","select music difficulty icons 1x6") );
				m_DifficultyIcon[p].SetFromNotes( (PlayerNumber)p, GAMESTATE->m_pCurNotes[p] );
				m_DifficultyIcon[p].SetXY( DIFFICULTY_ICON_X(p), DIFFICULTY_ICON_Y(p) );
				this->AddChild( &m_DifficultyIcon[p] );
			}
		}
		break;
	case RM_ARCADE_SUMMARY:
		{
			// crop down to 3
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( GAMESTATE->m_apSongsPlayed.size() > STAGES_TO_SHOW_IN_SUMMARY )
					GAMESTATE->m_apSongsPlayed.erase( GAMESTATE->m_apSongsPlayed.begin(), GAMESTATE->m_apSongsPlayed.begin() + (GAMESTATE->m_apSongsPlayed.size() - STAGES_TO_SHOW_IN_SUMMARY) );
			}

			const unsigned iSongsToShow = GAMESTATE->m_apSongsPlayed.size();
			ASSERT( iSongsToShow > 0 );

			for( unsigned i=0; i<iSongsToShow; i++ )
			{
				m_BannerWithFrame[i].LoadFromSong( GAMESTATE->m_apSongsPlayed[i] );
				float fBannerOffset = i - (iSongsToShow-1)/2.0f;
				m_BannerWithFrame[i].SetXY( BANNER_X + fBannerOffset*32, BANNER_Y + fBannerOffset*16 );
				m_BannerWithFrame[i].SetZoom( 0.70f );
				this->AddChild( &m_BannerWithFrame[i] );
			}
		}
		break;
	case RM_ONI:
		m_BannerWithFrame[0].LoadFromCourse( GAMESTATE->m_pCurCourse );
		m_BannerWithFrame[0].SetXY( BANNER_X, BANNER_Y );
		this->AddChild( &m_BannerWithFrame[0] );
		break;
	}


	//////////////////////////
	// Init graphic elements
	//////////////////////////
	m_Menu.Load(
		THEME->GetPathTo("BGAnimations","evaluation"), 
		THEME->GetPathTo("Graphics",m_ResultMode==RM_ARCADE_SUMMARY?"evaluation summary top edge":"evaluation top edge"),
		HELP_TEXT, true, true, TIMER_SECONDS 
		);
	this->AddChild( &m_Menu );

	for( p=0; p<NUM_PLAYERS; p++ ) 
	{
		if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )	// If EZ2 wants to hide this graphic, place it somewhere off screen using theme metrics
			continue;	// skip

		m_ScoreDisplay[p].LoadFromNumbers( THEME->GetPathTo("Numbers","evaluation score numbers") );
		m_ScoreDisplay[p].SetXY( SCORE_NUMBERS_X(p), SCORE_Y );
		m_ScoreDisplay[p].SetZoomY( 1.0 );
		m_ScoreDisplay[p].SetDiffuse( PlayerToColor(p) );
		this->AddChild( &m_ScoreDisplay[p] );
	}



	//
	// Calculate grades
	//
	Grade grade[NUM_PLAYERS];
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p)  ||  GAMESTATE->m_fSecondsBeforeFail[p] != -1 )
		{
			grade[p] = GRADE_E;
			continue;
		}
		/* Based on the percentage of your total "Dance Points" to the maximum
		 * possible number, the following rank is assigned: 
		 *
		 * 100% - AAA
		 *  93% - AA
		 *  80% - A
		 *  65% - B
		 *  45% - C
		 * Less - D
		 * Fail - E
		 */
		float fPercentDancePoints = iActualDancePoints[p] / (float)iPossibleDancePoints[p];
		fPercentDancePoints = max( fPercentDancePoints, 0 );
		LOG->Trace( "iActualDancePoints: %i", iActualDancePoints[p] );
		LOG->Trace( "iPossibleDancePoints: %i", iPossibleDancePoints[p] );
		LOG->Trace( "fPercentDancePoints: %f", fPercentDancePoints  );

		if     ( fPercentDancePoints >= 1.00 )	grade[p] = GRADE_AAA;
		else if( fPercentDancePoints >= 0.93 )	grade[p] = GRADE_AA;
		else if( fPercentDancePoints >= 0.80 )	grade[p] = GRADE_A;
		else if( fPercentDancePoints >= 0.65 )	grade[p] = GRADE_B;
		else if( fPercentDancePoints >= 0.45 )	grade[p] = GRADE_C;
		else									grade[p] = GRADE_D;
	}

	Grade max_grade = GRADE_NO_DATA;
	for( p=0; p<NUM_PLAYERS; p++ )
		max_grade = max( max_grade, grade[p] ); 


	////////////////////////
	// update persistent statistics
	////////////////////////
	bool bNewRecord[NUM_PLAYERS];
	for( p=0; p<NUM_PLAYERS; p++ )
		bNewRecord[p] = false;

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;

		if( grade[p] == GRADE_E )
			continue;

		switch( m_ResultMode )
		{
		case RM_ARCADE_STAGE:

			Notes* pNotes = GAMESTATE->m_pCurNotes[p];
			pNotes->m_iNumTimesPlayed++;

			if( iMaxCombo[p] > pNotes->m_iMaxCombo )
				pNotes->m_iMaxCombo = iMaxCombo[p];

			if( fScore[p] > pNotes->m_iTopScore )
			{
				pNotes->m_iTopScore = (int)fScore[p];
				bNewRecord[p] = true;
			}

			if( grade[p] > pNotes->m_TopGrade )
				pNotes->m_TopGrade = grade[p];
			break;
		}
	}
	SONGMAN->SaveStatisticsToDisk();


	
	m_bTryExtraStage = false;
	if( (GAMESTATE->IsFinalStage() || GAMESTATE->IsExtraStage())  &&  m_ResultMode==RM_ARCADE_STAGE )
	{
		for( p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
				continue;	// skip

			if( GAMESTATE->m_pCurNotes[p]->m_Difficulty == DIFFICULTY_HARD  &&  grade[p] >= GRADE_AA )
				m_bTryExtraStage = true;
		}
	}
	if( PREFSMAN->m_bEventMode )
		m_bTryExtraStage = false;


	/////////////////////////////////
	// Init ResultMode-specific displays
	/////////////////////////////////
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
			continue;	// skip

		// If EZ2 shouldn't have a grade frame, then make a theme that has a 1x1 transparent graphic for the grade frame.

		switch( m_ResultMode )
		{
		case RM_ONI:
			{
				m_sprPercentFrame[p].Load( THEME->GetPathTo("Graphics","evaluation percent frame") );
				m_sprPercentFrame[p].StopAnimating();
				m_sprPercentFrame[p].SetState( p );
				m_sprPercentFrame[p].SetXY( GRADE_X(p), GRADE_Y );
				this->AddChild( &m_sprPercentFrame[p] );

				m_textOniPercentLarge[p].LoadFromNumbers( THEME->GetPathTo("Numbers","evaluation percent numbers") );
				m_textOniPercentLarge[p].TurnShadowOff();
				m_textOniPercentLarge[p].SetXY( PERCENT_BASE_X(p), PERCENT_BASE_Y );
				m_textOniPercentLarge[p].SetHorizAlign( Actor::align_right );
				m_textOniPercentLarge[p].SetVertAlign( Actor::align_bottom );
				m_textOniPercentLarge[p].SetEffectGlowing( 1.0f );
				this->AddChild( &m_textOniPercentLarge[p] );

				m_textOniPercentSmall[p].LoadFromNumbers( THEME->GetPathTo("Numbers","evaluation percent numbers") );
				m_textOniPercentSmall[p].TurnShadowOff();
				m_textOniPercentSmall[p].SetZoom( 0.5f );
				m_textOniPercentSmall[p].SetXY( PERCENT_BASE_X(p), PERCENT_BASE_Y );
				m_textOniPercentSmall[p].SetHorizAlign( Actor::align_left );
				m_textOniPercentSmall[p].SetVertAlign( Actor::align_bottom );
				m_textOniPercentSmall[p].SetEffectGlowing( 1.0f );
				this->AddChild( &m_textOniPercentSmall[p] );

				iPossibleDancePoints[p] = max( 1, iPossibleDancePoints[p] );
				float fPercentDancePoints =  iActualDancePoints[p] / (float)iPossibleDancePoints[p] + 0.0001f;	// correct for rounding errors
				fPercentDancePoints = max( fPercentDancePoints, 0 );
				int iPercentDancePointsLarge = int(fPercentDancePoints*100);
				int iPercentDancePointsSmall = int( (fPercentDancePoints*100 - int(fPercentDancePoints*100)) * 10 );
				m_textOniPercentLarge[p].SetText( ssprintf("%02d", iPercentDancePointsLarge) );
				m_textOniPercentSmall[p].SetText( ssprintf(".%01d%%", iPercentDancePointsSmall) );

				// StageInfo stuff
				m_sprCourseFrame[p].Load( THEME->GetPathTo("Graphics","evaluation stage frame 2x1") );
				m_sprCourseFrame[p].StopAnimating();
				m_sprCourseFrame[p].SetState( p );
				m_sprCourseFrame[p].SetXY( BONUS_X(p), BONUS_Y );
				this->AddChild( &m_sprCourseFrame[p] );
		
				m_textSongsSurvived[p].LoadFromNumbers( THEME->GetPathTo("Numbers","evaluation stage numbers") );
				m_textSongsSurvived[p].TurnShadowOff();
				m_textSongsSurvived[p].SetXY( SONGS_SURVIVED_X(p), SONGS_SURVIVED_Y );
				m_textSongsSurvived[p].SetText( ssprintf("%02d", GAMESTATE->m_iSongsBeforeFail[p]) );
				this->AddChild( &m_textSongsSurvived[p] );
			}
			break;
		case RM_ARCADE_STAGE:
		case RM_ARCADE_SUMMARY:
			{
				m_sprGradeFrame[p].Load( THEME->GetPathTo("Graphics","evaluation grade frame 1x2") );
				m_sprGradeFrame[p].StopAnimating();
				m_sprGradeFrame[p].SetState( p );
				m_sprGradeFrame[p].SetXY( GRADE_X(p), GRADE_Y );
				this->AddChild( &m_sprGradeFrame[p] );

				// Ez2dancer should control the grade tween using theme metrics
				m_Grades[p].SetXY( GRADE_X(p), GRADE_Y );
				m_Grades[p].SetZ( 2 );
				m_Grades[p].SetZoom( 1.0f );
				m_Grades[p].SetEffectGlowing( 5.0f, GRADES_GLOW_COLOR_1, GRADES_GLOW_COLOR_2 );
				if( SPIN_GRADES )
					m_Grades[p].SpinAndSettleOn( grade[p] );
				else
					m_Grades[p].SetGrade( (PlayerNumber)p, grade[p] );
				this->AddChild( &m_Grades[p] );

				// Bonus info frame
				m_sprBonusFrame[p].Load( THEME->GetPathTo("Graphics","evaluation bonus frame 2x1") );
				m_sprBonusFrame[p].StopAnimating();
				m_sprBonusFrame[p].SetState( p );
				m_sprBonusFrame[p].SetXY( BONUS_X(p), BONUS_Y );
				this->AddChild( &m_sprBonusFrame[p] );
	
				for( int l=0; l<NUM_RADAR_VALUES; l++ )	// foreach line
				{
					m_sprPossibleBar[p][l].Load( THEME->GetPathTo("Graphics","evaluation bars possible 1x2") );
					m_sprPossibleBar[p][l].SetState( p );
					m_sprPossibleBar[p][l].SetHorizAlign( Actor::align_left );
					m_sprPossibleBar[p][l].SetX( BAR_BASE_X(p) );
					m_sprPossibleBar[p][l].SetY( BAR_START_Y + BAR_SPACING_Y*l );
					m_sprPossibleBar[p][l].SetRotation( BAR_ROTATION(p) );
					m_sprPossibleBar[p][l].SetZoomX( 0 );
					m_sprPossibleBar[p][l].ZoomToHeight( BAR_HEIGHT );
					m_sprPossibleBar[p][l].BeginTweening( 0.2f + l*0.1f );	// sleep
					m_sprPossibleBar[p][l].BeginTweening( 0.5f );			// tween
					m_sprPossibleBar[p][l].SetTweenZoomToWidth( BAR_WIDTH*fPossibleRadarValues[p][l] );
					this->AddChild( &m_sprPossibleBar[p][l] );

					m_sprActualBar[p][l].Load( THEME->GetPathTo("Graphics","evaluation bars actual 1x2") );
					m_sprActualBar[p][l].SetState( p );
					m_sprActualBar[p][l].StopAnimating();
					m_sprActualBar[p][l].SetHorizAlign( Actor::align_left );
					m_sprActualBar[p][l].SetX( BAR_BASE_X(p) );
					m_sprActualBar[p][l].SetY( BAR_START_Y + BAR_SPACING_Y*l );
					m_sprActualBar[p][l].SetRotation( BAR_ROTATION(p) );
					m_sprActualBar[p][l].SetZoomX( 0 );
					m_sprActualBar[p][l].ZoomToHeight( BAR_HEIGHT );
					m_sprActualBar[p][l].BeginTweening( 1.0f + l*0.2f );	// sleep
					m_sprActualBar[p][l].BeginTweening( 1.0f );				// tween
					m_sprActualBar[p][l].SetTweenZoomToWidth( BAR_WIDTH*fActualRadarValues[p][l] );
					if( fActualRadarValues[p][l] == fPossibleRadarValues[p][l] )
						m_sprActualBar[p][l].SetEffectGlowing();
					this->AddChild( &m_sprActualBar[p][l] );
				}
				break;
			}
		}

		//	Chris:  If EZ2 wants to hide these things, place them off screen using theme metrics
		if( bNewRecord[p] )
		{
			m_sprNewRecord[p].Load( THEME->GetPathTo("Graphics","evaluation new record") );
			m_sprNewRecord[p].SetXY( NEW_RECORD_X(p), NEW_RECORD_Y );
			m_sprNewRecord[p].SetEffectGlowing( 1.0f );
			this->AddChild( &m_sprNewRecord[p] );
		}
	}
		
	//////////////////////////
	// Init non-ResultMode specific displays.
	// Do this after Result-specific displays so that the text will draw on top of 
	// the bonus frame.
	//////////////////////////
	for( l=0; l<NUM_JUDGE_LINES; l++ ) 
	{
		// EZ2 should hide these things by placing them off screen with theme metrics
		m_sprJudgeLabels[l].Load( THEME->GetPathTo("Graphics","evaluation judge labels 1x7") );
		m_sprJudgeLabels[l].StopAnimating();
		m_sprJudgeLabels[l].SetState( l );
		m_sprJudgeLabels[l].SetXY( JUDGE_LABELS_X, JUDGE_Y(l) );
		m_sprJudgeLabels[l].SetZoom( 1.0f );
		this->AddChild( &m_sprJudgeLabels[l] );
	}

	m_sprScoreLabel.Load( THEME->GetPathTo("Graphics","evaluation score labels") );
	m_sprScoreLabel.SetState( m_ResultMode==RM_ONI ? 1 : 0 );
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
			m_textJudgeNumbers[l][p].LoadFromNumbers( THEME->GetPathTo("Numbers","evaluation judge numbers") );
			m_textJudgeNumbers[l][p].TurnShadowOff();
			m_textJudgeNumbers[l][p].SetXY( JUDGE_X(m_ResultMode==RM_ONI,p,l), JUDGE_Y(l) );
			m_textJudgeNumbers[l][p].SetZoom( 1.0f );
			m_textJudgeNumbers[l][p].SetDiffuse( PlayerToColor(p) );
			this->AddChild( &m_textJudgeNumbers[l][p] );
		}

		m_textJudgeNumbers[0][p].SetText( ssprintf("%4d", iTapNoteScores[p][TNS_PERFECT]) );
		m_textJudgeNumbers[1][p].SetText( ssprintf("%4d", iTapNoteScores[p][TNS_GREAT]) );
		m_textJudgeNumbers[2][p].SetText( ssprintf("%4d", iTapNoteScores[p][TNS_GOOD]) );
		m_textJudgeNumbers[3][p].SetText( ssprintf("%4d", iTapNoteScores[p][TNS_BOO]) );
		m_textJudgeNumbers[4][p].SetText( ssprintf("%4d", iTapNoteScores[p][TNS_MISS]) );
		m_textJudgeNumbers[5][p].SetText( ssprintf("%4d", iHoldNoteScores[p][HNS_OK]) );
		m_textJudgeNumbers[6][p].SetText( ssprintf("%4d", iMaxCombo[p]) );


		if( m_ResultMode==RM_ONI )
			m_ScoreDisplay[p].SetText( SecondsToTime(GAMESTATE->GetPlayerSurviveTime( (PlayerNumber)p )) );
		else
			m_ScoreDisplay[p].SetScore( fScore[p] );
	}



	bool bOneHasNewRecord = false;
	for( p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsPlayerEnabled(p) && bNewRecord[p] )
			bOneHasNewRecord = true;
	
	if( m_bTryExtraStage )
	{
		m_sprTryExtraStage.Load( THEME->GetPathTo("Graphics",GAMESTATE->IsExtraStage()?"evaluation try extra stage2":"evaluation try extra stage1") );
		m_sprTryExtraStage.SetXY( TRY_EXTRA_STAGE_X, TRY_EXTRA_STAGE_Y );
		m_sprTryExtraStage.SetEffectGlowing( 1.0f );
		this->AddChild( &m_sprTryExtraStage );

		SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","evaluation extra stage") );
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
			switch( max_grade )
			{
			case GRADE_E:	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation e") );		break;
			case GRADE_D:	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation d") );		break;
			case GRADE_C:	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation c") );		break;
			case GRADE_B:	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation b") );		break;
			case GRADE_A:	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation a") );		break;
			case GRADE_AA:	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation aa") );	break;
			case GRADE_AAA:	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation aaa") );	break;
			case GRADE_NO_DATA:
			default:
				ASSERT(0);	// invalid grade
			}
			break;
		case RM_ONI:
		case RM_ARCADE_SUMMARY:
			switch( max_grade )
			{
			case GRADE_E:	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation final e") );	break;
			case GRADE_D:	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation final d") );	break;
			case GRADE_C:	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation final c") );	break;
			case GRADE_B:	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation final b") );	break;
			case GRADE_A:	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation final a") );	break;
			case GRADE_AA:	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation final aa") );	break;
			case GRADE_AAA:	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation final aaa") );	break;
			case GRADE_NO_DATA:
			default:
				ASSERT(0);	// invalid grade
			}
			break;
		default:
			ASSERT(0);
		}
	}

	switch( max_grade )
	{
	case GRADE_AA:
	case GRADE_AAA:	
		this->SendScreenMessage( SM_PlayCheer, 2.5f );	
		break;
	}

	if( bSummary )
		m_Menu.ImmedOnScreenFromMenu();
	else
		m_Menu.TweenOnScreenFromBlack( SM_None );

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","evaluation music") );
}


void ScreenEvaluation::TweenOnScreen()
{
	int i, p;

	m_Menu.TweenOnScreenFromBlack( SM_None );

	float fOriginalX, fOriginalY;

	for( i=0; i<STAGES_TO_SHOW_IN_SUMMARY; i++ )
	{
		fOriginalY = m_BannerWithFrame[i].GetY();
		m_BannerWithFrame[i].SetY( fOriginalY + SCREEN_HEIGHT );
		m_BannerWithFrame[i].BeginTweening( 0.0f );
		m_BannerWithFrame[i].BeginTweening( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_BIAS_BEGIN );
		m_BannerWithFrame[i].SetTweenY( fOriginalY );
	}
	
	fOriginalY = m_textStage.GetY();
	m_textStage.BeginTweening( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_BIAS_BEGIN );
	m_textStage.SetTweenY( fOriginalY );

	for( p=0; p<NUM_PLAYERS; p++ ) 
		m_DifficultyIcon[p].FadeOn( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );

	for( i=0; i<NUM_JUDGE_LINES; i++ ) 
	{
		fOriginalY = m_sprJudgeLabels[i].GetY();
		m_sprJudgeLabels[i].SetY( fOriginalY + SCREEN_HEIGHT );
		m_sprJudgeLabels[i].BeginTweening( 0.2f + 0.1f*i );
		m_sprJudgeLabels[i].BeginTweening( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_BIAS_BEGIN );
		m_sprJudgeLabels[i].SetTweenY( fOriginalY );

		for( int p=0; p<NUM_PLAYERS; p++ ) 
		{
			fOriginalX = m_textJudgeNumbers[i][p].GetX();
			m_textJudgeNumbers[i][p].SetX( fOriginalX + SCREEN_WIDTH/2*(p==PLAYER_1 ? 1 : -1) );
			m_textJudgeNumbers[i][p].BeginTweening( 0.2f + 0.1f*i );
			m_textJudgeNumbers[i][p].BeginTweening( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_BIAS_BEGIN );
			m_textJudgeNumbers[i][p].SetTweenX( fOriginalX );
		}
	}

	//  Chris:  If EZ2 wants to hide these things, position them off screen using theme metrics

	fOriginalY = m_sprScoreLabel.GetY();
	m_sprScoreLabel.SetY( fOriginalY + SCREEN_HEIGHT );
	m_sprScoreLabel.BeginTweening( 0.8f + 0.1f*i );
	m_sprScoreLabel.BeginTweening( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_BIAS_BEGIN );
	m_sprScoreLabel.SetTweenY( fOriginalY );
	
	for( p=0; p<NUM_PLAYERS; p++ ) 
	{
		fOriginalX = m_ScoreDisplay[p].GetX();
		m_ScoreDisplay[p].SetX( fOriginalX + SCREEN_WIDTH/2*(p==PLAYER_1 ? 1 : -1) );
		m_ScoreDisplay[p].BeginTweening( 0.8f + 0.1f*i );
		m_ScoreDisplay[p].BeginTweening( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_BIAS_BEGIN );
		m_ScoreDisplay[p].SetTweenX( fOriginalX );
	}

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		unsigned i;

		CArray<Actor*,Actor*> apActorsInBonusOrStageInfo;
		apActorsInBonusOrStageInfo.push_back( &m_sprBonusFrame[p] );
		for( i=0; i<NUM_RADAR_VALUES; i++ )
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
			apActorsInBonusOrStageInfo[i]->BeginTweening( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_BIAS_BEGIN );
			apActorsInBonusOrStageInfo[i]->SetTweenX( fOriginalX );
		}

		CArray<Actor*,Actor*> apActorsInGradeOrPercentFrame;
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

	for( i=0; i<STAGES_TO_SHOW_IN_SUMMARY; i++ )
		m_BannerWithFrame[i].FadeOff( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );

	m_textStage.FadeOff( 0, "foldy", MENU_ELEMENTS_TWEEN_TIME );

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

		for( i=0; i<NUM_RADAR_VALUES; i++ )
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

	if( m_Menu.IsClosing() )
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
	case SM_GoToMusicScroll:
		SCREENMAN->SetNewScreen( "ScreenMusicScroll" );
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
	m_Grades[pn].SettleQuickly();
}

void ScreenEvaluation::MenuRight( PlayerNumber pn )
{
	m_Grades[pn].SettleQuickly();
}

void ScreenEvaluation::MenuBack( PlayerNumber pn )
{
	MenuStart( pn );
}

void ScreenEvaluation::MenuStart( PlayerNumber pn )
{
	TweenOffScreen();

	m_Menu.StopTimer();

	if( PREFSMAN->m_bEventMode )
	{
		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_ARCADE:
			m_Menu.TweenOffScreenToMenu( SM_GoToSelectMusic );
			break;
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:
			m_Menu.TweenOffScreenToMenu( SM_GoToSelectCourse );
			break;
		default:
			ASSERT(0);
		}
	}
	else	// not event mode
	{
		if( m_bTryExtraStage )
			m_Menu.TweenOffScreenToMenu( SM_GoToSelectMusic );
		else if( m_ResultMode == RM_ARCADE_STAGE  &&  GAMESTATE->m_iCurrentStageIndex == PREFSMAN->m_iNumArcadeStages-1  )
		{
			/* Tween the screen out, but leave the MenuElements where they are.
			 * Play the "swoosh" sound manually (would normally be played by the ME
			 * tween out). */
			SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","menu swoosh") );
			TweenOffScreen();
			SCREENMAN->SendMessageToTopScreen( SM_GoToFinalEvaluation, MENU_ELEMENTS_TWEEN_TIME );
		}
		else if( m_ResultMode == RM_ARCADE_STAGE  &&  GAMESTATE->m_iCurrentStageIndex < PREFSMAN->m_iNumArcadeStages-1  )
			m_Menu.TweenOffScreenToMenu( SM_GoToSelectMusic );
		else
			m_Menu.TweenOffScreenToBlack( SM_GoToMusicScroll, false );
	}

	//
	// Increment the stage counter.
	//
	GAMESTATE->m_iCurrentStageIndex++;
}

