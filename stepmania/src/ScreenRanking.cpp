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
#include "Course.h"
#include "song.h"
#include "PrefsManager.h"
#include "NoteData.h"
#include "NoteDataUtil.h"
#include "ScoreKeeperMAX2.h"
#include "ScoreKeeper5th.h"

#define CATEGORY_X				THEME->GetMetricF("ScreenRanking","CategoryX")
#define CATEGORY_Y				THEME->GetMetricF("ScreenRanking","CategoryY")
#define CATEGORY_WIDTH			THEME->GetMetricF("ScreenRanking","CategoryWidth")
#define BANNER_X				THEME->GetMetricF("ScreenRanking","BannerX")
#define BANNER_Y				THEME->GetMetricF("ScreenRanking","BannerY")
#define TYPE_X					THEME->GetMetricF("ScreenRanking","TypeX")
#define TYPE_Y					THEME->GetMetricF("ScreenRanking","TypeY")
#define LINE_SPACING_X			THEME->GetMetricF("ScreenRanking","LineSpacingX")
#define LINE_SPACING_Y			THEME->GetMetricF("ScreenRanking","LineSpacingY")
#define BULLETS_START_X			THEME->GetMetricF("ScreenRanking","BulletsStartX")
#define BULLETS_START_Y			THEME->GetMetricF("ScreenRanking","BulletsStartY")
#define TEXT_ZOOM				THEME->GetMetricF("ScreenRanking","TextZoom")
#define TEXT_SONGZOOM			THEME->GetMetricF("ScreenRanking","TextSongZoom")
#define TEXT_COLOR( i )			THEME->GetMetricC("ScreenRanking",ssprintf("TextColor%d",i+1))
#define NAMES_START_X			THEME->GetMetricF("ScreenRanking","NamesStartX")
#define NAMES_START_Y			THEME->GetMetricF("ScreenRanking","NamesStartY")
#define SCORES_START_X			THEME->GetMetricF("ScreenRanking","ScoresStartX")
#define SCORES_START_Y			THEME->GetMetricF("ScreenRanking","ScoresStartY")
#define POINTS_START_X			THEME->GetMetricF("ScreenRanking","PointsStartX")
#define POINTS_START_Y			THEME->GetMetricF("ScreenRanking","PointsStartY")
#define TIME_START_X			THEME->GetMetricF("ScreenRanking","TimeStartX")
#define TIME_START_Y			THEME->GetMetricF("ScreenRanking","TimeStartY")
#define PERCENT_START_X			THEME->GetMetricF("ScreenRanking","PercentStartX")
#define PERCENT_START_Y			THEME->GetMetricF("ScreenRanking","PercentStartY")
#define PERCENT_COL_SPACING_X	THEME->GetMetricF("ScreenRanking","PercentColSpacingX")
#define PERCENT_COL_SPACING_Y	THEME->GetMetricF("ScreenRanking","PercentColSpacingY")
#define SECONDS_PER_PAGE		THEME->GetMetricF("ScreenRanking","SecondsPerPage")
#define SHOW_CATEGORIES			THEME->GetMetricB("ScreenRanking","ShowCategories")
#define SHOW_ALL_SONGS			THEME->GetMetricB("ScreenRanking","ShowAllSongs")
#define NOTES_TYPES_TO_HIDE		THEME->GetMetric ("ScreenRanking","NotesTypesToHide")
#define EMPTY_SCORE_NAME		THEME->GetMetric ("ScreenRanking","EmptyScoreName")
#define SHOW_SONG_DIFFICULTIES	THEME->GetMetric ("ScreenRanking","ShowSongDifficulties")

#define COURSES_TO_SHOW			PREFSMAN->m_sCoursesToShowRanking

const ScreenMessage SM_ShowNextPage		=	(ScreenMessage)(SM_User+67);
const ScreenMessage SM_HidePage			=	(ScreenMessage)(SM_User+68);


