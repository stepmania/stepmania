#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: EditCoursesMenu

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "EditCoursesMenu.h"
#include "RageLog.h"
#include "SongManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "GameManager.h"
#include "Steps.h"
#include "song.h"
#include "Course.h"

//
// Defines specific to EditCoursesMenu
//
#define ARROWS_X( i )			THEME->GetMetricF("EditCoursesMenu",ssprintf("Arrows%dX",i+1))
#define COURSE_BANNER_X			THEME->GetMetricF("EditCoursesMenu","CourseBannerX")
#define COURSE_BANNER_Y			THEME->GetMetricF("EditCoursesMenu","CourseBannerY")
#define COURSE_BANNER_WIDTH		THEME->GetMetricF("EditCoursesMenu","CourseBannerWidth")
#define COURSE_BANNER_HEIGHT	THEME->GetMetricF("EditCoursesMenu","CourseBannerHeight")
#define COURSE_TEXT_BANNER_X	THEME->GetMetricF("EditCoursesMenu","CourseTextBannerX")
#define COURSE_TEXT_BANNER_Y	THEME->GetMetricF("EditCoursesMenu","CourseTextBannerY")
#define ENTRY_BANNER_X			THEME->GetMetricF("EditCoursesMenu","EntryBannerX")
#define ENTRY_BANNER_Y			THEME->GetMetricF("EditCoursesMenu","EntryBannerY")
#define ENTRY_BANNER_WIDTH		THEME->GetMetricF("EditCoursesMenu","EntryBannerWidth")
#define ENTRY_BANNER_HEIGHT		THEME->GetMetricF("EditCoursesMenu","EntryBannerHeight")
#define ENTRY_TEXT_BANNER_X		THEME->GetMetricF("EditCoursesMenu","EntryTextBannerX")
#define ENTRY_TEXT_BANNER_Y		THEME->GetMetricF("EditCoursesMenu","EntryTextBannerY")
#define ROW_LABELS_X			THEME->GetMetricF("EditCoursesMenu","RowLabelsX")
#define ROW_VALUE_X( i )		THEME->GetMetricF("EditCoursesMenu",ssprintf("RowValue%dX",i+1))
#define ROW_Y( i )				THEME->GetMetricF("EditCoursesMenu",ssprintf("Row%dY",i+1))


const int MAX_EDIT_COURSES_ENTRIES = 100;

EditCoursesMenu::EditCoursesMenu()
{
	LOG->Trace( "ScreenEditCoursesMenu::ScreenEditCoursesMenu()" );

	int i;

	for( i=0; i<2; i++ )
	{
		m_sprArrows[i].Load( THEME->GetPathToG(ssprintf("EditCoursesMenu %s",(i==0?"left":"right"))) );
		m_sprArrows[i].SetX( ARROWS_X(i) );
		this->AddChild( &m_sprArrows[i] );
	}

	m_SelectedRow = (Row)0;

	ZERO( m_iSelection );



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

	m_CourseBanner.SetXY( COURSE_BANNER_X, COURSE_BANNER_Y );
	this->AddChild( &m_CourseBanner );

	m_EntryBanner.SetXY( ENTRY_BANNER_X, ENTRY_BANNER_Y );
	this->AddChild( &m_EntryBanner );

	m_EntryTextBanner.SetXY( ENTRY_TEXT_BANNER_X, ENTRY_TEXT_BANNER_Y );
	this->AddChild( &m_EntryTextBanner );
	

	m_soundChangeRow.Load( THEME->GetPathToS("EditCoursesMenu row") );
	m_soundChangeValue.Load( THEME->GetPathToS("EditCoursesMenu value") );


	// fill in data structures
	SONGMAN->GetAllCourses( m_pCourses, false );
	
	ChangeToRow( (Row)0 );
	OnRowValueChanged( (Row)0 );
}

EditCoursesMenu::~EditCoursesMenu()
{

}

void EditCoursesMenu::DrawPrimitives()
{
	ActorFrame::DrawPrimitives();
}

bool EditCoursesMenu::CanGoUp()
{
	return m_SelectedRow != 0;
}

bool EditCoursesMenu::CanGoDown()
{
	return m_SelectedRow != NUM_ROWS-1;
}

bool EditCoursesMenu::CanGoLeft()
{
	return m_iSelection[m_SelectedRow] != 0;
}

