#include "global.h"
#include "DifficultyList.h"
#include "GameState.h"
#include "Song.h"
#include "Steps.h"
#include "Style.h"
#include "StepsDisplay.h"
#include "StepsUtil.h"
#include "CommonMetrics.h"
#include "SongUtil.h"
#include "XmlFile.h"

#include <cstddef>
#include <vector>


/** @brief Specifies the max number of charts available for a song.
 *
 * This includes autogenned charts. */
inline constexpr int MAX_METERS = (Enum::to_integral(NUM_Difficulty) * Enum::to_integral(NUM_StepsType)) + MAX_EDITS_PER_SONG;

REGISTER_ACTOR_CLASS( StepsDisplayList );

StepsDisplayList::StepsDisplayList()
{
	m_bShown = true;

	FOREACH_ENUM( PlayerNumber, pn )
	{
		SubscribeToMessage( (MessageID)(Message_CurrentStepsP1Changed+Enum::to_integral(pn)) );
		SubscribeToMessage( (MessageID)(Message_CurrentTrailP1Changed+Enum::to_integral(pn)) );
	}
}

StepsDisplayList::~StepsDisplayList()
{
}

void StepsDisplayList::LoadFromNode( const XNode* pNode )
{
	ActorFrame::LoadFromNode( pNode );

	if(m_sName.empty())
	{
		LuaHelpers::ReportScriptError("StepsDisplayList must have a Name");
		return;
	}

	ITEMS_SPACING_Y.Load( m_sName, "ItemsSpacingY" );
	NUM_SHOWN_ITEMS.Load( m_sName, "NumShownItems" );
	CAPITALIZE_DIFFICULTY_NAMES.Load( m_sName, "CapitalizeDifficultyNames" );
	MOVE_COMMAND.Load( m_sName, "MoveCommand" );

	m_Lines.resize( MAX_METERS );
	m_CurSong = nullptr;

	FOREACH_ENUM( PlayerNumber, pn )
	{
		const XNode *pChild = pNode->GetChild( ssprintf("CursorP%i",pn+1) );
		if( pChild == nullptr )
		{
			LuaHelpers::ReportScriptErrorFmt("%s: StepsDisplayList: missing the node \"CursorP%d\"", ActorUtil::GetWhere(pNode).c_str(), pn+1);
		}
		else
		{
			m_Cursors[pn].LoadActorFromNode( pChild, this );
		}

		/* Hack: we need to tween cursors both up to down (cursor motion) and visible to
		 * invisible (fading).  Cursor motion needs to stoptweening, so multiple motions
		 * don't queue and look unresponsive.  However, that stoptweening interrupts fading,
		 * resulting in the cursor remaining invisible or partially invisible.  So, do them
		 * in separate tweening stacks.  This means the Cursor command can't change diffuse
		 * colors; I think we do need a diffuse color stack ... */
		pChild = pNode->GetChild( ssprintf("CursorP%iFrame",pn+1) );
		if( pChild == nullptr )
		{
			LuaHelpers::ReportScriptErrorFmt("%s: StepsDisplayList: missing the node \"CursorP%dFrame\"", ActorUtil::GetWhere(pNode).c_str(), pn+1);
		}
		else
		{
			m_CursorFrames[pn].LoadFromNode( pChild );
			m_CursorFrames[pn].AddChild( m_Cursors[pn] );
			this->AddChild( &m_CursorFrames[pn] );
		}
	}

	for( unsigned m = 0; m < m_Lines.size(); ++m )
	{
		// todo: Use Row1, Row2 for names? also m_sName+"Row" -aj
		m_Lines[m].m_Meter.SetName( "Row" );
		m_Lines[m].m_Meter.Load( "StepsDisplayListRow", nullptr );
		this->AddChild( &m_Lines[m].m_Meter );
	}

	UpdatePositions();
	PositionItems();
}

int StepsDisplayList::GetCurrentRowIndex( PlayerNumber pn ) const
{
	Difficulty ClosestDifficulty = GAMESTATE->GetClosestShownDifficulty(pn);

	for( unsigned i=0; i<m_Rows.size(); i++ )
	{
		const Row &row = m_Rows[i];

		if( GAMESTATE->m_pCurSteps[pn] == nullptr )
		{
			if( row.m_dc == ClosestDifficulty )
				return i;
		}
		else
		{
			if( GAMESTATE->m_pCurSteps[pn].Get() == row.m_Steps )
				return i;
		}
	}

	return 0;
}

