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
#include "Steps.h"
#include "PrefsManager.h"
#include "NoteData.h"
#include "NoteDataUtil.h"
#include "ActorUtil.h"
#include "ProfileManager.h"


#define STEPS_TYPES_TO_HIDE			THEME->GetMetric ("ScreenRanking","StepsTypesToHide")
#define SHOW_CATEGORIES				THEME->GetMetricB("ScreenRanking","ShowCategories")
#define SHOW_ALL_STEPS_SCORES		THEME->GetMetricB("ScreenRanking","ShowAllStepsScores")
#define DIFFICULTIES_TO_SHOW		THEME->GetMetric ("ScreenRanking","DifficultiesToShow")
#define COURSES_TO_SHOW				PREFSMAN->m_sCoursesToShowRanking
#define SECONDS_PER_PAGE			THEME->GetMetricF("ScreenRanking","SecondsPerPage")
#define PAGE_FADE_SECONDS			THEME->GetMetricF("ScreenRanking","PageFadeSeconds")
#define PERCENT_DECIMAL_PLACES		THEME->GetMetricI("ScreenRanking","PercentDecimalPlaces")
#define PERCENT_TOTAL_SIZE			THEME->GetMetricI("ScreenRanking","PercentTotalSize")
#define EMPTY_SCORE_NAME			THEME->GetMetric ("ScreenRanking","EmptyScoreName")
#define NO_SCORE_NAME				THEME->GetMetric ("ScreenRanking","NoScoreName")

#define ROW_SPACING_X				THEME->GetMetricF("ScreenRanking","RowSpacingX")
#define ROW_SPACING_Y				THEME->GetMetricF("ScreenRanking","RowSpacingY")
#define COL_SPACING_X				THEME->GetMetricF("ScreenRanking","ColSpacingX")
#define COL_SPACING_Y				THEME->GetMetricF("ScreenRanking","ColSpacingY")
#define STEPS_TYPE_COLOR( i )		THEME->GetMetricC("ScreenRanking",ssprintf("StepsTypeColor%d",i+1))
#define SONG_SCORE_ROWS_TO_SHOW		THEME->GetMetricI("ScreenRanking","SongScoreRowsToShow")
#define SONG_SCORE_SECONDS_PER_ROW	THEME->GetMetricF("ScreenRanking","SongScoreSecondsPerRow")

#define BULLET_START_X				THEME->GetMetricF("ScreenRanking","BulletStartX")
#define BULLET_START_Y				THEME->GetMetricF("ScreenRanking","BulletStartY")
#define NAME_START_X				THEME->GetMetricF("ScreenRanking","NameStartX")
#define NAME_START_Y				THEME->GetMetricF("ScreenRanking","NameStartY")
#define SCORE_START_X				THEME->GetMetricF("ScreenRanking","ScoreStartX")
#define SCORE_START_Y				THEME->GetMetricF("ScreenRanking","ScoreStartY")
#define POINTS_START_X				THEME->GetMetricF("ScreenRanking","PointsStartX")
#define POINTS_START_Y				THEME->GetMetricF("ScreenRanking","PointsStartY")
#define TIME_START_X				THEME->GetMetricF("ScreenRanking","TimeStartX")
#define TIME_START_Y				THEME->GetMetricF("ScreenRanking","TimeStartY")
#define DIFFICULTY_START_X			THEME->GetMetricF("ScreenRanking","DifficultyStartX")
#define DIFFICULTY_Y				THEME->GetMetricF("ScreenRanking","DifficultyY")
#define SONG_TITLE_OFFSET_X			THEME->GetMetricF("ScreenRanking","SongTitleOffsetX")
#define SONG_TITLE_OFFSET_Y			THEME->GetMetricF("ScreenRanking","SongTitleOffsetY")
#define SONG_FRAME_OFFSET_X			THEME->GetMetricF("ScreenRanking","SongFrameOffsetX")
#define SONG_FRAME_OFFSET_Y			THEME->GetMetricF("ScreenRanking","SongFrameOffsetY")
#define STEPS_SCORE_OFFSET_START_X	THEME->GetMetricF("ScreenRanking","StepsScoreOffsetStartX")
#define STEPS_SCORE_OFFSET_Y		THEME->GetMetricF("ScreenRanking","StepsScoreOffsetY")

