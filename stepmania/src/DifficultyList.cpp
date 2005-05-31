#include "global.h"
#include "DifficultyList.h"
#include "GameState.h"
#include "song.h"
#include "Steps.h"
#include "Style.h"
#include "DifficultyMeter.h"
#include "RageLog.h"
#include "BitmapText.h"
#include "SongManager.h"
#include "ThemeManager.h"
#include "StepsUtil.h"
#include "CommonMetrics.h"
#include "Command.h"
#include "Foreach.h"

#define MAX_METERS NUM_DIFFICULTIES + MAX_EDITS_PER_SONG

DifficultyList::DifficultyList()
{
	m_bShown = true;
}

DifficultyList::~DifficultyList()
{
}

void DifficultyList::SetName( const CString &sName )
{
	ActorFrame::SetName( sName );

	ITEMS_SPACING_Y.Load( m_sName, "ItemsSpacingY" );
	NUM_SHOWN_ITEMS.Load( m_sName, "NumShownItems" );
	CAPITALIZE_DIFFICULTY_NAMES.Load( m_sName, "CapitalizeDifficultyNames" );
	MOVE_COMMAND.Load( m_sName, "MoveCommand" );
}

void DifficultyList::Load()
{
	m_Lines.resize( MAX_METERS );
	m_CurSong = NULL;

	FOREACH_HumanPlayer( pn )
	{
		m_Cursors[pn].Load( THEME->GetPathG(m_sName,ssprintf("cursor p%i",pn+1)) );
		m_Cursors[pn]->SetName( ssprintf("CursorP%i",pn+1) );

		/* Hack: we need to tween cursors both up to down (cursor motion) and visible to
		 * invisible (fading).  Cursor motion needs to stoptweening, so multiple motions
		 * don't queue and look unresponsive.  However, that stpotweening interrupts fading,
		 * resulting in the cursor remaining invisible or partially invisible.  So, do them
		 * in separate tweening stacks.  This means the Cursor command can't change diffuse
		 * colors; I think we do need a diffuse color stack ... */
		m_CursorFrames[pn].SetName( ssprintf("CursorP%i",pn+1) );
		ActorUtil::LoadCommand( m_CursorFrames[pn], m_sName,  "Change" );
		m_CursorFrames[pn].AddChild( m_Cursors[pn] );
		this->AddChild( &m_CursorFrames[pn] );
	}

	for( unsigned m = 0; m < m_Lines.size(); ++m )
	{
		m_Lines[m].m_Meter.SetName( "Row" );
		m_Lines[m].m_Meter.Load( "DifficultyListRow" );
		this->AddChild( &m_Lines[m].m_Meter );
	}

	FOREACH_HumanPlayer( pn )
		ON_COMMAND( m_Cursors[pn] );

	for( int m = 0; m < MAX_METERS; ++m )
		ON_COMMAND( m_Lines[m].m_Meter );

	UpdatePositions();
	PositionItems();
}

