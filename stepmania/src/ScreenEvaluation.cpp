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
#include "ScreenSelectMusic.h"
#include "SongManager.h"
#include "ScreenManager.h"
#include "GameManager.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "Notes.h"
#include "ThemeManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "ScreenMusicScroll.h"


const float BANNER_X					= CENTER_X;
const float BANNER_Y					= SCREEN_TOP + 100;

const float GRADE_X[NUM_PLAYERS]		= { CENTER_X-226, CENTER_X+226 };
const float GRADE_Y						=	BANNER_Y;

const float JUDGE_LABELS_X				= CENTER_X;
const float JUDGE_NUMBERS_X[NUM_PLAYERS]= { CENTER_X-88, CENTER_X+88 };
const float JUDGE_START_Y				=	CENTER_Y - 70;
const float JUDGE_SPACING				=	32;

const float SCORE_LABEL_X				= CENTER_X;
const float SCORE_DISPLAY_X[NUM_PLAYERS]= { CENTER_X-170, CENTER_X+170 };
const float SCORE_Y						=	CENTER_Y + 140;

const float BONUS_FRAME_X[NUM_PLAYERS]	= { CENTER_X-220, CENTER_X+220 };
const float BONUS_FRAME_Y				=	CENTER_Y+10;

const float NEW_RECORD_X[NUM_PLAYERS]	= { SCORE_DISPLAY_X[0], SCORE_DISPLAY_X[1] };
const float NEW_RECORD_Y				=	SCORE_Y - 20;

const float TRY_EXTRA_STAGE_X			= CENTER_X;
const float TRY_EXTRA_STAGE_Y			= SCREEN_BOTTOM - 60;


const ScreenMessage SM_GoToNextState		=	ScreenMessage(SM_User+1);


