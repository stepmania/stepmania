#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenRanking

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenRanking.h"
#include "ThemeManager.h"
#include "SongManager.h"
#include "GameState.h"
#include "GameManager.h"


#define CATEGORY_X				THEME->GetMetricF("ScreenRanking","CategoryX")
#define CATEGORY_Y				THEME->GetMetricF("ScreenRanking","CategoryY")
#define TYPE_X					THEME->GetMetricF("ScreenRanking","TypeX")
#define TYPE_Y					THEME->GetMetricF("ScreenRanking","TypeY")
#define LINE_SPACING_X			THEME->GetMetricF("ScreenRanking","LineSpacingX")
#define LINE_SPACING_Y			THEME->GetMetricF("ScreenRanking","LineSpacingY")
#define BULLETS_START_X			THEME->GetMetricF("ScreenRanking","BulletsStartX")
#define BULLETS_START_Y			THEME->GetMetricF("ScreenRanking","BulletsStartY")
#define TEXT_ZOOM				THEME->GetMetricF("ScreenRanking","TextZoom")
#define TEXT_COLOR				THEME->GetMetricC("ScreenRanking","TextColor")
#define NAMES_START_X			THEME->GetMetricF("ScreenRanking","NamesStartX")
#define NAMES_START_Y			THEME->GetMetricF("ScreenRanking","NamesStartY")
#define SCORES_START_X			THEME->GetMetricF("ScreenRanking","ScoresStartX")
#define SCORES_START_Y			THEME->GetMetricF("ScreenRanking","ScoresStartY")
#define POINTS_START_X			THEME->GetMetricF("ScreenRanking","PointsStartX")
#define POINTS_START_Y			THEME->GetMetricF("ScreenRanking","PointsStartY")
#define TIME_START_X			THEME->GetMetricF("ScreenRanking","TimeStartX")
#define TIME_START_Y			THEME->GetMetricF("ScreenRanking","TimeStartY")
#define SECONDS_PER_PAGE		THEME->GetMetricF("ScreenRanking","SecondsPerPage")
#define SHOW_CATEGORIES			THEME->GetMetricB("ScreenRanking","ShowCategories")
#define COURSES_TO_SHOW			THEME->GetMetric("ScreenRanking","CoursesToShow")
#define NOTES_TYPES_TO_HIDE		THEME->GetMetric("ScreenRanking","NotesTypesToHide")


const ScreenMessage SM_ShowNextPage		=	(ScreenMessage)(SM_User+67);
const ScreenMessage SM_HidePage			=	(ScreenMessage)(SM_User+68);