// Update m_fY and m_bHidden[].
void StepsDisplayList::UpdatePositions()
{
	int iCurrentRow[NUM_PLAYERS];
	FOREACH_HumanPlayer( p )
		iCurrentRow[p] = GetCurrentRowIndex( p );

	const int total = NUM_SHOWN_ITEMS;
	const int halfsize = total / 2;

	int first_start, first_end, second_start, second_end;

	// Choices for each player. If only one player is active, it's the same for both.
	int P1Choice = GAMESTATE->IsHumanPlayer(PLAYER_1)? iCurrentRow[PLAYER_1]: GAMESTATE->IsHumanPlayer(PLAYER_2)? iCurrentRow[PLAYER_2]: 0;
	int P2Choice = GAMESTATE->IsHumanPlayer(PLAYER_2)? iCurrentRow[PLAYER_2]: GAMESTATE->IsHumanPlayer(PLAYER_1)? iCurrentRow[PLAYER_1]: 0;

	std::vector<Row> &Rows = m_Rows;

	const bool BothPlayersActivated = GAMESTATE->IsHumanPlayer(PLAYER_1) && GAMESTATE->IsHumanPlayer(PLAYER_2);
	if( !BothPlayersActivated )
	{
		// Simply center the cursor.
		first_start = std::max( P1Choice - halfsize, 0 );
		first_end = first_start + total;
		second_start = second_end = first_end;
	}
	else
	{
		// First half:
		const int earliest = std::min( P1Choice, P2Choice );
		first_start = std::max( earliest - halfsize/2, 0 );
		first_end = first_start + halfsize;

		// Second half:
		const int latest = std::max( P1Choice, P2Choice );

		second_start = std::max( latest - halfsize/2, 0 );

		// Don't overlap.
		second_start = std::max( second_start, first_end );

		second_end = second_start + halfsize;
	}

	first_end = std::min( first_end, (int) Rows.size() );
	second_end = std::min( second_end, (int) Rows.size() );

	/* If less than total (and Rows.size()) are displayed, fill in the empty
	 * space intelligently. */
	for(;;)
	{
		const int sum = (first_end - first_start) + (second_end - second_start);
		if( sum >= (int) Rows.size() || sum >= total)
			break; // nothing more to display, or no room

		/* First priority: expand the top of the second half until it meets
		 * the first half. */
		if( second_start > first_end )
			second_start--;
		// Otherwise, expand either end.
		else if( first_start > 0 )
			first_start--;
		else if( second_end < (int) Rows.size() )
			second_end++;
		else
			FAIL_M("Do we have room to grow, or don't we?");
	}

	int pos = 0;
	for( int i=0; i<(int) Rows.size(); i++ ) // foreach row
	{
		float ItemPosition;
		if( i < first_start )
			ItemPosition = -0.5f;
		else if( i < first_end )
			ItemPosition = (float) pos++;
		else if( i < second_start )
			ItemPosition = halfsize - 0.5f;
		else if( i < second_end )
			ItemPosition = (float) pos++;
		else
			ItemPosition = (float) total - 0.5f;

		Row &row = Rows[i];

		float fY = ITEMS_SPACING_Y*ItemPosition;
		row.m_fY = fY;
		row.m_bHidden = i < first_start ||
							(i >= first_end && i < second_start) ||
							i >= second_end;
	}
}


void StepsDisplayList::PositionItems()
{
	for( std::size_t i = 0; i < MAX_METERS; ++i )
	{
		bool bUnused = ( i >= m_Rows.size() );
		m_Lines[i].m_Meter.SetVisible( !bUnused );
	}

	for( std::size_t m = 0; m < m_Rows.size(); ++m )
	{
		Row &row = m_Rows[m];
		bool bHidden = row.m_bHidden;
		if( !m_bShown )
			bHidden = true;

		const float fDiffuseAlpha = bHidden? 0.0f:1.0f;
		if( m_Lines[m].m_Meter.GetDestY() != row.m_fY ||
			m_Lines[m].m_Meter.DestTweenState().diffuse[0][3] != fDiffuseAlpha )
		{
			m_Lines[m].m_Meter.RunCommands( MOVE_COMMAND.GetValue() );
			m_Lines[m].m_Meter.RunCommandsOnChildren( MOVE_COMMAND.GetValue() );
		}

		m_Lines[m].m_Meter.SetY( row.m_fY );
	}

	for( std::size_t m=0; m < MAX_METERS; ++m )
	{
		bool bHidden = true;
		if( m_bShown && m < m_Rows.size() )
			bHidden = m_Rows[m].m_bHidden;

		float fDiffuseAlpha = bHidden?0.0f:1.0f;

		m_Lines[m].m_Meter.SetDiffuseAlpha( fDiffuseAlpha );
	}


	FOREACH_HumanPlayer( pn )
	{
		int iCurrentRow = GetCurrentRowIndex( pn );

		float fY = 0;
		if( iCurrentRow < (int) m_Rows.size() )
			fY = m_Rows[iCurrentRow].m_fY;

		m_CursorFrames[pn].PlayCommand( "Change" );
		m_CursorFrames[pn].SetY( fY );
	}
}