ScreenEvaluation::ScreenEvaluation( bool bSummary )
{
	LOG->WriteLine( "ScreenEvaluation::ScreenEvaluation()" );
	
	int l, p;	// for counting


	///////////////////////////
	// Set m_ResultMode.  This enum will make our life easier later when we init different pieces depending on context.
	///////////////////////////
	switch( PREFSMAN->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
		m_ResultMode = bSummary ? RM_ARCADE_SUMMARY : RM_ARCADE_STAGE;
		break;
	case PLAY_MODE_ONI:
		m_ResultMode = RM_ONI;
		break;
	default:
		ASSERT(0);
	}

/*
	//////////////////////////
	// Fill in some fake information for debugging
	//////////////////////////
	m_ResultMode = RM_ARCADE_SUMMARY;
	GAMEMAN->m_CurStyle = STYLE_DANCE_SINGLE;
	SONGMAN->SetCurrentSong( SONGMAN->m_pSongs[0] );
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		SONGMAN->SetCurrentNotes((PlayerNumber)p, SONGMAN->GetCurrentSong()->m_arrayNotes[0]);
		{
		SONGMAN->m_aGameplayStatistics[p].Add( GameplayStatistics() );
		GameplayStatistics &GS = SONGMAN->m_aGameplayStatistics[p][SONGMAN->m_aGameplayStatistics[p].GetSize()-1];
		GS.pSong = SONGMAN->m_pSongs[0];
		GS.dc = CLASS_EASY;
		GS.meter = 5;
		GS.iPossibleDancePoints = 300;
		GS.iActualDancePoints = 255;
		GS.failed = false;

		GS.perfect = GS.great = GS.good = GS.boo = GS.miss = GS.ok = GS.ng = GS.max_combo = 100; 
		GS.score = 100;
		for( int r=0; r<NUM_RADAR_VALUES; r++ )
			GS.fRadarPossible[r] = GS.fRadarActual[r] = 0;
		}
		{
		SONGMAN->m_aGameplayStatistics[p].Add( GameplayStatistics() );
		GameplayStatistics &GS = SONGMAN->m_aGameplayStatistics[p][SONGMAN->m_aGameplayStatistics[p].GetSize()-1];
		GS.pSong = SONGMAN->m_pSongs[0];
		GS.dc = CLASS_EASY;
		GS.meter = 5;
		GS.iPossibleDancePoints = 300;
		GS.iActualDancePoints = 255;
		GS.failed = false;

		GS.perfect = GS.great = GS.good = GS.boo = GS.miss = GS.ok = GS.ng = GS.max_combo = 100; 
		GS.score = 100;
		for( int r=0; r<NUM_RADAR_VALUES; r++ )
			GS.fRadarPossible[r] = GS.fRadarActual[r] = 0;
		}
		{
		SONGMAN->m_aGameplayStatistics[p].Add( GameplayStatistics() );
		GameplayStatistics &GS = SONGMAN->m_aGameplayStatistics[p][SONGMAN->m_aGameplayStatistics[p].GetSize()-1];
		GS.pSong = SONGMAN->m_pSongs[0];
		GS.dc = CLASS_EASY;
		GS.meter = 5;
		GS.iPossibleDancePoints = 300;
		GS.iActualDancePoints = 255;
		GS.failed = false;

		GS.perfect = GS.great = GS.good = GS.boo = GS.miss = GS.ok = GS.ng = GS.max_combo = 100; 
		GS.score = 100;
		for( int r=0; r<NUM_RADAR_VALUES; r++ )
			GS.fRadarPossible[r] = GS.fRadarActual[r] = 0;
		}
	}
*/


	///////////////////////////
	// Calculate total statistics depending on m_ResultMode
	///////////////////////////
	GameplayStatistics total_statistics[NUM_PLAYERS];
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		PlayerNumber pn = (PlayerNumber)p;

		if( !GAMEMAN->IsPlayerEnabled(pn) )
			continue;	// skip

		switch( m_ResultMode )
		{
		case RM_ARCADE_STAGE:
			total_statistics[p] = SONGMAN->GetLatestGameplayStatistics((PlayerNumber)p);
			break;
		case RM_ARCADE_SUMMARY:
			{
				int iStageIndex = PREFSMAN->GetStageIndex();
				int s = 0;
				for( int i=max(0,iStageIndex-STAGES_TO_SHOW_IN_SUMMARY); i<=iStageIndex; i++, s++ )
				{
						total_statistics[p] = SONGMAN->GetLatestGameplayStatistics((PlayerNumber)p);
				}
			}
			break;
		case RM_ONI:
			{
				int iStageIndex = PREFSMAN->GetStageIndex();
				int s = 0;
				for( int i=0; i<=iStageIndex; i++ )
					total_statistics[p] = SONGMAN->GetLatestGameplayStatistics((PlayerNumber)p);
			}
			break;
		}
	}



	///////////////////////////
	// Init the song banners depending on m_ResultMode
	///////////////////////////
	switch( m_ResultMode )
	{
	case RM_ARCADE_STAGE:
		m_BannerWithFrame[0].LoadFromSong( SONGMAN->GetCurrentSong() );
		m_BannerWithFrame[0].SetXY( BANNER_X, BANNER_Y );
		this->AddActor( &m_BannerWithFrame[0] );
		break;
	case RM_ARCADE_SUMMARY:
		{
			if( SONGMAN->m_aGameplayStatistics[0].GetSize() > STAGES_TO_SHOW_IN_SUMMARY )
			{
				// crop down to 3
				for( int p=0; p<NUM_PLAYERS; p++ )
					SONGMAN->m_aGameplayStatistics[p].RemoveAt( 0, SONGMAN->m_aGameplayStatistics[p].GetSize() - STAGES_TO_SHOW_IN_SUMMARY );
			}

			const int iSongsToShow = SONGMAN->m_aGameplayStatistics[0].GetSize();
			ASSERT( iSongsToShow > 0 );

			for( int i=0; i<iSongsToShow; i++ )
			{
				GameplayStatistics &GS = SONGMAN->m_aGameplayStatistics[0][i];
				m_BannerWithFrame[i].LoadFromSong( GS.pSong );
				float fBannerOffset = i - (iSongsToShow-1)/2.0f;
				m_BannerWithFrame[i].SetXY( BANNER_X + fBannerOffset*32, BANNER_Y + fBannerOffset*16 );
				m_BannerWithFrame[i].SetZoom( 0.70f );
				this->AddActor( &m_BannerWithFrame[i] );
			}
		}
		break;
	case RM_ONI:
		m_BannerWithFrame[0].LoadFromGroup( SONGMAN->m_sPreferredGroup );
		m_BannerWithFrame[0].SetXY( BANNER_X, BANNER_Y );
		this->AddActor( &m_BannerWithFrame[0] );
		break;
	}




	//////////////////////////
	// Init graphic elements
	//////////////////////////
	m_Menu.Load(
		THEME->GetPathTo(GRAPHIC_EVALUATION_BACKGROUND), 
		m_ResultMode==RM_ARCADE_SUMMARY ? THEME->GetPathTo(GRAPHIC_EVALUATION_SUMMARY_TOP_EDGE) : THEME->GetPathTo(GRAPHIC_EVALUATION_TOP_EDGE),
		"Press START to continue."
		);
	this->AddActor( &m_Menu );



	for( l=0; l<NUM_JUDGE_LINES; l++ ) 
	{
		m_sprJudgeLabels[l].Load( THEME->GetPathTo(GRAPHIC_EVALUATION_JUDGE_LABELS) );
		m_sprJudgeLabels[l].StopAnimating();
		m_sprJudgeLabels[l].SetState( l );
		m_sprJudgeLabels[l].SetXY( JUDGE_LABELS_X, JUDGE_START_Y + l*JUDGE_SPACING );
		m_sprJudgeLabels[l].SetZoom( 1.0f );
		this->AddActor( &m_sprJudgeLabels[l] );

		for( int p=0; p<NUM_PLAYERS; p++ ) 
		{
			m_textJudgeNumbers[l][p].Load( THEME->GetPathTo(FONT_SCORE_NUMBERS) );
			m_textJudgeNumbers[l][p].TurnShadowOff();
			m_textJudgeNumbers[l][p].SetXY( JUDGE_NUMBERS_X[p], JUDGE_START_Y + l*JUDGE_SPACING );
			m_textJudgeNumbers[l][p].SetZoom( 0.7f );
			m_textJudgeNumbers[l][p].SetDiffuseColor( PlayerToColor(p) );
			this->AddActor( &m_textJudgeNumbers[l][p] );
		}
	}

	m_sprScoreLabel.Load( THEME->GetPathTo(GRAPHIC_EVALUATION_SCORE_LABELS) );
	m_sprScoreLabel.StopAnimating();
	m_sprScoreLabel.SetXY( SCORE_LABEL_X, SCORE_Y );
	m_sprScoreLabel.SetZoom( 1.0f );
	this->AddActor( &m_sprScoreLabel );

	for( p=0; p<NUM_PLAYERS; p++ ) 
	{
		if( !GAMEMAN->IsPlayerEnabled( (PlayerNumber)p ) )
			continue;	// skip

		m_ScoreDisplay[p].SetXY( SCORE_DISPLAY_X[p], SCORE_Y );
		m_ScoreDisplay[p].SetZoomY( 0.9f );
		m_ScoreDisplay[p].SetDiffuseColor( PlayerToColor(p) );
		this->AddActor( &m_ScoreDisplay[p] );
	}




	Grade grade[NUM_PLAYERS];

	//////////////////////////
	// Set Numbers 
	//////////////////////////
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMEMAN->IsPlayerEnabled( (PlayerNumber)p ) )
			continue;	// skip

		GameplayStatistics GS[NUM_PLAYERS];	// fill this in depending on m_ResultMode
		
		switch( m_ResultMode )
		{
		case RM_ARCADE_STAGE:
			{
				GS[p] = SONGMAN->m_aGameplayStatistics[p][SONGMAN->m_aGameplayStatistics[p].GetSize()-1];
			}
			break;
		case RM_ARCADE_SUMMARY:
		case RM_ONI:
			{
				const int iSongsToShow = SONGMAN->m_aGameplayStatistics[0].GetSize();
				ASSERT( iSongsToShow > 0 );

				for( int i=0; i<iSongsToShow; i++ )
				{
					GameplayStatistics &GSstage = SONGMAN->m_aGameplayStatistics[p][i];

					GS[p].iPossibleDancePoints += GSstage.iPossibleDancePoints;
					GS[p].iActualDancePoints += GSstage.iActualDancePoints;

					GS[p].perfect	+= GSstage.perfect;
					GS[p].great		+= GSstage.great;
					GS[p].good		+= GSstage.good;
					GS[p].boo		+= GSstage.boo;
					GS[p].miss		+= GSstage.miss;
					GS[p].ok		+= GSstage.ok;
					GS[p].ng		+= GSstage.ng;
					GS[p].max_combo += GSstage.max_combo;
					GS[p].score		+= GSstage.score;
					for( int i=0; i<NUM_RADAR_VALUES; i++ )
					{
						GS[p].fRadarPossible[i] += GSstage.fRadarPossible[i];
						GS[p].fRadarActual[i] += GSstage.fRadarActual[i];
					}
					
				}
			}
			break;
		}

		
		
		
		grade[p] = GS[p].GetGrade();

		m_textJudgeNumbers[0][p].SetText( ssprintf("%4d", GS[p].perfect) );
		m_textJudgeNumbers[1][p].SetText( ssprintf("%4d", GS[p].great) );
		m_textJudgeNumbers[2][p].SetText( ssprintf("%4d", GS[p].good) );
		m_textJudgeNumbers[3][p].SetText( ssprintf("%4d", GS[p].boo) );
		m_textJudgeNumbers[4][p].SetText( ssprintf("%4d", GS[p].miss) );
		m_textJudgeNumbers[5][p].SetText( ssprintf("%4d", GS[p].ok) );

		m_ScoreDisplay[p].SetScore( GS[p].max_combo * 1000 );
		m_ScoreDisplay[p].SetScore( GS[p].score );

		switch( m_ResultMode )
		{
		case RM_ARCADE_STAGE:
		case RM_ARCADE_SUMMARY:
			m_Grades[p].SpinAndSettleOn( grade[p] );
			break;
		case RM_ONI:
			m_textOniPercent[p].SetText( "100.0%" );
			break;
		}


		m_BonusInfoFrame[p].SetBonusInfo( (PlayerNumber)p, GS[p].fRadarPossible, GS[p].fRadarActual, GS[p].max_combo );


		switch( m_ResultMode )
		{
		case RM_ARCADE_STAGE:
			////////////////////////
			// update song stats
			////////////////////////

			Notes* pNotes = SONGMAN->GetCurrentNotes((PlayerNumber)p);
			pNotes->m_iNumTimesPlayed++;

			if( GS[p].max_combo > pNotes->m_iMaxCombo )
				pNotes->m_iMaxCombo = GS[p].max_combo;

			if( GS[p].score > pNotes->m_iTopScore )
			{
				pNotes->m_iTopScore = (int)GS[p].score;
				m_bNewRecord[p] = true;
			}

			if( grade[p] > pNotes->m_TopGrade )
				pNotes->m_TopGrade = grade[p];
			break;
		}
	}
	


	Grade max_grade = GRADE_NO_DATA;
	for( p=0; p<NUM_PLAYERS; p++ )
		max_grade = max( max_grade, grade[p] ); 

	
	m_bTryExtraStage = false;
	if( (PREFSMAN->IsFinalStage() || PREFSMAN->IsExtraStage())  &&  m_ResultMode==RM_ARCADE_STAGE )
	{
		for( p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMEMAN->IsPlayerEnabled( (PlayerNumber)p ) )
				continue;	// skip

			if( SONGMAN->GetCurrentNotes((PlayerNumber)p)->m_DifficultyClass == CLASS_HARD  &&  grade[p] >= GRADE_AA )
				m_bTryExtraStage = true;
		}
	}


	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMEMAN->IsPlayerEnabled( (PlayerNumber)p ) )
			continue;	// skip

		m_sprGradeFrame[p].Load( THEME->GetPathTo(GRAPHIC_EVALUATION_GRADE_FRAME) );
		m_sprGradeFrame[p].StopAnimating();
		m_sprGradeFrame[p].SetState( p );
		m_sprGradeFrame[p].SetXY( GRADE_X[p], GRADE_Y );
		this->AddActor( &m_sprGradeFrame[p] );

		switch( m_ResultMode )
		{
		case RM_ONI:
			m_textOniPercent[p].Load( THEME->GetPathTo(FONT_HEADER1) );
			m_textOniPercent[p].SetXY( GRADE_X[p], GRADE_Y );
			m_textOniPercent[p].SetShadowLength( 2 );
			m_textOniPercent[p].SetZoom( 3 );
			m_textOniPercent[p].SetEffectGlowing( 1.0f );
			this->AddActor( &m_textOniPercent[p] );
			break;
		case RM_ARCADE_STAGE:
		case RM_ARCADE_SUMMARY:
			m_Grades[p].SetXY( GRADE_X[p], GRADE_Y );
			m_Grades[p].SetZ( -2 );
			m_Grades[p].SetZoom( 1.0f );
			m_Grades[p].SetEffectGlowing( 1.0f );
			this->AddActor( &m_Grades[p] );
			break;
		}

		m_BonusInfoFrame[p].SetXY( BONUS_FRAME_X[p], BONUS_FRAME_Y );
		this->AddActor( &m_BonusInfoFrame[p] );

		m_bNewRecord[p] = false;

		if( m_bNewRecord[p] )
		{
			m_textNewRecord[p].Load( THEME->GetPathTo(FONT_HEADER1) );
			m_textNewRecord[p].SetXY( NEW_RECORD_X[p], NEW_RECORD_Y );
			m_textNewRecord[p].SetShadowLength( 2 );
			m_textNewRecord[p].SetText( "IT'S A NEW RECORD!" );
			m_textNewRecord[p].SetZoom( 0.5f );
			m_textNewRecord[p].SetEffectGlowing( 1.0f );
			this->AddActor( &m_textNewRecord[p] );
		}
	}
	
	
	if( m_bTryExtraStage )
	{
		m_textTryExtraStage.Load( THEME->GetPathTo(FONT_HEADER1) );
		m_textTryExtraStage.SetXY( TRY_EXTRA_STAGE_X, TRY_EXTRA_STAGE_Y );
		if( PREFSMAN->IsExtraStage() )
			m_textTryExtraStage.SetText( "Try Another Extra Stage!" );
		else
			m_textTryExtraStage.SetText( "Try Extra Stage!" );
		m_textTryExtraStage.SetZoom( 1 );
		m_textTryExtraStage.SetEffectGlowing( 1.0f );
		this->AddActor( &m_textTryExtraStage );

		SOUND->PlayOnceStreamed( THEME->GetPathTo(SOUND_EVALUATION_EXTRA_STAGE) );
	}
	else
	{
		
		switch( max_grade )
		{
		case GRADE_E:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_EVALUATION_E) );		break;
		case GRADE_D:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_EVALUATION_D) );		break;
		case GRADE_C:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_EVALUATION_C) );		break;
		case GRADE_B:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_EVALUATION_B) );		break;
		case GRADE_A:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_EVALUATION_A) );		break;
		case GRADE_AA:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_EVALUATION_AA) );	break;
		case GRADE_AAA:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_EVALUATION_AAA) );	break;
		case GRADE_NO_DATA:
		default:
			ASSERT(0);	// invalid grade
		}
	}

	m_Menu.TweenOnScreenFromBlack( SM_None );
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
		m_BannerWithFrame[i].BeginTweeningQueued( 0.0f );
		m_BannerWithFrame[i].BeginTweeningQueued( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_BIAS_BEGIN );
		m_BannerWithFrame[i].SetTweenY( fOriginalY );
	}

	for( i=0; i<NUM_JUDGE_LINES; i++ ) 
	{
		fOriginalY = m_sprJudgeLabels[i].GetY();
		m_sprJudgeLabels[i].SetY( fOriginalY + SCREEN_HEIGHT );
		m_sprJudgeLabels[i].BeginTweeningQueued( 0.2f + 0.1f*i );
		m_sprJudgeLabels[i].BeginTweeningQueued( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_BIAS_BEGIN );
		m_sprJudgeLabels[i].SetTweenY( fOriginalY );

		for( int p=0; p<NUM_PLAYERS; p++ ) 
		{
			fOriginalX = m_textJudgeNumbers[i][p].GetX();
			m_textJudgeNumbers[i][p].SetX( fOriginalX + SCREEN_WIDTH/2*(p==PLAYER_1 ? 1 : -1) );
			m_textJudgeNumbers[i][p].BeginTweeningQueued( 0.2f + 0.1f*i );
			m_textJudgeNumbers[i][p].BeginTweeningQueued( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_BIAS_BEGIN );
			m_textJudgeNumbers[i][p].SetTweenX( fOriginalX );
		}
	}

	fOriginalY = m_sprScoreLabel.GetY();
	m_sprScoreLabel.SetY( fOriginalY + SCREEN_HEIGHT );
	m_sprScoreLabel.BeginTweeningQueued( 0.8f + 0.1f*i );
	m_sprScoreLabel.BeginTweeningQueued( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_BIAS_BEGIN );
	m_sprScoreLabel.SetTweenY( fOriginalY );

	for( p=0; p<NUM_PLAYERS; p++ ) 
	{
		fOriginalX = m_ScoreDisplay[p].GetX();
		m_ScoreDisplay[p].SetX( fOriginalX + SCREEN_WIDTH/2*(p==PLAYER_1 ? 1 : -1) );
		m_ScoreDisplay[p].BeginTweeningQueued( 0.8f + 0.1f*i );
		m_ScoreDisplay[p].BeginTweeningQueued( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_BIAS_BEGIN );
		m_ScoreDisplay[p].SetTweenX( fOriginalX );
	}

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		fOriginalX = m_BonusInfoFrame[p].GetX();
		m_BonusInfoFrame[p].SetX( fOriginalX + SCREEN_WIDTH/2*(p==PLAYER_1 ? 1 : -1) );
		m_BonusInfoFrame[p].BeginTweeningQueued( 0.2f + 0.1f*i );
		m_BonusInfoFrame[p].BeginTweeningQueued( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_BIAS_BEGIN );
		m_BonusInfoFrame[p].SetTweenX( fOriginalX );

		m_sprGradeFrame[p].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
		m_sprGradeFrame[p].SetTweenZoomY( 0 );

		m_Grades[p].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
		m_Grades[p].SetTweenZoomY( 0 );

		m_textOniPercent[p].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
		m_textOniPercent[p].SetTweenZoomY( 0 );

		m_textNewRecord[p].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
		m_textNewRecord[p].SetTweenZoomY( 0 );
	}
	
	m_textTryExtraStage.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_textTryExtraStage.SetTweenZoomY( 0 );
}

