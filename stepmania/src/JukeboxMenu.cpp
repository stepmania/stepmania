#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: JukeboxMenu

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "JukeboxMenu.h"
#include "RageLog.h"
#include "SongManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "GameManager.h"

//
// Defines specific to JukeboxMenu
//
#define ARROWS_X( i )			THEME->GetMetricF("JukeboxMenu",ssprintf("Arrows%dX",i+1))
#define ROW_LABELS_X			THEME->GetMetricF("JukeboxMenu","RowLabelsX")
#define ROW_VALUE_X( i )		THEME->GetMetricF("JukeboxMenu",ssprintf("RowValue%dX",i+1))
#define ROW_Y( i )				THEME->GetMetricF("JukeboxMenu",ssprintf("Row%dY",i+1))


JukeboxMenu::JukeboxMenu()
{
	LOG->Trace( "ScreenJukeboxMenu::ScreenJukeboxMenu()" );

	int i;

	for( i=0; i<2; i++ )
	{
		m_sprArrows[i].Load( THEME->GetPathTo("Graphics",ssprintf("jukebox menu %s",(i==0?"left":"right"))) );
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

	m_soundChangeRow.Load( THEME->GetPathTo("sounds","jukebox menu row") );
	m_soundChangeValue.Load( THEME->GetPathTo("sounds","jukebox menu value") );


	// fill in data structures
	GAMEMAN->GetStylesForGame( GAMESTATE->m_CurGame, m_Styles );
	SONGMAN->GetGroupNames( m_sGroups );
	m_sGroups.insert( m_sGroups.begin(), "ALL MUSIC" );
	m_sDifficulties.push_back( "all difficulties" );
	for( int d=0; d<NUM_DIFFICULTIES; d++ )
		m_sDifficulties.push_back( DifficultyToString( (Difficulty)d ) );
	m_sModifiers.push_back( "no modifiers" );
	m_sModifiers.push_back( "random modifiers" );

	ChangeToRow( (Row)0 );
	OnRowValueChanged( (Row)0 );
}

JukeboxMenu::~JukeboxMenu()
{

}

void JukeboxMenu::DrawPrimitives()
{
	ActorFrame::DrawPrimitives();
}

bool JukeboxMenu::CanGoUp()
{
	return m_SelectedRow != 0;
}

bool JukeboxMenu::CanGoDown()
{
	return m_SelectedRow != NUM_ROWS-1;
}

bool JukeboxMenu::CanGoLeft()
{
	return m_iSelection[m_SelectedRow] != 0;
}

bool JukeboxMenu::CanGoRight()
{
	int num_values[NUM_ROWS] = 
	{
		m_Styles.size(),
		m_sGroups.size(),
		m_sDifficulties.size(),
		m_sModifiers.size()
	};

	return m_iSelection[m_SelectedRow] != (num_values[m_SelectedRow]-1);
}

void JukeboxMenu::Up()
{
	if( CanGoUp() )
	{
		ChangeToRow( Row(m_SelectedRow-1) );
		m_soundChangeRow.PlayRandom();
	}
}

void JukeboxMenu::Down()
{
	if( CanGoDown() )
	{
		ChangeToRow( Row(m_SelectedRow+1) );
		m_soundChangeRow.PlayRandom();
	}
}

void JukeboxMenu::Left()
{
	if( CanGoLeft() )
	{
		m_iSelection[m_SelectedRow]--;
		OnRowValueChanged( m_SelectedRow );
		m_soundChangeValue.PlayRandom();
	}
}

void JukeboxMenu::Right()
{
	if( CanGoRight() )
	{
		m_iSelection[m_SelectedRow]++;
		OnRowValueChanged( m_SelectedRow );
		m_soundChangeValue.PlayRandom();
	}
}


void JukeboxMenu::ChangeToRow( Row newRow )
{
	m_SelectedRow = newRow;

	for( int i=0; i<2; i++ )
		m_sprArrows[i].SetY( ROW_Y(newRow) );
	m_sprArrows[0].SetDiffuse( CanGoLeft()?RageColor(1,1,1,1):RageColor(0.2f,0.2f,0.2f,1) );
	m_sprArrows[1].SetDiffuse( CanGoRight()?RageColor(1,1,1,1):RageColor(0.2f,0.2f,0.2f,1) );
	m_sprArrows[0].EnableAnimation( CanGoLeft() );
	m_sprArrows[1].EnableAnimation( CanGoRight() );
}

void JukeboxMenu::OnRowValueChanged( Row row )
{
	m_sprArrows[0].SetDiffuse( CanGoLeft()?RageColor(1,1,1,1):RageColor(0.2f,0.2f,0.2f,1) );
	m_sprArrows[1].SetDiffuse( CanGoRight()?RageColor(1,1,1,1):RageColor(0.2f,0.2f,0.2f,1) );
	m_sprArrows[0].EnableAnimation( CanGoLeft() );
	m_sprArrows[1].EnableAnimation( CanGoRight() );

	switch( row )
	{
	case ROW_STYLE:
		m_textValue[ROW_STYLE].SetText( GAMEMAN->GetStyleDefForStyle(GetSelectedStyle())->m_szName );
		// fall through
	case ROW_GROUP:
		m_textValue[ROW_GROUP].SetText( SONGMAN->ShortenGroupName(GetSelectedGroup()) );
		// fall through
	case ROW_DIFFICULTY:
		m_textValue[ROW_DIFFICULTY].SetText( GetSelectedDifficultyString() );
		// fall through
	case ROW_MODIFIERS:
		m_textValue[ROW_MODIFIERS].SetText( m_sModifiers[GetSelectedModifiers()] );
		break;
	default:
		ASSERT(0);	// invalid row
	}
}

