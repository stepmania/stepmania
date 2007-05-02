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
#include "ActorUtil.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "RageLog.h"
#include "UnlockManager.h"
#include "ScreenDimensions.h"
#include "PercentageDisplay.h"

static const char *PageTypeNames[] = {
	"Category",
	"Course",
	"AllSteps",
	"NonstopCourses",
	"OniCourses",
	"SurvivalCourses",
	"AllCourses",
};
StringToX( PageType );

#define TYPE						THEME->GetMetric(m_sName,"Type")
#define COURSES_TO_SHOW				THEME->GetMetric(m_sName,"CoursesToShow")

AutoScreenMessage( SM_ShowNextPage )
AutoScreenMessage( SM_HidePage )


static void GetAllSongsToShow( vector<Song*> &vpOut, bool bShowOnlyMostRecentScores, int iNumMostRecentScoresToShow )
{
	vpOut.clear();
	FOREACH_CONST( Song*, SONGMAN->GetSongs(), s )
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
	if( ct == CourseType_Invalid )
		SONGMAN->GetAllCourses( vpCourses, false );
	else
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

static RString STEPS_TYPE_COLOR_NAME( size_t i ) { return ssprintf("StepsTypeColor%d",int(i+1)); }

REGISTER_SCREEN_CLASS( ScreenRanking );
REGISTER_SCREEN_CLASS( ScreenRankingScroller );
REGISTER_SCREEN_CLASS( ScreenRankingLines );

ScreenRanking::ScreenRanking():
	ScreenAttract( false /*dont reset GAMESTATE*/ )
{
}

void ScreenRanking::Init()
{
	// watch out: ThemeMetricStepsTypesToShow inverts the results
	STEPS_TYPES_TO_SHOW.Load          ( m_sName,"StepsTypesToHide" );

	SECONDS_PER_PAGE.Load             ( m_sName,"SecondsPerPage" );
	PAGE_FADE_SECONDS.Load            ( m_sName,"PageFadeSeconds" );
	MANUAL_SCROLLING.Load             ( m_sName, "ManualScrolling" );

	ScreenAttract::Init();

	m_PageType = StringToPageType( TYPE );

	m_textStepsType.SetName( "StepsType" );
	m_textStepsType.LoadFromFont( THEME->GetPathF(m_sName,"steps type") );
	m_textStepsType.SetShadowLength( 0 );
	this->AddChild( &m_textStepsType );
	LOAD_ALL_COMMANDS( m_textStepsType );
}

void ScreenRanking::BeginScreen()
{
	m_iNextPageToShow = 0;

	ScreenAttract::BeginScreen();

	this->HandleScreenMessage( SM_ShowNextPage );
}

void ScreenRanking::Input( const InputEventPlus &input )
{
	LOG->Trace( "ScreenRanking::Input()" );

	if( IsTransitioning() )
		return;

	// If manually scrolling, then pass the input to Scree::Input so it will call Menu*
	if( (bool)MANUAL_SCROLLING )
		Screen::Input( input );
	else
		ScreenAttract::Input( input );
}

void ScreenRanking::MenuStart( const InputEventPlus &input )
{
	if( !IsTransitioning() )
		StartTransitioningScreen( SM_GoToNextScreen );
	SCREENMAN->PlayStartSound();
}

void ScreenRanking::MenuBack( const InputEventPlus &input )
{
	if( !IsTransitioning() )
		StartTransitioningScreen( SM_GoToNextScreen );
	SCREENMAN->PlayStartSound();
}

void ScreenRanking::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_ShowNextPage )
	{
		if( m_iNextPageToShow < m_vPagesToShow.size() )
		{
			float fSecsToShow = SetPage( m_vPagesToShow[m_iNextPageToShow] );
			++m_iNextPageToShow;
			this->SortByDrawOrder();
			
			// If manually scrolling, don't automatically change pages.
			if( !(bool)MANUAL_SCROLLING )
				this->PostScreenMessage( SM_HidePage, fSecsToShow-PAGE_FADE_SECONDS );
		}
		else
		{
			StartTransitioningScreen(SM_GoToNextScreen);
		}
	}
	else if( SM == SM_HidePage )
	{
		this->PlayCommand( "SwitchPage" );
		this->PostScreenMessage( SM_ShowNextPage, PAGE_FADE_SECONDS );
	}

	ScreenAttract::HandleScreenMessage( SM );
}

