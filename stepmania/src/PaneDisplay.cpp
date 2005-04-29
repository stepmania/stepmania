#include "global.h"
#include "PaneDisplay.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "song.h"
#include "Steps.h"
#include "RageLog.h"
#include "ProfileManager.h"
#include "SongManager.h"
#include "Course.h"
#include "Style.h"
#include "Command.h"
#include "ActorUtil.h"
#include "Foreach.h"
#include "PercentageDisplay.h"

#define SHIFT_X(p)			THEME->GetMetricF(m_sName, ssprintf("ShiftP%iX", p+1))
#define SHIFT_Y(p)			THEME->GetMetricF(m_sName, ssprintf("ShiftP%iY", p+1))
#define NUM_ITEM_COLORS(s)	THEME->GetMetricI(m_sName, ssprintf("%sNumLevels",s))
#define ITEM_COLOR(s,n)		THEME->GetMetric (m_sName, ssprintf("%sLevel%i",s,n+1))

enum { NEED_NOTES=1, NEED_COURSE=2, NEED_PROFILE=4 };
struct Content_t
{
	const char *name;
	PaneTypes	type;
	int req;
};

static const Content_t g_Contents[NUM_PANE_CONTENTS] =
{
	{ "SongNumSteps",			PANE_SONG_DIFFICULTY,		NEED_NOTES },
	{ "SongJumps",				PANE_SONG_DIFFICULTY,		NEED_NOTES },
	{ "SongHolds",				PANE_SONG_DIFFICULTY,		NEED_NOTES },
	{ "SongRolls",				PANE_SONG_DIFFICULTY,		NEED_NOTES },
	{ "SongMines",				PANE_SONG_DIFFICULTY,		NEED_NOTES },
	{ "SongHands",				PANE_SONG_DIFFICULTY,		NEED_NOTES },
	{ "DifficultyStream",		NUM_PANES,					NEED_NOTES }, // hide
	{ "DifficultyChaos",		NUM_PANES,					NEED_NOTES },
	{ "DifficultyFreeze",		NUM_PANES,					NEED_NOTES },
	{ "DifficultyAir",			NUM_PANES,					NEED_NOTES },
	{ "DifficultyVoltage",		NUM_PANES,					NEED_NOTES },
	{ "MachineHighScore",		PANE_SONG_DIFFICULTY,		NEED_NOTES },
	{ "MachineNumPlays",		NUM_PANES,					NEED_NOTES },
	{ "MachineRank",			NUM_PANES,					NEED_NOTES },
	{ "MachineHighName",		PANE_SONG_DIFFICULTY,		NEED_NOTES },
	{ "ProfileHighScore",		PANE_SONG_DIFFICULTY,		NEED_NOTES|NEED_PROFILE },
	{ "ProfileNumPlays",		NUM_PANES,					NEED_NOTES|NEED_PROFILE },
	{ "ProfileRank",			NUM_PANES,					NEED_NOTES|NEED_PROFILE },
	{ "CourseMachineHighScore",	PANE_COURSE_MACHINE_SCORES,	NEED_COURSE },
	{ "CourseMachineNumPlays",	NUM_PANES,					NEED_COURSE },
	{ "CourseMachineRank",		NUM_PANES,					NEED_COURSE },
	{ "CourseMachineHighName",	PANE_COURSE_MACHINE_SCORES,	NEED_COURSE },
	{ "CourseProfileHighScore",	PANE_COURSE_MACHINE_SCORES,	NEED_COURSE|NEED_PROFILE },
	{ "CourseProfileNumPlays",	NUM_PANES,					NEED_COURSE|NEED_PROFILE },
	{ "CourseProfileRank",		NUM_PANES,					NEED_COURSE|NEED_PROFILE },
	{ "CourseNumSteps",			PANE_COURSE_MACHINE_SCORES,	NEED_COURSE },
	{ "CourseJumps",			PANE_COURSE_MACHINE_SCORES,	NEED_COURSE },
	{ "CourseHolds",			PANE_COURSE_MACHINE_SCORES,	NEED_COURSE },
	{ "CourseMines",			PANE_COURSE_MACHINE_SCORES,	NEED_COURSE },
	{ "CourseHands",			PANE_COURSE_MACHINE_SCORES,	NEED_COURSE },
	{ "CourseRolls",			PANE_COURSE_MACHINE_SCORES,	NEED_COURSE }
};

