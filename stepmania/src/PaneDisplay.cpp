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

enum { NEED_NOTES=1, NEED_PROFILE=2 };
struct Content_t
{
	const char *name;
	int req;
};

static const Content_t g_Contents[NUM_PaneContents] =
{
	{ "SongNumSteps",		NEED_NOTES },
	{ "SongJumps",			NEED_NOTES },
	{ "SongHolds",			NEED_NOTES },
	{ "SongRolls",			NEED_NOTES },
	{ "SongMines",			NEED_NOTES },
	{ "SongHands",			NEED_NOTES },
	{ "MachineHighScore",		NEED_NOTES },
	{ "MachineHighName",		NEED_NOTES },
	{ "ProfileHighScore",		NEED_NOTES|NEED_PROFILE },
};

REGISTER_ACTOR_CLASS( PaneDisplay )

void PaneDisplay::Load( const RString &sMetricsGroup, PlayerNumber pn )
{
	m_PlayerNumber = pn;

	EMPTY_MACHINE_HIGH_SCORE_NAME.Load( sMetricsGroup, "EmptyMachineHighScoreName" );
	NOT_AVAILABLE.Load( sMetricsGroup, "NotAvailable" );


	FOREACH_PaneContents( p )
	{
		m_textContents[p].LoadFromFont( THEME->GetPathF(sMetricsGroup,"text") );
		m_textContents[p].SetName( ssprintf("%sText", g_Contents[p].name) );
		ActorUtil::LoadAllCommands( m_textContents[p], sMetricsGroup );
		ActorUtil::SetXY( m_textContents[p], sMetricsGroup );
		m_ContentsFrame.AddChild( &m_textContents[p] );

		m_Labels[p].Load( THEME->GetPathG(sMetricsGroup,RString(g_Contents[p].name)+" label") );
		m_Labels[p]->SetName( ssprintf("%sLabel", g_Contents[p].name) );
		ActorUtil::LoadAllCommands( *m_Labels[p], sMetricsGroup );
		ActorUtil::SetXY( m_Labels[p], sMetricsGroup );
		m_ContentsFrame.AddChild( m_Labels[p] );

		ActorUtil::LoadAllCommandsFromName( m_textContents[p], sMetricsGroup, g_Contents[p].name );
	}

	m_ContentsFrame.SetXY( SHIFT_X(m_PlayerNumber), SHIFT_Y(m_PlayerNumber) );
	this->AddChild( &m_ContentsFrame );
}

void PaneDisplay::LoadFromNode( const XNode *pNode )
{
	bool b;

	RString sMetricsGroup;
	b = pNode->GetAttrValue( "MetricsGroup", sMetricsGroup );
	ASSERT( b );

	Lua *L = LUA->Get();
	b = pNode->PushAttrValue( L, "PlayerNumber" );
	ASSERT( b );
	PlayerNumber pn;
	LuaHelpers::Pop( L, pn );
	LUA->Release( L );

	Load( sMetricsGroup, pn );

	ActorFrame::LoadFromNode( pNode );
}

void PaneDisplay::SetContent( PaneContents c )
{
	RString str = "";	// fill this in
	float val = 0;	// fill this in

	const Song *pSong = GAMESTATE->m_pCurSong;
	const Steps *pSteps = GAMESTATE->m_pCurSteps[m_PlayerNumber];
	const Course *pCourse = GAMESTATE->m_pCurCourse;
	const Trail *pTrail = GAMESTATE->m_pCurTrail[m_PlayerNumber];
	const Profile *pProfile = PROFILEMAN->IsPersistentProfile(m_PlayerNumber) ? PROFILEMAN->GetProfile(m_PlayerNumber) : NULL;
	bool bIsPlayerEdit = pSteps && pSteps->IsAPlayerEdit();

	if( (g_Contents[c].req&NEED_NOTES) && !pSteps && !pTrail )
		goto done;
	if( (g_Contents[c].req&NEED_PROFILE) && !pProfile )
	{
		str = NOT_AVAILABLE;
		goto done;
	}

	{
		RadarValues rv;
		HighScoreList *pHSL = NULL;
		ProfileSlot slot = ProfileSlot_Machine;
		switch( c )
		{
		case SONG_PROFILE_HIGH_SCORE:
			slot = (ProfileSlot) m_PlayerNumber;
		}

		if( pSteps )
		{
			rv = pSteps->GetRadarValues( m_PlayerNumber );
			pHSL = &PROFILEMAN->GetProfile(slot)->GetStepsHighScoreList(pSong, pSteps);
		}
		else if( pTrail )
		{
			rv = pTrail->GetRadarValues();
			pHSL = &PROFILEMAN->GetProfile(slot)->GetCourseHighScoreList(pCourse, pTrail);
		}

		switch( c )
		{
		case SONG_NUM_STEPS:	val = rv[RadarCategory_TapsAndHolds]; break;
		case SONG_JUMPS:	val = rv[RadarCategory_Jumps]; break;
		case SONG_HOLDS:	val = rv[RadarCategory_Holds]; break;
		case SONG_ROLLS:	val = rv[RadarCategory_Rolls]; break;
		case SONG_MINES:	val = rv[RadarCategory_Mines]; break;
		case SONG_HANDS:	val = rv[RadarCategory_Hands]; break;
		case SONG_PROFILE_HIGH_SCORE:
		case SONG_MACHINE_HIGH_NAME: /* set val for color */
		case SONG_MACHINE_HIGH_SCORE:
			CHECKPOINT;
			val = pHSL->GetTopScore().GetPercentDP();
			break;
		};

		if( val == RADAR_VAL_UNKNOWN )
			goto done;

		switch( c )
		{
		case SONG_MACHINE_HIGH_NAME:
			if( pHSL->vHighScores.empty() )
			{
				str = EMPTY_MACHINE_HIGH_SCORE_NAME;
			}
			else
			{
				str = pHSL->GetTopScore().GetName();
				if( str.empty() )
					str = "????";
			}
			break;
		case SONG_MACHINE_HIGH_SCORE:
		case SONG_PROFILE_HIGH_SCORE:
			// Don't show or save machine high scores for edits loaded from a player profile.
			if( bIsPlayerEdit )
				str = NOT_AVAILABLE;
			else
				str = PercentageDisplay::FormatPercentScore( val );
			break;
		case SONG_NUM_STEPS:
		case SONG_JUMPS:
		case SONG_HOLDS:
		case SONG_ROLLS:
		case SONG_MINES:
		case SONG_HANDS:
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
	/* Don't update text that doesn't apply to the current mode.  It's still tweening off. */
	FOREACH_PaneContents( i )
		SetContent( i );
}

// lua start
#include "LuaBinding.h"

class LunaPaneDisplay: public Luna<PaneDisplay>
{
public:
	static int SetFromGameState( T* p, lua_State *L )	{ p->SetFromGameState(); return 0; }

	LunaPaneDisplay()
	{
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
