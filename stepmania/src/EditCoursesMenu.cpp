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
							  
enum CourseEntryMenuRow
{
	repeat,
	randomize,
	lives,
};

Menu g_CourseOptionsMenu
(
	"Course Options",
	MenuRow( "Repeat",		true, 0, "NO","YES" ),
	MenuRow( "Randomize",	true, 0, "NO","YES" ),
	MenuRow( "Lives",		true, 4, "Use Bar Life","1","2","3","4","5","6","7","8","9","10" )
);

enum CourseOptionsMenuRow
{
	song,
	group,
	difficulty,
	low_meter,
	high_meter,
	best_worst_value,
	NUM_COURSE_OPTIONS_MENU_ROWS
};

Menu g_CourseEntryMenu
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
	m_soundSave.Load( THEME->GetPathToS("EditCoursesMenu save") );


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
		g_CourseOptionsMenu.rows[repeat].defaultChoice = GetSelectedCourse()->m_bRepeat ? 1 : 0;
		g_CourseOptionsMenu.rows[randomize].defaultChoice = GetSelectedCourse()->m_bRandomize ? 1 : 0;
		g_CourseOptionsMenu.rows[lives].defaultChoice = GetSelectedCourse()->m_iLives;
		if( g_CourseOptionsMenu.rows[lives].defaultChoice == -1 )
			g_CourseOptionsMenu.rows[lives].defaultChoice = 0;
		SCREENMAN->MiniMenu( &g_CourseOptionsMenu, SM_BackFromCourseOptionsMenu );
		break;
	case ROW_ACTION:
		switch( GetSelectedAction() )
		{
		case save:
			m_soundSave.Play();
			GetSelectedCourse()->Save();
			SCREENMAN->SystemMessage( "Course saved." );
			break;
		case add_entry:
			m_soundStart.Play();
			GetSelectedCourse()->m_entries.push_back( CourseEntry() );
			m_iSelection[ROW_ENTRY] = GetSelectedCourse()->m_entries.size()-1; 
			OnRowValueChanged( ROW_ENTRY );
			break;
		case delete_selected_entry:
			if( GetSelectedCourse()->m_entries.size() == 1 )
			{
				m_soundInvalid.Play();
				SCREENMAN->SystemMessage( "Cannot delete the last entry from a course" );
				break;
			}

			m_soundStart.Play();
			GetSelectedCourse()->m_entries.erase( GetSelectedCourse()->m_entries.begin()+m_iSelection[ROW_ENTRY] );
			CLAMP( m_iSelection[ROW_ENTRY], 0, (int) GetSelectedCourse()->m_entries.size()-1 );
			OnRowValueChanged( ROW_ENTRY );
			break;
		default:
			ASSERT(0);
		}

		OnRowValueChanged( ROW_ENTRY );		
		break;
	case ROW_ENTRY_OPTIONS:	
		{
			unsigned i;

			//
			// populate songs list
			//
			g_CourseEntryMenu.rows[song].choices.clear();
			g_CourseEntryMenu.rows[song].defaultChoice = 0;

			vector<Song*> vSongs = SONGMAN->GetAllSongs();
			for( i=0; i<vSongs.size(); i++ )
			{
				g_CourseEntryMenu.rows[song].choices.push_back( vSongs[i]->m_sGroupName + " - " + vSongs[i]->GetTranslitMainTitle() );
				if( GetSelectedEntry()->pSong == vSongs[i] )
					g_CourseEntryMenu.rows[song].defaultChoice = i;
			}

			//
			// populate group list
			//
			g_CourseEntryMenu.rows[group].choices.clear();
			g_CourseEntryMenu.rows[group].defaultChoice = 0;

			CStringArray vGroups;
			SONGMAN->GetGroupNames( vGroups );
			for( i=0; i<vGroups.size(); i++ )
			{
				g_CourseEntryMenu.rows[group].choices.push_back( vGroups[i] );
				if( GetSelectedEntry()->group_name == vGroups[i] )
					g_CourseEntryMenu.rows[group].defaultChoice = i;
			}

			//
			// populate difficulty list
			//
			g_CourseEntryMenu.rows[difficulty].choices.clear();
			g_CourseEntryMenu.rows[difficulty].defaultChoice = NUM_DIFFICULTIES;

			for( i=0; i<=NUM_DIFFICULTIES; i++ )
			{
				CString sDifficulty = (i==NUM_DIFFICULTIES) ? "Don't care" : DifficultyToString((Difficulty)i);
				g_CourseEntryMenu.rows[difficulty].choices.push_back( sDifficulty );
				if( GetSelectedEntry()->difficulty == (int) i )
					g_CourseEntryMenu.rows[difficulty].defaultChoice = i;
			}

			//
			// populate difficulty list
			//
			g_CourseEntryMenu.rows[low_meter].choices.clear();
			g_CourseEntryMenu.rows[low_meter].defaultChoice = 0;
			g_CourseEntryMenu.rows[high_meter].choices.clear();
			g_CourseEntryMenu.rows[high_meter].defaultChoice = 0;

			for( i=0; i<=MAX_METER; i++ )
			{
				CString sMeter = (i==0) ? "Don't care" : ssprintf("%d", i);
				g_CourseEntryMenu.rows[low_meter].choices.push_back( sMeter );
				g_CourseEntryMenu.rows[high_meter].choices.push_back( sMeter );
				if( GetSelectedEntry()->low_meter == (int) i )
					g_CourseEntryMenu.rows[low_meter].defaultChoice = i;
				if( GetSelectedEntry()->high_meter == (int) i )
					g_CourseEntryMenu.rows[high_meter].defaultChoice = i;
			}

			//
			// populate best/worst list
			//
			g_CourseEntryMenu.rows[best_worst_value].choices.clear();
			g_CourseEntryMenu.rows[best_worst_value].defaultChoice = 7;

			for( i=1; i<40; i++ )
			{
				CString sNum = ssprintf("%d", i);
				g_CourseEntryMenu.rows[high_meter].choices.push_back( sNum );
				if( GetSelectedEntry()->players_index == (int) i-1 )
					g_CourseEntryMenu.rows[best_worst_value].defaultChoice = i;
			}


			// update enabled/disabled lines
			for( i=0; i<g_CourseEntryMenu.rows.size(); i++ )
				g_CourseEntryMenu.rows[i].enabled = false;

			switch( GetSelectedEntry()->type )
			{
				case COURSE_ENTRY_FIXED:
					g_CourseEntryMenu.rows[song].enabled = true;
					break;
				case COURSE_ENTRY_RANDOM:
					g_CourseEntryMenu.rows[difficulty].enabled = true;
					g_CourseEntryMenu.rows[low_meter].enabled = true;
					g_CourseEntryMenu.rows[high_meter].enabled = true;
					break;
				case COURSE_ENTRY_RANDOM_WITHIN_GROUP:
					g_CourseEntryMenu.rows[group].enabled = true;
					g_CourseEntryMenu.rows[difficulty].enabled = true;
					g_CourseEntryMenu.rows[low_meter].enabled = true;
					g_CourseEntryMenu.rows[high_meter].enabled = true;
					break;
				case COURSE_ENTRY_BEST:
					g_CourseEntryMenu.rows[best_worst_value].enabled = true;
					break;
				case COURSE_ENTRY_WORST:
					g_CourseEntryMenu.rows[best_worst_value].enabled = true;
					break;
				default:
					ASSERT(0);
					break;
			}
			SCREENMAN->MiniMenu( &g_CourseEntryMenu, SM_BackFromCourseEntryOptionsMenu );
		}
		break;
	case ROW_ENTRY_PLAYER_OPTIONS:
		m_soundStart.Play();
			
		GAMESTATE->m_PlayerOptions[PLAYER_1] = PlayerOptions();
		GAMESTATE->m_PlayerOptions[PLAYER_1].FromString( GetSelectedEntry()->modifiers );

		SCREENMAN->AddNewScreenToTop( "ScreenPlayerOptions" );
		break;
	case ROW_ENTRY_SONG_OPTIONS:	
		m_soundStart.Play();

		GAMESTATE->m_SongOptions = SongOptions();
		GAMESTATE->m_SongOptions.FromString( GetSelectedEntry()->modifiers );

		SCREENMAN->AddNewScreenToTop( "ScreenSongOptions" );
		break;
	default:
		m_soundInvalid.Play();
		return;
	}
}


