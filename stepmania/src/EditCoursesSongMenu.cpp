#include "global.h"
#include "EditCoursesSongMenu.h"
#include "RageLog.h"
#include "SongManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "GameManager.h"
#include "Steps.h"
#include "song.h"
#include "Course.h"
#include "ScreenManager.h"

#define ARROWS_X( i )			THEME->GetMetricF("EditCoursesSongMenu",ssprintf("Arrows%dX",i+1))
#define ROW_LABELS_X			THEME->GetMetricF("EditCoursesSongMenu","RowLabelsX")
#define ROW_VALUE_X( i )		THEME->GetMetricF("EditCoursesSongMenu",ssprintf("RowValue%dX",i+1))
#define ROW_Y( i )				THEME->GetMetricF("EditCoursesSongMenu",ssprintf("Row%dY",i+1))


const bool g_bRowEnabledForType[NUM_COURSE_ENTRY_TYPES][EditCoursesSongMenu::NUM_ROWS] = 
{				     // group,  song,   type,	difficulty, low_meter, high_meter, best_worst
	/* fixed */	      { true,   true,   true,   true,       true,      true,       false },
	/* random */      { false,  false,  true,   true,       true,      true,       false },
	/* random_group */{ true,   false,  true,   true,       true,      true,       false },
	/* best */        { false,  false,  true,   false,      false,     false,      true },
	/* worst */       { false,  false,  true,   false,      false,     false,      true }
};

EditCoursesSongMenu::EditCoursesSongMenu()
{
	LOG->Trace( "ScreenEditCoursesSongMenu::ScreenEditCoursesSongMenu()" );

	for( int i=0; i<2; i++ )
	{
		m_sprArrows[i].Load( THEME->GetPathToG(ssprintf("EditCoursesSongMenu %s",(i==0?"left":"right"))) );
		m_sprArrows[i].SetX( ARROWS_X(i) );
		this->AddChild( &m_sprArrows[i] );
	}

	for( int i=0; i<NUM_ROWS; i++ )
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


	m_soundChangeRow.Load( THEME->GetPathToS("EditCoursesSongMenu row") );
	m_soundChangeValue.Load( THEME->GetPathToS("EditCoursesSongMenu value") );

	// fill in data structures
	SONGMAN->GetGroupNames( m_aGroups );

	m_SelectedRow = (Row)0;
}


EditCoursesSongMenu::~EditCoursesSongMenu()
{

}

bool EditCoursesSongMenu::CanGoLeft()
{
	return m_iSelection[m_SelectedRow] != 0;
}

bool EditCoursesSongMenu::CanGoRight()
{
	int num_values[NUM_ROWS] = 
	{
		m_aGroups.size(),
		m_aSongs.size(),
		NUM_COURSE_ENTRY_TYPES,
		NUM_DIFFICULTIES+1,
		11,
		11,
		40,
	};

	return m_iSelection[m_SelectedRow] != (num_values[m_SelectedRow]-1);
}

bool EditCoursesSongMenu::ChangeRow( int add )
{
	Row dest = m_SelectedRow;
	do
	{
		dest = (Row) (dest+add);
	}
	while( dest >= 0 && dest < NUM_ROWS && !g_bRowEnabledForType[GetSelectedType()][dest] );

	if( dest < 0 || dest >= NUM_ROWS )
		return false;

	ChangeToRow( dest );
	return true;
}

void EditCoursesSongMenu::Up()
{
	if( ChangeRow( -1 ) )
		m_soundChangeRow.Play();
}

void EditCoursesSongMenu::Down()
{
	if( ChangeRow( +1 ) )
		m_soundChangeRow.Play();
}

