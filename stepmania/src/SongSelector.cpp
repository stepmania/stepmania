#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: SongSelector

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "SongSelector.h"
#include "RageLog.h"
#include "SongManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "GameManager.h"

//
// Defines specific to SongSelector
//
#define ARROWS_X( i )			THEME->GetMetricF("SongSelector",ssprintf("Arrows%dX",i+1))
#define SONG_BANNER_X			THEME->GetMetricF("SongSelector","SongBannerX")
#define SONG_BANNER_Y			THEME->GetMetricF("SongSelector","SongBannerY")
#define SONG_BANNER_WIDTH		THEME->GetMetricF("SongSelector","SongBannerWidth")
#define SONG_BANNER_HEIGHT		THEME->GetMetricF("SongSelector","SongBannerHeight")
#define SONG_TEXT_BANNER_X		THEME->GetMetricF("SongSelector","SongTextBannerX")
#define SONG_TEXT_BANNER_Y		THEME->GetMetricF("SongSelector","SongTextBannerY")
#define GROUP_BANNER_X			THEME->GetMetricF("SongSelector","GroupBannerX")
#define GROUP_BANNER_Y			THEME->GetMetricF("SongSelector","GroupBannerY")
#define GROUP_BANNER_WIDTH		THEME->GetMetricF("SongSelector","GroupBannerWidth")
#define GROUP_BANNER_HEIGHT		THEME->GetMetricF("SongSelector","GroupBannerHeight")
#define METER_X					THEME->GetMetricF("SongSelector","MeterX")
#define METER_Y					THEME->GetMetricF("SongSelector","MeterY")
#define SOURCE_METER_X			THEME->GetMetricF("SongSelector","SourceMeterX")
#define SOURCE_METER_Y			THEME->GetMetricF("SongSelector","SourceMeterY")
#define ROW_LABELS_X			THEME->GetMetricF("SongSelector","RowLabelsX")
#define ROW_VALUE_X( i )		THEME->GetMetricF("SongSelector",ssprintf("RowValue%dX",i+1))
#define ROW_Y( i )				THEME->GetMetricF("SongSelector",ssprintf("Row%dY",i+1))


SongSelector::SongSelector()
{
	LOG->Trace( "ScreenEditMenu::ScreenEditMenu()" );

	m_bAllowNewNotes = true;

	int i;

	for( i=0; i<2; i++ )
	{
		m_sprArrows[i].Load( THEME->GetPathTo("Graphics",ssprintf("edit menu %s",(i==0?"left":"right"))) );
		m_sprArrows[i].SetX( ARROWS_X(i) );
		this->AddChild( &m_sprArrows[i] );
	}

	m_SelectedRow = (Row)0;

	ZERO( m_iSelection );

	for( i=0; i<NUM_ROWS; i++ )
	{
		m_textLabel[i].LoadFromFont( THEME->GetPathTo("Fonts","header2") );
		m_textLabel[i].SetXY( ROW_LABELS_X, ROW_Y(i) );
		m_textLabel[i].SetText( RowToString((Row)i) );
		m_textLabel[i].SetZoom( 0.8f );
		m_textLabel[i].SetHorizAlign( Actor::align_left );
		this->AddChild( &m_textLabel[i] );

		m_textValue[i].LoadFromFont( THEME->GetPathTo("Fonts","normal") );
		m_textValue[i].SetXY( ROW_VALUE_X(i), ROW_Y(i) );
		m_textValue[i].SetText( "blah" );
		m_textValue[i].SetZoom( 0.8f );
		this->AddChild( &m_textValue[i] );
	}

	m_GroupBanner.SetXY( GROUP_BANNER_X, GROUP_BANNER_Y );
	m_GroupBanner.SetCroppedSize( GROUP_BANNER_WIDTH, GROUP_BANNER_HEIGHT );
	this->AddChild( &m_GroupBanner );

	m_SongBanner.SetXY( SONG_BANNER_X, SONG_BANNER_Y );
	m_SongBanner.SetCroppedSize( SONG_BANNER_WIDTH, SONG_BANNER_HEIGHT );
	this->AddChild( &m_SongBanner );

	m_SongTextBanner.SetXY( SONG_TEXT_BANNER_X, SONG_TEXT_BANNER_Y );
	this->AddChild( &m_SongTextBanner );
	
	m_Meter.SetXY( METER_X, METER_Y );
	this->AddChild( &m_Meter );
	
	m_SourceMeter.SetXY( SOURCE_METER_X, SOURCE_METER_Y );
	this->AddChild( &m_SourceMeter );
	

	m_soundChangeRow.Load( THEME->GetPathTo("sounds","edit menu row") );
	m_soundChangeValue.Load( THEME->GetPathTo("sounds","edit menu value") );


	// fill in data structures
	SONGMAN->GetGroupNames( m_sGroups );
	GAMEMAN->GetNotesTypesForGame( GAMESTATE->m_CurGame, m_NotesTypes );
	
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
	}

	ChangeToRow( (Row)0 );
	OnRowValueChanged( (Row)0 );
}