ScreenRanking::ScreenRanking( CString sClassName ) : ScreenAttract( sClassName )
{
	m_sprCategory.SetXY( CATEGORY_X, CATEGORY_Y );
	this->AddChild( &m_sprCategory );

	m_banner.SetXY( BANNER_X, BANNER_Y );
	this->AddChild( &m_banner );

	m_textCategory.LoadFromFont( THEME->GetPathToF("ScreenRanking title") );
	m_textCategory.EnableShadow( false );
	m_textCategory.SetXY( CATEGORY_X, CATEGORY_Y );
	this->AddChild( &m_textCategory );

	m_sprType.SetXY( TYPE_X, TYPE_Y );
	this->AddChild( &m_sprType );


	// make the list of difficulties to show
	vector<CString> sShowDiffs;
	split( SHOW_SONG_DIFFICULTIES, ",", sShowDiffs, true );

	for( vector<CString>::const_iterator iter = sShowDiffs.begin(); iter != sShowDiffs.end(); iter++ )
	{
		m_vDiffsToShow.push_back( StringToDifficulty( *iter ) );
	}
	

	for( int i=0; i<NUM_RANKING_LINES; i++ )
	{
		m_sprBullets[i].Load( THEME->GetPathToG(("ScreenRanking bullets 1x5")) );
		m_sprBullets[i].SetXY( BULLETS_START_X+LINE_SPACING_X*i, BULLETS_START_Y+LINE_SPACING_Y*i );
		m_sprBullets[i].SetDiffuse( RageColor(1,1,1,0) );
		m_sprBullets[i].StopAnimating();
		m_sprBullets[i].SetState(i);
		this->AddChild( &m_sprBullets[i] );

		m_textNames[i].LoadFromFont( THEME->GetPathToF("ScreenRanking letters") );
		m_textNames[i].EnableShadow( false );
		m_textNames[i].SetXY( NAMES_START_X+LINE_SPACING_X*i, NAMES_START_Y+LINE_SPACING_Y*i );
		m_textNames[i].SetZoom( TEXT_ZOOM );
		m_textNames[i].SetHorizAlign( Actor::align_left );
		this->AddChild( &m_textNames[i] );

		m_textScores[i].LoadFromFont( THEME->GetPathToF("ScreenRanking letters") );
		m_textScores[i].EnableShadow( false );
		m_textScores[i].SetXY( SCORES_START_X+LINE_SPACING_X*i, SCORES_START_Y+LINE_SPACING_Y*i );
		m_textScores[i].SetZoom( TEXT_ZOOM );
		m_textScores[i].SetHorizAlign( Actor::align_right );
		this->AddChild( &m_textScores[i] );

		m_textPoints[i].LoadFromFont( THEME->GetPathToF("ScreenRanking letters") );
		m_textPoints[i].EnableShadow( false );
		m_textPoints[i].SetXY( POINTS_START_X+LINE_SPACING_X*i, POINTS_START_Y+LINE_SPACING_Y*i );
		m_textPoints[i].SetZoom( TEXT_ZOOM );
		m_textPoints[i].SetHorizAlign( Actor::align_right );
		this->AddChild( &m_textPoints[i] );
		
		m_textTime[i].LoadFromFont( THEME->GetPathToF("ScreenRanking letters") );
		m_textTime[i].EnableShadow( false );
		m_textTime[i].SetXY( TIME_START_X+LINE_SPACING_X*i, TIME_START_Y+LINE_SPACING_Y*i );
		m_textTime[i].SetZoom( TEXT_ZOOM );
		m_textTime[i].SetHorizAlign( Actor::align_right );
		this->AddChild( &m_textTime[i] );



		if( PREFSMAN->m_sCoursesToShowRanking == "" )
			PREFSMAN->m_sCoursesToShowRanking = THEME->GetMetric("ScreenRanking","CoursesToShow");
	}


	// init all the difficulty graphics
	for( int d=0; d<NUM_DIFFICULTIES; d++ )
	{
		m_sprDiffHeaders[d].Load( THEME->GetPathToG(("ScreenRanking difficulty 1x5")) );
		m_sprDiffHeaders[d].SetDiffuse( RageColor(1,1,1,0) );
		m_sprDiffHeaders[d].StopAnimating();
		m_sprDiffHeaders[d].SetState(d);

		for( int i=0; i<NUM_RANKING_LINES; i++ )
		{
			m_textPercent[i][d].LoadFromFont( THEME->GetPathToF("ScreenRanking letters") );
			m_textPercent[i][d].EnableShadow( false );
			m_textPercent[i][d].SetZoom( TEXT_SONGZOOM );
		}
	}

	// enable the difficulty graphics that we care about	
	DiffVectorCIter pDiff = m_vDiffsToShow.begin();
	for( int j=0; pDiff != m_vDiffsToShow.end() && j<NUM_DIFFICULTIES; pDiff++, j++ )
	{
		int d = (int)*pDiff;

		m_sprDiffHeaders[d].SetX( PERCENT_START_X+PERCENT_COL_SPACING_X*j );
		m_sprDiffHeaders[d].SetY( PERCENT_START_Y-LINE_SPACING_Y );
		this->AddChild( &m_sprDiffHeaders[d] );


		for( int i=0; i<NUM_RANKING_LINES; i++ )
		{
			m_textPercent[i][d].SetX( PERCENT_START_X+LINE_SPACING_X*i+PERCENT_COL_SPACING_X*j );
			m_textPercent[i][d].SetY( PERCENT_START_Y+LINE_SPACING_Y*i+PERCENT_COL_SPACING_Y*j );
			this->AddChild( &m_textPercent[i][d] );
		}
	}



	vector<StepsType> aNotesTypesToShow;
	GAMEMAN->GetNotesTypesForGame( GAMESTATE->m_CurGame, aNotesTypesToShow );

	// subtract hidden NotesTypes
	{
		vector<CString> asNotesTypesToHide;
		split( NOTES_TYPES_TO_HIDE, ",", asNotesTypesToHide, true );
		for( unsigned i=0; i<asNotesTypesToHide.size(); i++ )
		{
			StepsType nt = GameManager::StringToNotesType(asNotesTypesToHide[i]);
			if( nt != STEPS_TYPE_INVALID )
			{
				const vector<StepsType>::iterator iter = find( aNotesTypesToShow.begin(), aNotesTypesToShow.end(), nt );
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
				pts.colorIndex = i;
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
				pts.colorIndex = i;
				pts.nt = aNotesTypesToShow[i];
				pts.pCourse = SONGMAN->GetCourseFromPath( asCoursePaths[c] );
				if( pts.pCourse )
					m_vPagesToShow.push_back( pts );
			}
		}
	}

	if( SHOW_ALL_SONGS )
	{
		vector<Song*> vpSongs = SONGMAN->GetAllSongs();
		for( unsigned i=0; i<aNotesTypesToShow.size(); i++ )
		{
			for( unsigned j=0; j<vpSongs.size(); j++ )
			{
				PageToShow pts;
				pts.type = PageToShow::TYPE_SONG;
				pts.colorIndex = i;
				pts.pSong = vpSongs[j];
				pts.nt = aNotesTypesToShow[i];
				m_vPagesToShow.push_back( pts );
			}
		}
	}

	this->MoveToTail( &m_In );		// put it in the back so it covers up the stuff we just added
	this->MoveToTail( &m_Out );		// put it in the back so it covers up the stuff we just added

	this->ClearMessageQueue( SM_BeginFadingOut );	// ignore ScreenAttract's SecsToShow

	this->PostScreenMessage( SM_ShowNextPage, 0.5f );
}

