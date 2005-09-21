#include "global.h"
#include "ScreenRanking.h"
#include "ScreenManager.h"
#include "ThemeManager.h"
#include "SongManager.h"
#include "GameState.h"
#include "GameManager.h"
#include "Course.h"
#include "song.h"
#include "Steps.h"
#include "PrefsManager.h"
#include "ActorUtil.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "RageLog.h"
#include "UnlockManager.h"
#include "ScreenDimensions.h"
#include "PercentageDisplay.h"

static const CString PageTypeNames[] = {
	"Category",
	"Course",
	"AllSteps",
	"NonstopCourses",
	"OniCourses",
	"SurvivalCourses",
};
XToString( PageType, NUM_PAGE_TYPES );
StringToX( PageType );

#define TYPE						THEME->GetMetric(m_sName,"Type")

#define COURSES_TO_SHOW				PREFSMAN->m_sCoursesToShowRanking
#define COURSES_TO_SHOW2			THEME->GetMetric(m_sName,"CoursesToShow")

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
#define SCORE_OFFSET_X(col)			(SCORE_OFFSET_START_X+COL_SPACING_X*col)

#define COURSE_DIFFICULTY_X(col)	(COURSE_DIFFICULTY_START_X+COL_SPACING_X*col)

AutoScreenMessage( SM_ShowNextPage )
AutoScreenMessage( SM_HidePage )


static void GetAllSongsToShow( vector<Song*> &vpOut, bool bShowOnlyMostRecentScores, int iNumMostRecentScoresToShow )
{
	vpOut.clear();
	FOREACH_CONST( Song*, SONGMAN->GetAllSongs(), s )
	{
		if( UNLOCKMAN->SongIsLocked(*s) )
			continue;	// skip
		if( !(*s)->ShowInDemonstrationAndRanking() )
			continue;	// skip
		vpOut.push_back( *s );
	}

	if( bShowOnlyMostRecentScores )
	{
		SongUtil::SortSongPointerArrayByTitle( vpOut );
		SongUtil::SortByMostRecentlyPlayedForMachine( vpOut );
		if( (int) vpOut.size() > iNumMostRecentScoresToShow )
			vpOut.erase( vpOut.begin()+iNumMostRecentScoresToShow, vpOut.end() );
	}
}

static void GetAllCoursesToShow( vector<Course*> &vpOut, CourseType ct, bool bShowOnlyMostRecentScores, int iNumMostRecentScoresToShow )
{
	vpOut.clear();
	vector<Course*> vpCourses;
	SONGMAN->GetCourses( ct, vpCourses, false );

	FOREACH_CONST( Course*, vpCourses, c)
	{
		if( UNLOCKMAN->CourseIsLocked(*c) )
			continue;	// skip
		if( !(*c)->ShowInDemonstrationAndRanking() )
			continue;	// skip
		vpOut.push_back( *c );
	}
	if( bShowOnlyMostRecentScores )
	{
		CourseUtil::SortCoursePointerArrayByTitle( vpOut );
		CourseUtil::SortByMostRecentlyPlayedForMachine( vpOut );
		if( (int) vpOut.size() > iNumMostRecentScoresToShow )
			vpOut.erase( vpOut.begin()+iNumMostRecentScoresToShow, vpOut.end() );
	}
}


CString STEPS_TYPE_COLOR_NAME( size_t i ) { return ssprintf("StepsTypeColor%d",int(i+1)); }

