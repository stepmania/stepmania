#include "global.h"

/*
 * Styles define a set of columns for each player, and information about those
 * columns, like what Instruments are used play those columns and what track
 * to use to populate the column's notes.
 * A "track" is the term used to descibe a particular vertical sting of note
 * in NoteData.
 * A "column" is the term used to describe the vertical string of notes that
 * a player sees on the screen while they're playing.  Column notes are
 * picked from a track, but columns and tracks don't have a 1-to-1
 * correspondance.  For example, dance-versus has 8 columns but only 4 tracks
 * because two players place from the same set of 4 tracks.
 */

#include "Style.h"
#include "GameState.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "InputMapper.h"
#include "NoteData.h"

#include <cfloat>
#include <vector>


bool Style::GetUsesCenteredArrows() const
{
	switch( m_StyleType )
	{
	case StyleType_OnePlayerTwoSides:
	case StyleType_TwoPlayersSharedSides:
		return true;
	default:
		return false;
	}
}

void Style::GetTransformedNoteDataForStyle( PlayerNumber pn, const NoteData& original, NoteData& noteDataOut ) const
{
	ASSERT( pn >=0 && pn <= NUM_PLAYERS );

	int iNewToOriginalTrack[MAX_COLS_PER_PLAYER];
	for( int col=0; col<m_iColsPerPlayer; col++ )
	{
		ColumnInfo colInfo = m_ColumnInfo[pn][col];
		iNewToOriginalTrack[col] = colInfo.track;
	}

	noteDataOut.LoadTransformed( original, m_iColsPerPlayer, iNewToOriginalTrack );
}

void Style::StyleInputToGameInput( int iCol, PlayerNumber pn, std::vector<GameInput>& ret ) const
{
	ASSERT_M( pn < NUM_PLAYERS  &&  iCol < MAX_COLS_PER_PLAYER,
		ssprintf("P%i C%i", pn, iCol) );
	bool bUsingOneSide = m_StyleType != StyleType_OnePlayerTwoSides && m_StyleType != StyleType_TwoPlayersSharedSides;

	FOREACH_ENUM( GameController, gc)
	{
		if( bUsingOneSide && gc != (int) pn )
			continue;

		int iButtonsPerController = INPUTMAPPER->GetInputScheme()->m_iButtonsPerController;
		for( GameButton gb=GAME_BUTTON_NEXT; gb < iButtonsPerController; gb=(GameButton)(gb+1) )
		{
			int iThisInputCol = m_iInputColumn[gc][gb-GAME_BUTTON_NEXT];
			if( iThisInputCol == END_MAPPING )
				break;

			// A style can have multiple game inputs mapped to a single column, so
			// we have to return all the game inputs that are valid.  If only the
			// first is returned, then holds will drop on other inputs that should
			// be valid. -Kyz
			if( iThisInputCol == iCol )
			{
				ret.push_back(GameInput( gc, gb ));
			}
		}
	}
	if(unlikely(ret.empty()))
	{
		FAIL_M( ssprintf("Invalid column number %i for player %i in the style %s", iCol, pn, m_szName) );
	}
};

int Style::GameInputToColumn( const GameInput &GameI ) const
{
	if( GameI.button < GAME_BUTTON_NEXT )
		return Column_Invalid;
	int iColumnIndex = GameI.button - GAME_BUTTON_NEXT;
	if( m_iInputColumn[GameI.controller][iColumnIndex] == NO_MAPPING )
		return Column_Invalid;

	for( int i = 0; i <= iColumnIndex; ++i )
	{
		if( m_iInputColumn[GameI.controller][i] == END_MAPPING )
		{
			return Column_Invalid;
		}
	}

	return m_iInputColumn[GameI.controller][iColumnIndex];
}


void Style::GetMinAndMaxColX( PlayerNumber pn, float& fMixXOut, float& fMaxXOut ) const
{
	ASSERT( pn != PLAYER_INVALID );

	fMixXOut = FLT_MAX;
	fMaxXOut = FLT_MIN;
	for( int i=0; i<m_iColsPerPlayer; i++ )
	{
		fMixXOut = std::min( fMixXOut, m_ColumnInfo[pn][i].fXOffset );
		fMaxXOut = std::max( fMaxXOut, m_ColumnInfo[pn][i].fXOffset );
	}
}

