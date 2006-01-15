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
};
XToString( PageType, NUM_PAGE_TYPES );
StringToX( PageType );

#define TYPE						THEME->GetMetric(m_sName,"Type")
#define COURSES_TO_SHOW				THEME->GetMetric(m_sName,"CoursesToShow")

#define DIFFICULTY_X(col)			(DIFFICULTY_START_X+COL_SPACING_X*col)
#define SCORE_OFFSET_X(col)			(SCORE_OFFSET_START_X+COL_SPACING_X*col)

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


static CString STEPS_TYPE_COLOR_NAME( size_t i ) { return ssprintf("StepsTypeColor%d",int(i+1)); }

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

	ROW_SPACING_X.Load                ( m_sName,"RowSpacingX" );
	ROW_SPACING_Y.Load                ( m_sName,"RowSpacingY" );

	STEPS_TYPE_COLOR.Load             ( m_sName,STEPS_TYPE_COLOR_NAME,5 );
	SHOW_ONLY_MOST_RECENT_SCORES.Load ( m_sName,"ShowOnlyMostRecentScores" );
	SECONDS_PER_PAGE.Load             ( m_sName,"SecondsPerPage" );
	PAGE_FADE_SECONDS.Load            ( m_sName,"PageFadeSeconds" );
	NO_SCORE_NAME.Load                ( m_sName,"NoScoreName" );
	MANUAL_SCROLLING.Load             ( m_sName, "ManualScroling" );

	ScreenAttract::Init();

	m_PageType = StringToPageType( TYPE );

	m_textStepsType.SetName( "StepsType" );
	m_textStepsType.LoadFromFont( THEME->GetPathF(m_sName,"steps type") );
	m_textStepsType.SetShadowLength( 0 );
	this->AddChild( &m_textStepsType );

	m_sprPageType.Load( THEME->GetPathG(m_sName, "PageType "+PageTypeToString(m_PageType)) );
	m_sprPageType->SetName( "PageType" );
	this->AddChild( m_sprPageType );
}

void ScreenRanking::BeginScreen()
{
	SET_XY( m_textStepsType );
	SET_XY( m_sprPageType );

	m_iNextPageToShow = 0;

	ScreenAttract::BeginScreen();

	this->HandleScreenMessage( SM_ShowNextPage );
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

void ScreenRanking::MenuStart( PlayerNumber pn )
{
	if( !IsTransitioning() )
		StartTransitioningScreen( SM_GoToNextScreen );
	SCREENMAN->PlayStartSound();
}

void ScreenRanking::MenuBack( PlayerNumber pn )
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
	m_textStepsType.SetText( GameManager::StepsTypeToLocalizedString(pts.st) );

	return 0;
}

ScoreScroller::ScoreScroller()
{
	this->DeleteChildrenWhenDone();
}

void ScoreScroller::SetStepsType( StepsType st, RageColor color )
{
	m_StepsType = st;

	for( unsigned s=0; s<m_SubActors.size(); s++ )
	{
		ScoreRowItem *pItem = (ScoreRowItem *) m_SubActors[s];

		FOREACH_CONST( Difficulty, CommonMetrics::DIFFICULTIES_TO_SHOW.GetValue(), cd )
			pItem->m_textScore[*cd].SetDiffuse( color );
	}

	ShiftSubActors( INT_MAX );
}

void ScoreScroller::Scroll( int iDir )
{
	float fDest = GetDestinationItem();
	float fOldDest = fDest;
	fDest += iDir;
	CLAMP( fDest, (m_metricSongScoreRowsToDraw-1)/2.0f, m_vScoreRowItemData.size()-(m_metricSongScoreRowsToDraw-1)/2.0f-1 );
	if( fOldDest != fDest )
	{
		// TODO: play sound
		SetDestinationItem( fDest );
		SetDestinationItem( fDest );
	}
}

void ScoreScroller::ScrollTop()
{
	SetCurrentAndDestinationItem( (m_metricSongScoreRowsToDraw-1)/2.0f );
}

