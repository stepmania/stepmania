#include "stdafx.h"
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
#define NAMES_START_X			THEME->GetMetricF("ScreenRanking","NamesStartX")
#define NAMES_START_Y			THEME->GetMetricF("ScreenRanking","NamesStartY")
#define NAMES_ZOOM				THEME->GetMetricF("ScreenRanking","NamesZoom")
#define NAMES_COLOR				THEME->GetMetricC("ScreenRanking","NamesColor")
#define SCORES_START_X			THEME->GetMetricF("ScreenRanking","ScoresStartX")
#define SCORES_START_Y			THEME->GetMetricF("ScreenRanking","ScoresStartY")
#define SCORES_ZOOM				THEME->GetMetricF("ScreenRanking","ScoresZoom")
#define SCORES_COLOR			THEME->GetMetricC("ScreenRanking","ScoresColor")
#define SECS_BETWEEN_PAGES		THEME->GetMetricF("ScreenRanking","SecsBetweenPages")
#define SHOW_CATEGORIES			THEME->GetMetricB("ScreenRanking","ShowCategories")
#define COURSES_TO_SHOW			THEME->GetMetric("ScreenRanking","CoursesToShow")
#define NOTES_TYPES_TO_HIDE		THEME->GetMetric("ScreenRanking","NotesTypesToHide")


const ScreenMessage SM_NextCategory	=	(ScreenMessage)(SM_User+67);

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

	for( int i=0; i<NUM_HIGH_SCORE_LINES; i++ )
	{
		m_sprBullets[i].Load( THEME->GetPathTo("Graphics",("ranking bullets 1x5")) );
		m_sprBullets[i].SetXY( BULLETS_START_X+LINE_SPACING_X*i, BULLETS_START_Y+LINE_SPACING_Y*i );
		m_sprBullets[i].StopAnimating();
		m_sprBullets[i].SetState(i);
		this->AddChild( &m_sprBullets[i] );

		m_textNames[i].LoadFromFont( THEME->GetPathTo("Fonts","ranking") );
		m_textNames[i].TurnShadowOff();
		m_textNames[i].SetXY( NAMES_START_X+LINE_SPACING_X*i, NAMES_START_Y+LINE_SPACING_Y*i );
		m_textNames[i].SetZoom( NAMES_ZOOM );
		m_textNames[i].SetDiffuse( NAMES_COLOR );
		this->AddChild( &m_textNames[i] );

		m_textScores[i].LoadFromFont( THEME->GetPathTo("Fonts","ranking") );
		m_textScores[i].TurnShadowOff();
		m_textScores[i].SetXY( SCORES_START_X+LINE_SPACING_X*i, SCORES_START_Y+LINE_SPACING_Y*i );
		m_textScores[i].SetZoom( SCORES_ZOOM );
		m_textScores[i].SetDiffuse( SCORES_COLOR );
		this->AddChild( &m_textScores[i] );
	}


	vector<NotesType> aNotesTypesToHide;
	{
		vector<CString> asNotesTypesToHide;
		split( NOTES_TYPES_TO_HIDE, ",", asNotesTypesToHide, true );
		for( unsigned i=0; i<asNotesTypesToHide.size(); i++ )
			if( GameManager::StringToNotesType(asNotesTypesToHide[i]) != NOTES_TYPE_INVALID )
				aNotesTypesToHide.push_back( GameManager::StringToNotesType(asNotesTypesToHide[i]) );
	}

	// fill m_vCategoriesToShow
	if( SHOW_CATEGORIES )
	{
		for( NotesType nt=(NotesType)0; nt<NUM_NOTES_TYPES; nt=(NotesType)(nt+1) )
		{
			for( int i=0; i<NUM_RANKING_CATEGORIES; i++ )
			{
				if( find(aNotesTypesToHide.begin(), aNotesTypesToHide.end(), nt) != aNotesTypesToHide.end() )	// hidden
					continue;	// skip

				CategoryToShow cts;
				cts.type = CategoryToShow::TYPE_CATEGORY;
				cts.category = (RankingCategory)i;
				cts.nt = nt;
				m_vCategoriesToShow.push_back( cts );
			}
		}
	}

	vector<CString> asCoursePaths;
	split( COURSES_TO_SHOW, ",", asCoursePaths, true );
	for( NotesType nt=(NotesType)0; nt<NUM_NOTES_TYPES; nt=(NotesType)(nt+1) )
	{
		for( unsigned i=0; i<asCoursePaths.size(); i++ )
		{
			if( find(aNotesTypesToHide.begin(), aNotesTypesToHide.end(), nt) == aNotesTypesToHide.end() )	// hidden
				continue;	// skip

			CategoryToShow cts;
			cts.type = CategoryToShow::TYPE_COURSE;
			cts.nt = nt;
			cts.pCourse = SONGMAN->GetCourseFromPath( asCoursePaths[i] );
			if( cts.pCourse )
				m_vCategoriesToShow.push_back( cts );
		}
	}


	this->MoveToTail( &m_Fade );

	this->SendScreenMessage( SM_NextCategory, 0.5f );
}