SongSelector::~SongSelector()
{

}

void SongSelector::Refresh()
{
	ChangeToRow( (Row)0 );
	OnRowValueChanged( (Row)0 );
}

void SongSelector::DrawPrimitives()
{
	ActorFrame::DrawPrimitives();
}

bool SongSelector::CanGoUp()
{
	return m_SelectedRow != 0;
}

bool SongSelector::CanGoDown()
{
	return m_SelectedRow != NUM_ROWS-1;
}

bool SongSelector::CanGoLeft()
{
	return m_iSelection[m_SelectedRow] != 0;
}

bool SongSelector::CanGoRight()
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

void SongSelector::Up()
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

void SongSelector::Down()
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

void SongSelector::Left()
{
	if( CanGoLeft() )
	{
		m_iSelection[m_SelectedRow]--;
		OnRowValueChanged( m_SelectedRow );
		m_soundChangeValue.PlayRandom();
	}
}

void SongSelector::Right()
{
	if( CanGoRight() )
	{
		m_iSelection[m_SelectedRow]++;
		OnRowValueChanged( m_SelectedRow );
		m_soundChangeValue.PlayRandom();
	}
}


void SongSelector::ChangeToRow( Row newRow )
{
	m_SelectedRow = newRow;

	for( int i=0; i<2; i++ )
		m_sprArrows[i].SetY( ROW_Y(newRow) );
	m_sprArrows[0].SetDiffuse( CanGoLeft()?RageColor(1,1,1,1):RageColor(0.2f,0.2f,0.2f,1) );
	m_sprArrows[1].SetDiffuse( CanGoRight()?RageColor(1,1,1,1):RageColor(0.2f,0.2f,0.2f,1) );
}

void SongSelector::OnRowValueChanged( Row row )
{
	m_sprArrows[0].SetDiffuse( CanGoLeft()?RageColor(1,1,1,1):RageColor(0.2f,0.2f,0.2f,1) );
	m_sprArrows[1].SetDiffuse( CanGoRight()?RageColor(1,1,1,1):RageColor(0.2f,0.2f,0.2f,1) );

	switch( row )
	{
	case ROW_GROUP:
		m_textValue[ROW_GROUP].SetText( SONGMAN->ShortenGroupName(GetSelectedGroup()) );
		m_GroupBanner.LoadFromGroup( GetSelectedGroup() );
		m_pSongs.clear();
		SONGMAN->GetSongsInGroup( GetSelectedGroup(), m_pSongs );
		m_iSelection[ROW_SONG] = 0;
		// fall through
	case ROW_SONG:
		m_textValue[ROW_SONG].SetText( "" );
		m_SongBanner.LoadFromSong( GetSelectedSong() );
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

Notes* SongSelector::GetSelectedNotes()
{
	return GetSelectedSong()->GetNotesThatMatch(GetSelectedNotesType(),GetSelectedDifficulty(), false);
}

Notes* SongSelector::GetSelectedSourceNotes()
{
	return GetSelectedSong()->GetNotesThatMatch(GetSelectedSourceNotesType(),GetSelectedSourceDifficulty(), false);
}