void EditCoursesMenu::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_BackFromCourseOptionsMenu:
		GetSelectedCourse()->m_bRepeat = !!ScreenMiniMenu::s_iLastAnswers[repeat];
		GetSelectedCourse()->m_bRandomize = !!ScreenMiniMenu::s_iLastAnswers[randomize];
		GetSelectedCourse()->m_iLives = ScreenMiniMenu::s_iLastAnswers[lives];
		if( GetSelectedCourse()->m_iLives == 0 )
			GetSelectedCourse()->m_iLives = -1;
		
		OnRowValueChanged( ROW_COURSE_OPTIONS );
		break;
	case SM_BackFromCourseEntryOptionsMenu:
		{
			vector<Song*> vSongs = SONGMAN->GetAllSongs();
			CStringArray vGroups;
			SONGMAN->GetGroupNames( vGroups );

			GetSelectedEntry()->pSong = vSongs[ ScreenMiniMenu::s_iLastAnswers[song] ];
			GetSelectedEntry()->group_name = vGroups[ ScreenMiniMenu::s_iLastAnswers[group] ];
			GetSelectedEntry()->difficulty = (Difficulty)ScreenMiniMenu::s_iLastAnswers[difficulty];
			if( GetSelectedEntry()->difficulty == NUM_DIFFICULTIES )
				GetSelectedEntry()->difficulty = DIFFICULTY_INVALID;
			GetSelectedEntry()->low_meter = ScreenMiniMenu::s_iLastAnswers[low_meter];
			if( GetSelectedEntry()->low_meter == 0 )
				GetSelectedEntry()->low_meter = -1;
			GetSelectedEntry()->high_meter = ScreenMiniMenu::s_iLastAnswers[high_meter];
			if( GetSelectedEntry()->high_meter == 0 )
				GetSelectedEntry()->high_meter = -1;
			GetSelectedEntry()->players_index = ScreenMiniMenu::s_iLastAnswers[best_worst_value];
			
			OnRowValueChanged( ROW_ENTRY_OPTIONS );
		}
		break;
	case SM_RegainingFocus:
		// coming back from PlayerOptions or SongOptions
		GetSelectedEntry()->modifiers = GAMESTATE->m_PlayerOptions[PLAYER_1].GetString() + "," + GAMESTATE->m_SongOptions.GetString();
		OnRowValueChanged( ROW_ENTRY_PLAYER_OPTIONS );
		break;
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
				"(START)  %s, %s, ",
				pCourse->m_bRepeat ? "repeat" : "no repeat",
				pCourse->m_bRandomize ? "randomize" : "no randomize" ) + 
			ssprintf(
				(pCourse->m_iLives==-1) ? "use bar life" : "%d lives",
				pCourse->m_iLives ) );
		// fall through
	case ROW_ACTION:
		m_textValue[ROW_ACTION].SetText( "(START) " + ActionToString(GetSelectedAction()) );
		// fall through
	case ROW_ENTRY:
		m_textValue[ROW_ENTRY].SetText( ssprintf("%d of %d",m_iSelection[ROW_ENTRY]+1, (int)GetSelectedCourse()->m_entries.size()) );
		m_iSelection[ROW_ENTRY_TYPE] = pEntry->type;
		// fall through
	case ROW_ENTRY_TYPE:
		m_textValue[ROW_ENTRY_TYPE].SetText( pEntry ? CourseEntryTypeToString(pEntry->type) : "(none)" );
		// fall through
	case ROW_ENTRY_OPTIONS:
		{
			CStringArray as;
			bool bShow[NUM_COURSE_OPTIONS_MENU_ROWS];
			ZERO( bShow );

			switch( pEntry->type )
			{
				case COURSE_ENTRY_FIXED:
					bShow[song] = true;
					break;
				case COURSE_ENTRY_RANDOM:
					bShow[difficulty] = true;
					bShow[low_meter] = true;
					bShow[high_meter] = true;
					break;
				case COURSE_ENTRY_RANDOM_WITHIN_GROUP:
					bShow[group] = true;
					bShow[difficulty] = true;
					bShow[low_meter] = true;
					bShow[high_meter] = true;
					break;
				case COURSE_ENTRY_BEST:
					bShow[best_worst_value] = true;
					break;
				case COURSE_ENTRY_WORST:
					bShow[best_worst_value] = true;
					break;
				default:
					ASSERT(0);
					break;
			}

			if( bShow[song] )
				as.push_back( pEntry->pSong ? "(missing song)" : pEntry->pSong->GetFullTranslitTitle() );
			if( bShow[group] )
				as.push_back( "group: " + (pEntry->group_name.empty() ? "(none)" : pEntry->group_name) );
			if( bShow[difficulty] )
				if( pEntry->difficulty != DIFFICULTY_INVALID )
					as.push_back( DifficultyToString(pEntry->difficulty) );
			if( bShow[low_meter] )
				if( pEntry->low_meter > 0 )
					as.push_back( ssprintf("low meter %d", pEntry->low_meter) );
			if( bShow[high_meter] )
				if( pEntry->high_meter > 0 )
					as.push_back( ssprintf("high meter %d", pEntry->high_meter) );
			if( bShow[best_worst_value] )
				if( pEntry->players_index != -1 )
					as.push_back( ssprintf("rank %d", pEntry->players_index+1) );

			m_textValue[ROW_ENTRY_OPTIONS].SetText( "(START) " + join(", ",as) );
		}
		// fall through
	case ROW_ENTRY_PLAYER_OPTIONS:
		{
			CString s = "(START) ";
		
			PlayerOptions po;
			po.FromString( pEntry->modifiers );
			if( po.GetString().empty() )
				s += "(none)";
			else
				s += po.GetString();
		
			m_textValue[ROW_ENTRY_PLAYER_OPTIONS].SetText( s );
		}
		// fall through
	case ROW_ENTRY_SONG_OPTIONS:
		{
			CString s = "(START) ";

			SongOptions so;
			so.FromString( pEntry->modifiers );
			if( so.GetString().empty() )
				s += "(none)";
			else
				s += so.GetString();

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