#define BULLET_X(row)				(BULLET_START_X+ROW_SPACING_X*row)
#define BULLET_Y(row)				(BULLET_START_Y+ROW_SPACING_Y*row)
#define NAME_X(row)					(NAME_START_X+ROW_SPACING_X*row)
#define NAME_Y(row)					(NAME_START_Y+ROW_SPACING_Y*row)
#define SCORE_X(row)				(SCORE_START_X+ROW_SPACING_X*row)
#define SCORE_Y(row)				(SCORE_START_Y+ROW_SPACING_Y*row)
#define POINTS_X(row)				(POINTS_START_X+ROW_SPACING_X*row)
#define POINTS_Y(row)				(POINTS_START_Y+ROW_SPACING_Y*row)
#define TIME_X(row)					(TIME_START_X+ROW_SPACING_X*row)
#define TIME_Y(row)					(TIME_START_Y+ROW_SPACING_Y*row)
#define DIFFICULTY_X(col)			(DIFFICULTY_START_X+COL_SPACING_X*col)
#define STEPS_SCORE_OFFSET_X(col)	(STEPS_SCORE_OFFSET_START_X+COL_SPACING_X*col)

const ScreenMessage SM_ShowNextPage		=	(ScreenMessage)(SM_User+67);
const ScreenMessage SM_HidePage			=	(ScreenMessage)(SM_User+68);