static ProfileSlot PlayerMemCard( PlayerNumber pn )
{
	switch( pn )
	{
	case PLAYER_1: return PROFILE_SLOT_PLAYER_1;
	case PLAYER_2: return PROFILE_SLOT_PLAYER_2;
	default: ASSERT(0);	return PROFILE_SLOT_MACHINE;
	};
}

PaneDisplay::PaneDisplay()
{
	m_CurPane = PANE_INVALID;
}

void PaneDisplay::Load( PlayerNumber pn )
{
	m_PlayerNumber = pn;

	m_sprPaneUnder.Load( THEME->GetPathG("PaneDisplay",ssprintf("under p%i",pn+1)) );
	m_sprPaneUnder->SetName( "Under" );
	ON_COMMAND( m_sprPaneUnder );
	this->AddChild( m_sprPaneUnder );

	FOREACH_PaneContents(p)
	{
		ArrayLevels &levels = m_Levels[p];
		levels.clear();

		if( g_Contents[p].type == NUM_PANES )
			continue;

		const int num = NUM_ITEM_COLORS( g_Contents[p].name );
		for( int c = 0; c < num; ++c )
		{
			levels.push_back( Level() );
			Level &level = levels.back();

			const CString metric = ITEM_COLOR(g_Contents[p].name, c);

			Commands cmds;
			ParseCommands( metric, cmds );
			if( cmds.v.size() < 2 )
				RageException::Throw( "Metric '%s' malformed", metric.c_str() );

			level.m_fIfLessThan = cmds.v[0].GetArg(0);
			cmds.v.erase( cmds.v.begin(), cmds.v.begin()+1 );

			// TODO: clean this up
			vector<CString> vs;
			FOREACH_CONST( Command, cmds.v, cmd )
				vs.push_back( cmd->GetOriginalCommandString() );

			level.m_Command = apActorCommands( new ActorCommands(join(";",vs)) );
		}
	}


	for( int p = 0; p < NUM_PANE_CONTENTS; ++p )
	{
		if( g_Contents[p].type == NUM_PANES )
			continue; /* skip, disabled */

		m_textContents[p].LoadFromFont( THEME->GetPathF("PaneDisplay","text") );
		m_textContents[p].SetName( ssprintf("%sText", g_Contents[p].name) );
		SET_XY_AND_ON_COMMAND( m_textContents[p] );
		m_ContentsFrame.AddChild( &m_textContents[p] );

		m_Labels[p].Load( THEME->GetPathG("PaneDisplay",CString(g_Contents[p].name)+" label") );
		m_Labels[p]->SetName( ssprintf("%sLabel", g_Contents[p].name) );
		SET_XY_AND_ON_COMMAND( m_Labels[p] );
		m_ContentsFrame.AddChild( m_Labels[p] );
	}

	m_ContentsFrame.SetXY( SHIFT_X(m_PlayerNumber), SHIFT_Y(m_PlayerNumber) );
	this->AddChild( &m_ContentsFrame );

	m_sprPaneOver.Load( THEME->GetPathG("PaneDisplay",ssprintf("over p%i", pn+1)) );
	m_sprPaneOver->SetName( "Over" );
	ON_COMMAND( m_sprPaneOver );
	this->AddChild( m_sprPaneOver );

	for( unsigned i = 0; i < NUM_PANE_CONTENTS; ++i )
	{
		if( g_Contents[i].type == NUM_PANES )
			continue; /* skip, disabled */
		COMMAND( m_textContents[i], "LoseFocus"  );
		COMMAND( m_Labels[i], "LoseFocus"  );
		m_textContents[i].FinishTweening();
		m_Labels[i]->FinishTweening();
	}

	m_CurPane = PANE_INVALID;
	SetFocus( GetPane() );
}

void PaneDisplay::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );
}