void EditCoursesSongMenu::SaveToCourseEntry( CourseEntry *pEntry )
{
	*pEntry = CourseEntry();
	pEntry->type = (CourseEntryType) m_iSelection[ROW_TYPE];

	/* Only set things relevant to this type. */
	if( g_bRowEnabledForType[pEntry->type][ROW_GROUP] )
		pEntry->group_name = GetSelectedGroup();
	if( g_bRowEnabledForType[pEntry->type][ROW_SONG] )
		pEntry->pSong = GetSelectedSong();
	if( g_bRowEnabledForType[pEntry->type][ROW_DIFFICULTY] )
		pEntry->difficulty = GetSelectedDifficulty();
	if( g_bRowEnabledForType[pEntry->type][ROW_LOW_METER] )
		pEntry->low_meter = GetLowMeter();
	if( g_bRowEnabledForType[pEntry->type][ROW_HIGH_METER] )
		pEntry->high_meter = GetHighMeter();
	if( g_bRowEnabledForType[pEntry->type][ROW_BEST_WORST_VALUE] )
		pEntry->players_index = GetBestWorst();

	if( pEntry->high_meter < pEntry->low_meter )
		swap( pEntry->low_meter, pEntry->high_meter );
}

//void EditCoursesSongMenu::SetSongsFromGroup( const CourseEntry *pEntry )

void EditCoursesSongMenu::SetGroupByName( CString sGroup )
{
	unsigned i;
	for( i = 0; i < m_aGroups.size(); ++i )
		if( !sGroup.CompareNoCase( m_aGroups[i] ) )
			break;
	ASSERT_M( i < m_aGroups.size(), ssprintf("%s", sGroup.c_str() ) );
	m_iSelection[ROW_GROUP] = i;

	UpdateSongList();
}

void EditCoursesSongMenu::UpdateSongList()
{
	CString sGroup = GetSelectedGroup();

	m_aSongs.clear();
	const vector<Song*> &aSongs = SONGMAN->GetAllSongs();
	for( unsigned i = 0; i < aSongs.size(); ++i )
		if( !aSongs[i]->m_sGroupName.CompareNoCase(sGroup) )
			m_aSongs.push_back( aSongs[i] );

	m_iSelection[ROW_SONG] = 0;
}

void EditCoursesSongMenu::LoadFromCourseEntry( const CourseEntry *pEntry )
{
	ZERO( m_iSelection );

	if( pEntry->pSong )
	{
		/* Set this song's group. */
		SetGroupByName( pEntry->pSong->m_sGroupName );

		unsigned i;
		for( i = 0; i < m_aSongs.size(); ++i )
			if( m_aSongs[i] == pEntry->pSong )
				break;
		ASSERT_M( i < m_aSongs.size(), ssprintf("%s", pEntry->pSong->GetTranslitMainTitle().c_str()) );

		m_iSelection[ROW_SONG] = i;
	}
	else if( pEntry->group_name != "" )
	{
		SetGroupByName( pEntry->group_name );
	}

	m_iSelection[ROW_TYPE] = pEntry->type;
	m_iSelection[ROW_DIFFICULTY] = (pEntry->difficulty == DIFFICULTY_INVALID)? 0:pEntry->difficulty+1;
	m_iSelection[ROW_LOW_METER] = (pEntry->low_meter == -1)? 0:pEntry->low_meter;
	m_iSelection[ROW_HIGH_METER] = (pEntry->high_meter == -1)? 0:pEntry->high_meter;
	m_iSelection[ROW_BEST_WORST_VALUE] = pEntry->players_index;

	/* Make sure we're on an active row. */
	if( !g_bRowEnabledForType[GetSelectedType()][m_SelectedRow] )
		ChangeRow( -1 );
	if( !g_bRowEnabledForType[GetSelectedType()][m_SelectedRow] )
		ChangeRow( +1 );
	ASSERT( g_bRowEnabledForType[GetSelectedType()][m_SelectedRow] );

	ChangeToRow( m_SelectedRow );
	OnRowValueChanged( (Row)0 );
}

void EditCoursesSongMenu::Left()
{
	if( CanGoLeft() )
	{
		m_iSelection[m_SelectedRow]--;

		OnRowValueChanged( m_SelectedRow );
		m_soundChangeValue.Play();
	}
}

void EditCoursesSongMenu::Right()
{
	if( CanGoRight() )
	{
		m_iSelection[m_SelectedRow]++;

		OnRowValueChanged( m_SelectedRow );
		m_soundChangeValue.Play();
	}
}

