#include "global.h"
#include "DifficultyList.h"
#include "GameState.h"
#include "song.h"
#include "Steps.h"
#include "StyleDef.h"
#include "DifficultyMeter.h"
#include "RageLog.h"
#include "BitmapText.h"
#include "SongManager.h"
#include "ThemeManager.h"
#include "StepsUtil.h"

#define ITEMS_SPACING_Y							THEME->GetMetricF(m_sName,"ItemsSpacingY")
#define DESCRIPTION_MAX_WIDTH					THEME->GetMetricF(m_sName,"DescriptionMaxWidth")
#define NUM_SHOWN_ITEMS							THEME->GetMetricI(m_sName,"NumShownItems")
#define MOVE_COMMAND							THEME->GetMetric (m_sName,"MoveCommand")

DifficultyList::DifficultyList()
{
	m_bShown = true;
	m_Meters = NULL; // defer alloc to Load
}

DifficultyList::~DifficultyList()
{
	delete [] m_Meters;
}

void DifficultyList::Load()
{
	ASSERT( !m_Meters );
	m_Meters = new DifficultyMeter[MAX_METERS];
	m_CurSong = NULL;

	{	
		for( int pn = 0; pn < NUM_PLAYERS; ++pn )
		{
			if( !GAMESTATE->IsHumanPlayer(pn) )
				continue;

			m_Cursors[pn].Load( THEME->GetPathToG(ssprintf("%s cursor p%i",m_sName.c_str(), pn+1)) );
			m_Cursors[pn]->SetName( ssprintf("CursorP%i",pn+1) );

			/* Hack: we need to tween cursors both up to down (cursor motion) and visible to
			 * invisible (fading).  Cursor motion needs to stoptweening, so multiple motions
			 * don't queue and look unresponsive.  However, that stpotweening interrupts fading,
			 * resulting in the cursor remaining invisible or partially invisible.  So, do them
			 * in separate tweening stacks.  This means the Cursor command can't change diffuse
			 * colors; I think we do need a diffuse color stack ... */
			m_CursorFrames[pn].SetName( ssprintf("CursorP%i",pn+1) );
			m_CursorFrames[pn].AddChild( m_Cursors[pn] );
			this->AddChild( &m_CursorFrames[pn] );
		}
	}

	{
		for( int m = 0; m < MAX_METERS; ++m )
		{
			m_Meters[m].SetName( "DifficultySummaryRow", "Row" );
			m_Meters[m].Load();
			this->AddChild( &m_Meters[m] );

			m_Descriptions[m].SetName( "Description" );
			m_Descriptions[m].LoadFromFont( THEME->GetPathToF(ssprintf("%s description",m_sName.c_str())) );
			this->AddChild( &m_Descriptions[m] );

			m_Number[m].SetName( "Number" );
			m_Number[m].LoadFromFont( THEME->GetPathToF(ssprintf("%s number",m_sName.c_str())) );
			this->AddChild( &m_Number[m] );
		}
	}

	{
		for( int pn = 0; pn < NUM_PLAYERS; ++pn )
			if( GAMESTATE->IsHumanPlayer(pn) )
				ON_COMMAND( m_Cursors[pn] );
	}

	{
		for( int m = 0; m < MAX_METERS; ++m )
		{
			ON_COMMAND( m_Meters[m] );
			ON_COMMAND( m_Descriptions[m] );
			ON_COMMAND( m_Number[m] );
		}
	}

	UpdatePositions();
	PositionItems();
}

void DifficultyList::GetCurrentRows( int iCurrentRow[NUM_PLAYERS] ) const
{
	for( int pn = 0;pn < NUM_PLAYERS; ++pn )
	{
		if( !GAMESTATE->IsHumanPlayer(pn) )
			continue;

		for( iCurrentRow[pn] = 0; iCurrentRow[pn] < (int) m_Rows.size(); ++iCurrentRow[pn] )
		{
			if( GAMESTATE->m_pCurNotes[pn] == m_Rows[iCurrentRow[pn]]->m_Steps )
				break;
			if( GAMESTATE->m_pCurNotes[pn] == NULL && 
				m_Rows[iCurrentRow[pn]]->m_Steps->GetDifficulty() == GAMESTATE->m_PreferredDifficulty[pn] )
				break;
		}
	}
}

