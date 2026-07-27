#include "global.h"
#include "ScreenHighScores.h"
#include "ScreenManager.h"
#include "SongManager.h"
#include "Song.h"
#include "RageLog.h"
#include "UnlockManager.h"

#include <cstddef>
#include <vector>


RString COLUMN_DIFFICULTY_NAME( std::size_t i );
RString COLUMN_STEPS_TYPE_NAME( std::size_t i );

static const char *HighScoresTypeNames[] = {
	"AllSteps",
	"NonstopCourses",
	"OniCourses",
	"SurvivalCourses",
	"AllCourses",
};
XToString( HighScoresType );
LuaXType( HighScoresType );

REGISTER_SCREEN_CLASS( ScreenHighScores );

static void GetAllSongsToShow( std::vector<Song*> &vpOut, int iNumMostRecentScoresToShow )
{
	vpOut.clear();
	for (Song *s : SONGMAN->GetAllSongs())
	{
		if( !s->NormallyDisplayed() )
			continue;	// skip
		if( !s->ShowInDemonstrationAndRanking() )
			continue;	// skip
		vpOut.push_back( s );
	}

	if( (int)vpOut.size() > iNumMostRecentScoresToShow )
	{
		SongUtil::SortSongPointerArrayByTitle( vpOut );
		SongUtil::SortByMostRecentlyPlayedForMachine( vpOut );
		if( (int) vpOut.size() > iNumMostRecentScoresToShow )
			vpOut.erase( vpOut.begin()+iNumMostRecentScoresToShow, vpOut.end() );
	}
}

static void GetAllCoursesToShow( std::vector<Course*> &vpOut, CourseType ct, int iNumMostRecentScoresToShow )
{
	vpOut.clear();
	std::vector<Course*> vpCourses;
	if( ct == CourseType_Invalid )
		SONGMAN->GetAllCourses( vpCourses, false );
	else
		SONGMAN->GetCourses( ct, vpCourses, false );

	for (Course *c : vpCourses)
	{
		if( UNLOCKMAN->CourseIsLocked(c) )
			continue;	// skip
		if( !c->ShowInDemonstrationAndRanking() )
			continue;	// skip
		vpOut.push_back( c );
	}
	if( (int)vpOut.size() > iNumMostRecentScoresToShow )
	{
		CourseUtil::SortCoursePointerArrayByTitle( vpOut );
		CourseUtil::SortByMostRecentlyPlayedForMachine( vpOut );
		if( (int) vpOut.size() > iNumMostRecentScoresToShow )
			vpOut.erase( vpOut.begin()+iNumMostRecentScoresToShow, vpOut.end() );
	}
}

/////////////////////////////////////////////

ScoreScroller::ScoreScroller()
{
	this->DeleteChildrenWhenDone();
}

void ScoreScroller::SetDisplay( const std::vector<DifficultyAndStepsType> &DifficultiesToShow )
{
	m_DifficultiesToShow = DifficultiesToShow;
	ShiftSubActors( INT_MAX );
}

bool ScoreScroller::Scroll( int iDir )
{
	if( (int)m_vScoreRowItemData.size() <= SCROLLER_ITEMS_TO_DRAW )
		return false;

	float fDest = GetDestinationItem();
	float fOldDest = fDest;
	fDest += iDir;
	float fLowClamp = (SCROLLER_ITEMS_TO_DRAW-1)/2.0f;
	float fHighClamp = m_vScoreRowItemData.size()-(SCROLLER_ITEMS_TO_DRAW-1)/2.0f-1;
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
	SetCurrentAndDestinationItem( (SCROLLER_ITEMS_TO_DRAW-1)/2.0f );
}