REGISTER_SCREEN_CLASS( ScreenRanking );
ScreenRanking::ScreenRanking( CString sClassName ) : ScreenAttract( sClassName, false/*dont reset GAMESTATE*/ ),
	STEPS_TYPES_TO_SHOW			(m_sName,"StepsTypesToHide"),
	DIFFICULTIES_TO_SHOW		(m_sName,"DifficultiesToShow"),

	SHOW_ONLY_MOST_RECENT_SCORES	(m_sName,"ShowOnlyMostRecentScores"),
	NUM_MOST_RECENT_SCORES_TO_SHOW	(m_sName,"NumMostRecentScoresToShow"),
	SECONDS_PER_PAGE			(m_sName,"SecondsPerPage"),
	PAGE_FADE_SECONDS			(m_sName,"PageFadeSeconds"),
	NO_SCORE_NAME				(m_sName,"NoScoreName"),

	ROW_SPACING_X				(m_sName,"RowSpacingX"),
	ROW_SPACING_Y				(m_sName,"RowSpacingY"),
	COL_SPACING_X				(m_sName,"ColSpacingX"),
	COL_SPACING_Y				(m_sName,"ColSpacingY"),
	STEPS_TYPE_COLOR			(m_sName,STEPS_TYPE_COLOR_NAME,5),
	SONG_SCORE_ROWS_TO_SHOW		(m_sName,"SongScoreRowsToShow"),
	SONG_SCORE_SECONDS_PER_ROW	(m_sName,"SongScoreSecondsPerRow"),
	MANUAL_SCROLLING			(m_sName,"ManualScroling"),
	SHOW_SURVIVAL_TIME			(m_sName,"ShowSurvivalTime"),
	
	BULLET_START_X				(m_sName,"BulletStartX"),
	BULLET_START_Y				(m_sName,"BulletStartY"),
	NAME_START_X				(m_sName,"NameStartX"),
	NAME_START_Y				(m_sName,"NameStartY"),
	SCORE_START_X				(m_sName,"ScoreStartX"),
	SCORE_START_Y				(m_sName,"ScoreStartY"),
	POINTS_START_X				(m_sName,"PointsStartX"),
	POINTS_START_Y				(m_sName,"PointsStartY"),
	TIME_START_X				(m_sName,"TimeStartX"),
	TIME_START_Y				(m_sName,"TimeStartY"),
	DIFFICULTY_START_X			(m_sName,"DifficultyStartX"),
	DIFFICULTY_Y				(m_sName,"DifficultyY"),
	COURSE_DIFFICULTY_START_X	(m_sName,"CourseDifficultyStartX"),
	COURSE_DIFFICULTY_Y			(m_sName,"CourseDifficultyY"),
	SCORE_OFFSET_START_X		(m_sName,"ScoreOffsetStartX"),
	SCORE_OFFSET_Y				(m_sName,"ScoreOffsetY")
{
}

