#include "global.h"
#include "DifficultyList.h"
#include "GameState.h"
#include "song.h"
#include "StyleDef.h"
#include "DifficultyMeter.h"
#include "RageLog.h"
#include "BitmapText.h"
#include "SongManager.h"
#include "ThemeManager.h"

#define ITEMS_SPACING_Y							THEME->GetMetricF(m_sName,"ItemsSpacingY")
#define DESCRIPTION_MAX_WIDTH					THEME->GetMetricF(m_sName,"DescriptionMaxWidth")
#define TWEEN_ON_STAGGER_SECONDS				THEME->GetMetricF(m_sName,"TweenOnStaggerSeconds")

const int RowsOnScreen=6;

DifficultyList::DifficultyList()
{
	m_Meters = NULL; // defer alloc to Load
	m_Frames = NULL;
	m_Descriptions = NULL;
}

DifficultyList::~DifficultyList()
{
	delete [] m_Meters;
	delete [] m_Frames;
	delete [] m_Descriptions;
}

void DifficultyList::Load()
{
	ASSERT( !m_Meters );
	m_Meters = new DifficultyMeter[MAX_METERS];
	m_Frames = new ActorFrame[MAX_METERS];
	m_Descriptions = new BitmapText[MAX_METERS];
	m_CurSong = NULL;

	int pn, m;
	for( pn = 0; pn < NUM_PLAYERS; ++pn )
	{
		if( !GAMESTATE->IsHumanPlayer(pn) )
			continue;

		m_Cursors[pn].Load( THEME->GetPathToG(ssprintf("%s cursor p%i",m_sName.c_str(), pn+1)) );
		m_Cursors[pn]->SetName( ssprintf("CursorP%i",pn+1) );
		ON_COMMAND( m_Cursors[pn] );
		this->AddChild( m_Cursors[pn] );
	}

	for( m = 0; m < MAX_METERS; ++m )
	{
		m_Meters[m].SetName( "DifficultySummaryRow", "Row" );
		m_Meters[m].Load();
		this->AddChild( &m_Meters[m] );

		m_Descriptions[m].SetName( "Description" );
		m_Descriptions[m].LoadFromFont( THEME->GetPathToF(ssprintf("%s description",m_sName.c_str())) );
		this->AddChild( &m_Descriptions[m] );
	}

	PositionItems();

	for( pn = 0; pn < NUM_PLAYERS; ++pn )
		if( GAMESTATE->IsHumanPlayer(pn) )
			ON_COMMAND( m_Cursors[pn] );

	for( m = 0; m < MAX_METERS; ++m )
	{
		ON_COMMAND( m_Meters[m] );
		ON_COMMAND( m_Descriptions[m] );
	}
}

void DifficultyList::PositionItems()
{
	int pos = 0;
	for( int m = 0; m < MAX_METERS; ++m )
	{
		DifficultyMeter &met = m_Meters[m];

		float ItemPosition;
		if( m < RowsOnScreen )
			ItemPosition = (float) pos++;
		else
		{
			met.SetHidden( true );
//			ItemPosition = ItemPosition = (float) pos++;
			continue;
		}
		const float fY = ITEMS_SPACING_Y*ItemPosition;
		LOG->Trace("row %i, pos %f, fY %f",
			m, ItemPosition, fY);

		m_Descriptions[m].SetY( fY );
		met.SetY( fY );
	}

	for( int pn = 0;pn < NUM_PLAYERS; ++pn )
	{
		if( !GAMESTATE->IsHumanPlayer(pn) )
			continue;

		unsigned ItemPosition;
		for( ItemPosition = 0; ItemPosition  < m_CurSteps.size(); ++ItemPosition  )
			if( GAMESTATE->m_pCurNotes[pn] == m_CurSteps[ItemPosition] )
				break;

		if( ItemPosition == m_CurSteps.size() )
			continue;

		const float fY = ITEMS_SPACING_Y*ItemPosition;

		COMMAND( m_Cursors[pn], "Change" );
		m_Cursors[pn]->SetY( fY );
	}
}

void DifficultyList::SetFromGameState()
{
	Song *song = GAMESTATE->m_pCurSong;
	if( !song )
		return;

	/* If the song has changed, update displays: */
	if( song != m_CurSong )
	{
		m_CurSong = song;
		m_CurSteps.clear();
		song->GetSteps( m_CurSteps, GAMESTATE->GetCurrentStyleDef()->m_StepsType );

		/* Should match the sort in ScreenSelectMusic::AfterMusicChange. */
		SortNotesArrayByDifficulty( m_CurSteps );

		for( int m = 0; m < MAX_METERS; ++m )
		{
			DifficultyMeter &met = m_Meters[m];
			if( m >= (int) m_CurSteps.size() )
			{
				met.Unset();
				m_Descriptions[m].SetText( "" );
				continue;
			}

			met.SetFromNotes( m_CurSteps[m] );
			const CString desc = m_CurSteps[m]->GetDescription();
			m_Descriptions[m].SetZoomX(1);
			m_Descriptions[m].SetTextMaxWidth( DESCRIPTION_MAX_WIDTH, SONGMAN->GetDifficultyThemeName(desc) );
			/* Don't mess with alpha; it might be fading on. */
			m_Descriptions[m].SetDiffuseColor( SONGMAN->GetDifficultyColor( m_CurSteps[m]->GetDifficulty() ) );
		}
	}

	PositionItems();
}

void DifficultyList::TweenOnScreen()
{
	for( int m = 0; m < MAX_METERS; ++m )
	{
		m_Descriptions[m].BeginTweening( m * TWEEN_ON_STAGGER_SECONDS );
		COMMAND( m_Descriptions[m], "TweenOn" );
		COMMAND( m_Meters[m], "TweenOn" );
	}

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
	for( int m = 0; m < MAX_METERS; ++m )
	{
		// XXX: m_Meters[m].Show
		COMMAND( m_Descriptions[m], "Show" );
		COMMAND( m_Meters[m], "Show" );
	}

	for( int pn = 0; pn < NUM_PLAYERS; ++pn )
	{
		if( !GAMESTATE->IsHumanPlayer(pn) )
			continue;
		COMMAND( m_Cursors[pn], "Show" );
	}
}

void DifficultyList::Hide()
{
	for( int m = 0; m < MAX_METERS; ++m )
	{
		// XXX: m_Meters[m].Hide
		COMMAND( m_Descriptions[m], "Hide" );
		COMMAND( m_Meters[m], "Hide" );
	}

	for( int pn = 0; pn < NUM_PLAYERS; ++pn )
	{
		if( !GAMESTATE->IsHumanPlayer(pn) )
			continue;
		COMMAND( m_Cursors[pn], "Hide" );
	}
}