void ScreenRanking::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_NextCategory:
		if( m_vCategoriesToShow.size() > 0 )
		{
			ShowCategory( m_vCategoriesToShow[0] );
			m_vCategoriesToShow.erase( m_vCategoriesToShow.begin() );
			this->SendScreenMessage( SM_NextCategory, 5 );
		}
		else
		{
			m_Fade.CloseWipingRight(SM_GoToNextScreen);
		}
		break;
	}

	ScreenAttract::HandleScreenMessage( SM );
}

void ScreenRanking::ShowCategory( CategoryToShow cts )
{
	switch( cts.type )
	{
	case CategoryToShow::TYPE_CATEGORY:
		{
			m_textCategory.SetText( ssprintf("Type %c", 'A'+cts.category) );
			m_textType.SetText( GameManager::NotesTypeToString(cts.nt) );
			for( int l=0; l<NUM_HIGH_SCORE_LINES; l++ )
			{
				CString sName = SONGMAN->m_MachineScores[cts.nt][cts.category][l].sName;
				float fScore = SONGMAN->m_MachineScores[cts.nt][cts.category][l].fScore;
				m_textNames[l].SetText( sName );
				m_textScores[l].SetText( ssprintf("%.0f",fScore) );
			}
		}
		break;
	case CategoryToShow::TYPE_COURSE:
		{
			m_textCategory.SetText( cts.pCourse->m_sName );
			m_textType.SetText( GameManager::NotesTypeToString(cts.nt) );
			for( int l=0; l<NUM_HIGH_SCORE_LINES; l++ )
			{
				CString sName = cts.pCourse->m_MachineScores[cts.nt][l].sName;
				int iDancePoints = cts.pCourse->m_MachineScores[cts.nt][l].iDancePoints;
				m_textNames[l].SetText( sName );
				m_textScores[l].SetText( ssprintf("%d",iDancePoints) );
			}
		}
		break;
	default:
		ASSERT(0);
	}

	m_textCategory.SetZoomY(0);
	m_textCategory.BeginTweening(0.0f);		// sleep
	m_textCategory.BeginTweening(0.25f);
	m_textCategory.SetTweenZoomY(1);
	m_textCategory.BeginTweening(3.5f);		// sleep
	m_textCategory.BeginTweening(0.25f);
	m_textCategory.SetTweenZoomY(0);

	m_textType.SetZoomY(0);
	m_textType.BeginTweening(0.2f);		// sleep
	m_textType.BeginTweening(0.25f);
	m_textType.SetTweenZoomY(1);
	m_textType.BeginTweening(3.5f);		// sleep
	m_textType.BeginTweening(0.25f);
	m_textType.SetTweenZoomY(0);

	for( int l=0; l<NUM_HIGH_SCORE_LINES; l++ )
	{
		m_textNames[l].SetZoomY(0);
		m_textNames[l].BeginTweening(0+l*0.1f+0.4f);	// sleep
		m_textNames[l].BeginTweening(0.25f);
		m_textNames[l].SetTweenZoomY(1);
		m_textNames[l].BeginTweening(3.5f);		// sleep
		m_textNames[l].BeginTweening(0.25f);
		m_textNames[l].SetTweenZoomY(0);

		m_textScores[l].SetZoomY(0);
		m_textScores[l].BeginTweening(0+l*0.1f+0.4f);	// sleep
		m_textScores[l].BeginTweening(0.25f);
		m_textScores[l].SetTweenZoomY(1);
		m_textScores[l].BeginTweening(3.5f);		// sleep
		m_textScores[l].BeginTweening(0.25f);
		m_textScores[l].SetTweenZoomY(0);
	}
}
