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
	{ "SongMines",				PANE_SONG_DIFFICULTY,		NEED_NOTES },
	{ "SongHands",				PANE_SONG_DIFFICULTY,		NEED_NOTES },
	{ "DifficultyStream",		NUM_PANES,					NEED_NOTES }, // hide
	{ "DifficultyChaos",		NUM_PANES,					NEED_NOTES },
	{ "DifficultyFreeze",		NUM_PANES,					NEED_NOTES },
	{ "DifficultyAir",			NUM_PANES,					NEED_NOTES },
	{ "DifficultyVoltage",		NUM_PANES,					NEED_NOTES },
	{ "MachineHighScore",		PANE_SONG_DIFFICULTY,		NEED_NOTES },
	{ "MachineNumPlays",		PANE_SONG_MACHINE_SCORES,	NEED_NOTES },
	{ "MachineRank",			PANE_SONG_MACHINE_SCORES,	NEED_NOTES },
	{ "MachineHighName",		PANE_SONG_DIFFICULTY,		NEED_NOTES },
	{ "ProfileHighScore",		PANE_SONG_DIFFICULTY,		NEED_NOTES|NEED_PROFILE },
	{ "ProfileNumPlays",		PANE_SONG_PROFILE_SCORES,	NEED_NOTES|NEED_PROFILE },
	{ "ProfileRank",			PANE_SONG_PROFILE_SCORES,	NEED_NOTES|NEED_PROFILE },
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
	{ "CourseHands",			PANE_COURSE_MACHINE_SCORES,	NEED_COURSE }
};

static const PaneModes PaneMode[NUM_PANES] =
{
	PANEMODE_SONG,
	PANEMODE_SONG,
	PANEMODE_SONG,
//	PANEMODE_SONG,
	PANEMODE_BATTLE,
	PANEMODE_COURSE,
	PANEMODE_COURSE
//	PANEMODE_COURSE
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
	m_CurMode = PANEMODE_SONG;
}

void PaneDisplay::Load( PlayerNumber pn )
{
	m_PlayerNumber = pn;
	m_PreferredPaneForMode[PANEMODE_SONG] = PANE_SONG_DIFFICULTY;
	m_PreferredPaneForMode[PANEMODE_BATTLE] = PANE_BATTLE_DIFFICULTY;
	m_PreferredPaneForMode[PANEMODE_COURSE] = PANE_COURSE_MACHINE_SCORES;

	m_sprPaneUnder.Load( THEME->GetPathToG( ssprintf("PaneDisplay under p%i", pn+1)) );
	m_sprPaneUnder->SetName( "Under" );
	ON_COMMAND( m_sprPaneUnder );
	this->AddChild( m_sprPaneUnder );

	int p;
	for( p = 0; p < NUM_PANE_CONTENTS; ++p )
	{
		if( g_Contents[p].type == NUM_PANES )
			continue; /* skip, disabled */

		m_textContents[p].LoadFromFont( THEME->GetPathToF( "PaneDisplay text" ) );
		m_textContents[p].SetName( ssprintf("%sText", g_Contents[p].name) );
		SET_XY_AND_ON_COMMAND( m_textContents[p] );
		m_ContentsFrame.AddChild( &m_textContents[p] );

		m_Labels[p].Load( THEME->GetPathToG( ssprintf("PaneDisplay %s label", g_Contents[p].name)) );
		m_Labels[p]->SetName( ssprintf("%sLabel", g_Contents[p].name) );
		SET_XY_AND_ON_COMMAND( m_Labels[p] );
		m_ContentsFrame.AddChild( m_Labels[p] );
	}

	m_ContentsFrame.SetXY( SHIFT_X(m_PlayerNumber), SHIFT_Y(m_PlayerNumber) );
	this->AddChild( &m_ContentsFrame );

	m_sprPaneOver.Load( THEME->GetPathToG( ssprintf("PaneDisplay over p%i", pn+1)) );
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

	m_CurMode = GetMode();
	SetFocus( GetNext( m_PreferredPaneForMode[m_CurMode], 0 ) );
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
			val = 100.0f * PROFILEMAN->GetProfile(m_PlayerNumber)->GetStepsHighScoreList(pSong,pSteps).GetTopScore().fPercentDP;
			break;
		case SONG_PROFILE_NUM_PLAYS:
			val = (float) PROFILEMAN->GetProfile(m_PlayerNumber)->GetStepsNumTimesPlayed(pSong,pSteps);
			break;

		case SONG_MACHINE_HIGH_NAME: /* set val for color */
		case SONG_MACHINE_HIGH_SCORE:
			CHECKPOINT;
			if( bIsEdit )	
				goto done;	// no machine scores for edits
			val = 100.0f * PROFILEMAN->GetMachineProfile()->GetStepsHighScoreList(pSong,pSteps).GetTopScore().fPercentDP;
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
			val = 100.0f * PROFILEMAN->GetMachineProfile()->GetCourseHighScoreList(pCourse,pTrail).GetTopScore().fPercentDP;
			break;

		case COURSE_MACHINE_NUM_PLAYS:
			val = (float) PROFILEMAN->GetMachineProfile()->GetCourseNumTimesPlayed( pCourse );
			break;

		case COURSE_MACHINE_RANK:
			{
			const vector<Course*> best = SONGMAN->GetBestCourses( PROFILE_SLOT_MACHINE );
			val = (float) FindIndex( best.begin(), best.end(), pCourse );
			val += 1;
			}
			break;

		case COURSE_PROFILE_HIGH_SCORE:
			val = 100.0f * PROFILEMAN->GetProfile(m_PlayerNumber)->GetCourseHighScoreList(pCourse,pTrail).GetTopScore().fPercentDP;
			break;
		case COURSE_PROFILE_NUM_PLAYS:
			val = (float) PROFILEMAN->GetProfile(m_PlayerNumber)->GetCourseNumTimesPlayed( pCourse );
			break;

		case COURSE_PROFILE_RANK:
			const vector<Course*> best = SONGMAN->GetBestCourses( PlayerMemCard(m_PlayerNumber) );
			val = (float) FindIndex( best.begin(), best.end(), pCourse );
			val += 1;
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
			str = PROFILEMAN->GetMachineProfile()->GetStepsHighScoreList(pSong,pSteps).GetTopScore().sName;
			break;
		case COURSE_MACHINE_HIGH_NAME:

			str = PROFILEMAN->GetMachineProfile()->GetCourseHighScoreList(pCourse,pTrail).GetTopScore().sName;
			break;

		case SONG_MACHINE_HIGH_SCORE:
		case COURSE_MACHINE_HIGH_SCORE:
		case SONG_PROFILE_HIGH_SCORE:
		case COURSE_PROFILE_HIGH_SCORE:
			str = ssprintf( "%.2f%%", val );
			break;
		case SONG_NUM_STEPS:
		case SONG_JUMPS:
		case SONG_HOLDS:
		case SONG_MINES:
		case SONG_HANDS:
		case COURSE_NUM_STEPS:
		case COURSE_JUMPS:
		case COURSE_HOLDS:
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

	const int num = NUM_ITEM_COLORS( g_Contents[c].name );
	for( int p = 0; p < num; ++p )
	{
		const CString metric = ITEM_COLOR(g_Contents[c].name, p);
		CStringArray spec;
		split( metric, ";", spec );
		if( spec.size() < 2 )
			RageException::Throw( "Metric '%s' malformed", metric.c_str() );

		const float n = strtof( spec[0], NULL );
		if( val >= n )
			continue;
		spec.erase( spec.begin(), spec.begin()+1 );

		m_textContents[c].Command( join(";", spec) );
		break;
	}
}