ScreenRanking::ScreenRanking( CString sClassName ) : ScreenAttract( sClassName )
{
	// init Actors for category and course
	{
		m_Banner.SetName( "Banner" );
		m_Banner.SetHidden( true );
		this->AddChild( &m_Banner );

		m_sprBannerFrame.SetName( "BannerFrame" );
		m_sprBannerFrame.SetHidden( true );
		this->AddChild( &m_sprBannerFrame );

		m_textCategory.SetName( "Category" );
		m_textCategory.LoadFromFont( THEME->GetPathToF("ScreenRanking category") );
		m_textCategory.EnableShadow( false );
		m_textCategory.SetHidden( true );
		this->AddChild( &m_textCategory );

		m_textStepsType.SetName( "StepsType" );
		m_textStepsType.LoadFromFont( THEME->GetPathToF("ScreenRanking steps type") );
		m_textStepsType.EnableShadow( false );
		m_textStepsType.SetHidden( true );
		this->AddChild( &m_textStepsType );


		for( int l=0; l<NUM_RANKING_LINES; l++ )
		{
			m_sprBullets[l].SetName( ssprintf("Bullet%d",l+1) );
			m_sprBullets[l].Load( THEME->GetPathToG( ssprintf("ScreenRanking bullets 1x%d",NUM_RANKING_LINES) ) );
			m_sprBullets[l].SetState( l );
			m_sprBullets[l].StopAnimating();
			m_sprBullets[l].SetHidden( true );
			this->AddChild( &m_sprBullets[l] );

			m_textNames[l].SetName( ssprintf("Name%d",l+1) );
			m_textNames[l].LoadFromFont( THEME->GetPathToF("ScreenRanking name") );
			m_textNames[l].SetHidden( true );
			this->AddChild( &m_textNames[l] );

			m_textScores[l].SetName( ssprintf("Score%d",l+1) );
			m_textScores[l].LoadFromFont( THEME->GetPathToF("ScreenRanking score") );
			m_textScores[l].SetHidden( true );
			this->AddChild( &m_textScores[l] );

			m_textPoints[l].SetName( ssprintf("Points%d",l+1) );
			m_textPoints[l].LoadFromFont( THEME->GetPathToF("ScreenRanking points") );
			m_textPoints[l].SetHidden( true );
			this->AddChild( &m_textPoints[l] );
			
			m_textTime[l].SetName( ssprintf("Time%d",l+1) );
			m_textTime[l].LoadFromFont( THEME->GetPathToF("ScreenRanking time") );
			m_textTime[l].SetHidden( true );
			this->AddChild( &m_textTime[l] );

			// TODO: Think of a better way to handle this
			if( PREFSMAN->m_sCoursesToShowRanking == "" )
				PREFSMAN->m_sCoursesToShowRanking = THEME->GetMetric("ScreenRanking","CoursesToShow");
		}
	}

	// make the list of difficulties to show
	{
		vector<CString> sShowDiffs;
		split( DIFFICULTIES_TO_SHOW, ",", sShowDiffs, true );

		for( vector<CString>::const_iterator iter = sShowDiffs.begin(); iter != sShowDiffs.end(); iter++ )
		{
			m_vDiffsToShow.push_back( StringToDifficulty( *iter ) );
		}
	}

	// init Actors for all_steps
	{
		const unsigned num_songs = SONGMAN->GetAllSongs().size();
		for( int d=0; d<NUM_DIFFICULTIES; d++ )
		{
			bool bShowThis = find(m_vDiffsToShow.begin(), m_vDiffsToShow.end(), d) != m_vDiffsToShow.end();
			if( !bShowThis )
				continue;	// skip

			m_sprDifficulty[d].SetName( ssprintf("Difficulty%d",d) );
			m_sprDifficulty[d].Load( THEME->GetPathToG(ssprintf("ScreenRanking difficulty 1x%d",NUM_DIFFICULTIES)) );
			m_sprDifficulty[d].SetHidden( true );
			this->AddChild( &m_sprDifficulty[d] );
		}
		for( unsigned s=0; s<num_songs; s++ )
		{
			StepsScoreRowItem* pStepsScoreRowItem = new StepsScoreRowItem;

			pStepsScoreRowItem->m_sprSongFrame.SetName( "SongFrame" );
			pStepsScoreRowItem->m_sprSongFrame.SetHidden( true );
			pStepsScoreRowItem->m_sprSongFrame.Load( THEME->GetPathToG("ScreenRanking song frame") );
			pStepsScoreRowItem->AddChild( &pStepsScoreRowItem->m_sprSongFrame );

			pStepsScoreRowItem->m_textSongTitle.SetName( "SongTitle" );
			pStepsScoreRowItem->m_textSongTitle.SetHidden( true );
			pStepsScoreRowItem->m_textSongTitle.LoadFromFont( THEME->GetPathToF("ScreenRanking song title") );
			pStepsScoreRowItem->AddChild( &pStepsScoreRowItem->m_textSongTitle );

			for( int d=0; d<NUM_DIFFICULTIES; d++ )
			{
				pStepsScoreRowItem->m_textStepsScore[d].SetName( "StepsScore" );
				pStepsScoreRowItem->m_textStepsScore[d].LoadFromFont( THEME->GetPathToF("ScreenRanking steps score") );
				pStepsScoreRowItem->m_textStepsScore[d].SetHidden( true );
				pStepsScoreRowItem->AddChild( &pStepsScoreRowItem->m_textStepsScore[d] );
			}

			m_vpStepsScoreRowItem.push_back( pStepsScoreRowItem );
		}

		m_ListScoreRowItems.SetName( "ListScoreRowItems" );
		this->AddChild( &m_ListScoreRowItems );
		m_ListScoreRowItems.SetXY( CENTER_X, CENTER_Y );
	}

	// calculate which StepsTypes to show
	vector<StepsType> aStepsTypesToShow;
	{
		GAMEMAN->GetNotesTypesForGame( GAMESTATE->m_CurGame, aStepsTypesToShow );

		// subtract hidden StepsTypes
		{
			vector<CString> asStepsTypesToHide;
			split( STEPS_TYPES_TO_HIDE, ",", asStepsTypesToHide, true );
			for( unsigned i=0; i<asStepsTypesToHide.size(); i++ )
			{
				StepsType nt = GameManager::StringToNotesType(asStepsTypesToHide[i]);
				if( nt != STEPS_TYPE_INVALID )
				{
					const vector<StepsType>::iterator iter = find( aStepsTypesToShow.begin(), aStepsTypesToShow.end(), nt );
					if( iter != aStepsTypesToShow.end() )
						aStepsTypesToShow.erase( iter );
				}
			}
		}
	}

	//
	// fill m_vPagesToShow
	//
	if( SHOW_CATEGORIES )
	{
		for( unsigned i=0; i<aStepsTypesToShow.size(); i++ )
		{
			for( int c=0; c<NUM_RANKING_CATEGORIES; c++ )
			{
				PageToShow pts;
				pts.type = PageToShow::TYPE_CATEGORY;
				pts.colorIndex = i;
				pts.category = (RankingCategory)c;
				pts.nt = aStepsTypesToShow[i];
				m_vPagesToShow.push_back( pts );
			}
		}
	}

	{
		vector<CString> asCoursePaths;
		split( COURSES_TO_SHOW, ",", asCoursePaths, true );
		for( unsigned i=0; i<aStepsTypesToShow.size(); i++ )
		{
			for( unsigned c=0; c<asCoursePaths.size(); c++ )
			{
				PageToShow pts;
				pts.type = PageToShow::TYPE_COURSE;
				pts.colorIndex = i;
				pts.nt = aStepsTypesToShow[i];
				pts.pCourse = SONGMAN->GetCourseFromPath( asCoursePaths[c] );
				if( pts.pCourse )
					m_vPagesToShow.push_back( pts );
			}
		}
	}

	if( SHOW_ALL_STEPS_SCORES )
	{
		vector<Song*> vpSongs = SONGMAN->GetAllSongs();
		if( !vpSongs.empty() )
		{
			for( unsigned i=0; i<aStepsTypesToShow.size(); i++ )
			{
				PageToShow pts;
				pts.type = PageToShow::TYPE_ALL_STEPS;
				pts.colorIndex = i;
				pts.nt = aStepsTypesToShow[i];
				m_vPagesToShow.push_back( pts );
			}
		}
	}

	this->MoveToTail( &m_In );		// put it in the back so it covers up the stuff we just added
	this->MoveToTail( &m_Out );		// put it in the back so it covers up the stuff we just added

	this->ClearMessageQueue( SM_BeginFadingOut );	// ignore ScreenAttract's SecsToShow

	this->PostScreenMessage( SM_ShowNextPage, 0.5f );
}