void ScoreScroller::ConfigureActor( Actor *pActor, int iItem )
{
	ScoreRowItemData &data = m_vScoreRowItemData[iItem];
	if( data.m_pSong != NULL )
	{
		ScoreRowItem &item = *(ScoreRowItem *) pActor;
		const Song* pSong = data.m_pSong;

		item.m_textTitle.SetText( pSong->GetDisplayFullTitle() );
		item.m_textTitle.SetDiffuse( SONGMAN->GetSongColor(pSong) );

		FOREACH_CONST( Difficulty, m_DifficultiesToShow, iter )
		{							
			const Steps* pSteps = pSong->GetStepsByDifficulty( m_StepsType, *iter, false );
			if( pSteps  &&  UNLOCKMAN->StepsIsLocked(pSong, pSteps) )
				pSteps = NULL;
			BitmapText* pTextStepsScore = &item.m_textScore[*iter];
			pTextStepsScore->SetHidden( pSteps == NULL );
			if( pSteps == NULL )
				continue;

			const HighScoreList &hsl = PROFILEMAN->GetMachineProfile()->GetStepsHighScoreList(pSong,pSteps);
			SetScoreFromHighScoreList( pTextStepsScore, hsl );
		}
	}
	else if( data.m_pCourse != NULL )
	{
		const Course* pCourse = data.m_pCourse;

		ScoreRowItem &item = *(ScoreRowItem *) pActor;
		item.m_textTitle.SetText( pCourse->GetDisplayFullTitle() );
		item.m_textTitle.SetDiffuse( SONGMAN->GetCourseColor(pCourse) );

		FOREACH_CONST( Difficulty, m_DifficultiesToShow, cd )
		{
			BitmapText* pTextStepsScore = &item.m_textScore[*cd];

			const Trail *pTrail = pCourse->GetTrail( m_StepsType, *cd );
			if( UNLOCKMAN->CourseIsLocked(pCourse) )
				pTrail = NULL;
			pTextStepsScore->SetHidden( pTrail==NULL );
			if( pTrail == NULL )
				continue;

			const HighScoreList &hsl = PROFILEMAN->GetMachineProfile()->GetCourseHighScoreList( pCourse, pTrail );
			SetScoreFromHighScoreList( pTextStepsScore, hsl );
		}
	}
}

ScoreScroller::ScoreRowItem::ScoreRowItem()
{
	this->AddChild( &m_textTitle );
}

