#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: EditMenu

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "EditMenu.h"
#include "RageLog.h"
#include "SongManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "GameManager.h"
#include "Notes.h"
#include "song.h"

//
// Defines specific to EditMenu
//
#define ARROWS_X( i )			THEME->GetMetricF("EditMenu",ssprintf("Arrows%dX",i+1))
#define SONG_BANNER_X			THEME->GetMetricF("EditMenu","SongBannerX")
#define SONG_BANNER_Y			THEME->GetMetricF("EditMenu","SongBannerY")
#define SONG_BANNER_WIDTH		THEME->GetMetricF("EditMenu","SongBannerWidth")
#define SONG_BANNER_HEIGHT		THEME->GetMetricF("EditMenu","SongBannerHeight")
#define SONG_TEXT_BANNER_X		THEME->GetMetricF("EditMenu","SongTextBannerX")
#define SONG_TEXT_BANNER_Y		THEME->GetMetricF("EditMenu","SongTextBannerY")
#define GROUP_BANNER_X			THEME->GetMetricF("EditMenu","GroupBannerX")
#define GROUP_BANNER_Y			THEME->GetMetricF("EditMenu","GroupBannerY")
#define GROUP_BANNER_WIDTH		THEME->GetMetricF("EditMenu","GroupBannerWidth")
#define GROUP_BANNER_HEIGHT		THEME->GetMetricF("EditMenu","GroupBannerHeight")
#define METER_X					THEME->GetMetricF("EditMenu","MeterX")
#define METER_Y					THEME->GetMetricF("EditMenu","MeterY")
#define SOURCE_METER_X			THEME->GetMetricF("EditMenu","SourceMeterX")
#define SOURCE_METER_Y			THEME->GetMetricF("EditMenu","SourceMeterY")
#define ROW_LABELS_X			THEME->GetMetricF("EditMenu","RowLabelsX")
#define ROW_VALUE_X( i )		THEME->GetMetricF("EditMenu",ssprintf("RowValue%dX",i+1))
#define ROW_Y( i )				THEME->GetMetricF("EditMenu",ssprintf("Row%dY",i+1))


EditMenu::EditMenu()
{
	LOG->Trace( "ScreenEditMenu::ScreenEditMenu()" );

	int i;

	for( i=0; i<2; i++ )
	{
		m_sprArrows[i].Load( THEME->GetPathToG(ssprintf("EditMenu %s",(i==0?"left":"right"))) );
		m_sprArrows[i].SetX( ARROWS_X(i) );
		this->AddChild( &m_sprArrows[i] );
	}

	m_SelectedRow = (Row)0;

	ZERO( m_iSelection );

	
	// start out on easy, not beginner
	m_iSelection[ROW_DIFFICULTY] = DIFFICULTY_EASY;
	m_iSelection[ROW_SOURCE_DIFFICULTY] = DIFFICULTY_EASY;



	for( i=0; i<NUM_ROWS; i++ )
	{
		m_textLabel[i].LoadFromFont( THEME->GetPathToF("Common title") );
		m_textLabel[i].SetXY( ROW_LABELS_X, ROW_Y(i) );
		m_textLabel[i].SetText( RowToString((Row)i) );
		m_textLabel[i].SetZoom( 0.8f );
		m_textLabel[i].SetHorizAlign( Actor::align_left );
		this->AddChild( &m_textLabel[i] );

		m_textValue[i].LoadFromFont( THEME->GetPathToF("Common normal") );
		m_textValue[i].SetXY( ROW_VALUE_X(i), ROW_Y(i) );
		m_textValue[i].SetText( "blah" );
		m_textValue[i].SetZoom( 0.8f );
		this->AddChild( &m_textValue[i] );
	}

	m_GroupBanner.SetXY( GROUP_BANNER_X, GROUP_BANNER_Y );
	this->AddChild( &m_GroupBanner );

	m_SongBanner.SetXY( SONG_BANNER_X, SONG_BANNER_Y );
	this->AddChild( &m_SongBanner );

	m_SongTextBanner.SetXY( SONG_TEXT_BANNER_X, SONG_TEXT_BANNER_Y );
	this->AddChild( &m_SongTextBanner );
	
	m_Meter.SetXY( METER_X, METER_Y );
	this->AddChild( &m_Meter );
	
	m_SourceMeter.SetXY( SOURCE_METER_X, SOURCE_METER_Y );
	this->AddChild( &m_SourceMeter );
	

	m_soundChangeRow.Load( THEME->GetPathToS("EditMenu row") );
	m_soundChangeValue.Load( THEME->GetPathToS("EditMenu value") );


	// fill in data structures
	SONGMAN->GetGroupNames( m_sGroups );
	GAMEMAN->GetNotesTypesForGame( GAMESTATE->m_CurGame, m_NotesTypes );
	
	ChangeToRow( (Row)0 );
	OnRowValueChanged( (Row)0 );

	// Select the current song if any
	if( GAMESTATE->m_pCurSong )
	{
		unsigned i;

		for( i=0; i<m_sGroups.size(); i++ )
			if( GAMESTATE->m_pCurSong->m_sGroupName == m_sGroups[i] )
				m_iSelection[ROW_GROUP] = i;
		OnRowValueChanged( ROW_GROUP );

		for( i=0; i<m_pSongs.size(); i++ )
			if( GAMESTATE->m_pCurSong == m_pSongs[i] )
				m_iSelection[ROW_SONG] = i;
		OnRowValueChanged( ROW_SONG );

		// Select the current NotesType and difficulty if any
		if( GAMESTATE->m_pCurNotes )
		{
			for( i=0; i<m_NotesTypes.size(); i++ )
				if( m_NotesTypes[i] == GAMESTATE->m_pCurNotes[PLAYER_1]->m_NotesType )
				{
					m_iSelection[ROW_NOTES_TYPE] = i;
					OnRowValueChanged( ROW_NOTES_TYPE );
				}

			m_iSelection[ROW_DIFFICULTY] = GAMESTATE->m_pCurNotes[PLAYER_1]->GetDifficulty();
			OnRowValueChanged( ROW_DIFFICULTY );
		}

	}
}