void PaneDisplay::SetFromGameState()
{
	m_CurMode = GetMode();
	if( PaneMode[m_CurPane] != m_CurMode )
		SetFocus( GetNext( m_PreferredPaneForMode[m_CurMode], 0 ) );

	/* Don't update text that doesn't apply to the current mode.  It's still tweening off. */
	for( unsigned i = 0; i < NUM_PANE_CONTENTS; ++i )
	{
		if( g_Contents[i].type != m_CurPane )
			continue;
		SetContent( (PaneContents) i );
	}
}

PaneModes PaneDisplay::GetMode() const
{
	switch( GAMESTATE->m_SortOrder )
	{
	case SORT_ALL_COURSES:
	case SORT_NONSTOP_COURSES:
	case SORT_ONI_COURSES:
	case SORT_ENDLESS_COURSES:
		return PANEMODE_COURSE;
	case SORT_SORT_MENU:
	case SORT_MODE_MENU:
		return m_CurMode; // leave it
	default:
		return PANEMODE_SONG;
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
	m_PreferredPaneForMode[m_CurMode] = NewPane;

	SetFromGameState();
}

bool PaneDisplay::PaneIsValid( PaneTypes p ) const
{
	if( PaneMode[p] != m_CurMode )
		return false;

	switch( p )
	{
	case PANE_SONG_PROFILE_SCORES:
	case PANE_COURSE_PROFILE_SCORES:
		if( !PROFILEMAN->IsUsingProfile( m_PlayerNumber ) )
			return false;
	}

	return true;
}

PaneTypes PaneDisplay::GetNext( PaneTypes current, int dir ) const
{
	if( dir == 0 )
	{
		/* Only move the selection if the current selection is invalid. */
		if( PaneIsValid( current ) )
			return current;

		dir = 1;
	}

	PaneTypes ret = current;
	while( 1 )
	{
		ret = (PaneTypes) (ret + dir);
		wrap( (int&) ret, NUM_PANES );

		if( PaneIsValid( ret ) )
			break;
	}

	LOG->Trace("pane %i", ret);

	return ret;
}

void PaneDisplay::Move( int dir )
{
	const PaneTypes NewPane = GetNext( m_CurPane, dir );
	SetFocus( NewPane );
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