void ScoreScroller::ConfigureActor( Actor *pActor, int iItem )
{
	LOG->Trace("ScoreScroller::ConfigureActor");

	Actor &item = *dynamic_cast<Actor *>(pActor);
	const ScoreRowItemData &data = m_vScoreRowItemData[iItem];

	Message msg("Set");
	if( data.m_pSong != nullptr )
		msg.SetParam( "Song", data.m_pSong );
	if( data.m_pCourse != nullptr )
		msg.SetParam( "Course", data.m_pCourse );


	Lua *L = LUA->Get();
	lua_newtable( L );
	lua_pushvalue( L, -1 );
	msg.SetParamFromStack( L, "Entries" );

	int i = 0;
	for (DifficultyAndStepsType &iter : m_DifficultiesToShow)
	{
		Difficulty dc = iter.first;
		StepsType st = iter.second;

		if( data.m_pSong != nullptr )
		{
			const Song* pSong = data.m_pSong;
			Steps *pSteps = SongUtil::GetStepsByDifficulty( pSong, st, dc, false );
			if( pSteps  &&  UNLOCKMAN->StepsIsLocked(pSong, pSteps) )
				pSteps = nullptr;
			LuaHelpers::Push( L, pSteps );
		}
		else if( data.m_pCourse != nullptr )
		{
			const Course* pCourse = data.m_pCourse;
			Trail *pTrail = pCourse->GetTrail( st, dc );
			LuaHelpers::Push( L, pTrail );
		}
		// Because pSteps or pTrail can be nullptr, what we're creating in Lua is not an array.
		// It must be iterated using pairs(), not ipairs().
		lua_setfield( L, -2, ssprintf("%d",i+1) );
	}
	lua_pop( L, 1 );
	LUA->Release( L );


	item.HandleMessage( msg );
	LOG->Trace("end ScoreScroller::ConfigureActor");
}

void ScoreScroller::LoadSongs( int iNumRecentScores )
{
	std::vector<Song*> vpSongs;
	GetAllSongsToShow( vpSongs, iNumRecentScores );
	m_vScoreRowItemData.resize( vpSongs.size() );
	for( unsigned i=0; i<m_vScoreRowItemData.size(); ++i )
		m_vScoreRowItemData[i].m_pSong = vpSongs[i];
}

void ScoreScroller::LoadCourses( CourseType ct, int iNumRecentScores )
{
	std::vector<Course*> vpCourses;
	GetAllCoursesToShow( vpCourses, ct, iNumRecentScores );
	m_vScoreRowItemData.resize( vpCourses.size() );
	for( unsigned i=0; i<m_vScoreRowItemData.size(); ++i )
		m_vScoreRowItemData[i].m_pCourse = vpCourses[i];
}

void ScoreScroller::Load( RString sMetricsGroup )
{
	SCROLLER_ITEMS_TO_DRAW.Load(sMetricsGroup, "ScrollerItemsToDraw");
	SCROLLER_SECONDS_PER_ITEM.Load(sMetricsGroup, "ScrollerSecondsPerItem");


	int iNumCopies = SCROLLER_ITEMS_TO_DRAW+1;
	for( int i=0; i<iNumCopies; ++i )
	{
		Actor *pActor = ActorUtil::MakeActor( THEME->GetPathG(sMetricsGroup,"ScrollerItem") );
		if( pActor != nullptr )
			this->AddChild( pActor );
	}

	DynamicActorScroller::SetTransformFromReference( THEME->GetMetricR(sMetricsGroup,"ScrollerItemTransformFunction") );
	DynamicActorScroller::SetNumItemsToDraw( (float) SCROLLER_ITEMS_TO_DRAW );
	DynamicActorScroller::SetSecondsPerItem( SCROLLER_SECONDS_PER_ITEM );
	DynamicActorScroller::Load2();

	m_iNumItems = m_vScoreRowItemData.size();
}

/////////////////////////////////////////////

RString COLUMN_DIFFICULTY_NAME( std::size_t i ) { return ssprintf("ColumnDifficulty%d",int(i+1)); }
RString COLUMN_STEPS_TYPE_NAME( std::size_t i ) { return ssprintf("ColumnStepsType%d",int(i+1)); }