EditMenu::~EditMenu()
{

}

void EditMenu::RefreshNotes()
{
	OnRowValueChanged( ROW_SONG );
}

void EditMenu::DrawPrimitives()
{
	ActorFrame::DrawPrimitives();
}

bool EditMenu::CanGoUp()
{
	return m_SelectedRow != 0;
}

bool EditMenu::CanGoDown()
{
	return m_SelectedRow != NUM_ROWS-1;
}

bool EditMenu::CanGoLeft()
{
	return m_iSelection[m_SelectedRow] != 0;
}

bool EditMenu::CanGoRight()
{
	int num_values[NUM_ROWS] = 
	{
		m_sGroups.size(),
		m_pSongs.size(),
		m_NotesTypes.size(),
		NUM_DIFFICULTIES,
		m_NotesTypes.size(),
		NUM_DIFFICULTIES,
		m_Actions.size()
	};

	return m_iSelection[m_SelectedRow] != (num_values[m_SelectedRow]-1);
}

void EditMenu::Up()
{
	if( CanGoUp() )
	{
		if( GetSelectedNotes() && m_SelectedRow==ROW_ACTION )
			ChangeToRow( ROW_DIFFICULTY );
		else
			ChangeToRow( Row(m_SelectedRow-1) );
		m_soundChangeRow.PlayRandom();
	}
}

void EditMenu::Down()
{
	if( CanGoDown() )
	{
		if( GetSelectedNotes() && m_SelectedRow==ROW_DIFFICULTY )
			ChangeToRow( ROW_ACTION );
		else
			ChangeToRow( Row(m_SelectedRow+1) );
		m_soundChangeRow.PlayRandom();
	}
}

void EditMenu::Left()
{
	if( CanGoLeft() )
	{
		m_iSelection[m_SelectedRow]--;
		OnRowValueChanged( m_SelectedRow );
		m_soundChangeValue.PlayRandom();
	}
}

void EditMenu::Right()
{
	if( CanGoRight() )
	{
		m_iSelection[m_SelectedRow]++;
		OnRowValueChanged( m_SelectedRow );
		m_soundChangeValue.PlayRandom();
	}
}


void EditMenu::ChangeToRow( Row newRow )
{
	m_SelectedRow = newRow;

	for( int i=0; i<2; i++ )
		m_sprArrows[i].SetY( ROW_Y(newRow) );
	m_sprArrows[0].SetDiffuse( CanGoLeft()?RageColor(1,1,1,1):RageColor(0.2f,0.2f,0.2f,1) );
	m_sprArrows[1].SetDiffuse( CanGoRight()?RageColor(1,1,1,1):RageColor(0.2f,0.2f,0.2f,1) );
	m_sprArrows[0].EnableAnimation( CanGoLeft() );
	m_sprArrows[1].EnableAnimation( CanGoRight() );
}