void PaneDisplay::SetContent( PaneContents c )
{
	CString str = "?";	// fill this in
	float val = 0;	// fill this in

	const Song *pSong = GAMESTATE->m_pCurSong;
	const Steps *pSteps = GAMESTATE->m_pCurSteps[m_PlayerNumber];
	const Course *pCourse = GAMESTATE->m_pCurCourse;
	const Trail *pTrail = GAMESTATE->m_pCurTrail[m_PlayerNumber];
	const Profile *pProfile = PROFILEMAN->GetProfile( m_PlayerNumber );
	bool bIsEdit = pSteps && pSteps->GetDifficulty() == DIFFICULTY_EDIT;

	if( (g_Contents[c].req&NEED_NOTES) && !pSteps )
		goto done;
	if( (g_Contents[c].req&NEED_COURSE) && !pTrail )
		goto done;
	if( (g_Contents[c].req&NEED_PROFILE) && !pProfile )
	{
		str = "N/A";
		goto done;
	}

	{
		RadarValues rv;

		if( g_Contents[c].req&NEED_NOTES )
			rv = pSteps->GetRadarValues();
		else if( g_Contents[c].req&NEED_COURSE )
			rv = pTrail->GetRadarValues();

		switch( c )
		{
		case COURSE_NUM_STEPS:
		case SONG_NUM_STEPS:				val = rv[RADAR_NUM_TAPS_AND_HOLDS]; break;
		case COURSE_JUMPS:
		case SONG_JUMPS:					val = rv[RADAR_NUM_JUMPS]; break;
		case COURSE_HOLDS:
		case SONG_HOLDS:					val = rv[RADAR_NUM_HOLDS]; break;
		case COURSE_ROLLS:
		case SONG_ROLLS:					val = rv[RADAR_NUM_ROLLS]; break;
		case COURSE_MINES:
		case SONG_MINES:					val = rv[RADAR_NUM_MINES]; break;
		case COURSE_HANDS:
		case SONG_HANDS:					val = rv[RADAR_NUM_HANDS]; break;
		case SONG_DIFFICULTY_RADAR_STREAM:	val = rv[RADAR_STREAM]; break;
		case SONG_DIFFICULTY_RADAR_VOLTAGE:	val = rv[RADAR_VOLTAGE]; break;
		case SONG_DIFFICULTY_RADAR_AIR:		val = rv[RADAR_AIR]; break;
		case SONG_DIFFICULTY_RADAR_FREEZE:	val = rv[RADAR_FREEZE]; break;
		case SONG_DIFFICULTY_RADAR_CHAOS:	val = rv[RADAR_CHAOS]; break;
		case SONG_PROFILE_HIGH_SCORE:
			val = PROFILEMAN->GetProfile(m_PlayerNumber)->GetStepsHighScoreList(pSong,pSteps).GetTopScore().fPercentDP;
			break;
		case SONG_PROFILE_NUM_PLAYS:
			val = (float) PROFILEMAN->GetProfile(m_PlayerNumber)->GetStepsNumTimesPlayed(pSong,pSteps);
			break;

		case SONG_MACHINE_HIGH_NAME: /* set val for color */
		case SONG_MACHINE_HIGH_SCORE:
			CHECKPOINT;
			if( bIsEdit )	
				goto done;	// no machine scores for edits
			val = PROFILEMAN->GetMachineProfile()->GetStepsHighScoreList(pSong,pSteps).GetTopScore().fPercentDP;
			break;

		case SONG_MACHINE_RANK:
			{
			const vector<Song*> best = SONGMAN->GetBestSongs( PROFILE_SLOT_MACHINE );
			val = (float) FindIndex( best.begin(), best.end(), pSong );
			val += 1;
			break;
			}

		case SONG_PROFILE_RANK:
			{
			const vector<Song*> best = SONGMAN->GetBestSongs( PlayerMemCard(m_PlayerNumber) );
			val = (float) FindIndex( best.begin(), best.end(), pSong );
			val += 1;
			break;
			}

		case COURSE_MACHINE_HIGH_NAME: /* set val for color */
		case COURSE_MACHINE_HIGH_SCORE:
			val = PROFILEMAN->GetMachineProfile()->GetCourseHighScoreList(pCourse,pTrail).GetTopScore().fPercentDP;
			break;

		case COURSE_MACHINE_NUM_PLAYS:
			val = (float) PROFILEMAN->GetMachineProfile()->GetCourseNumTimesPlayed( pCourse );
			break;

		case COURSE_MACHINE_RANK:
			{
				CourseType ct = PlayModeToCourseType( GAMESTATE->m_PlayMode );
				const vector<Course*> best = SONGMAN->GetBestCourses( ct, PROFILE_SLOT_MACHINE );
				val = (float) FindIndex( best.begin(), best.end(), pCourse );
				val += 1;
			}
			break;

		case COURSE_PROFILE_HIGH_SCORE:
			val = PROFILEMAN->GetProfile(m_PlayerNumber)->GetCourseHighScoreList(pCourse,pTrail).GetTopScore().fPercentDP;
			break;
		case COURSE_PROFILE_NUM_PLAYS:
			val = (float) PROFILEMAN->GetProfile(m_PlayerNumber)->GetCourseNumTimesPlayed( pCourse );
			break;

		case COURSE_PROFILE_RANK:
			{
				CourseType ct = PlayModeToCourseType( GAMESTATE->m_PlayMode );
				const vector<Course*> best = SONGMAN->GetBestCourses( ct, PlayerMemCard(m_PlayerNumber) );
				val = (float) FindIndex( best.begin(), best.end(), pCourse );
				val += 1;
			}
			break;
		};

		if( val == RADAR_VAL_UNKNOWN )
			goto done;

		/* Scale, round, clamp, etc. for floats: */
		switch( c )
		{
		case SONG_DIFFICULTY_RADAR_STREAM:
		case SONG_DIFFICULTY_RADAR_VOLTAGE:
		case SONG_DIFFICULTY_RADAR_AIR:
		case SONG_DIFFICULTY_RADAR_FREEZE:
		case SONG_DIFFICULTY_RADAR_CHAOS:
			val = roundf( SCALE( val, 0, 1, 0, 10 ) );
			val = clamp( val, 0, 10 );
			str = ssprintf( "%.0f", val );
			break;
		}

		switch( c )
		{
		case SONG_MACHINE_HIGH_NAME:
			if( PROFILEMAN->GetMachineProfile()->GetStepsHighScoreList(pSong,pSteps).vHighScores.empty() )
			{
				str = "";
			}
			else
			{
				str = PROFILEMAN->GetMachineProfile()->GetStepsHighScoreList(pSong,pSteps).GetTopScore().sName;
				if( str.empty() )
					str = "????";
			}
			break;
		case COURSE_MACHINE_HIGH_NAME:
			if( PROFILEMAN->GetMachineProfile()->GetCourseHighScoreList(pCourse,pTrail).vHighScores.empty() )
			{
				str = "";
			}
			else
			{
				str = PROFILEMAN->GetMachineProfile()->GetCourseHighScoreList(pCourse,pTrail).GetTopScore().sName;
				if( str.empty() )
					str = "????";
			}
			break;

		case SONG_MACHINE_HIGH_SCORE:
		case COURSE_MACHINE_HIGH_SCORE:
		case SONG_PROFILE_HIGH_SCORE:
		case COURSE_PROFILE_HIGH_SCORE:
			/* "100.00%" is bigger than anything else, and since we'll never
			 * be any higher, we don't need to display the decimal places.
			 * Display 1.0 as "100%" instead. */
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
		case SONG_MACHINE_NUM_PLAYS:
		case COURSE_MACHINE_NUM_PLAYS:
		case SONG_PROFILE_NUM_PLAYS:
		case COURSE_PROFILE_NUM_PLAYS:
		case SONG_MACHINE_RANK:
		case COURSE_MACHINE_RANK:
		case SONG_PROFILE_RANK:
		case COURSE_PROFILE_RANK:
			str = ssprintf( "%.0f", val );
		}
	}


done:
	m_textContents[c].SetText( str );

	const ArrayLevels &levels = m_Levels[c];
	unsigned p;
	for( p = 0; p+1 < levels.size(); ++p )
		if( val < levels[p].m_fIfLessThan )
			break;

	if( p < levels.size() )
		m_textContents[c].RunCommands( levels[p].m_Command );
}

void PaneDisplay::SetFromGameState( SortOrder so )
{
	m_SortOrder = so;
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
		if( g_Contents[i].type == NUM_PANES )
			continue; /* skip, disabled */

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

	SetFromGameState( m_SortOrder );
}

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