void EditCoursesSongMenu::Start()
{
	// XXX
	// SCREENMAN->PopTopScreen( m_SMSendOnOK );
}


void EditCoursesSongMenu::HandleScreenMessage( const ScreenMessage SM )
{
}

void EditCoursesSongMenu::ChangeToRow( Row newRow )
{
	m_SelectedRow = newRow;

	for( int i=0; i<2; i++ )
		m_sprArrows[i].SetY( ROW_Y(newRow) );
	m_sprArrows[0].SetDiffuse( CanGoLeft()?RageColor(1,1,1,1):RageColor(0.2f,0.2f,0.2f,1) );
	m_sprArrows[1].SetDiffuse( CanGoRight()?RageColor(1,1,1,1):RageColor(0.2f,0.2f,0.2f,1) );
	m_sprArrows[0].EnableAnimation( CanGoLeft() );
	m_sprArrows[1].EnableAnimation( CanGoRight() );
}

void EditCoursesSongMenu::OnRowValueChanged( Row row )
{
	LOG->Trace( "EditCoursesSongMenu::OnRowValueChanged(%i)", row );

	m_sprArrows[0].SetDiffuse( CanGoLeft()?RageColor(1,1,1,1):RageColor(0.2f,0.2f,0.2f,1) );
	m_sprArrows[1].SetDiffuse( CanGoRight()?RageColor(1,1,1,1):RageColor(0.2f,0.2f,0.2f,1) );
	m_sprArrows[0].EnableAnimation( CanGoLeft() );
	m_sprArrows[1].EnableAnimation( CanGoRight() );

	CString sGroup = GetSelectedGroup();
	Song *pSong = GetSelectedSong();
	
	switch( row )
	{
	case ROW_GROUP:
	{
		CHECKPOINT;
		m_textValue[ROW_GROUP].SetText( sGroup );

		UpdateSongList();

		pSong = GetSelectedSong();
	}
		// fall through
	case ROW_SONG:
		CHECKPOINT;
		m_textValue[ROW_SONG].SetText( pSong? pSong->GetTranslitMainTitle():CString("") );
		// fall through

	case ROW_TYPE:
		CHECKPOINT;
		m_textValue[ROW_TYPE].SetText( CourseEntryTypeToString(GetSelectedType()) );
		for( int i = 0; i < NUM_ROWS; ++i )
			m_textValue[i].SetDiffuse( 
				g_bRowEnabledForType[GetSelectedType()][i]?
					RageColor(1,1,1,1):RageColor(0.4f,0.4f,0.4f,1) );
		// fall through

	case ROW_DIFFICULTY:
	{
		CHECKPOINT;
		Difficulty dc = GetSelectedDifficulty();
		if( dc == DIFFICULTY_INVALID )
			m_textValue[ROW_DIFFICULTY].SetText( "(any)" );
		else
			m_textValue[ROW_DIFFICULTY].SetText( DifficultyToString(dc) );
		// fall through
	}
	case ROW_LOW_METER:
		CHECKPOINT;
		if( GetLowMeter() == -1 )
			m_textValue[ROW_LOW_METER].SetText( "(any)" );
		else
			m_textValue[ROW_LOW_METER].SetText( ssprintf("%d",GetLowMeter()) );
		// fall through

	case ROW_HIGH_METER:
		CHECKPOINT;
		if( GetHighMeter() == -1 )
			m_textValue[ROW_HIGH_METER].SetText( "(any)" );
		else
			m_textValue[ROW_HIGH_METER].SetText( ssprintf("%d",GetHighMeter()) );
		// fall through

	case ROW_BEST_WORST_VALUE:
		CHECKPOINT;
		m_textValue[ROW_BEST_WORST_VALUE].SetText( ssprintf("%d",GetBestWorst()+1) );
		break;

	default:
		ASSERT(0);	// invalid row
	}
}

Song *EditCoursesSongMenu::GetSelectedSong() const
{
	if( m_iSelection[ROW_SONG] < (int) m_aSongs.size() )
		return m_aSongs[ m_iSelection[ROW_SONG] ];
	else
		return NULL;
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