void ScreenRanking::Init()
{
	ScreenAttract::Init();

	m_PageType = StringToPageType( TYPE );

	// init Actors for category and course
	{
		if( m_PageType == PAGE_TYPE_TRAIL )
		{
			m_Banner.SetName( "Banner" );
			this->AddChild( &m_Banner );

			m_textCourseTitle.SetName( "CourseTitle" );
			m_textCourseTitle.LoadFromFont( THEME->GetPathF(m_sName,"course title") );
			m_textCourseTitle.SetShadowLength( 0 );
			this->AddChild( &m_textCourseTitle );
		}

		m_sprBannerFrame.SetName( "BannerFrame" );
		this->AddChild( &m_sprBannerFrame );

		if( m_PageType == PAGE_TYPE_CATEGORY )
		{
			m_textCategory.SetName( "Category" );
			m_textCategory.LoadFromFont( THEME->GetPathF(m_sName,"category") );
			m_textCategory.SetShadowLength( 0 );
			this->AddChild( &m_textCategory );
		}

		m_textStepsType.SetName( "StepsType" );
		m_textStepsType.LoadFromFont( THEME->GetPathF(m_sName,"steps type") );
		m_textStepsType.SetShadowLength( 0 );
		this->AddChild( &m_textStepsType );

		m_sprPageType.Load( THEME->GetPathG(m_sName, "PageType "+PageTypeToString(m_PageType)) );
		m_sprPageType->SetName( "PageType" );
		this->AddChild( m_sprPageType );

		switch( m_PageType )
		{
		case PAGE_TYPE_CATEGORY:
		case PAGE_TYPE_TRAIL:
			for( int l=0; l<NUM_RANKING_LINES; l++ )
			{
				m_sprBullets[l].SetName( ssprintf("Bullet%d",l+1) );
				m_sprBullets[l].Load( THEME->GetPathG( m_sName, ssprintf("bullets 1x%d",NUM_RANKING_LINES) ) );
				m_sprBullets[l].StopAnimating();
				this->AddChild( &m_sprBullets[l] );

				m_textNames[l].SetName( ssprintf("Name%d",l+1) );
				m_textNames[l].LoadFromFont( THEME->GetPathF(m_sName,"name") );
				this->AddChild( &m_textNames[l] );

				m_textScores[l].SetName( ssprintf("Score%d",l+1) );
				m_textScores[l].LoadFromFont( THEME->GetPathF(m_sName,"score") );
				this->AddChild( &m_textScores[l] );

				m_textPoints[l].SetName( ssprintf("Points%d",l+1) );
				m_textPoints[l].LoadFromFont( THEME->GetPathF(m_sName,"points") );
				m_textPoints[l].SetHidden( true );
				this->AddChild( &m_textPoints[l] );
				
				m_textTime[l].SetName( ssprintf("Time%d",l+1) );
				m_textTime[l].LoadFromFont( THEME->GetPathF(m_sName,"time") );
				m_textTime[l].SetHidden( true );
				this->AddChild( &m_textTime[l] );
			}
		}

		// TODO: Think of a better way to handle this
		if( PREFSMAN->m_sCoursesToShowRanking.Get() == "" )
			PREFSMAN->m_sCoursesToShowRanking.Set( COURSES_TO_SHOW2 );
	}

	//
	// fill m_vPagesToShow
	//
	if( m_PageType == PAGE_TYPE_CATEGORY )
	{
		for( unsigned i=0; i<STEPS_TYPES_TO_SHOW.GetValue().size(); i++ )
		{
			for( int c=0; c<NUM_RANKING_CATEGORIES; c++ )
			{
				PageToShow pts;
				pts.colorIndex = i;
				pts.category = (RankingCategory)c;
				pts.st = STEPS_TYPES_TO_SHOW.GetValue()[i];
				m_vPagesToShow.push_back( pts );
			}
		}
	}

	{
		vector<CString> asCoursePaths;
		split( COURSES_TO_SHOW, ",", asCoursePaths, true );
		for( unsigned i=0; i<STEPS_TYPES_TO_SHOW.GetValue().size(); i++ )
		{
			for( unsigned c=0; c<asCoursePaths.size(); c++ )
			{
				PageToShow pts;
				pts.colorIndex = i;
				pts.st = STEPS_TYPES_TO_SHOW.GetValue()[i];
				pts.pCourse = SONGMAN->GetCourseFromPath( asCoursePaths[c] );
				if( pts.pCourse == NULL )
					continue;

				pts.pTrail = pts.pCourse->GetTrail( pts.st );
				if( pts.pTrail == NULL )
					continue;

				m_vPagesToShow.push_back( pts );
			}
		}
	}


	if( m_PageType == PAGE_TYPE_ALL_STEPS )
	{
		m_vScoreRowItem.clear();
		vector<Song*> vpSongs;
		GetAllSongsToShow( vpSongs, SHOW_ONLY_MOST_RECENT_SCORES, NUM_MOST_RECENT_SCORES_TO_SHOW );
		m_vScoreRowItem.resize( vpSongs.size() );
		FOREACH_CONST( Song*, vpSongs, s )
		{
			int i = s - vpSongs.begin();
			Song *pSong = *s;
			ScoreRowItem &item = m_vScoreRowItem[i];
			item.m_pSong = pSong;

			item.m_sprFrame.Load( THEME->GetPathG(m_sName,"song frame") );
			item.m_sprFrame->SetName( "Frame" );
			item.AddChild( item.m_sprFrame );

			item.m_textTitle.SetName( "Title" );
			item.m_textTitle.LoadFromFont( THEME->GetPathF(m_sName,"song title") );
			item.AddChild( &item.m_textTitle );

			for( int d=0; d<NUM_DIFFICULTIES; d++ )
			{
				item.m_textScore[d].SetName( "Score" );
				item.m_textScore[d].LoadFromFont( THEME->GetPathF(m_sName,"steps score") );
				item.AddChild( &item.m_textScore[d] );
			}
		}

		m_ListScoreRowItems.SetName( "ListScoreRowItems" );
		this->AddChild( &m_ListScoreRowItems );

		FOREACH_CONST( Difficulty, DIFFICULTIES_TO_SHOW.GetValue(), d )
		{
			m_sprDifficulty[*d].Load( THEME->GetPathG(m_sName,"difficulty "+DifficultyToString(*d)) );
			m_sprDifficulty[*d]->SetName( ssprintf("Difficulty%d",(*d)+1) );
			this->AddChild( m_sprDifficulty[*d] );
		}

		if( !vpSongs.empty() )
		{
			for( unsigned i=0; i<STEPS_TYPES_TO_SHOW.GetValue().size(); i++ )
			{
				PageToShow pts;
				pts.colorIndex = i;
				pts.st = STEPS_TYPES_TO_SHOW.GetValue()[i];
				m_vPagesToShow.push_back( pts );
			}
		}
	}

	if( m_PageType == PAGE_TYPE_NONSTOP_COURSES ||
		m_PageType == PAGE_TYPE_ONI_COURSES ||
		m_PageType == PAGE_TYPE_SURVIVAL_COURSES )
	{
		CourseType ct = m_PageType == PAGE_TYPE_NONSTOP_COURSES? COURSE_TYPE_NONSTOP :
						m_PageType == PAGE_TYPE_ONI_COURSES? COURSE_TYPE_ONI :
							COURSE_TYPE_SURVIVAL;
		vector<Course*> vpCourses;
		GetAllCoursesToShow( vpCourses, ct, SHOW_ONLY_MOST_RECENT_SCORES, NUM_MOST_RECENT_SCORES_TO_SHOW );
		LOG->Trace("rankings: adding %u courses", unsigned(vpCourses.size()));
		m_vScoreRowItem.resize( vpCourses.size() );

		FOREACH_CONST( Course*, vpCourses, c )
		{
			Course* pCourse = *c;
			int i = c - vpCourses.begin();

			ScoreRowItem &item = m_vScoreRowItem[i];
			item.m_pCourse = pCourse;

			item.m_sprFrame.Load( THEME->GetPathG(m_sName,"course frame") );
			item.m_sprFrame->SetName( "Frame" );
			item.AddChild( item.m_sprFrame );

			item.m_textTitle.SetName( "Title" );
			item.m_textTitle.LoadFromFont( THEME->GetPathF(m_sName,"course list title") );
			item.AddChild( &item.m_textTitle );

			FOREACH_CONST( Difficulty, DIFFICULTIES_TO_SHOW.GetValue(), cd )
			{
				item.m_textScore[*cd].SetName( "Score" );
				item.m_textScore[*cd].LoadFromFont( THEME->GetPathF(m_sName,"course list score") );
				item.AddChild( &item.m_textScore[*cd] );
			}
		}

		m_ListScoreRowItems.SetName( "ListScoreRowItems" );
		this->AddChild( &m_ListScoreRowItems );
		m_ListScoreRowItems.SetXY( SCREEN_CENTER_X, SCREEN_CENTER_Y );

		FOREACH_CONST( Difficulty, DIFFICULTIES_TO_SHOW.GetValue(), cd )
		{
			m_sprCourseDifficulty[*cd].Load( THEME->GetPathG(m_sName,"CourseDifficulty "+CourseDifficultyToString(*cd)) );
			m_sprCourseDifficulty[*cd]->SetName( "CourseDifficulty"+CourseDifficultyToString(*cd) );
			this->AddChild( m_sprCourseDifficulty[*cd] );
		}

		if( !vpCourses.empty() )
		{
			for( unsigned i=0; i<STEPS_TYPES_TO_SHOW.GetValue().size(); i++ )
			{
				PageToShow pts;
				pts.colorIndex = i;
				pts.st = STEPS_TYPES_TO_SHOW.GetValue()[i];
				m_vPagesToShow.push_back( pts );
			}
		}
	}

	this->HandleScreenMessage( SM_ShowNextPage );
}