void ScreenHighScores::Init()
{
	ScreenAttract::Init();

	MANUAL_SCROLLING.Load( m_sName,"ManualScrolling");
	HIGH_SCORES_TYPE.Load( m_sName,"HighScoresType");
	NUM_COLUMNS.Load( m_sName, "NumColumns" );
	COLUMN_DIFFICULTY.Load( m_sName, COLUMN_DIFFICULTY_NAME, NUM_COLUMNS );
	COLUMN_STEPS_TYPE.Load( m_sName, COLUMN_STEPS_TYPE_NAME, NUM_COLUMNS );
	MAX_ITEMS_TO_SHOW.Load( m_sName, "MaxItemsToShow" );

	m_Scroller.SetName( "Scroller" );
	LOAD_ALL_COMMANDS( m_Scroller );
	HighScoresType type = HIGH_SCORES_TYPE;
	switch( type )
	{
	DEFAULT_FAIL( type );
	case HighScoresType_AllSteps:
		m_Scroller.LoadSongs( MAX_ITEMS_TO_SHOW );
		break;
	case HighScoresType_NonstopCourses:
	case HighScoresType_OniCourses:
	case HighScoresType_SurvivalCourses:
	case HighScoresType_AllCourses:
		{
			CourseType ct;
			switch( type )
			{
			default:
				FAIL_M(ssprintf("Invalid HighScoresType: %i", type));
			case HighScoresType_NonstopCourses:	ct = COURSE_TYPE_NONSTOP;	break;
			case HighScoresType_OniCourses:		ct = COURSE_TYPE_ONI;		break;
			case HighScoresType_SurvivalCourses:	ct = COURSE_TYPE_SURVIVAL;	break;
			case HighScoresType_AllCourses:		ct = CourseType_Invalid;	break;
			}

			m_Scroller.LoadCourses( ct, MAX_ITEMS_TO_SHOW );
		}
		break;
	}

	m_Scroller.Load( m_sName );
	this->AddChild( &m_Scroller );
}

void ScreenHighScores::BeginScreen()
{
	ScreenAttract::BeginScreen();

	std::vector<DifficultyAndStepsType> vdast;
	for( int i=0; i<NUM_COLUMNS; i++ )
	{
		DifficultyAndStepsType dast( COLUMN_DIFFICULTY.GetValue(i), COLUMN_STEPS_TYPE.GetValue(i) );
		vdast.push_back( dast );
	}

	m_Scroller.SetDisplay( vdast );

	if( (bool)MANUAL_SCROLLING )
		m_Scroller.ScrollTop();
	else
		m_Scroller.ScrollThroughAllItems();

	if( !MANUAL_SCROLLING )
	{
		float fSecs = m_Scroller.GetSecondsForCompleteScrollThrough();
		this->PostScreenMessage( SM_BeginFadingOut, fSecs );
	}
}

bool ScreenHighScores::Input( const InputEventPlus &input )
{
	//LOG->Trace( "ScreenRanking::Input()" );
	if( IsTransitioning() )
		return false;

	// If manually scrolling, pass the input to Screen::Input so it will call Menu*
	if( (bool)MANUAL_SCROLLING )
		return Screen::Input( input );
	else
		return ScreenAttract::Input( input );
}

void ScreenHighScores::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_BeginFadingOut )	/* Screen is starting to tween out. */
	{
		StartTransitioningScreen( SM_GoToNextScreen );
	}

	ScreenAttract::HandleScreenMessage( SM );
}

bool ScreenHighScores::MenuStart( const InputEventPlus &input )
{
	if( !MANUAL_SCROLLING )
		return false;
	if( !IsTransitioning() )
		StartTransitioningScreen( SM_GoToNextScreen );
	SCREENMAN->PlayStartSound();
	return true;
}

bool ScreenHighScores::MenuBack( const InputEventPlus &input )
{
	if( !MANUAL_SCROLLING )
		return false;
	if( !IsTransitioning() )
		StartTransitioningScreen( SM_GoToNextScreen );
	SCREENMAN->PlayStartSound();
	return true;
}

void ScreenHighScores::DoScroll( int iDir )
{
	if( !m_Scroller.Scroll(iDir) )
		iDir = 0;
	Message msg("Scrolled");
	msg.SetParam( "Dir", iDir );
	this->HandleMessage( msg );
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
