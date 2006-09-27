#include "global.h"
#include "PaneDisplay.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "song.h"
#include "Steps.h"
#include "RageLog.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "Course.h"
#include "Style.h"
#include "ActorUtil.h"
#include "Foreach.h"
#include "PercentageDisplay.h"
#include "LuaManager.h"
#include "XmlFile.h"

#define SHIFT_X(p)	THEME->GetMetricF(sMetricsGroup, ssprintf("ShiftP%iX", p+1))
#define SHIFT_Y(p)	THEME->GetMetricF(sMetricsGroup, ssprintf("ShiftP%iY", p+1))

enum { NEED_NOTES=1, NEED_COURSE=2, NEED_PROFILE=4 };
struct Content_t
{
	const char *name;
	PaneTypes	type;
	int req;
};

static const Content_t g_Contents[NUM_PANE_CONTENTS] =
{
	{ "SongNumSteps",		PANE_SONG_DIFFICULTY,		NEED_NOTES },
	{ "SongJumps",			PANE_SONG_DIFFICULTY,		NEED_NOTES },
	{ "SongHolds",			PANE_SONG_DIFFICULTY,		NEED_NOTES },
	{ "SongRolls",			PANE_SONG_DIFFICULTY,		NEED_NOTES },
	{ "SongMines",			PANE_SONG_DIFFICULTY,		NEED_NOTES },
	{ "SongHands",			PANE_SONG_DIFFICULTY,		NEED_NOTES },
	{ "MachineHighScore",		PANE_SONG_DIFFICULTY,		NEED_NOTES },
	{ "MachineHighName",		PANE_SONG_DIFFICULTY,		NEED_NOTES },
	{ "ProfileHighScore",		PANE_SONG_DIFFICULTY,		NEED_NOTES|NEED_PROFILE },
	{ "CourseMachineHighScore",	PANE_COURSE_MACHINE_SCORES,	NEED_COURSE },
	{ "CourseMachineHighName",	PANE_COURSE_MACHINE_SCORES,	NEED_COURSE },
	{ "CourseProfileHighScore",	PANE_COURSE_MACHINE_SCORES,	NEED_COURSE|NEED_PROFILE },
	{ "CourseNumSteps",		PANE_COURSE_MACHINE_SCORES,	NEED_COURSE },
	{ "CourseJumps",		PANE_COURSE_MACHINE_SCORES,	NEED_COURSE },
	{ "CourseHolds",		PANE_COURSE_MACHINE_SCORES,	NEED_COURSE },
	{ "CourseMines",		PANE_COURSE_MACHINE_SCORES,	NEED_COURSE },
	{ "CourseHands",		PANE_COURSE_MACHINE_SCORES,	NEED_COURSE },
	{ "CourseRolls",		PANE_COURSE_MACHINE_SCORES,	NEED_COURSE }
};

REGISTER_ACTOR_CLASS( PaneDisplay )

PaneDisplay::PaneDisplay()
{
	m_CurPane = PANE_INVALID;
}

void PaneDisplay::Load( const RString &sMetricsGroup, PlayerNumber pn )
{
	m_PlayerNumber = pn;

	m_sprPaneUnder.Load( THEME->GetPathG(sMetricsGroup,ssprintf("under p%i",pn+1)) );
	m_sprPaneUnder->SetName( "Under" );
	ActorUtil::OnCommand( m_sprPaneUnder, sMetricsGroup );
	this->AddChild( m_sprPaneUnder );

	EMPTY_MACHINE_HIGH_SCORE_NAME.Load( sMetricsGroup, "EmptyMachineHighScoreName" );
	NOT_AVAILABLE.Load( sMetricsGroup, "NotAvailable" );


	for( int p = 0; p < NUM_PANE_CONTENTS; ++p )
	{
		if( g_Contents[p].type == NUM_PANES )
			continue; /* skip, disabled */

		m_textContents[p].LoadFromFont( THEME->GetPathF(sMetricsGroup,"text") );
		m_textContents[p].SetName( ssprintf("%sText", g_Contents[p].name) );
		ActorUtil::LoadAllCommands( m_textContents[p], sMetricsGroup );
		ActorUtil::SetXYAndOnCommand( m_textContents[p], sMetricsGroup );
		m_ContentsFrame.AddChild( &m_textContents[p] );

		m_Labels[p].Load( THEME->GetPathG(sMetricsGroup,RString(g_Contents[p].name)+" label") );
		m_Labels[p]->SetName( ssprintf("%sLabel", g_Contents[p].name) );
		ActorUtil::LoadAllCommands( *m_Labels[p], sMetricsGroup );
		ActorUtil::SetXYAndOnCommand( m_Labels[p], sMetricsGroup );
		m_ContentsFrame.AddChild( m_Labels[p] );

		ActorUtil::LoadAllCommandsFromName( m_textContents[p], sMetricsGroup, g_Contents[p].name );
	}

	m_ContentsFrame.SetXY( SHIFT_X(m_PlayerNumber), SHIFT_Y(m_PlayerNumber) );
	this->AddChild( &m_ContentsFrame );

	for( unsigned i = 0; i < NUM_PANE_CONTENTS; ++i )
	{
		m_textContents[i].PlayCommand( "LoseFocus" );
		m_Labels[i]->PlayCommand( "LoseFocus" );
		m_textContents[i].FinishTweening();
		m_Labels[i]->FinishTweening();
	}

	m_CurPane = PANE_INVALID;
}