float ScreenRanking::SetPage( const PageToShow &pts )
{
	// This is going to take a while to load.  Possibly longer than one frame.
	// So, zero the next update so we don't skip.
	SCREENMAN->ZeroNextUpdate();

	//
	// init page
	//
	m_textStepsType.SetText( GameManager::StepsTypeToLocalizedString(pts.aTypes.front().second) );

	return 0;
}

void ScreenRankingScroller::DoScroll( int iDir )
{
	if( !m_ListScoreRowItems.Scroll(iDir) )
		iDir = 0;
	Message msg("Scrolled");
	msg.SetParam( "Dir", iDir );
	this->HandleMessage( msg );
}

/////////////////////////////////////////////

ScoreScroller::ScoreScroller()
{
	this->DeleteChildrenWhenDone();
}

void ScoreScroller::SetDisplay( const vector<DifficultyAndStepsType> &DifficultiesToShow )
{
	m_DifficultiesToShow = DifficultiesToShow;
	ShiftSubActors( INT_MAX );
}

bool ScoreScroller::Scroll( int iDir )
{
	if( (int)m_vScoreRowItemData.size() <= SONG_SCORE_ROWS_TO_DRAW )
		return false;

	float fDest = GetDestinationItem();
	float fOldDest = fDest;
	fDest += iDir;
	float fLowClamp = (SONG_SCORE_ROWS_TO_DRAW-1)/2.0f;
	float fHighClamp = m_vScoreRowItemData.size()-(SONG_SCORE_ROWS_TO_DRAW-1)/2.0f-1;
	CLAMP( fDest, fLowClamp, fHighClamp );
	if( fOldDest != fDest )
	{
		SetDestinationItem( fDest );
		return true;
	}
	else
	{
		return false;
	}
}

void ScoreScroller::ScrollTop()
{
	SetCurrentAndDestinationItem( (SONG_SCORE_ROWS_TO_DRAW-1)/2.0f );
}

void ScoreScroller::ConfigureActor( Actor *pActor, int iItem )
{
	Actor &item = *dynamic_cast<Actor *>(pActor);
	const ScoreRowItemData &data = m_vScoreRowItemData[iItem];

	Message msg("Set");
	if( data.m_pSong != NULL )
		msg.SetParam( "Song", data.m_pSong );
	if( data.m_pCourse != NULL )
		msg.SetParam( "Course", data.m_pCourse );

	Lua *L = LUA->Get();
	lua_newtable( L );
	lua_pushvalue( L, -1 );
	msg.SetParamFromStack( L, "Entries" );

	FOREACH( DifficultyAndStepsType, m_DifficultiesToShow, iter )
	{				
		Difficulty dc = iter->first;
		StepsType st = iter->second;

		if( data.m_pSong != NULL )
		{
			const Song* pSong = data.m_pSong;

			Steps *pSteps = SongUtil::GetStepsByDifficulty( pSong, st, dc, false );
			if( pSteps  &&  UNLOCKMAN->StepsIsLocked(pSong, pSteps) )
				continue;

			LuaHelpers::Push( L, pSteps );
			lua_rawseti( L, -2, lua_objlen(L, -2)+1 );
		}
		else if( data.m_pCourse != NULL )
		{
			const Course* pCourse = data.m_pCourse;

			Trail *pTrail = pCourse->GetTrail( st, dc );
			if( UNLOCKMAN->CourseIsLocked(pCourse) )
				continue;

			LuaHelpers::Push( L, pTrail );
			lua_rawseti( L, -2, lua_objlen(L, -2)+1 );
		}
	}
	lua_pop( L, 1 );
	LUA->Release( L );

	item.HandleMessage( msg );
}

void ScoreScroller::LoadSongs( bool bOnlyRecentScores, int iNumRecentScores )
{
	vector<Song*> vpSongs;
	GetAllSongsToShow( vpSongs, bOnlyRecentScores, iNumRecentScores );
	m_vScoreRowItemData.resize( vpSongs.size() );
	for( unsigned i=0; i<m_vScoreRowItemData.size(); ++i )
		m_vScoreRowItemData[i].m_pSong = vpSongs[i];
}

void ScoreScroller::LoadCourses( CourseType ct, bool bOnlyRecentScores, int iNumRecentScores )
{
	vector<Course*> vpCourses;
	GetAllCoursesToShow( vpCourses, ct, bOnlyRecentScores, iNumRecentScores );
	m_vScoreRowItemData.resize( vpCourses.size() );
	for( unsigned i=0; i<m_vScoreRowItemData.size(); ++i )
		m_vScoreRowItemData[i].m_pCourse = vpCourses[i];
}

