#include "global.h"
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
#include "PlayerState.h"
#include "ScreenPlayerOptions.h"	// for SM_BackFromPlayerOptions
#include "ScreenSongOptions.h"	// for SM_BackFromSongOptions

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

AutoScreenMessage( SM_BackFromCourseOptionsMenu )
							  
enum CourseEntryMenuRow
{
	repeat,
	randomize,
	lives,
};

static Menu g_CourseOptionsMenu(
	"ScreenMiniMenuCourseOptions",
	MenuRow( repeat,	"Repeat",		true, EDIT_MODE_FULL, 0, "NO","YES" ),
	MenuRow( randomize,	"Randomize",	true, EDIT_MODE_FULL, 0, "NO","YES" ),
	MenuRow( lives,		"Lives",		true, EDIT_MODE_FULL, 4, "Use Bar Life","1","2","3","4","5","6","7","8","9","10" )
);

enum CourseOptionsMenuRow
{
	song,
	group,
	difficulty,
	low_meter,
	high_meter,
	best_worst_value,
	NUM_ENTRY_OPTIONS_MENU_ROWS
};



const bool g_bRowEnabledForType[NUM_CourseEntryType][NUM_ENTRY_OPTIONS_MENU_ROWS] = 
{				     // song,   group,  difficulty, low_meter, high_meter, best_worst
	/* fixed */	      { true,   false,  true,       true,      true,       false },
	/* random */      { false,  false,  true,       true,      true,       false },
	/* random_group */{ false,  true,   true,       true,      true,       false },
	/* best */        { false,  false,  false,      false,     false,      true },
	/* worst */       { false,  false,  false,      false,     false,      true }
};


EditCoursesMenu::EditCoursesMenu()
{
	LOG->Trace( "ScreenEditCoursesMenu::ScreenEditCoursesMenu()" );

	m_bInSongMenu = false;

	for( int i=0; i<2; i++ )
	{
		m_sprArrows[i].Load( THEME->GetPathG("EditCoursesMenu",i==0?"left":"right") );
		m_sprArrows[i].SetX( ARROWS_X(i) );
		this->AddChild( &m_sprArrows[i] );
	}

	m_SelectedRow = (Row)0;

	ZERO( m_iSelection );


	for( int i=0; i<NUM_ROWS; i++ )
	{
		m_textLabel[i].LoadFromFont( THEME->GetPathF("Common","title") );
		m_textLabel[i].SetXY( ROW_LABELS_X, ROW_Y(i) );
		m_textLabel[i].SetText( RowToString((Row)i) );
		m_textLabel[i].SetZoom( 0.8f );
		m_textLabel[i].SetHorizAlign( Actor::align_left );
		this->AddChild( &m_textLabel[i] );

		m_textValue[i].LoadFromFont( THEME->GetPathF("Common","normal") );
		m_textValue[i].SetXY( ROW_VALUE_X(i), ROW_Y(i) );
		m_textValue[i].SetText( "blah" );
		m_textValue[i].SetZoom( 0.6f );
		this->AddChild( &m_textValue[i] );
	}

	m_CourseBanner.SetXY( COURSE_BANNER_X, COURSE_BANNER_Y );
	this->AddChild( &m_CourseBanner );

	m_EntryBanner.SetXY( ENTRY_BANNER_X, ENTRY_BANNER_Y );
	this->AddChild( &m_EntryBanner );

	m_EntryTextBanner.SetName( "TextBanner" );
	m_EntryTextBanner.SetXY( ENTRY_TEXT_BANNER_X, ENTRY_TEXT_BANNER_Y );
	this->AddChild( &m_EntryTextBanner );
	

	m_soundChangeRow.Load( THEME->GetPathS("EditCoursesMenu","row") );
	m_soundChangeValue.Load( THEME->GetPathS("EditCoursesMenu","value") );
	m_soundSave.Load( THEME->GetPathS("EditCoursesMenu","save") );


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
	if( m_bInSongMenu )
		m_SongMenu.Draw();
	else
		ActorFrame::DrawPrimitives();
}