void PaneDisplay::LoadFromNode( const RString &sDir, const XNode *pNode )
{
	bool b;

	RString sMetricsGroup;
	b = pNode->GetAttrValue( "MetricsGroup", sMetricsGroup );
	ASSERT( b );

	RString sPlayerNumber;
	b = pNode->GetAttrValue( "PlayerNumber", sPlayerNumber );
	ASSERT( b );
	PlayerNumber pn = (PlayerNumber) LuaHelpers::RunExpressionI(sPlayerNumber);

	Load( sMetricsGroup, pn );

	ActorFrame::LoadFromNode( sDir, pNode );
}

void PaneDisplay::SetContent( PaneContents c )
{
	RString str = "?";	// fill this in
	float val = 0;	// fill this in

	const Song *pSong = GAMESTATE->m_pCurSong;
	const Steps *pSteps = GAMESTATE->m_pCurSteps[m_PlayerNumber];
	const Course *pCourse = GAMESTATE->m_pCurCourse;
	const Trail *pTrail = GAMESTATE->m_pCurTrail[m_PlayerNumber];
	const Profile *pProfile = PROFILEMAN->IsPersistentProfile(m_PlayerNumber) ? PROFILEMAN->GetProfile(m_PlayerNumber) : NULL;
	bool bIsPlayerEdit = pSteps && pSteps->IsAPlayerEdit();

	if( (g_Contents[c].req&NEED_NOTES) && !pSteps )
		goto done;
	if( (g_Contents[c].req&NEED_COURSE) && !pTrail )
		goto done;
	if( (g_Contents[c].req&NEED_PROFILE) && !pProfile )
	{
		str = NOT_AVAILABLE;
		goto done;
	}

	{
		RadarValues rv;

		if( g_Contents[c].req&NEED_NOTES )
			rv = pSteps->GetRadarValues( m_PlayerNumber );
		else if( g_Contents[c].req&NEED_COURSE )
			rv = pTrail->GetRadarValues();

		switch( c )
		{
		case COURSE_NUM_STEPS:
		case SONG_NUM_STEPS:	val = rv[RadarCategory_TapsAndHolds]; break;
		case COURSE_JUMPS:
		case SONG_JUMPS:	val = rv[RadarCategory_Jumps]; break;
		case COURSE_HOLDS:
		case SONG_HOLDS:	val = rv[RadarCategory_Holds]; break;
		case COURSE_ROLLS:
		case SONG_ROLLS:	val = rv[RadarCategory_Rolls]; break;
		case COURSE_MINES:
		case SONG_MINES:	val = rv[RadarCategory_Mines]; break;
		case COURSE_HANDS:
		case SONG_HANDS:	val = rv[RadarCategory_Hands]; break;
		case SONG_PROFILE_HIGH_SCORE:
			val = PROFILEMAN->GetProfile(m_PlayerNumber)->GetStepsHighScoreList(pSong,pSteps).GetTopScore().GetPercentDP();
			break;

		case SONG_MACHINE_HIGH_NAME: /* set val for color */
		case SONG_MACHINE_HIGH_SCORE:
			CHECKPOINT;
			val = PROFILEMAN->GetMachineProfile()->GetStepsHighScoreList(pSong,pSteps).GetTopScore().GetPercentDP();
			break;

		case COURSE_MACHINE_HIGH_NAME: /* set val for color */
		case COURSE_MACHINE_HIGH_SCORE:
			val = PROFILEMAN->GetMachineProfile()->GetCourseHighScoreList(pCourse,pTrail).GetTopScore().GetPercentDP();
			break;

		case COURSE_PROFILE_HIGH_SCORE:
			val = PROFILEMAN->GetProfile(m_PlayerNumber)->GetCourseHighScoreList(pCourse,pTrail).GetTopScore().GetPercentDP();
			break;
		};

		if( val == RADAR_VAL_UNKNOWN )
			goto done;

		switch( c )
		{
		case SONG_MACHINE_HIGH_NAME:
			if( PROFILEMAN->GetMachineProfile()->GetStepsHighScoreList(pSong,pSteps).vHighScores.empty() )
			{
				str = EMPTY_MACHINE_HIGH_SCORE_NAME;
			}
			else
			{
				str = PROFILEMAN->GetMachineProfile()->GetStepsHighScoreList(pSong,pSteps).GetTopScore().GetName();
				if( str.empty() )
					str = "????";
			}
			break;
		case COURSE_MACHINE_HIGH_NAME:
			if( PROFILEMAN->GetMachineProfile()->GetCourseHighScoreList(pCourse,pTrail).vHighScores.empty() )
			{
				str = EMPTY_MACHINE_HIGH_SCORE_NAME;
			}
			else
			{
				str = PROFILEMAN->GetMachineProfile()->GetCourseHighScoreList(pCourse,pTrail).GetTopScore().GetName();
				if( str.empty() )
					str = "????";
			}
			break;

		case SONG_MACHINE_HIGH_SCORE:
			// Don't show or save machine high scores for edits loaded from a player profile.
			if( bIsPlayerEdit )
				str = NOT_AVAILABLE;
			else
				str = PercentageDisplay::FormatPercentScore( val );
			break;
		case COURSE_MACHINE_HIGH_SCORE:
		case SONG_PROFILE_HIGH_SCORE:
		case COURSE_PROFILE_HIGH_SCORE:
			str = PercentageDisplay::FormatPercentScore( val );
			break;
		case SONG_NUM_STEPS:
		case SONG_JUMPS:
		case SONG_HOLDS:
		case SONG_ROLLS:
		case SONG_MINES:
		case SONG_HANDS:
		case COURSE_NUM_STEPS:
		case COURSE_JUMPS:
		case COURSE_HOLDS:
		case COURSE_ROLLS:
		case COURSE_MINES:
		case COURSE_HANDS:
			str = ssprintf( "%.0f", val );
		}
	}


done:
	m_textContents[c].SetText( str );

	Lua *L = LUA->Get();

	m_textContents[c].PushSelf( L );
	lua_pushstring( L, "PaneLevel" );
	lua_pushnumber( L, val );
	lua_settable( L, -3 );
	lua_pop( L, 1 );

	m_textContents[c].PlayCommand( "Level" );

	LUA->Release(L);
}