ScreenRanking::~ScreenRanking()
{
}

void ScreenRanking::Input( const InputEventPlus &input )
{
	LOG->Trace( "ScreenRanking::Input()" );

	// If manually scrolling, then pass the input to Scree::Input so it will call Menu*
	if( (bool)MANUAL_SCROLLING )
		Screen::Input( input );
	else
		ScreenAttract::Input( input );
}

void ScreenRanking::Scroll( int iDir )
{
	float fDest = m_ListScoreRowItems.GetDestinationItem();
	float fOldDest = fDest;
	fDest += iDir;
	CLAMP( fDest, (SONG_SCORE_ROWS_TO_SHOW-1)/2.0f, m_vScoreRowItem.size()-(SONG_SCORE_ROWS_TO_SHOW-1)/2.0f-1 );
	if( fOldDest != fDest )
	{
		// TODO: play sound
		m_ListScoreRowItems.SetDestinationItem( fDest );
		m_ListScoreRowItems.SetDestinationItem( fDest );
	}
}

void ScreenRanking::MenuStart( PlayerNumber pn )
{
	if( !IsTransitioning() )
		StartTransitioning( SM_GoToNextScreen );
}

void ScreenRanking::MenuBack( PlayerNumber pn )
{
	if( !IsTransitioning() )
		StartTransitioning( SM_GoToNextScreen );
}