void StepsDisplayList::SetFromGameState()
{
	const Song *pSong = GAMESTATE->m_pCurSong;
	unsigned i = 0;

	if( pSong == nullptr )
	{
		// FIXME: This clamps to between the min and the max difficulty, but
		// it really should round to the nearest difficulty that's in
		// DIFFICULTIES_TO_SHOW.
		const std::vector<Difficulty>& difficulties = CommonMetrics::DIFFICULTIES_TO_SHOW.GetValue();
		m_Rows.resize( difficulties.size() );
		for (Difficulty const &d : difficulties)
		{
			m_Rows[i].m_dc = d;
			m_Lines[i].m_Meter.SetFromStepsTypeAndMeterAndDifficultyAndCourseType( GAMESTATE->GetCurrentStyle(PLAYER_INVALID)->m_StepsType, 0, d, CourseType_Invalid );
			++i;
		}
	}
	else
	{
		std::vector<Steps*>	vpSteps;
		SongUtil::GetPlayableSteps( pSong, vpSteps );
		// Should match the sort in ScreenSelectMusic::AfterMusicChange.

		m_Rows.resize( vpSteps.size() );
		for (Steps const * s: vpSteps)
		{
			//LOG->Trace(ssprintf("setting steps for row %i",i));
			m_Rows[i].m_Steps = s;
			m_Lines[i].m_Meter.SetFromSteps( s );
			++i;
		}
	}

	while( i < MAX_METERS )
		m_Lines[i++].m_Meter.Unset();

	UpdatePositions();
	PositionItems();

	for( std::size_t m = 0; m < MAX_METERS; ++m )
		m_Lines[m].m_Meter.FinishTweening();
}

void StepsDisplayList::HideRows()
{
	for( unsigned m = 0; m < m_Rows.size(); ++m )
	{
		Line &l = m_Lines[m];

		l.m_Meter.FinishTweening();
		l.m_Meter.SetDiffuseAlpha(0);
	}
}

void StepsDisplayList::TweenOnScreen()
{
	FOREACH_HumanPlayer( pn )
		ON_COMMAND( m_Cursors[pn] );

	for( std::size_t m = 0; m < MAX_METERS; ++m )
		ON_COMMAND( m_Lines[m].m_Meter );

	this->SetHibernate( 0.5f );
	m_bShown = true;
	for( unsigned m = 0; m < m_Rows.size(); ++m )
	{
		Line &l = m_Lines[m];

		l.m_Meter.FinishTweening();
	}

	HideRows();
	PositionItems();

	FOREACH_HumanPlayer( pn )
		COMMAND( m_Cursors[pn], "TweenOn" );
}

void StepsDisplayList::TweenOffScreen()
{

}

void StepsDisplayList::Show()
{
	m_bShown = true;

	SetFromGameState();

	HideRows();
	PositionItems();

	FOREACH_HumanPlayer( pn )
		COMMAND( m_Cursors[pn], "Show" );
}

void StepsDisplayList::Hide()
{
	m_bShown = false;
	PositionItems();

	FOREACH_HumanPlayer( pn )
		COMMAND( m_Cursors[pn], "Hide" );
}

void StepsDisplayList::HandleMessage( const Message &msg )
{
	FOREACH_ENUM( PlayerNumber, pn )
	{
		if( msg.GetName() == MessageIDToString((MessageID)(Message_CurrentStepsP1Changed+Enum::to_integral(pn)))  ||
			msg.GetName() == MessageIDToString((MessageID)(Message_CurrentTrailP1Changed+Enum::to_integral(pn))) )
		SetFromGameState();
	}

	ActorFrame::HandleMessage(msg);
}


// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the StepsDisplayList. */
class LunaStepsDisplayList: public Luna<StepsDisplayList>
{
public:
	static int setfromgamestate( T* p, lua_State *L )		{ p->SetFromGameState(); COMMON_RETURN_SELF; }

	LunaStepsDisplayList()
	{
		ADD_METHOD( setfromgamestate );
	}
};

LUA_REGISTER_DERIVED_CLASS( StepsDisplayList, ActorFrame )
// lua end

/*
 * (c) 2003-2004 Glenn Maynard
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