void ScoreScroller::Load( RString sClassName )
{
	SONG_SCORE_ROWS_TO_DRAW.Load(sClassName, "SongScoreRowsToDraw");

	int iNumCopies = SONG_SCORE_ROWS_TO_DRAW+1;
	for( int i=0; i<iNumCopies; ++i )
	{
		Actor *pActor = ActorUtil::MakeActor( THEME->GetPathG(sClassName,"list item") );
		this->AddChild( pActor );
	}

	DynamicActorScroller::SetNumItemsToDraw( (float) SONG_SCORE_ROWS_TO_DRAW );
	DynamicActorScroller::Load2();

	m_iNumItems = m_vScoreRowItemData.size();
}

void ScreenRankingScroller::Init()
{
	ThemeMetric<bool>		SHOW_ONLY_MOST_RECENT_SCORES;
	ThemeMetric<RString>		DIFFICULTIES_TO_SHOW;
	ThemeMetric<int>		NUM_MOST_RECENT_SCORES_TO_SHOW;
	SHOW_ONLY_MOST_RECENT_SCORES.Load ( m_sName,"ShowOnlyMostRecentScores" );
	DIFFICULTIES_TO_SHOW.Load( m_sName, "DifficultiesToShow" );
	NUM_MOST_RECENT_SCORES_TO_SHOW.Load( m_sName, "NumMostRecentScoresToShow" );

	ScreenRanking::Init();

	m_ListScoreRowItems.SetName( "ListScoreRowItems" );
	LOAD_ALL_COMMANDS( m_ListScoreRowItems );
	switch( m_PageType )
	{
	default:	ASSERT(0);
	case PageType_AllSteps:
		m_ListScoreRowItems.LoadSongs( SHOW_ONLY_MOST_RECENT_SCORES, NUM_MOST_RECENT_SCORES_TO_SHOW );
		break;
	case PageType_NonstopCourses:
	case PageType_OniCourses:
	case PageType_SurvivalCourses:
	case PageType_AllCourses:
		{
			CourseType ct;
			switch( m_PageType )
			{
			default:	ASSERT(0);
			case PageType_NonstopCourses:	ct = COURSE_TYPE_NONSTOP;	break;
			case PageType_OniCourses:	ct = COURSE_TYPE_ONI;		break;
			case PageType_SurvivalCourses:	ct = COURSE_TYPE_SURVIVAL;	break;
			case PageType_AllCourses:	ct = CourseType_Invalid;	break;
			}

			m_ListScoreRowItems.LoadCourses( ct, SHOW_ONLY_MOST_RECENT_SCORES, NUM_MOST_RECENT_SCORES_TO_SHOW );
		}
		break;
	}

	m_ListScoreRowItems.Load( m_sName );
	this->AddChild( &m_ListScoreRowItems );

	for( unsigned i=0; i<STEPS_TYPES_TO_SHOW.GetValue().size(); i++ )
	{
		vector<RString> asDifficulties;
		split( DIFFICULTIES_TO_SHOW, ",", asDifficulties );
		
		PageToShow pts;
		for( unsigned d=0; d<asDifficulties.size(); ++d )
		{
			vector<RString> asBits;
			split( asDifficulties[d], ":", asBits, false );
			ASSERT( !asBits.empty() );
			Difficulty dc = StringToDifficulty(asBits[0]);

			StepsType st = STEPS_TYPES_TO_SHOW.GetValue()[i];
			if( asBits.size() > 1 )
				st = GAMEMAN->StringToStepsType( asBits[1] );

			pts.aTypes.push_back( make_pair(dc, st) );
		}
		m_vPagesToShow.push_back( pts );
	}
}

float ScreenRankingScroller::SetPage( const PageToShow &pts )
{
	ScreenRanking::SetPage( pts );

	m_ListScoreRowItems.SetDisplay( pts.aTypes );

	if( (bool)MANUAL_SCROLLING )
		m_ListScoreRowItems.ScrollTop();
	else
		m_ListScoreRowItems.ScrollThroughAllItems();

	return m_ListScoreRowItems.GetSecondsForCompleteScrollThrough();
}