ScreenRanking::ScreenRanking() : ScreenAttract("ScreenRanking","ranking")
{
	m_textCategory.LoadFromFont( THEME->GetPathTo("Fonts","Header2") );
	m_textCategory.TurnShadowOff();
	m_textCategory.SetXY( CATEGORY_X, CATEGORY_Y );
	this->AddChild( &m_textCategory );

	m_textType.LoadFromFont( THEME->GetPathTo("Fonts","Header2") );
	m_textType.TurnShadowOff();
	m_textType.SetXY( TYPE_X, TYPE_Y );
	this->AddChild( &m_textType );

	for( int i=0; i<NUM_RANKING_LINES; i++ )
	{
		m_sprBullets[i].Load( THEME->GetPathTo("Graphics",("ranking bullets 1x5")) );
		m_sprBullets[i].SetXY( BULLETS_START_X+LINE_SPACING_X*i, BULLETS_START_Y+LINE_SPACING_Y*i );
		m_sprBullets[i].SetDiffuse( RageColor(1,1,1,0) );
		m_sprBullets[i].StopAnimating();
		m_sprBullets[i].SetState(i);
		this->AddChild( &m_sprBullets[i] );

		m_textNames[i].LoadFromFont( THEME->GetPathTo("Fonts","ranking") );
		m_textNames[i].TurnShadowOff();
		m_textNames[i].SetXY( NAMES_START_X+LINE_SPACING_X*i, NAMES_START_Y+LINE_SPACING_Y*i );
		m_textNames[i].SetZoom( TEXT_ZOOM );
		m_textNames[i].SetHorizAlign( Actor::align_left );
		this->AddChild( &m_textNames[i] );

		m_textScores[i].LoadFromFont( THEME->GetPathTo("Fonts","ranking") );
		m_textScores[i].TurnShadowOff();
		m_textScores[i].SetXY( SCORES_START_X+LINE_SPACING_X*i, SCORES_START_Y+LINE_SPACING_Y*i );
		m_textScores[i].SetZoom( TEXT_ZOOM );
		m_textScores[i].SetHorizAlign( Actor::align_right );
		this->AddChild( &m_textScores[i] );

		m_textPoints[i].LoadFromFont( THEME->GetPathTo("Fonts","ranking") );
		m_textPoints[i].TurnShadowOff();
		m_textPoints[i].SetXY( POINTS_START_X+LINE_SPACING_X*i, POINTS_START_Y+LINE_SPACING_Y*i );
		m_textPoints[i].SetZoom( TEXT_ZOOM );
		m_textPoints[i].SetHorizAlign( Actor::align_right );
		this->AddChild( &m_textPoints[i] );
		
		m_textTime[i].LoadFromFont( THEME->GetPathTo("Fonts","ranking") );
		m_textTime[i].TurnShadowOff();
		m_textTime[i].SetXY( TIME_START_X+LINE_SPACING_X*i, TIME_START_Y+LINE_SPACING_Y*i );
		m_textTime[i].SetZoom( TEXT_ZOOM );
		m_textTime[i].SetHorizAlign( Actor::align_right );
		this->AddChild( &m_textTime[i] );
	}


	vector<NotesType> aNotesTypesToShow;
	GAMEMAN->GetNotesTypesForGame( GAMESTATE->m_CurGame, aNotesTypesToShow );

	// subtract hidden NotesTypes
	{
		vector<CString> asNotesTypesToHide;
		split( NOTES_TYPES_TO_HIDE, ",", asNotesTypesToHide, true );
		for( unsigned i=0; i<asNotesTypesToHide.size(); i++ )
		{
			NotesType nt = GameManager::StringToNotesType(asNotesTypesToHide[i]);
			if( nt != NOTES_TYPE_INVALID )
			{
				const vector<NotesType>::iterator iter = find( aNotesTypesToShow.begin(), aNotesTypesToShow.end(), nt );
				if( iter != aNotesTypesToShow.end() )
					aNotesTypesToShow.erase( iter );
			}
		}
	}


	// fill m_vPagesToShow
	if( SHOW_CATEGORIES )
	{
		for( unsigned i=0; i<aNotesTypesToShow.size(); i++ )
		{
			for( int c=0; c<NUM_RANKING_CATEGORIES; c++ )
			{
				PageToShow pts;
				pts.type = PageToShow::TYPE_CATEGORY;
				pts.category = (RankingCategory)c;
				pts.nt = aNotesTypesToShow[i];
				m_vPagesToShow.push_back( pts );
			}
		}
	}

	{
		vector<CString> asCoursePaths;
		split( COURSES_TO_SHOW, ",", asCoursePaths, true );
		for( unsigned i=0; i<aNotesTypesToShow.size(); i++ )
		{
			for( unsigned c=0; c<asCoursePaths.size(); c++ )
			{
				PageToShow pts;
				pts.type = PageToShow::TYPE_COURSE;
				pts.nt = aNotesTypesToShow[i];
				pts.pCourse = SONGMAN->GetCourseFromPath( asCoursePaths[c] );
				if( pts.pCourse )
					m_vPagesToShow.push_back( pts );
			}
		}
	}

	this->MoveToTail( &m_Fade );

	this->ClearMessageQueue( SM_BeginFadingOut );	// ignore ScreenAttract's SecsToShow

	this->SendScreenMessage( SM_ShowNextPage, 0.5f );
}

void ScreenRanking::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_BeginFadingOut:
		m_Fade.CloseWipingRight(SM_GoToNextScreen);
		break;
	case SM_ShowNextPage:
		if( m_vPagesToShow.size() > 0 )
		{
			SetPage( m_vPagesToShow[0] );
			m_vPagesToShow.erase( m_vPagesToShow.begin() );
			this->SendScreenMessage( SM_HidePage, SECONDS_PER_PAGE-1 );
			TweenPageOnScreen();
		}
		else
		{
			m_Fade.CloseWipingRight(SM_GoToNextScreen);
		}
		break;
	case SM_HidePage:
		TweenPageOffScreen();
		this->SendScreenMessage( SM_ShowNextPage, 1 );
		break;
	}

	ScreenAttract::HandleScreenMessage( SM );
}