void EditMenu::OnRowValueChanged( Row row )
{
	m_sprArrows[0].SetDiffuse( CanGoLeft()?RageColor(1,1,1,1):RageColor(0.2f,0.2f,0.2f,1) );
	m_sprArrows[1].SetDiffuse( CanGoRight()?RageColor(1,1,1,1):RageColor(0.2f,0.2f,0.2f,1) );
	m_sprArrows[0].EnableAnimation( CanGoLeft() );
	m_sprArrows[1].EnableAnimation( CanGoRight() );

	switch( row )
	{
	case ROW_GROUP:
		m_textValue[ROW_GROUP].SetText( SONGMAN->ShortenGroupName(GetSelectedGroup()) );
		m_GroupBanner.LoadFromGroup( GetSelectedGroup() );
		m_GroupBanner.ScaleToClipped( GROUP_BANNER_WIDTH, GROUP_BANNER_HEIGHT );
		m_pSongs.clear();
		SONGMAN->GetSongs( m_pSongs, GetSelectedGroup() );
		m_iSelection[ROW_SONG] = 0;
		// fall through
	case ROW_SONG:
		m_textValue[ROW_SONG].SetText( "" );
		m_SongBanner.LoadFromSong( GetSelectedSong() );
		m_SongBanner.ScaleToClipped( SONG_BANNER_WIDTH, SONG_BANNER_HEIGHT );
		m_SongTextBanner.LoadFromSong( GetSelectedSong() );
		// fall through
	case ROW_NOTES_TYPE:
		m_textValue[ROW_NOTES_TYPE].SetText( GAMEMAN->NotesTypeToString(GetSelectedNotesType()) );
		// fall through
	case ROW_DIFFICULTY:
		m_textValue[ROW_DIFFICULTY].SetText( DifficultyToString(GetSelectedDifficulty()) );
		m_Meter.SetFromNotes( GetSelectedNotes() );
		// fall through
	case ROW_SOURCE_NOTES_TYPE:
		m_textLabel[ROW_SOURCE_NOTES_TYPE].SetDiffuse( GetSelectedNotes()?RageColor(1,1,1,0):RageColor(1,1,1,1) );
		m_textValue[ROW_SOURCE_NOTES_TYPE].SetDiffuse( GetSelectedNotes()?RageColor(1,1,1,0):RageColor(1,1,1,1) );
		m_textValue[ROW_SOURCE_NOTES_TYPE].SetText( GAMEMAN->NotesTypeToString(GetSelectedSourceNotesType()) );
		// fall through
	case ROW_SOURCE_DIFFICULTY:
		m_textLabel[ROW_SOURCE_DIFFICULTY].SetDiffuse( GetSelectedNotes()?RageColor(1,1,1,0):RageColor(1,1,1,1) );
		m_textValue[ROW_SOURCE_DIFFICULTY].SetDiffuse( GetSelectedNotes()?RageColor(1,1,1,0):RageColor(1,1,1,1) );
		m_textValue[ROW_SOURCE_DIFFICULTY].SetText( DifficultyToString(GetSelectedSourceDifficulty()) );
		m_SourceMeter.SetFromNotes( GetSelectedSourceNotes() );
		m_SourceMeter.SetZoomY( GetSelectedNotes()?0.f:1.f );

		m_Actions.clear();
		if( GetSelectedNotes() )
		{
			m_Actions.push_back( ACTION_EDIT );
			m_Actions.push_back( ACTION_DELETE );
		}
		else if( GetSelectedSourceNotes() )
		{
			m_Actions.push_back( ACTION_COPY );
			m_Actions.push_back( ACTION_AUTOGEN );
			m_Actions.push_back( ACTION_BLANK );
		}
		else
		{
			m_Actions.push_back( ACTION_BLANK );
		}
		m_iSelection[ROW_ACTION] = 0;
		// fall through
	case ROW_ACTION:
		m_textValue[ROW_ACTION].SetText( ActionToString(GetSelectedAction()) );
		break;
	default:
		ASSERT(0);	// invalid row
	}
}

Notes* EditMenu::GetSelectedNotes()
{
	return GetSelectedSong()->GetNotes(GetSelectedNotesType(),GetSelectedDifficulty(), false);
}

Notes* EditMenu::GetSelectedSourceNotes()
{
	return GetSelectedSong()->GetNotes(GetSelectedSourceNotesType(),GetSelectedSourceDifficulty(), false);
}