// PageType_Category:
// PageType_Trail:
#define BULLET_X(row)				(BULLET_START_X+ROW_SPACING_X*row)
#define BULLET_Y(row)				(BULLET_START_Y+ROW_SPACING_Y*row)
#define NAME_X(row)				(NAME_START_X+ROW_SPACING_X*row)
#define NAME_Y(row)				(NAME_START_Y+ROW_SPACING_Y*row)
#define SCORE_X(row)				(SCORE_START_X+ROW_SPACING_X*row)
#define SCORE_Y(row)				(SCORE_START_Y+ROW_SPACING_Y*row)
#define POINTS_X(row)				(POINTS_START_X+ROW_SPACING_X*row)
#define POINTS_Y(row)				(POINTS_START_Y+ROW_SPACING_Y*row)
#define TIME_X(row)				(TIME_START_X+ROW_SPACING_X*row)
#define TIME_Y(row)				(TIME_START_Y+ROW_SPACING_Y*row)

void ScreenRankingLines::Init()
{
	NO_SCORE_NAME.Load( m_sName,"NoScoreName" );
	ROW_SPACING_X.Load( m_sName,"RowSpacingX" );
	ROW_SPACING_Y.Load( m_sName,"RowSpacingY" );
	BULLET_START_X.Load( m_sName, "BulletStartX" );
	BULLET_START_Y.Load( m_sName, "BulletStartY" );
	NAME_START_X.Load( m_sName, "NameStartX" );
	NAME_START_Y.Load( m_sName, "NameStartY" );
	SCORE_START_X.Load( m_sName, "ScoreStartX" );
	SCORE_START_Y.Load( m_sName, "ScoreStartY" );
	POINTS_START_X.Load( m_sName, "PointsStartX" );
	POINTS_START_Y.Load( m_sName, "PointsStartY" );
	TIME_START_X.Load( m_sName, "TimeStartX" );
	TIME_START_Y.Load( m_sName, "TimeStartY" );
	STEPS_TYPE_COLOR.Load( m_sName,STEPS_TYPE_COLOR_NAME,5 );

	ScreenRanking::Init();

	if( m_PageType == PageType_Category )
	{
		m_textCategory.SetName( "Category" );
		m_textCategory.LoadFromFont( THEME->GetPathF(m_sName,"category") );
		m_textCategory.SetShadowLength( 0 );
		this->AddChild( &m_textCategory );
		LOAD_ALL_COMMANDS( m_textCategory );

		for( unsigned i=0; i<STEPS_TYPES_TO_SHOW.GetValue().size(); i++ )
		{
			for( int c=0; c<NUM_RankingCategory; c++ )
			{
				PageToShow pts;
				pts.colorIndex = i;
				pts.category = (RankingCategory)c;
				StepsType st = STEPS_TYPES_TO_SHOW.GetValue()[i];
				pts.aTypes.push_back( make_pair(Difficulty_Invalid, st) );
				m_vPagesToShow.push_back( pts );
			}
		}
	}

	if( m_PageType == PageType_Trail )
	{
		m_Banner.SetName( "Banner" );
		this->AddChild( &m_Banner );
		LOAD_ALL_COMMANDS( m_Banner );

		m_textCourseTitle.SetName( "CourseTitle" );
		m_textCourseTitle.LoadFromFont( THEME->GetPathF(m_sName,"course title") );
		m_textCourseTitle.SetShadowLength( 0 );
		this->AddChild( &m_textCourseTitle );
		LOAD_ALL_COMMANDS( m_textCourseTitle );

		vector<RString> asCoursePaths;
		split( COURSES_TO_SHOW, ",", asCoursePaths, true );
		for( unsigned i=0; i<STEPS_TYPES_TO_SHOW.GetValue().size(); i++ )
		{
			for( unsigned c=0; c<asCoursePaths.size(); c++ )
			{
				PageToShow pts;
				pts.colorIndex = i;
				StepsType st = STEPS_TYPES_TO_SHOW.GetValue()[i];
				pts.aTypes.push_back( make_pair(Difficulty_Invalid, st) );
				pts.pCourse = SONGMAN->GetCourseFromPath( asCoursePaths[c] );
				if( pts.pCourse == NULL )
					continue;

				pts.pTrail = pts.pCourse->GetTrail( st );
				if( pts.pTrail == NULL )
					continue;

				m_vPagesToShow.push_back( pts );
			}
		}
	}

	for( int l=0; l<NUM_RANKING_LINES; l++ )
	{
		m_sprBullets[l].Load( THEME->GetPathG( m_sName, ssprintf("bullets 1x%d",NUM_RANKING_LINES) ) );
		m_sprBullets[l]->SetName( ssprintf("Bullet%d",l+1) );
		m_sprBullets[l]->StopAnimating();
		m_sprBullets[l]->SetState( l );
		m_sprBullets[l]->SetXY( BULLET_X(l), BULLET_Y(l) );
		ActorUtil::LoadAllCommands( *m_sprBullets[l], m_sName );
		this->AddChild( m_sprBullets[l] );

		m_textNames[l].SetName( ssprintf("Name%d",l+1) );
		m_textNames[l].LoadFromFont( THEME->GetPathF(m_sName,"name") );
		m_textNames[l].SetXY( NAME_X(l), NAME_Y(l) );
		ActorUtil::LoadAllCommands( m_textNames[l], m_sName );
		this->AddChild( &m_textNames[l] );

		m_textScores[l].SetName( ssprintf("Score%d",l+1) );
		m_textScores[l].LoadFromFont( THEME->GetPathF(m_sName,"score") );
		m_textScores[l].SetXY( SCORE_X(l), SCORE_Y(l) );
		ActorUtil::LoadAllCommands( m_textScores[l], m_sName );
		this->AddChild( &m_textScores[l] );

		m_textPoints[l].SetName( ssprintf("Points%d",l+1) );
		m_textPoints[l].LoadFromFont( THEME->GetPathF(m_sName,"points") );
		m_textPoints[l].SetVisible( false );
		m_textPoints[l].SetXY( POINTS_X(l), POINTS_Y(l) );
		ActorUtil::LoadAllCommands( m_textPoints[l], m_sName );
		this->AddChild( &m_textPoints[l] );
		
		m_textTime[l].SetName( ssprintf("Time%d",l+1) );
		m_textTime[l].LoadFromFont( THEME->GetPathF(m_sName,"time") );
		m_textTime[l].SetVisible( false );
		m_textTime[l].SetXY( TIME_X(l), TIME_Y(l) );
		ActorUtil::LoadAllCommands( m_textTime[l], m_sName );
		this->AddChild( &m_textTime[l] );
	}
}

