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

static int g_iLastSongShown = 0;


#define CATEGORY_WIDTH				THEME->GetMetricF("ScreenRanking","CategoryWidth")
#define BANNER_WIDTH				THEME->GetMetricF("ScreenRanking","BannerWidth")
#define BANNER_HEIGHT				THEME->GetMetricF("ScreenRanking","BannerHeight")
#define LINE_SPACING_X				THEME->GetMetricF("ScreenRanking","LineSpacingX")
#define LINE_SPACING_Y				THEME->GetMetricF("ScreenRanking","LineSpacingY")
#define BULLETS_START_X				THEME->GetMetricF("ScreenRanking","BulletsStartX")
#define BULLETS_START_Y				THEME->GetMetricF("ScreenRanking","BulletsStartY")
#define BULLETS_ON_COMMAND( l )		THEME->GetMetric ("ScreenRanking",ssprintf("Bullets%dOnCommand",l+1))
#define BULLETS_OFF_COMMAND( l )	THEME->GetMetric ("ScreenRanking",ssprintf("Bullets%dOffCommand",l+1))
#define TEXT_ZOOM					THEME->GetMetricF("ScreenRanking","TextZoom")
#define TEXT_SONGZOOM				THEME->GetMetricF("ScreenRanking","TextSongZoom")
#define TEXT_COLOR( i )				THEME->GetMetricC("ScreenRanking",ssprintf("TextColor%d",i+1))
#define NAMES_START_X				THEME->GetMetricF("ScreenRanking","NamesStartX")
#define NAMES_START_Y				THEME->GetMetricF("ScreenRanking","NamesStartY")
#define NAMES_ON_COMMAND( l )		THEME->GetMetric ("ScreenRanking",ssprintf("Names%dOnCommand",l+1))
#define NAMES_OFF_COMMAND( l )		THEME->GetMetric ("ScreenRanking",ssprintf("Names%dOffCommand",l+1))
#define SCORES_START_X				THEME->GetMetricF("ScreenRanking","ScoresStartX")
#define SCORES_START_Y				THEME->GetMetricF("ScreenRanking","ScoresStartY")
#define SCORES_ON_COMMAND( l )		THEME->GetMetric ("ScreenRanking",ssprintf("Scores%dOnCommand",l+1))
#define SCORES_OFF_COMMAND( l )		THEME->GetMetric ("ScreenRanking",ssprintf("Scores%dOffCommand",l+1))
#define POINTS_START_X				THEME->GetMetricF("ScreenRanking","PointsStartX")
#define POINTS_START_Y				THEME->GetMetricF("ScreenRanking","PointsStartY")
#define POINTS_ON_COMMAND( l )		THEME->GetMetric ("ScreenRanking",ssprintf("Points%dOnCommand",l+1))
#define POINTS_OFF_COMMAND( l )		THEME->GetMetric ("ScreenRanking",ssprintf("Points%dOffCommand",l+1))
#define TIME_START_X				THEME->GetMetricF("ScreenRanking","TimeStartX")
#define TIME_START_Y				THEME->GetMetricF("ScreenRanking","TimeStartY")
#define TIME_ON_COMMAND( l )		THEME->GetMetric ("ScreenRanking",ssprintf("Time%dOnCommand",l+1))
#define TIME_OFF_COMMAND( l )		THEME->GetMetric ("ScreenRanking",ssprintf("Time%dOffCommand",l+1))
#define PERCENT_START_X				THEME->GetMetricF("ScreenRanking","PercentStartX")
#define PERCENT_START_Y				THEME->GetMetricF("ScreenRanking","PercentStartY")
#define PERCENT_ON_COMMAND( l )		THEME->GetMetric ("ScreenRanking",ssprintf("Percent%dOnCommand",l+1))
#define PERCENT_OFF_COMMAND( l )	THEME->GetMetric ("ScreenRanking",ssprintf("Percent%dOffCommand",l+1))
#define PERCENT_DECIMAL_PLACES		THEME->GetMetricI("ScreenRanking","PercentDecimalPlaces")
#define PERCENT_TOTAL_SIZE			THEME->GetMetricI("ScreenRanking","PercentTotalSize")
#define DIFFICULTY_START_X			THEME->GetMetricF("ScreenRanking","DifficultyStartX")
#define DIFFICULTY_START_Y			THEME->GetMetricF("ScreenRanking","DifficultyStartY")
#define DIFFICULTY_SPACING_X		THEME->GetMetricF("ScreenRanking","DifficultySpacingX")
#define DIFFICULTY_SPACING_Y		THEME->GetMetricF("ScreenRanking","DifficultySpacingY")
#define DIFFICULTY_ON_COMMAND		THEME->GetMetric ("ScreenRanking","DifficultyOnCommand")
#define DIFFICULTY_OFF_COMMAND		THEME->GetMetric ("ScreenRanking","DifficultyOffCommand")
#define SECONDS_PER_PAGE			THEME->GetMetricF("ScreenRanking","SecondsPerPage")
#define PAGE_FADE_SECONDS			THEME->GetMetricF("ScreenRanking","PageFadeSeconds")
#define SHOW_CATEGORIES				THEME->GetMetricB("ScreenRanking","ShowCategories")
#define SHOW_SONG_SCORES			THEME->GetMetricB("ScreenRanking","ShowSongScores")
#define NUM_SONGS_TO_SHOW			THEME->GetMetricI("ScreenRanking","NumSongsToShow")
#define NOTES_TYPES_TO_HIDE			THEME->GetMetric ("ScreenRanking","NotesTypesToHide")
#define EMPTY_SCORE_NAME			THEME->GetMetric ("ScreenRanking","EmptyScoreName")
#define DIFFICULTIES_TO_SHOW		THEME->GetMetric ("ScreenRanking","DifficultiesToShow")