/* Update m_fY and m_bHidden[]. */
void DifficultyList::UpdatePositions()
{
	int iCurrentRow[NUM_PLAYERS];
	GetCurrentRows( iCurrentRow );

	const int total = NUM_SHOWN_ITEMS;
	const int halfsize = total / 2;

	int first_start, first_end, second_start, second_end;

	/* Choices for each player.  If only one player is active, it's the same for both. */
	int P1Choice = GAMESTATE->IsHumanPlayer(PLAYER_1)? iCurrentRow[PLAYER_1]: iCurrentRow[PLAYER_2];
	int P2Choice = GAMESTATE->IsHumanPlayer(PLAYER_2)? iCurrentRow[PLAYER_2]: iCurrentRow[PLAYER_1];

	vector<Row*> Rows( m_Rows );

	const bool BothPlayersActivated = GAMESTATE->IsHumanPlayer(PLAYER_1) && GAMESTATE->IsHumanPlayer(PLAYER_2);
	if( !BothPlayersActivated )
	{
		/* Simply center the cursor. */
		first_start = max( P1Choice - halfsize, 0 );
		first_end = first_start + total;
		second_start = second_end = first_end;
	} else {
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
			
		Row &row = *Rows[i];

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
		m_Descriptions[i].SetHidden( bUnused );
		m_Meters[i].SetHidden( bUnused );
		m_Number[i].SetHidden( bUnused );
	}

	int m;
	for( m = 0; m < (int)m_Rows.size(); ++m )
	{
		Row &row = *m_Rows[m];
		bool bHidden = row.m_bHidden;
		if( !m_bShown )
			bHidden = true;

		const float DiffuseAlpha = bHidden? 0.0f:1.0f;
		if( m_Number[m].GetDestY() != row.m_fY ||
			m_Number[m].DestTweenState().diffuse[0][3] != DiffuseAlpha )
		{
			m_Descriptions[m].Command( MOVE_COMMAND );
			m_Meters[m].Command( MOVE_COMMAND );
			m_Meters[m].RunCommandOnChildren( MOVE_COMMAND );
			m_Number[m].Command( MOVE_COMMAND );
		}

		m_Descriptions[m].SetY( row.m_fY );
		m_Meters[m].SetY( row.m_fY );
		m_Number[m].SetY( row.m_fY );
	}

	for( m=0; m < MAX_METERS; ++m )
	{
		bool bHidden = true;
		if( m_bShown && m < (int)m_Rows.size() )
			bHidden = m_Rows[m]->m_bHidden;

		const CString cmd = ssprintf( "diffusealpha,%f", bHidden? 0.0f:1.0f );
		m_Descriptions[m].Command( cmd );
		m_Meters[m].RunCommandOnChildren( cmd );
		m_Number[m].Command( cmd );
	}

	int iCurrentRow[NUM_PLAYERS];
	GetCurrentRows( iCurrentRow );

	for( int pn = 0;pn < NUM_PLAYERS; ++pn )
	{
		if( !GAMESTATE->IsHumanPlayer(pn) )
			continue;

		float fY = 0;
		if( iCurrentRow[pn] < (int) m_Rows.size() )
			fY = m_Rows[iCurrentRow[pn]]->m_fY;

		COMMAND( m_CursorFrames[pn], "Change" );
		m_CursorFrames[pn].SetY( fY );
	}
}

DifficultyList::Row::Row()
{
	m_Steps = NULL;
}

