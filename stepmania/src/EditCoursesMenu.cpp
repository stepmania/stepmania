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
#include "ScreenMiniMenu.h"
#include "ScreenManager.h"

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

const ScreenMessage SM_BackFromCourseOptionsMenu		= (ScreenMessage)(SM_User+1);
const ScreenMessage SM_BackFromCourseEntryOptionsMenu	= (ScreenMessage)(SM_User+2);
							  
Menu g_CourseOptionsMenu
(
	"Course Options",
	MenuRow( "Repeat",		true, 0, "NO","YES" ),
	MenuRow( "Randomize",	true, 0, "NO","YES" ),
	MenuRow( "Lives",		true, 4, "Use Bar Life","1","2","3","4","5","6","7","8","9","10" )
);

enum CourseEntryOptionsMenuRow
{
	song,
	group,
	difficulty,
	low_meter,
	high_meter,
	best_worst_value
};

Menu g_CourseEntryOptionsMenu
(
	"Course Entry Options",
	MenuRow( "Song",			true, 0 ),
	MenuRow( "Group",			true, 0 ),
	MenuRow( "Difficulty",		true, 0 ),
	MenuRow( "Low Meter",		true, 0 ),
	MenuRow( "High Meter",		true, 0 ),
	MenuRow( "Best/Worst value",true, 0 )
);


EditCoursesMenu::EditCoursesMenu()
{
	LOG->Trace( "ScreenEditCoursesMenu::ScreenEditCoursesMenu()" );

	GAMESTATE->m_bEditing = true;

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
		m_textValue[i].SetZoom( 0.6f );
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
	m_soundStart.Load( THEME->GetPathToS("Common start") );
	m_soundInvalid.Load( THEME->GetPathToS("Common invalid") );


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
		NUM_ACTIONS,
		(int)GetSelectedCourse()->m_entries.size(),
		NUM_COURSE_ENTRY_TYPES,
		1,
		1,
		1
	};

	return m_iSelection[m_SelectedRow] != (num_values[m_SelectedRow]-1);
}

void EditCoursesMenu::Up()
{
	if( CanGoUp() )
	{
		ChangeToRow( Row(m_SelectedRow-1) );
		m_soundChangeRow.Play();
	}
}

void EditCoursesMenu::Down()
{
	if( CanGoDown() )
	{
		ChangeToRow( Row(m_SelectedRow+1) );
		m_soundChangeRow.Play();
	}
}

void EditCoursesMenu::Left()
{
	if( CanGoLeft() )
	{
		m_iSelection[m_SelectedRow]--;

		switch( m_SelectedRow )
		{
		case ROW_ENTRY_TYPE:	
			GetSelectedEntry()->type = GetSelectedEntryType();
			break;
		}

		OnRowValueChanged( m_SelectedRow );
		m_soundChangeValue.Play();
	}
}

void EditCoursesMenu::Right()
{
	if( CanGoRight() )
	{
		m_iSelection[m_SelectedRow]++;

		switch( m_SelectedRow )
		{
		case ROW_ENTRY_TYPE:	
			GetSelectedEntry()->type = GetSelectedEntryType();
			break;
		}

		OnRowValueChanged( m_SelectedRow );
		m_soundChangeValue.Play();
	}
}