void ScreenRanking::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
		// redundant
//	case SM_BeginFadingOut:
//		m_Out.CloseWipingRight(SM_GoToNextScreen);
//		break;
	case SM_ShowNextPage:
		if( m_vPagesToShow.size() > 0 )
		{
			SetPage( m_vPagesToShow[0] );
			m_vPagesToShow.erase( m_vPagesToShow.begin() );
			this->PostScreenMessage( SM_HidePage, SECONDS_PER_PAGE-1 );
			TweenPageOnScreen();
		}
		else
		{
			m_Out.StartTransitioning(SM_GoToNextScreen);
		}
		break;
	case SM_HidePage:
		TweenPageOffScreen();
		this->PostScreenMessage( SM_ShowNextPage, 1 );
		break;
	}

	ScreenAttract::HandleScreenMessage( SM );
}

void ScreenRanking::SetPage( PageToShow pts )
{
	// Clear all text
	for( int l=0; l<NUM_RANKING_LINES; l++ )
	{
		m_textScores[l].SetText( "" );
		m_textPoints[l].SetText( "" );
		m_textTime[l].SetText( "" );
		for( DiffVectorCIter j=m_vDiffsToShow.begin(); j<m_vDiffsToShow.end(); j++ )
			m_textPercent[l][*j].SetText( "" );
		m_banner.UnloadTexture();
	}
	for( int d=0; d<NUM_DIFFICULTIES; d++ )
	{
		m_sprDiffHeaders[d].SetDiffuseAlpha(0);
	}

	switch( pts.type )
	{
	case PageToShow::TYPE_CATEGORY:
		{
			m_textCategory.SetDiffuseAlpha(0);
			m_sprCategory.SetDiffuse( RageColor(1,1,1,1) );
			m_sprCategory.Load( THEME->GetPathToG(ssprintf("ScreenRanking category %c", 'A'+pts.category)) );
			m_sprType.SetDiffuse( RageColor(1,1,1,1) );
			m_sprType.Load( THEME->GetPathToG("ScreenRanking type "+GameManager::NotesTypeToString(pts.nt)) );
			for( int l=0; l<NUM_RANKING_LINES; l++ )
			{
				SongManager::CategoryData::HighScore hs;
				if( l < (int)SONGMAN->m_CategoryDatas[pts.nt][pts.category].vHighScores.size() )
					hs = SONGMAN->m_CategoryDatas[pts.nt][pts.category].vHighScores[l];
				if( hs.sName.empty() )
					hs.sName = EMPTY_SCORE_NAME;

				m_sprBullets[l].SetDiffuse( RageColor(1,1,1,1) );
				m_textNames[l].SetText( hs.sName );
				m_textScores[l].SetText( ssprintf("%09i",hs.iScore) );
				m_textNames[l].SetDiffuse( TEXT_COLOR(pts.colorIndex) );
				m_textScores[l].SetDiffuse( TEXT_COLOR(pts.colorIndex) );

				bool bRecentHighScore = false;
				// FIXME
//				for( int p=0; p<NUM_PLAYERS; p++ )
//				{
//					bRecentHighScore |=
//						pts.nt == GAMESTATE->m_RankingNotesType  &&
//						GAMESTATE->m_RankingCategory[p] == pts.category  &&
//						GAMESTATE->m_iRankingIndex[p] == l;
//				}

				if( bRecentHighScore )
				{
					m_textNames[l].SetEffectGlowBlink(0.1f);
					m_textScores[l].SetEffectGlowBlink(0.1f);
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
			m_textCategory.SetDiffuseAlpha(0);
			m_sprCategory.SetDiffuseAlpha(0);
			CString path = THEME->GetPathToG("ScreenRanking category "+pts.pCourse->m_sName, true);
			if( IsAFile(path) )
			{
				m_sprCategory.Load( path );
				m_sprCategory.SetDiffuseAlpha(1);
			} else {
				m_textCategory.SetZoom(1);
				m_textCategory.SetTextMaxWidth( CATEGORY_WIDTH, pts.pCourse->m_sName );
				m_textCategory.SetDiffuseAlpha(1);
			}

			m_sprType.SetDiffuse( RageColor(1,1,1,1) );
			m_sprType.Load( THEME->GetPathToG("ScreenRanking type "+GameManager::NotesTypeToString(pts.nt)) );
			for( int l=0; l<NUM_RANKING_LINES; l++ )
			{
				Course::MemCardData::HighScore hs;
				if( l < (int)pts.pCourse->m_MemCardDatas[pts.nt][MEMORY_CARD_MACHINE].vHighScores.size() )
					hs = pts.pCourse->m_MemCardDatas[pts.nt][MEMORY_CARD_MACHINE].vHighScores[l];
				if( hs.sName.empty() )
					hs.sName = EMPTY_SCORE_NAME;
				
				m_sprBullets[l].SetDiffuse( RageColor(1,1,1,1) );
				m_textNames[l].SetText( hs.sName );
				if( pts.pCourse->IsOni() )
				{
					m_textPoints[l].SetText( ssprintf("%04d",hs.iScore) );
					m_textTime[l].SetText( SecondsToTime(hs.fSurviveTime) );
					m_textScores[l].SetText( "" );
				} else {
					m_textPoints[l].SetText( "" );
					m_textTime[l].SetText( "" );
					m_textScores[l].SetText( ssprintf("%09d",hs.iScore) );
				}
				m_textNames[l].SetDiffuse( TEXT_COLOR(pts.colorIndex) );
				m_textPoints[l].SetDiffuse( TEXT_COLOR(pts.colorIndex) );
				m_textTime[l].SetDiffuse( TEXT_COLOR(pts.colorIndex) );
				m_textScores[l].SetDiffuse( TEXT_COLOR(pts.colorIndex) );
				for( int p=0; p<NUM_PLAYERS; p++ )
				{
					bool bHighlight = false;
					// FIXME
//					bHighlight = pts.pCourse == GAMESTATE->m_pRankingCourse  &&
//						pts.nt == GAMESTATE->m_RankingNotesType  &&
//						GAMESTATE->m_iRankingIndex[p] == l;
					if( bHighlight )
					{
						m_textNames[l].SetEffectGlowBlink(0.1f);
						m_textScores[l].SetEffectGlowBlink(0.1f);
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
	case PageToShow::TYPE_SONG:
		{
			m_textCategory.SetDiffuseAlpha(0);
			m_sprCategory.SetDiffuseAlpha(0);

			m_banner.LoadFromSong( pts.pSong );
			m_banner.SetDiffuseAlpha(1);

			m_textCategory.SetZoom(1);
			m_textCategory.SetTextMaxWidth( CATEGORY_WIDTH, pts.pSong->GetFullTranslitTitle() );
			m_textCategory.SetDiffuseAlpha(1);
		
			m_sprType.SetDiffuse( RageColor(1,1,1,1) );
			m_sprType.Load( THEME->GetPathToG("ScreenRanking type "+GameManager::NotesTypeToString(pts.nt)) );

			for( DiffVectorCIter dIter = m_vDiffsToShow.begin(); dIter != m_vDiffsToShow.end(); dIter++ )
			{
				int d = (int)*dIter;

				Difficulty dc = (Difficulty)d;
				Steps* pSteps = pts.pSong->GetStepsByDifficulty( pts.nt, dc );

									
				if( pSteps == NULL )
				{
					// set alpha to 0.0 and continue
					m_sprDiffHeaders[d].SetDiffuse( RageColor(1,1,1,0) );	
					continue;
				}

				// set the header visible
				m_sprDiffHeaders[d].SetDiffuse( RageColor(1,1,1,1) );	

				for( int l=0; l<NUM_RANKING_LINES; l++ )
				{
					m_sprBullets[l].SetDiffuse( RageColor(1,1,1,1) );

					Steps::MemCardData::HighScore hs;
					if( l < (int)pSteps->m_MemCardDatas[MEMORY_CARD_MACHINE].vHighScores.size() )
						hs = pSteps->m_MemCardDatas[MEMORY_CARD_MACHINE].vHighScores[l];
					if( hs.sName.empty() )
						hs.sName = EMPTY_SCORE_NAME;

					m_textPercent[l][d].SetText( ssprintf("%s\n%0.3f%%", hs.sName.c_str(), hs.fPercentDP*100) );
					m_textPercent[l][d].SetDiffuse( TEXT_COLOR(pts.colorIndex) );
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
	m_sprCategory.FadeOn(0,"bounce right",0.5f);
	m_sprType.FadeOn(0.1f,"bounce right",0.5f);
	m_textCategory.FadeOn(0,"bounce right",0.5f);

	for( int l=0; l<NUM_RANKING_LINES; l++ )
	{
		m_sprBullets[l].FadeOn(0.2f+l*0.1f,"bounce right far",1.f);
		m_textNames[l].FadeOn(0.2f+l*0.1f,"bounce right far",1.f);
		m_textScores[l].FadeOn(0.2f+l*0.1f,"bounce right far",1.f);
		m_textPoints[l].FadeOn(0.2f+l*0.1f,"bounce right far",1.f);
		m_textTime[l].FadeOn(0.2f+l*0.1f,"bounce right far",1.f);
		for( int d=0; d<NUM_DIFFICULTIES; d++ )
			m_textPercent[l][d].FadeOn(0.2f+l*0.1f,"bounce right far",1.f);
	}
	for( int d=0; d<NUM_DIFFICULTIES; d++ )
		m_sprDiffHeaders[d].FadeOn(0,"bounce right far",1.f);

}

void ScreenRanking::TweenPageOffScreen()
{
	m_sprCategory.FadeOff(0,"fade",0.25f);
	m_sprType.FadeOff(0.1f,"fade",0.25f);
	m_textCategory.FadeOff(0.1f,"fade",0.25f);

	for( int l=0; l<NUM_RANKING_LINES; l++ )
	{
		m_sprBullets[l].FadeOff(0.2f+l*0.1f,"fade",0.25f);
		m_textNames[l].FadeOff(0.2f+l*0.1f,"fade",0.25f);
		m_textScores[l].FadeOff(0.2f+l*0.1f,"fade",0.25f);
		m_textPoints[l].FadeOff(0.2f+l*0.1f,"fade",0.25f);
		m_textTime[l].FadeOff(0.2f+l*0.1f,"fade",0.25f);
		for( int d=0; d<NUM_DIFFICULTIES; d++ )
			m_textPercent[l][d].FadeOff(0.2f+l*0.1f,"fade",0.25f);
	}
	for( int d=0; d<NUM_DIFFICULTIES; d++ )
		m_sprDiffHeaders[d].FadeOff(0,"fade",0.25f);
}