void EditCoursesMenu::Update( float fDeltaTime )
{
	if( m_bInSongMenu )
		m_SongMenu.Update( fDeltaTime );
	else
		ActorFrame::Update( fDeltaTime );
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
		NUM_CourseEntryType,
		1,
		1,
		1
	};

	return m_iSelection[m_SelectedRow] != (num_values[m_SelectedRow]-1);
}

void EditCoursesMenu::Up()
{
	if( m_bInSongMenu )
	{
		m_SongMenu.Up();
		return;
	}

	if( CanGoUp() )
	{
		ChangeToRow( Row(m_SelectedRow-1) );
		m_soundChangeRow.Play();
	}
}

void EditCoursesMenu::Down()
{
	if( m_bInSongMenu )
	{
		m_SongMenu.Down();
		return;
	}

	if( CanGoDown() )
	{
		ChangeToRow( Row(m_SelectedRow+1) );
		m_soundChangeRow.Play();
	}
}

void EditCoursesMenu::Left()
{
	if( m_bInSongMenu )
	{
		m_SongMenu.Left();
		return;
	}

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
	if( m_bInSongMenu )
	{
		m_SongMenu.Right();
		return;
	}

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
	if( m_bInSongMenu )
	{
		m_SongMenu.SaveToCourseEntry( GetSelectedEntry() );
		m_bInSongMenu = false;
		OnRowValueChanged( ROW_ENTRY );
		return;
	}

	Course* pCourse = GetSelectedCourse();
	CourseEntry* pEntry = GetSelectedEntry();

	switch( m_SelectedRow )
	{
	case ROW_COURSE_OPTIONS:
		g_CourseOptionsMenu.rows[repeat].iDefaultChoice = pCourse->m_bRepeat ? 1 : 0;
		g_CourseOptionsMenu.rows[randomize].iDefaultChoice = pCourse->m_bRandomize ? 1 : 0;
		g_CourseOptionsMenu.rows[lives].iDefaultChoice = pCourse->m_iLives;
		if( g_CourseOptionsMenu.rows[lives].iDefaultChoice == -1 )
			g_CourseOptionsMenu.rows[lives].iDefaultChoice = 0;
		SCREENMAN->MiniMenu( &g_CourseOptionsMenu, SM_BackFromCourseOptionsMenu );
		break;
	case ROW_ACTION:
		switch( GetSelectedAction() )
		{
		case save:
			m_soundSave.Play();
			pCourse->Save();
			SCREENMAN->SystemMessage( "Course saved." );
			break;
		case add_entry:
			SCREENMAN->PlayStartSound();
			pCourse->m_entries.insert( pCourse->m_entries.begin()+m_iSelection[ROW_ENTRY], pCourse->m_entries[m_iSelection[ROW_ENTRY]] );
			OnRowValueChanged( ROW_ENTRY );
			break;
		case delete_selected_entry:
			if( pCourse->m_entries.size() == 1 )
			{
				SCREENMAN->PlayInvalidSound();
				SCREENMAN->SystemMessage( "Cannot delete the last entry from a course" );
				break;
			}

			SCREENMAN->PlayStartSound();
			pCourse->m_entries.erase( pCourse->m_entries.begin()+m_iSelection[ROW_ENTRY] );
			CLAMP( m_iSelection[ROW_ENTRY], 0, (int) pCourse->m_entries.size()-1 );
			OnRowValueChanged( ROW_ENTRY );
			break;
		default:
			ASSERT(0);
		}

		OnRowValueChanged( ROW_ENTRY );		
		break;
	case ROW_ENTRY_OPTIONS:	
		m_SongMenu.LoadFromCourseEntry( GetSelectedEntry() );
		m_bInSongMenu = true;
		break;
	case ROW_ENTRY_PLAYER_OPTIONS:
		SCREENMAN->PlayStartSound();
			
		GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions = PlayerOptions();
		GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions.FromString( pEntry->modifiers );

		SCREENMAN->AddNewScreenToTop( "ScreenPlayerOptions" );
		break;
	case ROW_ENTRY_SONG_OPTIONS:	
		SCREENMAN->PlayStartSound();

		GAMESTATE->m_SongOptions = SongOptions();
		GAMESTATE->m_SongOptions.FromString( pEntry->modifiers );

		SCREENMAN->AddNewScreenToTop( "ScreenSongOptions" );
		break;
	default:
		SCREENMAN->PlayInvalidSound();
		return;
	}
}