void ScreenEvaluation::TweenOffScreen()
{
	int i, p;

	for( i=0; i<STAGES_TO_SHOW_IN_SUMMARY; i++ )
	{
		m_BannerWithFrame[i].BeginTweeningQueued( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_BIAS_END );
		m_BannerWithFrame[i].SetTweenZoomY( 0 );
	}

	for( i=0; i<NUM_JUDGE_LINES; i++ ) 
	{
		m_sprJudgeLabels[i].BeginTweeningQueued( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_BIAS_END );
		m_sprJudgeLabels[i].SetTweenZoomY( 0 );

		for( int p=0; p<NUM_PLAYERS; p++ ) 
		{
			m_textJudgeNumbers[i][p].BeginTweeningQueued( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_BIAS_END );
			m_textJudgeNumbers[i][p].SetTweenZoomY( 0 );
		}
	}

	m_sprScoreLabel.BeginTweeningQueued( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_BIAS_END );
	m_sprScoreLabel.SetTweenZoomY( 0 );

	for( p=0; p<NUM_PLAYERS; p++ ) 
	{
		m_ScoreDisplay[p].BeginTweeningQueued( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_BIAS_END );
		m_ScoreDisplay[p].SetTweenZoomY( 0 );
	}

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_BonusInfoFrame[p].BeginTweeningQueued( MENU_ELEMENTS_TWEEN_TIME, Actor::TWEEN_BIAS_END );
		m_BonusInfoFrame[p].SetTweenZoomY( 0 );

		m_sprGradeFrame[p].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
		m_sprGradeFrame[p].SetTweenZoomY( 0 );

		m_Grades[p].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
		m_Grades[p].SetTweenZoomY( 0 );

		m_textOniPercent[p].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
		m_textOniPercent[p].SetTweenZoomY( 0 );

		m_textNewRecord[p].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
		m_textNewRecord[p].SetTweenZoomY( 0 );
	}

	m_textTryExtraStage.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_textTryExtraStage.SetTweenZoomY( 0 );
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
	LOG->WriteLine( "ScreenEvaluation::Input()" );

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
	case SM_GoToNextState:
		switch( m_ResultMode )
		{
		case RM_ARCADE_SUMMARY:
			SCREENMAN->SetNewScreen( new ScreenMusicScroll );
			return;
		case RM_ARCADE_STAGE:
			if( m_bTryExtraStage )
				SCREENMAN->SetNewScreen( new ScreenSelectMusic );
			else
				SCREENMAN->SetNewScreen( new ScreenEvaluation(true) );
			return;
		case RM_ONI:
			SCREENMAN->SetNewScreen( new ScreenMusicScroll );
			return;
		break;
		}
	}
}

void ScreenEvaluation::MenuBack( const PlayerNumber p )
{
	MenuStart( p );
}

void ScreenEvaluation::MenuStart( const PlayerNumber p )
{
	TweenOffScreen();

	m_Menu.TweenOffScreenToMenu( SM_GoToNextState );

	PREFSMAN->m_iCurrentStageIndex++;	// increment the stage number before constructing the next screen
}