void DifficultyList::SetFromGameState()
{
	Song *song = GAMESTATE->m_pCurSong;

	const bool bSongChanged = (song != m_CurSong);

	/* If the song has changed, update displays: */
	if( bSongChanged )
	{
		m_CurSong = song;
		if( !song )
			return;

		for( int m = 0; m < MAX_METERS; ++m )
		{
			m_Meters[m].Unset();
			m_Descriptions[m].SetText( "" );
			m_Number[m].SetText( "" );
		}

		vector<Steps*>	CurSteps;
		song->GetSteps( CurSteps, GAMESTATE->GetCurrentStyleDef()->m_StepsType );

		/* Should match the sort in ScreenSelectMusic::AfterMusicChange. */
		StepsUtil::SortNotesArrayByDifficulty( CurSteps );

		unsigned i;
		for( i = 0; i < m_Rows.size(); ++i )
			delete m_Rows[i];
		m_Rows.clear();

		for( i = 0; i < CurSteps.size(); ++i )
		{
			m_Rows.push_back( new Row() );
			Row &row = *m_Rows[i];

			row.m_Steps = CurSteps[i];

			m_Meters[i].SetFromNotes( m_Rows[i]->m_Steps );

			Difficulty dc = row.m_Steps->GetDifficulty();
			
			CString s;
			if( row.m_Steps->GetDifficulty() == DIFFICULTY_EDIT )
				s = row.m_Steps->GetDescription();
			else
				s = DifficultyToThemedString(dc);
			m_Descriptions[i].SetMaxWidth( DESCRIPTION_MAX_WIDTH );
			m_Descriptions[i].SetText( s );
			/* Don't mess with alpha; it might be fading on. */
			m_Descriptions[i].SetDiffuseColor( SONGMAN->GetDifficultyColor(dc) );
			
			m_Number[i].SetZoomX(1);
			m_Number[i].SetDiffuseColor( SONGMAN->GetDifficultyColor(dc) );
			m_Number[i].SetText( ssprintf("%d",row.m_Steps->GetMeter()) );
		}
	}

	UpdatePositions();
	PositionItems();

	if( bSongChanged )
	{
		for( int m = 0; m < MAX_METERS; ++m )
		{
			m_Meters[m].FinishTweening();
			m_Descriptions[m].FinishTweening();
			m_Number[m].FinishTweening();
		}
	}
}

void DifficultyList::HideRows()
{
	for( unsigned m = 0; m < m_Rows.size(); ++m )
	{
		m_Descriptions[m].Command( "finishtweening;diffusealpha,0" );
		m_Meters[m].RunCommandOnChildren( "finishtweening;diffusealpha,0" );
		m_Number[m].Command( "finishtweening;diffusealpha,0" );
	}
}

void DifficultyList::TweenOnScreen()
{
	this->SetHibernate( 0.5f );
	m_bShown = true;
	for( unsigned m = 0; m < m_Rows.size(); ++m )
	{
		m_Descriptions[m].Command( "finishtweening" );
		m_Meters[m].RunCommandOnChildren( "finishtweening" );
		m_Number[m].Command( "finishtweening" );
	}

//	PositionItems();
	HideRows();
	
	PositionItems();

	for( int pn = 0; pn < NUM_PLAYERS; ++pn )
	{
		if( !GAMESTATE->IsHumanPlayer(pn) )
			continue;
		COMMAND( m_Cursors[pn], "TweenOn" );
	}
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

	for( int pn = 0; pn < NUM_PLAYERS; ++pn )
	{
		if( !GAMESTATE->IsHumanPlayer(pn) )
			continue;
		COMMAND( m_Cursors[pn], "Show" );
	}
}

void DifficultyList::Hide()
{
	m_bShown = false;
	PositionItems();

	for( int pn = 0; pn < NUM_PLAYERS; ++pn )
	{
		if( !GAMESTATE->IsHumanPlayer(pn) )
			continue;
		COMMAND( m_Cursors[pn], "Hide" );
	}
}

/*
 * Copyright (c) 2003-2004 by the person(s) listed below.  All rights reserved.
 *	Glenn Maynard
 */