void ScreenRanking::SetPage( PageToShow pts )
{
	switch( pts.type )
	{
	case PageToShow::TYPE_CATEGORY:
		{
			m_textCategory.SetDiffuse( RageColor(1,1,1,1) );
			m_textCategory.SetText( ssprintf("Type %c", 'A'+pts.category) );
			m_textType.SetDiffuse( RageColor(1,1,1,1) );
			m_textType.SetText( GameManager::NotesTypeToString(pts.nt) );
			for( int l=0; l<NUM_RANKING_LINES; l++ )
			{
				m_sprBullets[l].SetDiffuse( RageColor(1,1,1,1) );
				CString sName = SONGMAN->m_MachineScores[pts.nt][pts.category][l].sName;
				float fScore = SONGMAN->m_MachineScores[pts.nt][pts.category][l].fScore;
				m_textNames[l].SetText( sName );
				m_textScores[l].SetText( ssprintf("%09.0f",fScore) );
				m_textPoints[l].SetText( "" );
				m_textTime[l].SetText( "" );
				m_textNames[l].SetDiffuse( TEXT_COLOR );
				m_textScores[l].SetDiffuse( TEXT_COLOR );

				bool bRecentHighScore = false;
				for( int p=0; p<NUM_PLAYERS; p++ )
				{
					bRecentHighScore |=
						pts.nt == GAMESTATE->m_RankingNotesType  &&
						GAMESTATE->m_RankingCategory[p] == pts.category  &&
						GAMESTATE->m_iRankingIndex[p] == l;
				}

				if( bRecentHighScore )
				{
					m_textNames[l].SetEffectDiffuseBlinking( 10, TEXT_COLOR, RageColor(1,1,1,1) );
					m_textScores[l].SetEffectDiffuseBlinking( 10, TEXT_COLOR, RageColor(1,1,1,1) );
				}
				else
				{
					m_textNames[l].SetEffectNone();
					m_textScores[l].SetEffectNone();
				}
			}
		}
		break;
	case PageToShow::TYPE_COURSE:
		{
			m_textCategory.SetText( pts.pCourse->m_sName );
			m_textType.SetText( GameManager::NotesTypeToString(pts.nt) );
			for( int l=0; l<NUM_RANKING_LINES; l++ )
			{
				m_sprBullets[l].SetDiffuse( RageColor(1,1,1,1) );
				CString sName = pts.pCourse->m_RankingScores[pts.nt][l].sName;
				int iDancePoints = pts.pCourse->m_RankingScores[pts.nt][l].iDancePoints;
				float fSurviveTime = pts.pCourse->m_RankingScores[pts.nt][l].fSurviveTime;
				m_textNames[l].SetText( sName );
				m_textScores[l].SetText( "" );
				m_textPoints[l].SetText( ssprintf("%04d",iDancePoints) );
				m_textTime[l].SetText( SecondsToTime(fSurviveTime) );
				m_textNames[l].SetDiffuse( TEXT_COLOR );
				m_textPoints[l].SetDiffuse( TEXT_COLOR );
				m_textTime[l].SetDiffuse( TEXT_COLOR );
				for( int p=0; p<NUM_PLAYERS; p++ )
				{
					if( pts.pCourse == GAMESTATE->m_pRankingCourse  &&
						pts.nt == GAMESTATE->m_RankingNotesType  &&
						GAMESTATE->m_iRankingIndex[p] == l )
					{
						m_textNames[l].SetEffectDiffuseBlinking();
						m_textScores[l].SetEffectDiffuseBlinking();
					}
					else
					{
						m_textNames[l].SetEffectNone();
						m_textScores[l].SetEffectNone();
					}
				}
			}
		}
		break;
	default:
		ASSERT(0);
	}
}

void ScreenRanking::TweenPageOnScreen()
{
	m_textCategory.FadeOn(0,"bounce right",0.5f);
	m_textType.FadeOn(0.1f,"bounce right",0.5f);

	for( int l=0; l<NUM_RANKING_LINES; l++ )
	{
		m_sprBullets[l].FadeOn(0.2f+l*0.1f,"bounce right far",1.f);
		m_textNames[l].FadeOn(0.2f+l*0.1f,"bounce right far",1.f);
		m_textScores[l].FadeOn(0.2f+l*0.1f,"bounce right far",1.f);
		m_textPoints[l].FadeOn(0.2f+l*0.1f,"bounce right far",1.f);
		m_textTime[l].FadeOn(0.2f+l*0.1f,"bounce right far",1.f);
	}
}

void ScreenRanking::TweenPageOffScreen()
{
	m_textCategory.FadeOff(0,"fade",0.25f);
	m_textType.FadeOff(0.1f,"fade",0.25f);

	for( int l=0; l<NUM_RANKING_LINES; l++ )
	{
		m_sprBullets[l].FadeOff(0.2f+l*0.1f,"fade",0.25f);
		m_textNames[l].FadeOff(0.2f+l*0.1f,"fade",0.25f);
		m_textScores[l].FadeOff(0.2f+l*0.1f,"fade",0.25f);
		m_textPoints[l].FadeOff(0.2f+l*0.1f,"fade",0.25f);
		m_textTime[l].FadeOff(0.2f+l*0.1f,"fade",0.25f);
	}
}