float ScreenRankingLines::SetPage( const PageToShow &pts )
{
	ScreenRanking::SetPage( pts );

	bool bShowScores = false;
	bool bShowPoints = false;
	bool bShowTime = false;
	switch( m_PageType )
	{
	case PageType_Category:
		bShowScores = true;
		break;
	case PageType_Trail:
		bShowScores = !pts.pCourse->IsOni();
		bShowPoints = pts.pCourse->IsOni();
		bShowTime = pts.pCourse->IsOni();
		break;
	}

	for( int l=0; l<NUM_RANKING_LINES; l++ )
	{
		m_textNames[l].SetDiffuse( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );

		m_textScores[l].SetVisible( bShowScores );
		m_textScores[l].SetDiffuse( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );
		
		m_textPoints[l].SetVisible( bShowPoints );
		m_textPoints[l].SetDiffuse( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );
		
		m_textTime[l].SetVisible( bShowTime );
		m_textTime[l].SetDiffuse( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );
	}

	switch( m_PageType )
	{
	case PageType_Category:
		{
			m_textCategory.SetText( ssprintf("Type %c", 'A'+pts.category) );

			for( int l=0; l<NUM_RANKING_LINES; l++ )
			{
				StepsType st = pts.aTypes.front().second;
				HighScoreList& hsl = PROFILEMAN->GetMachineProfile()->GetCategoryHighScoreList(st, pts.category);
				HighScore hs;
				bool bRecentHighScore = false;
				if( l < (int)hsl.vHighScores.size() )
				{
					hs = hsl.vHighScores[l];
					RString *psName = hsl.vHighScores[l].GetNameMutable();
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
	case PageType_Trail:
		{
			m_textCourseTitle.SetText( pts.pCourse->GetDisplayFullTitle() );

			m_Banner.LoadFromCourse( pts.pCourse );

			const HighScoreList &hsl = PROFILEMAN->GetMachineProfile()->GetCourseHighScoreList( pts.pCourse, pts.pTrail );
			for( int l=0; l<NUM_RANKING_LINES; l++ )
			{
				HighScore hs;
				bool bRecentHighScore = false;
				if( l < (int)hsl.vHighScores.size() )
				{
					hs = hsl.vHighScores[l];
					const RString *psName = hsl.vHighScores[l].GetNameMutable();
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
	default:
		ASSERT(0);
		return 0;
	}
}

void ScreenRankingLines::BeginScreen()
{
	ScreenRanking::BeginScreen();
}

/*
 * (c) 2001-2007 Chris Danford, Glenn Maynard
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
