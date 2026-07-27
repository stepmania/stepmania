#include "global.h"
#include "ScreenRanking.h"
#include "ScreenManager.h"
#include "SongManager.h"
#include "GameState.h"
#include "GameManager.h"
#include "ProfileManager.h"
#include "Profile.h"

#include <cstddef>
#include <vector>


static const char *RankingTypeNames[] = {
	"Category",
	"Course",
};
XToString( RankingType );
LuaXType( RankingType );

AutoScreenMessage( SM_ShowNextPage );
AutoScreenMessage( SM_HidePage );

REGISTER_SCREEN_CLASS( ScreenRanking );

#define BULLET_X(row)	(BULLET_START_X+ROW_SPACING_X*row)
#define BULLET_Y(row)	(BULLET_START_Y+ROW_SPACING_Y*row)
#define NAME_X(row)	(NAME_START_X+ROW_SPACING_X*row)
#define NAME_Y(row)	(NAME_START_Y+ROW_SPACING_Y*row)
#define SCORE_X(row)	(SCORE_START_X+ROW_SPACING_X*row)
#define SCORE_Y(row)	(SCORE_START_Y+ROW_SPACING_Y*row)
#define POINTS_X(row)	(POINTS_START_X+ROW_SPACING_X*row)
#define POINTS_Y(row)	(POINTS_START_Y+ROW_SPACING_Y*row)
#define TIME_X(row)	(TIME_START_X+ROW_SPACING_X*row)
#define TIME_Y(row)	(TIME_START_Y+ROW_SPACING_Y*row)

static RString STEPS_TYPE_COLOR_NAME( std::size_t i ) { return ssprintf("StepsTypeColor%d",int(i+1)); }