void EditCoursesMenu::HandleScreenMessage( const ScreenMessage SM )
{
	Course* pCourse = GetSelectedCourse();
	CourseEntry* pEntry = GetSelectedEntry();

	if( SM == SM_BackFromCourseOptionsMenu )
	{
		pCourse->m_bRepeat = !!ScreenMiniMenu::s_viLastAnswers[repeat];
		pCourse->m_bRandomize = !!ScreenMiniMenu::s_viLastAnswers[randomize];
		pCourse->m_iLives = ScreenMiniMenu::s_viLastAnswers[lives];
		if( pCourse->m_iLives == 0 )
			pCourse->m_iLives = -1;
		
		OnRowValueChanged( ROW_COURSE_OPTIONS );
	}
	else if( 
		SM == SM_BackFromPlayerOptions || 
		SM == SM_BackFromSongOptions )
	{
		// coming back from PlayerOptions or SongOptions
		pEntry->modifiers = GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions.GetString() + "," + GAMESTATE->m_SongOptions.GetString();
		OnRowValueChanged( ROW_ENTRY_PLAYER_OPTIONS );
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
	LOG->Trace( "EditCoursesMenu::OnRowValueChanged(%i)", row );

	m_sprArrows[0].SetDiffuse( CanGoLeft()?RageColor(1,1,1,1):RageColor(0.2f,0.2f,0.2f,1) );
	m_sprArrows[1].SetDiffuse( CanGoRight()?RageColor(1,1,1,1):RageColor(0.2f,0.2f,0.2f,1) );
	m_sprArrows[0].EnableAnimation( CanGoLeft() );
	m_sprArrows[1].EnableAnimation( CanGoRight() );

	Course* pCourse = GetSelectedCourse();
	CourseEntry* pEntry = GetSelectedEntry();

	switch( row )
	{
	case ROW_COURSE:
		CHECKPOINT;
		m_textValue[ROW_COURSE].SetText( pCourse->GetDisplayFullTitle() );
		m_CourseBanner.LoadFromCourse( pCourse );
		m_CourseBanner.ScaleToClipped( COURSE_BANNER_WIDTH, COURSE_BANNER_HEIGHT );
		m_iSelection[ROW_ENTRY] = 0;
		pEntry = GetSelectedEntry();
		if( pEntry == NULL )
		{
			CourseEntry ce;
			const vector<Song*> &apSongs = SONGMAN->GetAllSongs();
			ASSERT( !apSongs.empty() );
			ce.pSong = apSongs[0];
			pCourse->m_entries.push_back( ce );
			pEntry = GetSelectedEntry();
		}
		// fall through
	case ROW_COURSE_OPTIONS:
		CHECKPOINT;
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
		CHECKPOINT;
		m_textValue[ROW_ACTION].SetText( "(START) " + ActionToString(GetSelectedAction()) );
		// fall through
	case ROW_ENTRY:
		CHECKPOINT;
		m_textValue[ROW_ENTRY].SetText( ssprintf("%d of %d",m_iSelection[ROW_ENTRY]+1, (int)GetSelectedCourse()->m_entries.size()) );
		m_iSelection[ROW_ENTRY_TYPE] = pEntry->type;
		// fall through
	case ROW_ENTRY_TYPE:
		CHECKPOINT;
		m_textValue[ROW_ENTRY_TYPE].SetText( pEntry ? CourseEntryTypeToString(pEntry->type) : CString("(none)") );
		// fall through
	case ROW_ENTRY_OPTIONS:
		CHECKPOINT;
		{
			CStringArray as;
			const bool *bShow = g_bRowEnabledForType[GetSelectedEntry()->type];

			if( bShow[song] )
				as.push_back( pEntry->pSong ? pEntry->pSong->GetTranslitFullTitle() : CString("(missing song)") );
			if( bShow[group] )
				as.push_back( pEntry->group_name.empty() ? CString("(no group)") : pEntry->group_name );
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
		CHECKPOINT;
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
		CHECKPOINT;
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

/*
 * (c) 2003-2004 Chris Danford
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