void PaneDisplay::SetFromGameState()
{
	m_SortOrder = GAMESTATE->m_SortOrder;
	SetFocus( GetPane() );

	/* Don't update text that doesn't apply to the current mode.  It's still tweening off. */
	for( unsigned i = 0; i < NUM_PANE_CONTENTS; ++i )
	{
		if( g_Contents[i].type != m_CurPane )
			continue;
		SetContent( (PaneContents) i );
	}
}

PaneTypes PaneDisplay::GetPane() const
{
	switch( m_SortOrder )
	{
	case SORT_ALL_COURSES:
	case SORT_NONSTOP_COURSES:
	case SORT_ONI_COURSES:
	case SORT_ENDLESS_COURSES:
		return PANE_COURSE_MACHINE_SCORES;
	case SORT_MODE_MENU:
		return m_CurPane; // leave it
	default:
		return PANE_SONG_DIFFICULTY;
	}
}


void PaneDisplay::SetFocus( PaneTypes NewPane )
{
	if( m_CurPane == NewPane )
		return;

	for( unsigned i = 0; i < NUM_PANE_CONTENTS; ++i )
	{
		if( g_Contents[i].type == m_CurPane )
		{
			COMMAND( m_textContents[i], "LoseFocus"  );
			COMMAND( m_Labels[i], "LoseFocus"  );
		}
		else if( g_Contents[i].type == NewPane )
		{
			COMMAND( m_textContents[i], "GainFocus" );
			COMMAND( m_Labels[i], "GainFocus" );
		}
	}

	m_CurPane = NewPane;
}


// lua start
#include "LuaBinding.h"

class LunaPaneDisplay: public Luna<PaneDisplay>
{
public:
	static int SetFromGameState( T* p, lua_State *L )	{ p->SetFromGameState(); return 0; }

	LunaPaneDisplay()
	{
		LUA->Register( Register );

		ADD_METHOD( SetFromGameState );
	}
};

LUA_REGISTER_DERIVED_CLASS( PaneDisplay, ActorFrame )
// lua end

/*
 * (c) 2003 Glenn Maynard
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