void ScreenRanking::Init()
{
	STEPS_TYPES_TO_SHOW.Load( m_sName,"StepsTypesToHide" );	// tricky: The metric name is "hide" because ThemeMetricStepsTypesToShow takes a list of what to hide and returns what StepsTypes to show
	PAGE_FADE_SECONDS.Load	( m_sName,"PageFadeSeconds" );

	ScreenAttract::Init();

	m_textStepsType.SetName( "StepsType" );
	m_textStepsType.LoadFromFont( THEME->GetPathF(m_sName,"StepsType") );
	m_textStepsType.SetShadowLength( 0 );
	this->AddChild( &m_textStepsType );
	LOAD_ALL_COMMANDS( m_textStepsType );


	RANKING_TYPE.Load	( m_sName,"RankingType");
	COURSES_TO_SHOW.Load	( m_sName,"CoursesToShow");
	SECONDS_PER_PAGE.Load	( m_sName,"SecondsPerPage" );

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
	STEPS_TYPE_COLOR.Load( m_sName,STEPS_TYPE_COLOR_NAME,NUM_RANKING_LINES );

	if( RANKING_TYPE == RankingType_Category )
	{
		m_textCategory.SetName( "Category" );
		m_textCategory.LoadFromFont( THEME->GetPathF(m_sName,"category") );
		//m_textCategory.SetShadowLength( 0 );
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
				pts.aTypes.push_back( std::make_pair(Difficulty_Invalid, st) );
				m_vPagesToShow.push_back( pts );
			}
		}
	}

	if( RANKING_TYPE == RankingType_SpecificTrail )
	{
		m_Banner.SetName( "Banner" );
		this->AddChild( &m_Banner );
		LOAD_ALL_COMMANDS( m_Banner );

		m_textCourseTitle.SetName( "CourseTitle" );
		m_textCourseTitle.LoadFromFont( THEME->GetPathF(m_sName,"course title") );
		//m_textCourseTitle.SetShadowLength( 0 );
		this->AddChild( &m_textCourseTitle );
		LOAD_ALL_COMMANDS( m_textCourseTitle );

		std::vector<RString> asCoursePaths;
		split( COURSES_TO_SHOW, ",", asCoursePaths, true );
		for( unsigned i=0; i<STEPS_TYPES_TO_SHOW.GetValue().size(); i++ )
		{
			for( unsigned c=0; c<asCoursePaths.size(); c++ )
			{
				PageToShow pts;
				pts.colorIndex = i;
				StepsType st = STEPS_TYPES_TO_SHOW.GetValue()[i];
				pts.aTypes.push_back( std::make_pair(Difficulty_Invalid, st) );
				pts.pCourse = SONGMAN->GetCourseFromPath( asCoursePaths[c] );
				if( pts.pCourse == nullptr )
					continue;

				pts.pTrail = pts.pCourse->GetTrail( st );
				if( pts.pTrail == nullptr )
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

void ScreenRanking::BeginScreen()
{
	m_iNextPageToShow = 0;

	ScreenAttract::BeginScreen();

	this->HandleScreenMessage( SM_ShowNextPage );
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

			this->PlayCommand( "NextPage" );
			this->PostScreenMessage( SM_HidePage, fSecsToShow-PAGE_FADE_SECONDS );
		}
		else
		{
			StartTransitioningScreen(SM_GoToNextScreen);
		}
	}
	else if( SM == SM_HidePage )
	{
		this->PlayCommand( "SwitchedPage" );
		this->PostScreenMessage( SM_ShowNextPage, PAGE_FADE_SECONDS );
	}

	ScreenAttract::HandleScreenMessage( SM );
}

float ScreenRanking::SetPage( const PageToShow &pts )
{
	// This is going to take a while to load.  Possibly longer than one frame.
	// So, zero the next update so we don't skip.
	SCREENMAN->ZeroNextUpdate();

	// init page
	StepsType st = pts.aTypes.front().second;
	m_textStepsType.SetText( GAMEMAN->GetStepsTypeInfo(st).GetLocalizedString() );


	bool bShowScores = false;
	bool bShowPoints = false;
	bool bShowTime = false;
	switch( RANKING_TYPE )
	{
		case RankingType_Category:
			bShowScores = true;
			break;
		case RankingType_SpecificTrail:
			bShowScores = !pts.pCourse->IsOni();
			bShowPoints = pts.pCourse->IsOni();
			bShowTime = pts.pCourse->IsOni();
			break;
		default: break;
	}

	for( int l=0; l<NUM_RANKING_LINES; l++ )
	{
		m_textNames[l].SetDiffuseColor( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );

		m_textScores[l].SetVisible( bShowScores );
		m_textScores[l].SetDiffuseColor( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );

		m_textPoints[l].SetVisible( bShowPoints );
		m_textPoints[l].SetDiffuseColor( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );

		m_textTime[l].SetVisible( bShowTime );
		m_textTime[l].SetDiffuseColor( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );
	}

	RankingType rtype = RANKING_TYPE;
	switch( rtype )
	{
	case RankingType_Category:
		{
			m_textCategory.SetText( ssprintf("Type %c", 'A'+pts.category) );

			for( int l=0; l<NUM_RANKING_LINES; l++ )
			{
				// No need to shadow at this point.
				st = pts.aTypes.front().second;
				HighScoreList &hsl = PROFILEMAN->GetMachineProfile()->GetCategoryHighScoreList(st, pts.category);
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
				m_textNames[l].SetDiffuseColor( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );
				m_textScores[l].SetDiffuseColor( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );

				if( bRecentHighScore )
				{
					m_textNames[l].SetEffectGlowBlink(0.1f, RageColor(1,1,1,0.2f), RageColor(1,1,1,0.8f));
					m_textScores[l].SetEffectGlowBlink(0.1f, RageColor(1,1,1,0.2f), RageColor(1,1,1,0.8f));
				}
				else
				{
					m_textNames[l].StopEffect();
					m_textScores[l].StopEffect();
				}
			}
		}
		return SECONDS_PER_PAGE;
	case RankingType_SpecificTrail:
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
				m_textNames[l].SetDiffuseColor( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );
				m_textPoints[l].SetDiffuseColor( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );
				m_textTime[l].SetDiffuseColor( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );
				m_textScores[l].SetDiffuseColor( STEPS_TYPE_COLOR.GetValue(pts.colorIndex) );

				if( bRecentHighScore )
				{
					m_textNames[l].SetEffectGlowBlink(0.1f, RageColor(1,1,1,0.2f), RageColor(1,1,1,0.8f));
					m_textScores[l].SetEffectGlowBlink(0.1f, RageColor(1,1,1,0.2f), RageColor(1,1,1,0.8f));
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
		FAIL_M(ssprintf("Invalid RankingType: %i", rtype));
	}
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