int DifficultyList::GetCurrentRowIndex( PlayerNumber pn ) const
{
	for( unsigned i=0; i<m_Rows.size(); i++ )
	{
		const Row &row = m_Rows[i];

		if( GAMESTATE->m_pCurSteps[pn] == NULL )
		{
			if( row.m_dc == GAMESTATE->m_PreferredDifficulty[pn] )
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

/* Update m_fY and m_bHidden[]. */
void DifficultyList::UpdatePositions()
{
	int iCurrentRow[NUM_PLAYERS];
	FOREACH_HumanPlayer( p )
		iCurrentRow[p] = GetCurrentRowIndex( p );

	const int total = NUM_SHOWN_ITEMS;
	const int halfsize = total / 2;

	int first_start, first_end, second_start, second_end;

	/* Choices for each player.  If only one player is active, it's the same for both. */
	int P1Choice = GAMESTATE->IsHumanPlayer(PLAYER_1)? iCurrentRow[PLAYER_1]: iCurrentRow[PLAYER_2];
	int P2Choice = GAMESTATE->IsHumanPlayer(PLAYER_2)? iCurrentRow[PLAYER_2]: iCurrentRow[PLAYER_1];

	vector<Row> &Rows = m_Rows;

	const bool BothPlayersActivated = GAMESTATE->IsHumanPlayer(PLAYER_1) && GAMESTATE->IsHumanPlayer(PLAYER_2);
	if( !BothPlayersActivated )
	{
		/* Simply center the cursor. */
		first_start = max( P1Choice - halfsize, 0 );
		first_end = first_start + total;
		second_start = second_end = first_end;
	}
	else
	{
		/* First half: */
		const int earliest = min( P1Choice, P2Choice );
		first_start = max( earliest - halfsize/2, 0 );
		first_end = first_start + halfsize;

		/* Second half: */
		const int latest = max( P1Choice, P2Choice );

		second_start = max( latest - halfsize/2, 0 );

		/* Don't overlap. */
		second_start = max( second_start, first_end );

		second_end = second_start + halfsize;
	}

	first_end = min( first_end, (int) Rows.size() );
	second_end = min( second_end, (int) Rows.size() );

	/* If less than total (and Rows.size()) are displayed, fill in the empty
	 * space intelligently. */
	while(1)
	{
		const int sum = (first_end - first_start) + (second_end - second_start);
		if( sum >= (int) Rows.size() || sum >= total)
			break; /* nothing more to display, or no room */

		/* First priority: expand the top of the second half until it meets
		 * the first half. */
		if( second_start > first_end )
			second_start--;
		/* Otherwise, expand either end. */
		else if( first_start > 0 )
			first_start--;
		else if( second_end < (int) Rows.size() )
			second_end++;
		else
			ASSERT(0); /* do we have room to grow or don't we? */
	}

	int pos = 0;
	for( int i=0; i<(int) Rows.size(); i++ )		// foreach row
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


void DifficultyList::PositionItems()
{
	for( int i = 0; i < MAX_METERS; ++i )
	{
		bool bUnused = ( i >= (int)m_Rows.size() );
		m_Lines[i].m_Meter.SetHidden( bUnused );
	}

	for( int m = 0; m < (int)m_Rows.size(); ++m )
	{
		Row &row = m_Rows[m];
		bool bHidden = row.m_bHidden;
		if( !m_bShown )
			bHidden = true;

		const float fDiffuseAlpha = bHidden? 0.0f:1.0f;
		if( m_Lines[m].m_Meter.GetDestY() != row.m_fY ||
			m_Lines[m].m_Meter.DestTweenState().diffuse[0][3] != fDiffuseAlpha )
		{
			m_Lines[m].m_Meter.RunCommands( MOVE_COMMAND );
			m_Lines[m].m_Meter.RunCommandsOnChildren( MOVE_COMMAND );
		}

		m_Lines[m].m_Meter.SetY( row.m_fY );
	}

	for( int m=0; m < MAX_METERS; ++m )
	{
		bool bHidden = true;
		if( m_bShown && m < (int)m_Rows.size() )
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

void DifficultyList::SetFromGameState()
{
	Song *pSong = GAMESTATE->m_pCurSong;

	const bool bSongChanged = (pSong != m_CurSong);

	/* If the song has changed, update displays: */
	if( bSongChanged )
	{
		m_CurSong = pSong;

		for( int m = 0; m < MAX_METERS; ++m )
		{
			m_Lines[m].m_Meter.Unset();
		}

		m_Rows.clear();

		if( pSong == NULL )
		{
			// FIXME: This clamps to between the min and the max difficulty, but
			// it really should round to the nearest difficulty that's in 
			// DIFFICULTIES_TO_SHOW.
			unsigned i=0;
			FOREACH_CONST( Difficulty, DIFFICULTIES_TO_SHOW.GetValue(), d )
			{
				m_Rows.resize( m_Rows.size()+1 );

				Row &row = m_Rows.back();

				row.m_dc = *d;

				m_Lines[i].m_Meter.SetFromMeterAndDifficulty( 0, *d );

				i++;
			}
		}
		else
		{
			vector<Steps*>	CurSteps;
			pSong->GetSteps( CurSteps, GAMESTATE->GetCurrentStyle()->m_StepsType );

			/* Should match the sort in ScreenSelectMusic::AfterMusicChange. */
			StepsUtil::RemoveLockedSteps( pSong, CurSteps );
			StepsUtil::SortNotesArrayByDifficulty( CurSteps );

			m_Rows.resize( CurSteps.size() );
			for( unsigned i = 0; i < CurSteps.size(); ++i )
			{
				Row &row = m_Rows[i];

				row.m_Steps = CurSteps[i];

				m_Lines[i].m_Meter.SetFromSteps( m_Rows[i].m_Steps );

				row.m_dc = row.m_Steps->GetDifficulty();
			}
		}
	}

	UpdatePositions();
	PositionItems();

	if( bSongChanged )
	{
		for( int m = 0; m < MAX_METERS; ++m )
			m_Lines[m].m_Meter.FinishTweening();
	}
}

void DifficultyList::HideRows()
{
	for( unsigned m = 0; m < m_Rows.size(); ++m )
	{
		Line &l = m_Lines[m];

		l.m_Meter.FinishTweening();
		l.m_Meter.SetDiffuseAlpha(0);
	}
}

void DifficultyList::TweenOnScreen()
{
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

void DifficultyList::TweenOffScreen()
{

}

void DifficultyList::Show()
{
	m_bShown = true;

	SetFromGameState();

	HideRows();
	PositionItems();

	FOREACH_HumanPlayer( pn )
		COMMAND( m_Cursors[pn], "Show" );
}

void DifficultyList::Hide()
{
	m_bShown = false;
	PositionItems();

	FOREACH_HumanPlayer( pn )
		COMMAND( m_Cursors[pn], "Hide" );
}

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