ScoreScroller::ScoreRowItem::ScoreRowItem( const ScoreRowItem &cpy ):
	ActorFrame( cpy ),
	m_sprFrame( cpy.m_sprFrame ),
	m_textTitle( cpy.m_textTitle ),
	m_textScore( cpy.m_textScore )
{
	if( m_sprFrame.IsLoaded() )
		this->AddChild( m_sprFrame );
	this->AddChild( &m_textTitle );
	FOREACH( BitmapText, m_textScore, bt )
		this->AddChild( &*bt );
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

void ScoreScroller::Load(
	CString sClassName,
	const vector<Difficulty> &DifficultiesToShow,
	float fItemHeight )
{
	SCORE_OFFSET_START_X.Load       (sClassName, "ScoreOffsetStartX");
	SCORE_OFFSET_Y.Load             (sClassName, "ScoreOffsetY");
	SHOW_SURVIVAL_TIME.Load         (sClassName, "ShowSurvivalTime");
	NO_SCORE_NAME.Load              (sClassName, "NoScoreName");
	COL_SPACING_X.Load              (sClassName, "ColSpacingX");
	SONG_SCORE_SECONDS_PER_ROW.Load (sClassName, "SongScoreSecondsPerRow");
	m_metricSongScoreRowsToDraw.Load(sClassName, "SongScoreRowsToDraw");

	m_DifficultiesToShow = DifficultiesToShow;

	ScoreScroller::ScoreRowItem ItemTemplate;
	{
		ItemTemplate.m_sprFrame.Load( THEME->GetPathG(sClassName,"list frame") );
		ItemTemplate.m_sprFrame->SetName( "Frame" );
		ActorUtil::LoadAllCommands( *ItemTemplate.m_sprFrame, sClassName );

		ItemTemplate.m_textTitle.SetName( "Title" );
		ItemTemplate.m_textTitle.LoadFromFont( THEME->GetPathF(sClassName,"list title") );
		ActorUtil::LoadAllCommands( ItemTemplate.m_textTitle, sClassName );

		ItemTemplate.m_textScore.resize( NUM_Difficulty );
		FOREACH_CONST( Difficulty, m_DifficultiesToShow, d )
		{
			ItemTemplate.m_textScore[*d].SetName( "Score" );
			ItemTemplate.m_textScore[*d].LoadFromFont( THEME->GetPathF(sClassName,"list score") );
			ItemTemplate.m_textScore[*d].SetXY( SCORE_OFFSET_X(*d), SCORE_OFFSET_Y );
			ActorUtil::LoadAllCommands( ItemTemplate.m_textScore[*d], sClassName );
		}
	}

	int iNumCopies = m_metricSongScoreRowsToDraw+1;
	for( int i=0; i<iNumCopies; ++i )
		this->AddChild( new ScoreRowItem(ItemTemplate) );

	DynamicActorScroller::Load2( (float) m_metricSongScoreRowsToDraw, false );
	DynamicActorScroller::SetTransformFromHeight( fItemHeight );
	DynamicActorScroller::SetSecondsPerItem( SONG_SCORE_SECONDS_PER_ROW );
	DynamicActorScroller::EnableMask( SCREEN_WIDTH, fItemHeight );
	DynamicActorScroller::ScrollThroughAllItems();

	m_iNumItems = m_vScoreRowItemData.size();
}

// PAGE_TYPE_ALL_STEPS:
// PAGE_TYPE_NONSTOP_COURSES:
// PAGE_TYPE_ONI_COURSES:
// PAGE_TYPE_SURVIVAL_COURSES:
void ScreenRankingScroller::Init()
{
	DIFFICULTIES_TO_SHOW.Load( m_sName, "DifficultiesToShow" );

	COL_SPACING_X.Load( m_sName, "ColSpacingX" );
	COL_SPACING_Y.Load( m_sName, "ColSpacingY" );
	
	DIFFICULTY_START_X.Load( m_sName, "DifficultyStartX" );
	DIFFICULTY_Y.Load( m_sName, "DifficultyY" );
	NUM_MOST_RECENT_SCORES_TO_SHOW.Load( m_sName, "NumMostRecentScoresToShow" );

	ScreenRanking::Init();

	FOREACH_CONST( Difficulty, DIFFICULTIES_TO_SHOW.GetValue(), d )
	{
		if( m_PageType == PAGE_TYPE_ALL_STEPS )
			m_sprDifficulty[*d].Load( THEME->GetPathG(m_sName,"difficulty "+DifficultyToString(*d)) );
		else
			m_sprDifficulty[*d].Load( THEME->GetPathG(m_sName,"CourseDifficulty "+CourseDifficultyToString(*d)) );
		m_sprDifficulty[*d]->SetName( "Difficulty"+DifficultyToString(*d) );
		m_sprDifficulty[*d]->SetXY( DIFFICULTY_X(*d), DIFFICULTY_Y );
		this->AddChild( m_sprDifficulty[*d] );
	}

	m_ListScoreRowItems.SetName( "ListScoreRowItems" );
	if( m_PageType == PAGE_TYPE_ALL_STEPS )
		m_ListScoreRowItems.LoadSongs( SHOW_ONLY_MOST_RECENT_SCORES, NUM_MOST_RECENT_SCORES_TO_SHOW );
	else if( m_PageType == PAGE_TYPE_NONSTOP_COURSES ||
		m_PageType == PAGE_TYPE_ONI_COURSES ||
		m_PageType == PAGE_TYPE_SURVIVAL_COURSES )
	{
		CourseType ct = m_PageType == PAGE_TYPE_NONSTOP_COURSES? COURSE_TYPE_NONSTOP :
						m_PageType == PAGE_TYPE_ONI_COURSES? COURSE_TYPE_ONI :
							COURSE_TYPE_SURVIVAL;
		m_ListScoreRowItems.LoadCourses( ct, SHOW_ONLY_MOST_RECENT_SCORES, NUM_MOST_RECENT_SCORES_TO_SHOW );
	}

	m_ListScoreRowItems.Load( m_sName, DIFFICULTIES_TO_SHOW.GetValue(), ROW_SPACING_Y );
	this->AddChild( &m_ListScoreRowItems );

	for( unsigned i=0; i<STEPS_TYPES_TO_SHOW.GetValue().size(); i++ )
	{
		PageToShow pts;
		pts.colorIndex = i;
		pts.st = STEPS_TYPES_TO_SHOW.GetValue()[i];
		m_vPagesToShow.push_back( pts );
	}
}

void ScreenRankingScroller::BeginScreen()
{
	ScreenRanking::BeginScreen();

	SET_XY( m_ListScoreRowItems );
}

void ScoreScroller::SetScoreFromHighScoreList( BitmapText *pTextStepsScore, const HighScoreList &hsl )
{
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

float ScreenRankingScroller::SetPage( const PageToShow &pts )
{
	ScreenRanking::SetPage( pts );

	m_ListScoreRowItems.SetStepsType( pts.st, STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );

	if( (bool)MANUAL_SCROLLING )
		m_ListScoreRowItems.ScrollTop();
	else
		m_ListScoreRowItems.ScrollThroughAllItems();

	return m_ListScoreRowItems.GetSecondsForCompleteScrollThrough();
}

// PAGE_TYPE_CATEGORY:
// PAGE_TYPE_TRAIL:
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

void ScreenRankingLines::Init()
{
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

	ScreenRanking::Init();

	if( m_PageType == PAGE_TYPE_CATEGORY )
	{
		m_textCategory.SetName( "Category" );
		m_textCategory.LoadFromFont( THEME->GetPathF(m_sName,"category") );
		m_textCategory.SetShadowLength( 0 );
		this->AddChild( &m_textCategory );

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

	if( m_PageType == PAGE_TYPE_TRAIL )
	{
		m_Banner.SetName( "Banner" );
		this->AddChild( &m_Banner );

		m_textCourseTitle.SetName( "CourseTitle" );
		m_textCourseTitle.LoadFromFont( THEME->GetPathF(m_sName,"course title") );
		m_textCourseTitle.SetShadowLength( 0 );
		this->AddChild( &m_textCourseTitle );

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
		m_textPoints[l].SetHidden( true );
		m_textPoints[l].SetXY( POINTS_X(l), POINTS_Y(l) );
		ActorUtil::LoadAllCommands( m_textPoints[l], m_sName );
		this->AddChild( &m_textPoints[l] );
		
		m_textTime[l].SetName( ssprintf("Time%d",l+1) );
		m_textTime[l].LoadFromFont( THEME->GetPathF(m_sName,"time") );
		m_textTime[l].SetHidden( true );
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
		m_textNames[l].SetDiffuse( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );

		m_textScores[l].SetHidden( !bShowScores );
		m_textScores[l].SetDiffuse( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );
		
		m_textPoints[l].SetHidden( !bShowPoints );
		m_textPoints[l].SetDiffuse( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );
		
		m_textTime[l].SetHidden( !bShowTime );
		m_textTime[l].SetDiffuse( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );
	}

	switch( m_PageType )
	{
	case PAGE_TYPE_CATEGORY:
		{
			m_textCategory.SetText( ssprintf("Type %c", 'A'+pts.category) );

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

			m_Banner.LoadFromCourse( pts.pCourse );

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
	default:
		ASSERT(0);
		return 0;
	}
}

void ScreenRankingLines::BeginScreen()
{
	if( m_PageType == PAGE_TYPE_CATEGORY )
		SET_XY( m_textCategory );

	if( m_PageType == PAGE_TYPE_TRAIL )
	{
		SET_XY( m_Banner );
		SET_XY( m_textCourseTitle );
	}

	ScreenRanking::BeginScreen();
}

/*
 * (c) 2001-2005 Chris Danford, Glenn Maynard
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