void ScreenRanking::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_ShowNextPage )
	{
		if( m_vPagesToShow.size() > 0 )
		{
			float fSecsToShow = SetPage( m_vPagesToShow[0] );
			this->SortByDrawOrder();
			m_vPagesToShow.erase( m_vPagesToShow.begin() );
			
			// If manually scrolling, don't automatically change pages.
			if( !(bool)MANUAL_SCROLLING )
				this->PostScreenMessage( SM_HidePage, fSecsToShow-PAGE_FADE_SECONDS );
		}
		else
		{
			StartTransitioning(SM_GoToNextScreen);
		}
	}
	else if( SM == SM_HidePage )
	{
		TweenPageOffScreen();
		this->PostScreenMessage( SM_ShowNextPage, PAGE_FADE_SECONDS );
	}

	ScreenAttract::HandleScreenMessage( SM );
}

float ScreenRanking::SetPage( PageToShow pts )
{
	// This is going to take a while to load.  Possibly longer than one frame.
	// So, zero the next update so we don't skip.
	SCREENMAN->ZeroNextUpdate();

	SET_XY_AND_ON_COMMAND( m_textStepsType );
	SET_XY_AND_ON_COMMAND( m_sprPageType );

	switch( m_PageType )
	{
	case PAGE_TYPE_CATEGORY:
	case PAGE_TYPE_TRAIL:
		bool bShowScores = false;
		bool bShowPoints = false;
		bool bShowTime = false;
		switch( m_PageType )
		{
		case PAGE_TYPE_CATEGORY:
			bShowScores = true;
			break;
		case PAGE_TYPE_TRAIL:
			bShowScores = !pts.pCourse->IsOni();
			bShowPoints = pts.pCourse->IsOni();
			bShowTime = pts.pCourse->IsOni();
			break;
		}

		for( int l=0; l<NUM_RANKING_LINES; l++ )
		{
			m_sprBullets[l].SetState( l );
			m_sprBullets[l].SetXY( BULLET_X(l), BULLET_Y(l) );
			ON_COMMAND( m_sprBullets[l] );

			m_textNames[l].SetXY( NAME_X(l), NAME_Y(l) );
			m_textNames[l].SetDiffuse( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );
			ON_COMMAND( m_textNames[l] );

			m_textScores[l].SetHidden( !bShowScores );
			if( bShowScores )
			{
				m_textScores[l].SetXY( SCORE_X(l), SCORE_Y(l) );
				m_textScores[l].SetDiffuse( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );
				ON_COMMAND( m_textScores[l] );
			}
			
			m_textPoints[l].SetHidden( !bShowPoints );
			if( bShowPoints )
			{
				m_textPoints[l].SetXY( POINTS_X(l), POINTS_Y(l) );
				m_textPoints[l].SetDiffuse( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );
				ON_COMMAND( m_textPoints[l] );
			}
			
			m_textTime[l].SetHidden( !bShowTime );
			if( bShowTime )
			{
				m_textTime[l].SetXY( TIME_X(l), TIME_Y(l) );
				m_textTime[l].SetDiffuse( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );
				ON_COMMAND( m_textTime[l] );
			}
		}
	}

	for( unsigned s=0; s<m_vScoreRowItem.size(); s++ )
	{
		ScoreRowItem &item = m_vScoreRowItem[s];
		item.m_sprFrame->SetZTestMode( ZTEST_WRITE_ON_PASS );
		ON_COMMAND( item.m_sprFrame );

		item.m_textTitle.SetZTestMode( ZTEST_WRITE_ON_PASS );
		ON_COMMAND( item.m_textTitle );

		FOREACH_CONST( Difficulty, DIFFICULTIES_TO_SHOW.GetValue(), cd )
		{
			item.m_textScore[*cd].SetXY( SCORE_OFFSET_X(*cd), SCORE_OFFSET_Y );
			item.m_textScore[*cd].SetZTestMode( ZTEST_WRITE_ON_PASS );
			item.m_textScore[*cd].SetDiffuse( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );
			ON_COMMAND( item.m_textScore[*cd] );
		}
	}

	switch( m_PageType )
	{
	case PAGE_TYPE_NONSTOP_COURSES:
	case PAGE_TYPE_ONI_COURSES:
	case PAGE_TYPE_SURVIVAL_COURSES:
		SET_XY_AND_ON_COMMAND( m_ListScoreRowItems );

		m_ListScoreRowItems.RemoveAllChildren();
		for( unsigned i=0; i<m_vScoreRowItem.size(); i++ )
			m_ListScoreRowItems.AddChild( &m_vScoreRowItem[i] );
		m_ListScoreRowItems.Load2( (float)SONG_SCORE_ROWS_TO_SHOW, SCREEN_WIDTH, ROW_SPACING_Y, false, SONG_SCORE_SECONDS_PER_ROW, 0 );

		if( (bool)MANUAL_SCROLLING )
		{
			m_ListScoreRowItems.SetCurrentAndDestinationItem( (SONG_SCORE_ROWS_TO_SHOW-1)/2.0f );
		}
	}
	
	//
	// init page
	//
	switch( m_PageType )
	{
	case PAGE_TYPE_CATEGORY:
		{
			SET_XY_AND_ON_COMMAND( m_textCategory );
			m_textCategory.SetText( ssprintf("Type %c", 'A'+pts.category) );
			m_textStepsType.SetText( GameManager::StepsTypeToThemedString(pts.st) );

			for( int l=0; l<NUM_RANKING_LINES; l++ )
			{
				HighScoreList& hsl = PROFILEMAN->GetMachineProfile()->GetCategoryHighScoreList(pts.st,pts.category);
				HighScore hs;
				bool bRecentHighScore = false;
				if( l < (int)hsl.vHighScores.size() )
				{
					hs = hsl.vHighScores[l];
					CString *psName = hsl.vHighScores[l].GetNameMutable();
					bRecentHighScore = find( GAMESTATE->m_vpsNamesThatWereFilled.begin(), GAMESTATE->m_vpsNamesThatWereFilled.end(), psName ) != GAMESTATE->m_vpsNamesThatWereFilled.end();
				}
				else
				{
					hs.SetName( NO_SCORE_NAME );				
				}

				m_textNames[l].SetText( hs.GetDisplayName() );
				m_textScores[l].SetText( ssprintf("%09i",hs.GetScore()) );
				m_textNames[l].SetDiffuse( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );
				m_textScores[l].SetDiffuse( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );

				if( bRecentHighScore )
				{
					m_textNames[l].SetEffectGlowBlink(0.1f);
					m_textScores[l].SetEffectGlowBlink(0.1f);
				}
				else
				{
					m_textNames[l].StopEffect();
					m_textScores[l].StopEffect();
				}
			}
		}
		return SECONDS_PER_PAGE;
	case PAGE_TYPE_TRAIL:
		{
			m_textCourseTitle.SetText( pts.pCourse->GetDisplayFullTitle() );
			SET_XY_AND_ON_COMMAND( m_textCourseTitle );

			m_Banner.LoadFromCourse( pts.pCourse );
			SET_XY_AND_ON_COMMAND( m_Banner );
			SET_XY_AND_ON_COMMAND( m_sprBannerFrame );
			m_textStepsType.SetText( GameManager::StepsTypeToThemedString(pts.st) );

			const HighScoreList &hsl = PROFILEMAN->GetMachineProfile()->GetCourseHighScoreList( pts.pCourse, pts.pTrail );
			for( int l=0; l<NUM_RANKING_LINES; l++ )
			{
				HighScore hs;
				bool bRecentHighScore = false;
				if( l < (int)hsl.vHighScores.size() )
				{
					hs = hsl.vHighScores[l];
					const CString *psName = hsl.vHighScores[l].GetNameMutable();
					bRecentHighScore = find( GAMESTATE->m_vpsNamesThatWereFilled.begin(), GAMESTATE->m_vpsNamesThatWereFilled.end(), psName ) != GAMESTATE->m_vpsNamesThatWereFilled.end();
				}
				else
				{
					hs.SetName( NO_SCORE_NAME );				
				}

				m_textNames[l].SetText( hs.GetDisplayName() );
				if( pts.pCourse->IsOni() )
				{
					m_textPoints[l].SetText( ssprintf("%04d",hs.GetScore()) );
					m_textTime[l].SetText( SecondsToMMSSMsMs(hs.GetSurviveSeconds()) );
					m_textScores[l].SetText( "" );
				} else {
					m_textPoints[l].SetText( "" );
					m_textTime[l].SetText( "" );
					m_textScores[l].SetText( ssprintf("%09d",hs.GetScore()) );
				}
				m_textNames[l].SetDiffuse( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );
				m_textPoints[l].SetDiffuse( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );
				m_textTime[l].SetDiffuse( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );
				m_textScores[l].SetDiffuse( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );

				if( bRecentHighScore )
				{
					m_textNames[l].SetEffectGlowBlink(0.1f);
					m_textScores[l].SetEffectGlowBlink(0.1f);
				}
				else
				{
					m_textNames[l].StopEffect();
					m_textScores[l].StopEffect();
				}
			}
		}
		return SECONDS_PER_PAGE;
	case PAGE_TYPE_ALL_STEPS:
		{
			SET_XY_AND_ON_COMMAND( m_ListScoreRowItems );

			m_ListScoreRowItems.RemoveAllChildren();
			for( unsigned i=0; i<m_vScoreRowItem.size(); i++ )
				m_ListScoreRowItems.AddChild( &m_vScoreRowItem[i] );
			m_ListScoreRowItems.Load2( (float)SONG_SCORE_ROWS_TO_SHOW, SCREEN_WIDTH, ROW_SPACING_Y, false, SONG_SCORE_SECONDS_PER_ROW, 0 );

			if( (bool)MANUAL_SCROLLING )
			{
				m_ListScoreRowItems.SetCurrentAndDestinationItem( (SONG_SCORE_ROWS_TO_SHOW-1)/2.0f );
			}

			m_textStepsType.SetText( GameManager::StepsTypeToThemedString(pts.st) );

			FOREACH_CONST( Difficulty, DIFFICULTIES_TO_SHOW.GetValue(), iter )
			{
				m_sprDifficulty[*iter]->SetXY( DIFFICULTY_X(*iter), DIFFICULTY_Y );
				ON_COMMAND( m_sprDifficulty[*iter] );
			}

			for( unsigned s=0; s<m_vScoreRowItem.size(); s++ )
			{
				ScoreRowItem &item = m_vScoreRowItem[s];
				const Song* pSong = item.m_pSong;

				item.m_textTitle.SetText( pSong->GetDisplayFullTitle() );
				item.m_textTitle.SetDiffuse( SONGMAN->GetSongColor(pSong) );

				FOREACH_CONST( Difficulty, DIFFICULTIES_TO_SHOW.GetValue(), iter )
				{							
					const Steps* pSteps = pSong->GetStepsByDifficulty( pts.st, *iter, false );
					if( pSteps  &&  UNLOCKMAN->StepsIsLocked(pSong, pSteps) )
						pSteps = NULL;
					BitmapText* pTextStepsScore = &item.m_textScore[*iter];
					pTextStepsScore->SetHidden( pSteps == NULL );

					if( pSteps != NULL )
					{
						HighScoreList &hsl = PROFILEMAN->GetMachineProfile()->GetStepsHighScoreList(pSong,pSteps);
						HighScore hs = hsl.GetTopScore();
						bool bRecentHighScore = false;
						if( !hsl.vHighScores.empty() )
						{
							hs = hsl.GetTopScore();
							const CString *psName = hsl.GetTopScore().GetNameMutable();
							bRecentHighScore = find( GAMESTATE->m_vpsNamesThatWereFilled.begin(), GAMESTATE->m_vpsNamesThatWereFilled.end(), psName ) != GAMESTATE->m_vpsNamesThatWereFilled.end();
						}
						else
						{
							hs.SetName( NO_SCORE_NAME );				
						}

						CString s = hs.GetDisplayName() + "\n" + PercentageDisplay::FormatPercentScore( hs.GetPercentDP() );
						if( SHOW_SURVIVAL_TIME )
							s += "   " + SecondsToMSSMsMs(hs.GetSurvivalSeconds());
						pTextStepsScore->SetText( s );
					}
				}
			}
		}
		return m_ListScoreRowItems.GetSecondsForCompleteScrollThrough();
	case PAGE_TYPE_NONSTOP_COURSES:
	case PAGE_TYPE_ONI_COURSES:
	case PAGE_TYPE_SURVIVAL_COURSES:
		{
			m_textStepsType.SetText( GameManager::StepsTypeToThemedString(pts.st) );

			FOREACH_CONST( Difficulty, DIFFICULTIES_TO_SHOW.GetValue(), cd )
			{
				m_sprCourseDifficulty[*cd]->SetXY( COURSE_DIFFICULTY_X(*cd), COURSE_DIFFICULTY_Y );
				ON_COMMAND( m_sprCourseDifficulty[*cd] );
			}

			for( unsigned c=0; c<m_vScoreRowItem.size(); c++ )
			{
				ScoreRowItem &item = m_vScoreRowItem[c];
				const Course* pCourse = item.m_pCourse;

				item.m_textTitle.SetText( pCourse->GetDisplayFullTitle() );
				item.m_textTitle.SetDiffuse( SONGMAN->GetCourseColor(pCourse) );

				FOREACH_CONST( Difficulty, DIFFICULTIES_TO_SHOW.GetValue(), cd )
				{
					BitmapText* pTextStepsScore = &item.m_textScore[*cd];

					Trail *pTrail = pCourse->GetTrail( pts.st, *cd );
					pTextStepsScore->SetHidden( pTrail==NULL );
					if( pTrail == NULL )
						continue;

					const HighScoreList &hsl = PROFILEMAN->GetMachineProfile()->GetCourseHighScoreList( pCourse, pTrail );

					HighScore hs;
					bool bRecentHighScore = false;
					if( !hsl.vHighScores.empty() )
					{
						hs = hsl.vHighScores[0];
						const CString *psName = hsl.GetTopScore().GetNameMutable();
						bRecentHighScore = find( GAMESTATE->m_vpsNamesThatWereFilled.begin(), GAMESTATE->m_vpsNamesThatWereFilled.end(), psName ) != GAMESTATE->m_vpsNamesThatWereFilled.end();
					}
					else
					{
						hs.SetName( NO_SCORE_NAME );
					}

					CString s = hs.GetDisplayName() + "\n" + PercentageDisplay::FormatPercentScore( hs.GetPercentDP() );
					if( SHOW_SURVIVAL_TIME )
						s += "   " + SecondsToMSSMsMs(hs.GetSurvivalSeconds());
					pTextStepsScore->SetText( s );
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
	OFF_COMMAND( m_textCourseTitle );
	OFF_COMMAND( m_textCategory );
	OFF_COMMAND( m_textStepsType );
	OFF_COMMAND( m_sprPageType );

	for( int l=0; l<NUM_RANKING_LINES; l++ )
	{
		OFF_COMMAND( m_sprBullets[l] );
		OFF_COMMAND( m_textNames[l] );
		OFF_COMMAND( m_textScores[l] );
		OFF_COMMAND( m_textPoints[l] );
		OFF_COMMAND( m_textTime[l] );
	}
	FOREACH_CONST( Difficulty, DIFFICULTIES_TO_SHOW.GetValue(), iter )
	{
		if( m_sprDifficulty[*iter].IsLoaded() )
			OFF_COMMAND( m_sprDifficulty[*iter] );
	}
	FOREACH_CONST( Difficulty, DIFFICULTIES_TO_SHOW.GetValue(), cd )
	{
		if( m_sprCourseDifficulty[*cd].IsLoaded() )
			OFF_COMMAND( m_sprCourseDifficulty[*cd] );
	}

	if( !m_ListScoreRowItems.GetHidden() )
		OFF_COMMAND( m_ListScoreRowItems );
}

/*
 * (c) 2001-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