void EditCoursesMenu::Start()
{
	switch( m_SelectedRow )
	{
	case ROW_COURSE_OPTIONS:	
		SCREENMAN->MiniMenu( &g_CourseOptionsMenu, SM_BackFromCourseOptionsMenu );
		break;
	case ROW_ACTION:	
		m_soundStart.Play();
		break;
	case ROW_ENTRY_OPTIONS:	
		{
			// update enabled/disabled lines
			for( unsigned i=0; i<g_CourseEntryOptionsMenu.rows.size(); i++ )
				g_CourseEntryOptionsMenu.rows[i].enabled = false;
			switch( GetSelectedEntry()->type )
			{
				case COURSE_ENTRY_FIXED:
					g_CourseEntryOptionsMenu.rows[song].enabled = true;
					break;
				case COURSE_ENTRY_RANDOM:
					g_CourseEntryOptionsMenu.rows[low_meter].enabled = true;
					g_CourseEntryOptionsMenu.rows[high_meter].enabled = true;
					break;
				case COURSE_ENTRY_RANDOM_WITHIN_GROUP:
					g_CourseEntryOptionsMenu.rows[group].enabled = true;
					g_CourseEntryOptionsMenu.rows[low_meter].enabled = true;
					g_CourseEntryOptionsMenu.rows[high_meter].enabled = true;
					break;
				case COURSE_ENTRY_BEST:
					g_CourseEntryOptionsMenu.rows[best_worst_value].enabled = true;
					break;
				case COURSE_ENTRY_WORST:
					g_CourseEntryOptionsMenu.rows[best_worst_value].enabled = true;
					break;
				default:
					ASSERT(0);
					break;
			}
			SCREENMAN->MiniMenu( &g_CourseEntryOptionsMenu, SM_BackFromCourseEntryOptionsMenu );
		}
		break;
	case ROW_ENTRY_PLAYER_OPTIONS:
		SCREENMAN->AddNewScreenToTop( "ScreenPlayerOptions" );
		break;
	case ROW_ENTRY_SONG_OPTIONS:	
		SCREENMAN->AddNewScreenToTop( "ScreenSongOptions" );
		break;
	default:
		m_soundInvalid.Play();
		return;
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
	CourseEntry* pEntry = GetSelectedEntry();

	switch( row )
	{
	case ROW_COURSE:
		m_textValue[ROW_COURSE].SetText( pCourse->m_sName );
		m_CourseBanner.LoadFromCourse( pCourse );
		m_CourseBanner.ScaleToClipped( COURSE_BANNER_WIDTH, COURSE_BANNER_HEIGHT );
		m_iSelection[ROW_ENTRY] = 0;
		// fall through
	case ROW_COURSE_OPTIONS:
		m_textValue[ROW_COURSE_OPTIONS].SetText( 
			ssprintf(
				"(START)  %s, %s, %d lives",
				pCourse->m_bRepeat ? "repeat" : "no repeat",
				pCourse->m_bRandomize ? "randomize" : "no randomize",
				pCourse->m_iLives ) );
		// fall through
	case ROW_ACTION:
		m_textValue[ROW_ACTION].SetText( "(START) " + ActionToString(GetSelectedAction()) );
		// fall through
	case ROW_ENTRY:
		m_textValue[ROW_ENTRY].SetText( ssprintf("%d of %d",m_iSelection[ROW_ENTRY]+1, (int)GetSelectedCourse()->m_entries.size()) );
		// fall through
	case ROW_ENTRY_TYPE:
		m_textValue[ROW_ENTRY_TYPE].SetText( pEntry ? CourseEntryTypeToString(pEntry->type) : "(none)" );
		// fall through
	case ROW_ENTRY_OPTIONS:
		{
			CString s = "(START) ";
			if( pEntry )
			{
				switch( pEntry->type )
				{
					case COURSE_ENTRY_FIXED:
						s += pEntry->pSong ? "(missing song)" : pEntry->pSong->GetFullTranslitTitle();
						break;
					case COURSE_ENTRY_RANDOM:
						s += "any group";
						break;
					case COURSE_ENTRY_RANDOM_WITHIN_GROUP:
						s += "group: " + pEntry->group_name;
						break;
					case COURSE_ENTRY_BEST:
						s += ssprintf( "rank %d", pEntry->players_index+1 );
						break;
					case COURSE_ENTRY_WORST:
						s += ssprintf( "rank %d", pEntry->players_index+1 );
						break;
					default:
						ASSERT(0);
						break;
				}
			}
			else
			{
				s = "n/a";
			}

			m_textValue[ROW_ENTRY_OPTIONS].SetText( s );
		}
		// fall through
	case ROW_ENTRY_PLAYER_OPTIONS:
		{
			CString s = "(START) ";
			if( pEntry )
			{
				if( pEntry->modifiers.empty() )
					s += "(none)";
				else
					s += pEntry->modifiers;
			}
			else
			{
				s = "n/a";
			}
		
			m_textValue[ROW_ENTRY_PLAYER_OPTIONS].SetText( s );
		}
		// fall through
	case ROW_ENTRY_SONG_OPTIONS:
		{
			CString s = "(START) ";
			if( pEntry )
			{
				if( pEntry->modifiers.empty() )
					s += "(none)";
				else
					s += pEntry->modifiers;
			}
			else
			{
				s = "n/a";
			}
		
			m_textValue[ROW_ENTRY_SONG_OPTIONS].SetText( s );
		}
		break;
	default:
		ASSERT(0);	// invalid row
	}
}

CourseEntry* EditCoursesMenu::GetSelectedEntry()
{
	Course* pCourse = GetSelectedCourse();
	if( m_iSelection[ROW_ENTRY] < int(pCourse->m_entries.size()) )
		return &pCourse->m_entries[m_iSelection[ROW_ENTRY]];
	else
		return NULL;
}