float Style::GetWidth(PlayerNumber pn) const
{
	float left, right;
	GetMinAndMaxColX(pn, left, right);
	// left and right are the center positions of the columns.  The full width
	// needs to be from the edges.
	float width= right - left;
	return width + (width / static_cast<float>(m_iColsPerPlayer-1));
}

RString Style::ColToButtonName( int iCol ) const
{
	const char *pzColumnName = m_ColumnInfo[PLAYER_1][iCol].pzName;
	if( pzColumnName != nullptr )
		return pzColumnName;

	std::vector<GameInput> GI;
	StyleInputToGameInput( iCol, PLAYER_1, GI );
	return INPUTMAPPER->GetInputScheme()->GetGameButtonName(GI[0].button);
}

// Lua bindings
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the Style. */
class LunaStyle: public Luna<Style>
{
public:
	static int GetName( T* p, lua_State *L )		{ LuaHelpers::Push( L, (RString) p->m_szName ); return 1; }
	DEFINE_METHOD( GetStyleType,		m_StyleType )
	DEFINE_METHOD( GetStepsType,		m_StepsType )
	DEFINE_METHOD( ColumnsPerPlayer,	m_iColsPerPlayer )
	static int NeedsZoomOutWith2Players(T* p, lua_State *L)
	{
		// m_bNeedsZoomOutWith2Players was removed in favor of having
		// ScreenGameplay use the style's width and margin values to calculate
		// the zoom.  So this always returns false. -Kyz
		lua_pushboolean(L, false);
		return 1;
	}
	static int GetWidth(T* p, lua_State* L)
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		lua_pushnumber(L, p->GetWidth(pn));
		return 1;
	}
	DEFINE_METHOD( LockedDifficulty,	m_bLockDifficulties )

	static int GetColumnInfo( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		int iCol = IArg(2) - 1;
		if( iCol < 0 || iCol >= p->m_iColsPerPlayer )
		{
			LuaHelpers::ReportScriptErrorFmt( "Style:GetColumnDrawOrder(): column %i out of range( 1 to %i )", iCol+1, p->m_iColsPerPlayer );
			return 0;
		}

		LuaTable ret;
		lua_pushnumber( L, p->m_ColumnInfo[pn][iCol].track+1 );
		ret.Set( L, "Track" );
		lua_pushnumber( L, p->m_ColumnInfo[pn][iCol].fXOffset );
		ret.Set( L,  "XOffset" );
		lua_pushstring( L, p->ColToButtonName(iCol) );
		ret.Set( L, "Name" );

		ret.PushSelf(L);
		return 1;
	}

	static int GetColumnDrawOrder( T* p, lua_State *L )
	{
		int iCol = IArg(1) - 1;
		if( iCol < 0 || iCol >= p->m_iColsPerPlayer*NUM_PLAYERS )
		{
			LuaHelpers::ReportScriptErrorFmt( "Style:GetColumnDrawOrder(): column %i out of range( 1 to %i )", iCol+1, p->m_iColsPerPlayer*NUM_PLAYERS );
			return 0;
		}
		lua_pushnumber( L, p->m_iColumnDrawOrder[iCol]+1 );
		return 1;
	}

	LunaStyle()
	{
		ADD_METHOD( GetName );
		ADD_METHOD( GetStyleType );
		ADD_METHOD( GetStepsType );
		ADD_METHOD( GetColumnInfo );
		ADD_METHOD( GetColumnDrawOrder );
		ADD_METHOD( ColumnsPerPlayer );
		ADD_METHOD( NeedsZoomOutWith2Players );
		ADD_METHOD( GetWidth );
		ADD_METHOD( LockedDifficulty );
	}
};

LUA_REGISTER_CLASS( Style )

/*
 * (c) 2001-2002 Chris Danford
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