bool EditCoursesMenu::CanGoRight()
{
	int num_values[NUM_ROWS] = 
	{
		m_pCourses.size(),
		1,
		1,
		MAX_EDIT_COURSES_ENTRIES,
		NUM_COURSE_ENTRY_TYPES,
		1,
		1
	};

	return m_iSelection[m_SelectedRow] != (num_values[m_SelectedRow]-1);
}

void EditCoursesMenu::Up()
{
	if( CanGoUp() )
	{
		m_soundChangeRow.PlayRandom();
	}
}

void EditCoursesMenu::Down()
{
	if( CanGoDown() )
	{
		m_soundChangeRow.PlayRandom();
	}
}

void EditCoursesMenu::Left()
{
	if( CanGoLeft() )
	{
		m_iSelection[m_SelectedRow]--;
		OnRowValueChanged( m_SelectedRow );
		m_soundChangeValue.PlayRandom();
	}
}

void EditCoursesMenu::Right()
{
	if( CanGoRight() )
	{
		m_iSelection[m_SelectedRow]++;
		OnRowValueChanged( m_SelectedRow );
		m_soundChangeValue.PlayRandom();
	}
}


void EditCoursesMenu::ChangeToRow( Row newRow )
{
	m_SelectedRow = newRow;

	for( int i=0; i<2; i++ )
		m_sprArrows[i].SetY( ROW_Y(newRow) );
	m_sprArrows[0].SetDiffuse( CanGoLeft()?RageColor(1,1,1,1):RageColor(0.2f,0.2f,0.2f,1) );
	m_sprArrows[1].SetDiffuse( CanGoRight()?RageColor(1,1,1,1):RageColor(0.2f,0.2f,0.2f,1) );
	m_sprArrows[0].EnableAnimation( CanGoLeft() );
	m_sprArrows[1].EnableAnimation( CanGoRight() );
}

void EditCoursesMenu::OnRowValueChanged( Row row )
{
	m_sprArrows[0].SetDiffuse( CanGoLeft()?RageColor(1,1,1,1):RageColor(0.2f,0.2f,0.2f,1) );
	m_sprArrows[1].SetDiffuse( CanGoRight()?RageColor(1,1,1,1):RageColor(0.2f,0.2f,0.2f,1) );
	m_sprArrows[0].EnableAnimation( CanGoLeft() );
	m_sprArrows[1].EnableAnimation( CanGoRight() );

	Course* pCourse = GetSelectedCourse();
	CourseEntry* pEntry = NULL;
	if( m_iSelection[ROW_ENTRY] < pCourse->m_entries.size() )
		pEntry = &pCourse->m_entries[m_iSelection[ROW_ENTRY]];

	switch( row )
	{
	case ROW_COURSE:
		m_textValue[ROW_COURSE].SetText( GetSelectedCourse()->m_sName );
		m_CourseBanner.LoadFromGroup( GetSelectedCourse()->m_sBannerPath );
		m_CourseBanner.ScaleToClipped( COURSE_BANNER_WIDTH, COURSE_BANNER_HEIGHT );
		m_iSelection[ROW_ENTRY] = 0;
		// fall through
	case ROW_SAVE:
		// fall through
	case ROW_COURSE_OPTIONS:
		m_textValue[ROW_COURSE_OPTIONS].SetText( 
			ssprintf(
				"%s, %s, %d",
				pCourse->m_bRepeat ? "repeat" : "no repeat",
				pCourse->m_bRandomize ? "randomize" : "no randomize",
				pCourse->m_iLives ) );
		// fall through
	case ROW_ENTRY:
		m_textValue[ROW_ENTRY].SetText( ssprintf("%d",m_iSelection[ROW_ENTRY]+1) );
		// fall through
	case ROW_ENTRY_TYPE:
		m_textValue[ROW_ENTRY_TYPE].SetText( pEntry ? CourseEntryTypeToString(pEntry->type) : "none" );
		// fall through
	case ROW_ENTRY_OPTIONS:
		m_textValue[ROW_ENTRY_OPTIONS].SetText( "coming soon" );
		// fall through
	case ROW_ENTRY_MODIFIERS:
		m_textValue[ROW_ENTRY_OPTIONS].SetText( "coming soon" );
		break;
	default:
		ASSERT(0);	// invalid row
	}
}