ScreenRanking::~ScreenRanking()
{
	const unsigned num_songs = SONGMAN->GetAllSongs().size();

	// delete dynamically allocated members
	for( unsigned s=0; s<num_songs; s++ )
		delete m_vpStepsScoreRowItem[s];
}

void ScreenRanking::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_ShowNextPage:
		if( m_vPagesToShow.size() > 0 )
		{
			float fSecsToShow = SetPage( m_vPagesToShow[0] );
			m_vPagesToShow.erase( m_vPagesToShow.begin() );
			this->PostScreenMessage( SM_HidePage, fSecsToShow-PAGE_FADE_SECONDS );
		}
		else
		{
			m_Out.StartTransitioning(SM_GoToNextScreen);
		}
		break;
	case SM_HidePage:
		TweenPageOffScreen();
		this->PostScreenMessage( SM_ShowNextPage, PAGE_FADE_SECONDS );
		break;
	}

	ScreenAttract::HandleScreenMessage( SM );
}

float ScreenRanking::SetPage( PageToShow pts )
{
	bool bBanner = false; 
	bool bBannerFrame = false;
	bool bShowCategory = false; 
	bool bShowStepsType = false; 
	bool bShowBullets = false; 
	bool bShowNames = false;
	bool bShowScores = false; 
	bool bShowPoints = false; 
	bool bShowTime = false;
	bool bShowDifficulty = false; 
	bool bShowStepsScore = false;
	switch( pts.type )
	{
	case PageToShow::TYPE_CATEGORY:
		bBanner = false; 
		bBannerFrame = false;
		bShowCategory = true;
		bShowStepsType = true;
		bShowBullets = true;
		bShowNames = true;
		bShowScores = true;
		bShowPoints = false;
		bShowTime = false;
		bShowDifficulty = false;
		bShowStepsScore = false;
		break;
	case PageToShow::TYPE_COURSE:
		bBanner = true; 
		bBannerFrame = true;
		bShowCategory = false;
		bShowStepsType = true;
		bShowBullets = true;
		bShowNames = true;
		bShowScores = !pts.pCourse->IsOni();
		bShowPoints = pts.pCourse->IsOni();
		bShowTime = pts.pCourse->IsOni();
		bShowDifficulty = false;
		bShowStepsScore = false;
		break;
	case PageToShow::TYPE_ALL_STEPS:
		bBanner = false; 
		bBannerFrame = false;
		bShowCategory = false;
		bShowStepsType = true;
		bShowBullets = false;
		bShowNames = false;
		bShowScores = false;
		bShowPoints = false;
		bShowTime = false;
		bShowDifficulty = true;
		bShowStepsScore = true;
		break;
	default:
		ASSERT(0);
	}


	// Reset
	m_Banner.SetHidden( !bBanner );
	if( bBanner )
	{
		m_Banner.Reset();
		SET_XY_AND_ON_COMMAND( m_Banner );
	}

	m_sprBannerFrame.SetHidden( !bBannerFrame );
	if( bBannerFrame )
	{
		m_sprBannerFrame.Reset();
		SET_XY_AND_ON_COMMAND( m_sprBannerFrame );
	}

	m_textCategory.SetHidden( !bShowCategory );
	if( bShowCategory )
	{
		m_textCategory.Reset();
		SET_XY_AND_ON_COMMAND( m_textCategory );
	}

	m_textStepsType.SetHidden( !bShowStepsType );
	if( bShowStepsType )
	{
		m_textStepsType.Reset();
		SET_XY_AND_ON_COMMAND( m_textStepsType );
	}

	{
		for( int l=0; l<NUM_RANKING_LINES; l++ )
		{
			m_sprBullets[l].SetHidden( !bShowBullets );
			if( bShowBullets )
			{
				m_sprBullets[l].Reset();
				m_sprBullets[l].StopAnimating();
				m_sprBullets[l].SetState( l );
				m_sprBullets[l].SetXY( BULLET_X(l), BULLET_Y(l) );
				ON_COMMAND( m_sprBullets[l] );
			}

			m_textNames[l].SetHidden( !bShowNames );
			if( bShowNames )
			{
				m_textNames[l].Reset();
				m_textNames[l].SetXY( NAME_X(l), NAME_Y(l) );
				m_textNames[l].SetDiffuse( STEPS_TYPE_COLOR(pts.colorIndex) );
				ON_COMMAND( m_textNames[l] );
			}

			m_textScores[l].SetHidden( !bShowScores );
			if( bShowScores )
			{
				m_textScores[l].Reset();
				m_textScores[l].SetXY( SCORE_X(l), SCORE_Y(l) );
				m_textScores[l].SetDiffuse( STEPS_TYPE_COLOR(pts.colorIndex) );
				ON_COMMAND( m_textScores[l] );
			}
			
			m_textPoints[l].SetHidden( !bShowPoints );
			if( bShowPoints )
			{
				m_textPoints[l].Reset();
				m_textPoints[l].SetXY( POINTS_X(l), POINTS_Y(l) );
				m_textPoints[l].SetDiffuse( STEPS_TYPE_COLOR(pts.colorIndex) );
				ON_COMMAND( m_textPoints[l] );
			}
			
			m_textTime[l].SetHidden( !bShowTime );
			if( bShowTime )
			{
				m_textTime[l].Reset();
				m_textTime[l].SetXY( TIME_X(l), TIME_Y(l) );
				m_textTime[l].SetDiffuse( STEPS_TYPE_COLOR(pts.colorIndex) );
				ON_COMMAND( m_textTime[l] );
			}
		}
	}

	{
		const unsigned num_songs = SONGMAN->GetAllSongs().size();
		for( vector<Difficulty>::iterator dc_iter=m_vDiffsToShow.begin(); dc_iter!=m_vDiffsToShow.end(); dc_iter++ )
		{
			m_sprDifficulty[*dc_iter].SetHidden( !bShowDifficulty );
			if( bShowDifficulty )
			{
				m_sprDifficulty[*dc_iter].Reset();
				m_sprDifficulty[*dc_iter].StopAnimating();
				m_sprDifficulty[*dc_iter].SetState( *dc_iter );
				m_sprDifficulty[*dc_iter].SetXY( DIFFICULTY_X(*dc_iter), DIFFICULTY_Y );
				ON_COMMAND( m_sprDifficulty[*dc_iter] );
			}
		}

		m_ListScoreRowItems.SetHidden( !bShowStepsScore );
		if( bShowStepsScore )
		{
			m_ListScoreRowItems.Reset();
			SET_XY_AND_ON_COMMAND( m_ListScoreRowItems );

			vector<Actor*> vpActors;
			for( unsigned i=0; i<m_vpStepsScoreRowItem.size(); i++ )
				vpActors.push_back( m_vpStepsScoreRowItem[i] );
			m_ListScoreRowItems.Load( vpActors, SONG_SCORE_ROWS_TO_SHOW, SCREEN_WIDTH, ROW_SPACING_Y, false, SONG_SCORE_SECONDS_PER_ROW, 0, false );

			for( unsigned s=0; s<num_songs; s++ )
			{
				StepsScoreRowItem *pStepsScoreRowItems = m_vpStepsScoreRowItem[s];
				pStepsScoreRowItems->m_sprSongFrame.Reset();
				pStepsScoreRowItems->m_sprSongFrame.SetXY( SONG_FRAME_OFFSET_X, SONG_FRAME_OFFSET_Y );
				pStepsScoreRowItems->m_sprSongFrame.SetUseZBuffer( true );
				ON_COMMAND( pStepsScoreRowItems->m_sprSongFrame );

				pStepsScoreRowItems->m_textSongTitle.Reset();
				pStepsScoreRowItems->m_textSongTitle.SetXY( SONG_TITLE_OFFSET_X, SONG_TITLE_OFFSET_Y );
				pStepsScoreRowItems->m_textSongTitle.SetUseZBuffer( true );
				ON_COMMAND( pStepsScoreRowItems->m_textSongTitle );

				for( vector<Difficulty>::iterator dc_iter=m_vDiffsToShow.begin(); dc_iter!=m_vDiffsToShow.end(); dc_iter++ )
				{
					pStepsScoreRowItems->m_textStepsScore[*dc_iter].Reset();
					pStepsScoreRowItems->m_textStepsScore[*dc_iter].SetXY( STEPS_SCORE_OFFSET_X(*dc_iter), STEPS_SCORE_OFFSET_Y );
					pStepsScoreRowItems->m_textStepsScore[*dc_iter].SetUseZBuffer( true );
					pStepsScoreRowItems->m_textStepsScore[*dc_iter].SetDiffuse( STEPS_TYPE_COLOR(pts.colorIndex) );
					ON_COMMAND( pStepsScoreRowItems->m_textStepsScore[*dc_iter] );
				}
			}
		}
	}

	// get ranking feat list
	

	//
	// init page
	//
	switch( pts.type )
	{
	case PageToShow::TYPE_CATEGORY:
		{
			m_textCategory.SetText( ssprintf("Type %c", 'A'+pts.category) );
			m_textStepsType.SetText( GameManager::NotesTypeToString(pts.nt) );

			for( int l=0; l<NUM_RANKING_LINES; l++ )
			{
				ProfileManager::CategoryData::HighScore hs;
				bool bRecentHighScore = false;
				if( l < (int)PROFILEMAN->m_CategoryDatas[MEMORY_CARD_MACHINE][pts.nt][pts.category].vHighScores.size() )
				{
					hs = PROFILEMAN->m_CategoryDatas[MEMORY_CARD_MACHINE][pts.nt][pts.category].vHighScores[l];
					CString *psName = &PROFILEMAN->m_CategoryDatas[MEMORY_CARD_MACHINE][pts.nt][pts.category].vHighScores[l].sName;
					bRecentHighScore = find( GAMESTATE->m_vpsNamesThatWereFilled.begin(), GAMESTATE->m_vpsNamesThatWereFilled.end(), psName ) != GAMESTATE->m_vpsNamesThatWereFilled.end();
				}
				else
				{
					hs.sName = NO_SCORE_NAME;				
				}
				if( hs.sName.empty() )
					hs.sName = EMPTY_SCORE_NAME;

				m_textNames[l].SetText( hs.sName );
				m_textScores[l].SetText( ssprintf("%09i",hs.iScore) );
				m_textNames[l].SetDiffuse( STEPS_TYPE_COLOR(pts.colorIndex) );
				m_textScores[l].SetDiffuse( STEPS_TYPE_COLOR(pts.colorIndex) );

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
		return SECONDS_PER_PAGE;
	case PageToShow::TYPE_COURSE:
		{
			m_Banner.LoadFromCourse( pts.pCourse );
			m_textStepsType.SetText( GameManager::NotesTypeToString(pts.nt) );

			for( int l=0; l<NUM_RANKING_LINES; l++ )
			{
				Course::MemCardData::HighScore hs;
				bool bRecentHighScore = false;
				if( l < (int)pts.pCourse->m_MemCardDatas[pts.nt][MEMORY_CARD_MACHINE].vHighScores.size() )
				{
					hs = pts.pCourse->m_MemCardDatas[pts.nt][MEMORY_CARD_MACHINE].vHighScores[l];
					CString *psName = &pts.pCourse->m_MemCardDatas[pts.nt][MEMORY_CARD_MACHINE].vHighScores[l].sName;
					bRecentHighScore = find( GAMESTATE->m_vpsNamesThatWereFilled.begin(), GAMESTATE->m_vpsNamesThatWereFilled.end(), psName ) != GAMESTATE->m_vpsNamesThatWereFilled.end();
				}
				else
				{
					hs.sName = NO_SCORE_NAME;				
				}
				if( hs.sName.empty() )
					hs.sName = EMPTY_SCORE_NAME;
				
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
				m_textNames[l].SetDiffuse( STEPS_TYPE_COLOR(pts.colorIndex) );
				m_textPoints[l].SetDiffuse( STEPS_TYPE_COLOR(pts.colorIndex) );
				m_textTime[l].SetDiffuse( STEPS_TYPE_COLOR(pts.colorIndex) );
				m_textScores[l].SetDiffuse( STEPS_TYPE_COLOR(pts.colorIndex) );

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
		return SECONDS_PER_PAGE;
	case PageToShow::TYPE_ALL_STEPS:
		{
			m_textStepsType.SetText( GameManager::NotesTypeToString(pts.nt) );

			const unsigned num_songs = SONGMAN->GetAllSongs().size();
			for( unsigned s=0; s<num_songs; s++ )
			{
				Song* pSong = SONGMAN->GetAllSongs()[s];
				StepsScoreRowItem *pStepsScoreRowItem = m_vpStepsScoreRowItem[s];

				pStepsScoreRowItem->m_textSongTitle.SetText( pSong->GetFullDisplayTitle() );

				for( vector<Difficulty>::iterator dc_iter = m_vDiffsToShow.begin(); dc_iter != m_vDiffsToShow.end(); dc_iter++ )
				{							
					Steps* pSteps = pSong->GetStepsByDifficulty( pts.nt, *dc_iter, false );
					BitmapText* pTextStepsScore = &pStepsScoreRowItem->m_textStepsScore[*dc_iter];

					if( pSteps == NULL )
					{
						pTextStepsScore->SetHidden( true );
					}
					else
					{
						Steps::MemCardData::HighScore hs;
						bool bRecentHighScore = false;
						if( !pSteps->m_MemCardDatas[MEMORY_CARD_MACHINE].vHighScores.empty() )
						{
							hs = pSteps->m_MemCardDatas[MEMORY_CARD_MACHINE].vHighScores[0];
							CString *psName = &pSteps->m_MemCardDatas[MEMORY_CARD_MACHINE].vHighScores[0].sName;
							bRecentHighScore = find( GAMESTATE->m_vpsNamesThatWereFilled.begin(), GAMESTATE->m_vpsNamesThatWereFilled.end(), psName ) != GAMESTATE->m_vpsNamesThatWereFilled.end();
						}
						else
						{
							hs.sName = NO_SCORE_NAME;				
						}
						if( hs.sName.empty() )
							hs.sName = EMPTY_SCORE_NAME;

						CString s;
						s = hs.sName + "\n";
						s += ssprintf( "%0*.*f%%", PERCENT_TOTAL_SIZE, PERCENT_DECIMAL_PLACES, hs.fPercentDP*100 );
						pTextStepsScore->SetText( s );
					}
				}
			}
		}
		return m_ListScoreRowItems.GetSecondsForCompleteScrollThrough();
	default:
		ASSERT(0);
		return 0;
	}
}

void ScreenRanking::TweenPageOffScreen()
{
	OFF_COMMAND( m_Banner );
	OFF_COMMAND( m_sprBannerFrame );
	OFF_COMMAND( m_textCategory );
	OFF_COMMAND( m_textStepsType );

	for( int l=0; l<NUM_RANKING_LINES; l++ )
	{
		OFF_COMMAND( m_sprBullets[l] );
		OFF_COMMAND( m_textNames[l] );
		OFF_COMMAND( m_textScores[l] );
		OFF_COMMAND( m_textPoints[l] );
		OFF_COMMAND( m_textTime[l] );
	}
	const unsigned num_songs = SONGMAN->GetAllSongs().size();
	for( vector<Difficulty>::iterator dc_iter=m_vDiffsToShow.begin(); dc_iter!=m_vDiffsToShow.end(); dc_iter++ )
	{
		OFF_COMMAND( m_sprDifficulty[*dc_iter] );
		for( unsigned s=0; s<num_songs; s++ )
		{
			// m_vpTextStepsScore[*dc_iter][s];			
		}
	}
	OFF_COMMAND( m_ListScoreRowItems );
}