#define COURSES_TO_SHOW			PREFSMAN->m_sCoursesToShowRanking

const ScreenMessage SM_ShowNextPage		=	(ScreenMessage)(SM_User+67);
const ScreenMessage SM_HidePage			=	(ScreenMessage)(SM_User+68);


ScreenRanking::ScreenRanking( CString sClassName ) : ScreenAttract( sClassName )
{
	m_sprCategory.SetName( "Category" );
	this->AddChild( &m_sprCategory );

	m_banner.SetName( "Banner" );
	this->AddChild( &m_banner );

	m_sprBannerFrame.SetName( "BannerFrame" );
	this->AddChild( &m_sprBannerFrame );

	m_textCategory.SetName( "Category" );
	m_textCategory.LoadFromFont( THEME->GetPathToF("ScreenRanking title") );
	m_textCategory.EnableShadow( false );
	this->AddChild( &m_textCategory );

	m_sprType.SetName( "Type" );
	this->AddChild( &m_sprType );


	// make the list of difficulties to show
	vector<CString> sShowDiffs;
	split( DIFFICULTIES_TO_SHOW, ",", sShowDiffs, true );

	for( vector<CString>::const_iterator iter = sShowDiffs.begin(); iter != sShowDiffs.end(); iter++ )
	{
		m_vDiffsToShow.push_back( StringToDifficulty( *iter ) );
	}
	

	for( int l=0; l<NUM_RANKING_LINES; l++ )
	{
		m_sprBullets[l].Load( THEME->GetPathToG("ScreenRanking bullets 1x5") );
		m_sprBullets[l].SetDiffuseAlpha( 0 );
		this->AddChild( &m_sprBullets[l] );

		m_textNames[l].LoadFromFont( THEME->GetPathToF("ScreenRanking letters") );
		m_textNames[l].SetDiffuseAlpha( 0 );
		this->AddChild( &m_textNames[l] );

		m_textScores[l].LoadFromFont( THEME->GetPathToF("ScreenRanking letters") );
		m_textScores[l].SetDiffuseAlpha( 0 );
		this->AddChild( &m_textScores[l] );

		m_textPoints[l].LoadFromFont( THEME->GetPathToF("ScreenRanking letters") );
		m_textPoints[l].SetDiffuseAlpha( 0 );
		this->AddChild( &m_textPoints[l] );
		
		m_textTime[l].LoadFromFont( THEME->GetPathToF("ScreenRanking letters") );
		m_textTime[l].SetDiffuseAlpha( 0 );
		this->AddChild( &m_textTime[l] );

		if( PREFSMAN->m_sCoursesToShowRanking == "" )
			PREFSMAN->m_sCoursesToShowRanking = THEME->GetMetric("ScreenRanking","CoursesToShow");
	}


	// init all the difficulty graphics
	for( int d=0; d<NUM_DIFFICULTIES; d++ )
	{
		bool bShowThis = find(m_vDiffsToShow.begin(), m_vDiffsToShow.end(), d) != m_vDiffsToShow.end();

		m_sprDiffHeaders[d].Load( THEME->GetPathToG(("ScreenRanking difficulty 1x5")) );
		m_sprDiffHeaders[d].SetDiffuseAlpha( 0 );
		if( bShowThis )
			this->AddChild( &m_sprDiffHeaders[d] );

		for( int l=0; l<NUM_RANKING_LINES; l++ )
		{
			m_textPercent[l][d].LoadFromFont( THEME->GetPathToF("ScreenRanking letters") );
			m_textPercent[l][d].SetDiffuseAlpha( 0 );
			if( bShowThis )
				this->AddChild( &m_textPercent[l][d] );
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

	if( SHOW_SONG_SCORES )
	{
		vector<Song*> vpSongs = SONGMAN->GetAllSongs();
		if( !vpSongs.empty() )
		{
			for( unsigned i=0; i<aNotesTypesToShow.size(); i++ )
			{
				/* Wrap first; the number of songs can change if we reload. */
				wrap( g_iLastSongShown,vpSongs.size() ); 

				int FirstSongShown = g_iLastSongShown;

				for( int j=0; j<NUM_SONGS_TO_SHOW; j++ )
				{
					PageToShow pts;
					pts.type = PageToShow::TYPE_SONG;
					pts.colorIndex = i;
					pts.pSong = vpSongs[g_iLastSongShown];
					pts.nt = aNotesTypesToShow[i];
					m_vPagesToShow.push_back( pts );
					
					g_iLastSongShown++;
					wrap( g_iLastSongShown,vpSongs.size() ); 

					if( g_iLastSongShown == FirstSongShown )
						break; /* don't show songs twice */
				}
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
			this->PostScreenMessage( SM_HidePage, SECONDS_PER_PAGE-PAGE_FADE_SECONDS );
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

void ScreenRanking::SetPage( PageToShow pts )
{
	bool bShowNames=false, bShowScores=false, bShowPoints=false, bShowTime=false, bShowDifficulty=false, bShowPercent=false;
	switch( pts.type )
	{
	case PageToShow::TYPE_CATEGORY:
		bShowNames = true;
		bShowScores = true;
		bShowPoints = false;
		bShowTime = false;
		bShowDifficulty = false;
		bShowPercent = false;
		break;
	case PageToShow::TYPE_COURSE:
		bShowNames = true;
		bShowScores = !pts.pCourse->IsOni();
		bShowPoints = pts.pCourse->IsOni();
		bShowTime = pts.pCourse->IsOni();
		bShowDifficulty = false;
		bShowPercent = false;
		break;
	case PageToShow::TYPE_SONG:
		bShowNames = false;
		bShowScores = false;
		bShowPoints = false;
		bShowTime = false;
		bShowDifficulty = true;
		bShowPercent = true;
		break;
	default:
		ASSERT(0);
	}


	int l, d;

	// Reset
	m_sprCategory.Reset();
	SET_XY_AND_ON_COMMAND( m_sprCategory );
	m_banner.Reset();
	SET_XY_AND_ON_COMMAND( m_banner );
	m_sprBannerFrame.Reset();
	SET_XY_AND_ON_COMMAND( m_sprBannerFrame );
	m_textCategory.Reset();
	SET_XY_AND_ON_COMMAND( m_textCategory );
	m_sprType.Reset();
	SET_XY_AND_ON_COMMAND( m_sprType );
	
	for( l=0; l<NUM_RANKING_LINES; l++ )
	{
		m_sprBullets[l].Reset();
		m_sprBullets[l].SetXY( BULLETS_START_X+LINE_SPACING_X*l, BULLETS_START_Y+LINE_SPACING_Y*l );
		m_sprBullets[l].StopAnimating();
		m_sprBullets[l].SetState(l);
		m_sprBullets[l].Command( BULLETS_ON_COMMAND(l) );

		m_textNames[l].Reset();
		m_textNames[l].SetXY( NAMES_START_X+LINE_SPACING_X*l, NAMES_START_Y+LINE_SPACING_Y*l );
		m_textNames[l].SetZoom( TEXT_ZOOM );
		m_textNames[l].SetDiffuseAlpha( 0 );
		m_textNames[l].EnableShadow( false );
		m_textNames[l].SetHorizAlign( Actor::align_left );
		if( bShowNames )
		{
			m_textNames[l].SetDiffuse( TEXT_COLOR(pts.colorIndex) );
			m_textNames[l].Command( NAMES_ON_COMMAND(l) );
		}

		m_textScores[l].Reset();
		m_textScores[l].SetXY( SCORES_START_X+LINE_SPACING_X*l, SCORES_START_Y+LINE_SPACING_Y*l );
		m_textScores[l].SetZoom( TEXT_ZOOM );
		m_textScores[l].SetDiffuseAlpha( 0 );
		m_textScores[l].EnableShadow( false );
		m_textScores[l].SetHorizAlign( Actor::align_right );
		if( bShowScores )
		{
			m_textScores[l].SetDiffuse( TEXT_COLOR(pts.colorIndex) );
			m_textScores[l].Command( SCORES_ON_COMMAND(l) );
		}

		m_textPoints[l].Reset();
		m_textPoints[l].SetXY( POINTS_START_X+LINE_SPACING_X*l, POINTS_START_Y+LINE_SPACING_Y*l );
		m_textPoints[l].SetZoom( TEXT_ZOOM );
		m_textPoints[l].SetDiffuseAlpha( 0 );
		m_textPoints[l].EnableShadow( false );
		m_textPoints[l].SetHorizAlign( Actor::align_right );
		if( bShowPoints )
		{
			m_textPoints[l].SetDiffuse( TEXT_COLOR(pts.colorIndex) );
			m_textPoints[l].Command( POINTS_ON_COMMAND(l) );
		}

		m_textTime[l].Reset();
		m_textTime[l].SetXY( TIME_START_X+LINE_SPACING_X*l, TIME_START_Y+LINE_SPACING_Y*l );
		m_textTime[l].SetZoom( TEXT_ZOOM );
		m_textTime[l].SetDiffuseAlpha( 0 );
		m_textTime[l].EnableShadow( false );
		m_textTime[l].SetHorizAlign( Actor::align_right );
		if( bShowTime )
		{
			m_textTime[l].SetDiffuse( TEXT_COLOR(pts.colorIndex) );
			m_textTime[l].Command( TIME_ON_COMMAND(l) );
		}
	}

	for( d=0; d<NUM_DIFFICULTIES; d++ )
	{
		m_sprDiffHeaders[d].Reset();
		m_sprDiffHeaders[d].SetXY( DIFFICULTY_START_X+DIFFICULTY_SPACING_X*d, DIFFICULTY_START_Y+DIFFICULTY_SPACING_Y*d );
		m_sprDiffHeaders[d].SetDiffuseAlpha( 0 );
		m_sprDiffHeaders[d].StopAnimating();
		m_sprDiffHeaders[d].SetState(d);
		if( bShowDifficulty )
		{
			m_sprDiffHeaders[d].SetDiffuseAlpha( 1 );
			m_sprDiffHeaders[d].Command( DIFFICULTY_ON_COMMAND );
		}

		for( int l=0; l<NUM_RANKING_LINES; l++ )
		{
			m_textPercent[l][d].Reset();
			m_textPercent[l][d].SetZoom( TEXT_SONGZOOM );
			m_textPercent[l][d].SetXY( PERCENT_START_X+DIFFICULTY_SPACING_X*d, PERCENT_START_Y+LINE_SPACING_Y*l );
			m_textPercent[l][d].SetDiffuseAlpha( 0 );
			m_textPercent[l][d].EnableShadow( false );
			if( bShowPercent )
			{
				m_textPercent[l][d].SetDiffuse( TEXT_COLOR(pts.colorIndex) );
				m_textPercent[l][d].Command( PERCENT_ON_COMMAND(l) );
			}
		}
	}


	// init page
	switch( pts.type )
	{
	case PageToShow::TYPE_CATEGORY:
		{
			m_sprCategory.Load( THEME->GetPathToG(ssprintf("ScreenRanking category %c", 'A'+pts.category)) );
			m_sprType.Load( THEME->GetPathToG("ScreenRanking type "+GameManager::NotesTypeToString(pts.nt)) );

			for( int l=0; l<NUM_RANKING_LINES; l++ )
			{
				ProfileManager::CategoryData::HighScore hs;
				if( l < (int)PROFILEMAN->m_CategoryDatas[MEMORY_CARD_MACHINE][pts.nt][pts.category].vHighScores.size() )
					hs = PROFILEMAN->m_CategoryDatas[MEMORY_CARD_MACHINE][pts.nt][pts.category].vHighScores[l];
				if( hs.sName.empty() )
					hs.sName = EMPTY_SCORE_NAME;

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
				m_textCategory.SetMaxWidth( CATEGORY_WIDTH );
				m_textCategory.SetText( pts.pCourse->m_sName );
				m_textCategory.SetDiffuseAlpha(1);
			}

			m_sprType.Load( THEME->GetPathToG("ScreenRanking type "+GameManager::NotesTypeToString(pts.nt)) );
			for( int l=0; l<NUM_RANKING_LINES; l++ )
			{
				Course::MemCardData::HighScore hs;
				if( l < (int)pts.pCourse->m_MemCardDatas[pts.nt][MEMORY_CARD_MACHINE].vHighScores.size() )
					hs = pts.pCourse->m_MemCardDatas[pts.nt][MEMORY_CARD_MACHINE].vHighScores[l];
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
			m_banner.ScaleToClipped( BANNER_WIDTH, BANNER_HEIGHT );
			m_banner.SetDiffuseAlpha(1);

			m_sprBannerFrame.Load( THEME->GetPathToG("ScreenRanking banner frame") );
			m_sprBannerFrame.SetDiffuseAlpha(1);

			m_textCategory.SetMaxWidth( CATEGORY_WIDTH );
			m_textCategory.SetText( pts.pSong->GetFullTranslitTitle() );
			m_textCategory.SetDiffuseAlpha(1);
		
			m_sprType.SetDiffuse( RageColor(1,1,1,1) );
			m_sprType.Load( THEME->GetPathToG("ScreenRanking type "+GameManager::NotesTypeToString(pts.nt)) );

			for( DiffVectorCIter dIter = m_vDiffsToShow.begin(); dIter != m_vDiffsToShow.end(); dIter++ )
			{
				int d = (int)*dIter;

				Difficulty dc = (Difficulty)d;
				Steps* pSteps = pts.pSong->GetStepsByDifficulty( pts.nt, dc );

									
				if( pSteps == NULL )
					m_sprDiffHeaders[d].SetHidden( true );
				else
					m_sprDiffHeaders[d].SetDiffuse( RageColor(1,1,1,1) );	

				for( int l=0; l<NUM_RANKING_LINES; l++ )
				{
					if( pSteps == NULL )
					{
						m_textPercent[l][d].SetHidden( true );
					}
					else
					{
						Steps::MemCardData::HighScore hs;
						if( l < (int)pSteps->m_MemCardDatas[MEMORY_CARD_MACHINE].vHighScores.size() )
							hs = pSteps->m_MemCardDatas[MEMORY_CARD_MACHINE].vHighScores[l];
						if( hs.sName.empty() )
							hs.sName = EMPTY_SCORE_NAME;

						CString s;
						s = hs.sName + "\n";
						s += ssprintf( "%0*.*f%%", PERCENT_TOTAL_SIZE, PERCENT_DECIMAL_PLACES, hs.fPercentDP*100 );
						m_textPercent[l][d].SetText( s );
						m_textPercent[l][d].SetDiffuse( TEXT_COLOR(pts.colorIndex) );
					}
				}
			}
		}
		break;
	default:
		ASSERT(0);
	}
}

void ScreenRanking::TweenPageOffScreen()
{
	int l, d;

	OFF_COMMAND( m_sprCategory );
	OFF_COMMAND( m_banner );
	OFF_COMMAND( m_sprBannerFrame );
	OFF_COMMAND( m_sprType );
	OFF_COMMAND( m_textCategory );

	for( l=0; l<NUM_RANKING_LINES; l++ )
	{
		m_sprBullets[l].Command( BULLETS_OFF_COMMAND(l) );
		m_textNames[l].Command( NAMES_OFF_COMMAND(l) );
		m_textScores[l].Command( SCORES_OFF_COMMAND(l) );
		m_textPoints[l].Command( POINTS_OFF_COMMAND(l) );
		m_textTime[l].Command( TIME_OFF_COMMAND(l) );
		for( d=0; d<NUM_DIFFICULTIES; d++ )
			m_textPercent[l][d].Command( PERCENT_OFF_COMMAND(l) );
	}
	for( d=0; d<NUM_DIFFICULTIES; d++ )
		m_sprDiffHeaders[d].Command( DIFFICULTY_OFF_COMMAND );
}
