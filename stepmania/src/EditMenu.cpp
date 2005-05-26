#include "global.h"
#include "EditMenu.h"
#include "RageLog.h"
#include "SongManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "GameManager.h"
#include "Steps.h"
#include "song.h"
#include "StepsUtil.h"
#include "Foreach.h"
#include "CommonMetrics.h"
#include "BannerCache.h"
#include "UnlockManager.h"
#include "SongUtil.h"


static const CString EditMenuRowNames[] = {
	"Group",
	"Song",
	"StepsType",
	"Steps",
	"SourceStepsType",
	"SourceSteps",
	"Action"
};
XToString( EditMenuRow, NUM_EDIT_MENU_ROWS );
XToThemedString( EditMenuRow, NUM_EDIT_MENU_ROWS );

static const CString EditMenuActionNames[] = {
	"Edit",
	"Delete",
	"Create",
};
XToString( EditMenuAction, NUM_EDIT_MENU_ACTIONS );
XToThemedString( EditMenuAction, NUM_EDIT_MENU_ACTIONS );
StringToX( EditMenuAction );

static CString ARROWS_X_NAME( size_t i )	{ return ssprintf("Arrows%dX",int(i+1)); }
static CString ROW_VALUE_X_NAME( size_t i )	{ return ssprintf("RowValue%dX",int(i+1)); }
static CString ROW_Y_NAME( size_t i )		{ return ssprintf("Row%dY",int(i+1)); }

static ThemeMetric1D<float> ARROWS_X					("EditMenu",ARROWS_X_NAME,NUM_ARROWS);
static ThemeMetric<RageColor> ARROWS_ENABLED_COLOR		("EditMenu","ArrowsEnabledColor");
static ThemeMetric<RageColor> ARROWS_DISABLED_COLOR		("EditMenu","ArrowsDisabledColor");
static ThemeMetric<float> SONG_BANNER_WIDTH				("EditMenu","SongBannerWidth");
static ThemeMetric<float> SONG_BANNER_HEIGHT			("EditMenu","SongBannerHeight");
static ThemeMetric<float> GROUP_BANNER_WIDTH			("EditMenu","GroupBannerWidth");
static ThemeMetric<float> GROUP_BANNER_HEIGHT			("EditMenu","GroupBannerHeight");
static ThemeMetric<float> ROW_LABELS_X					("EditMenu","RowLabelsX");
static ThemeMetric<apActorCommands> ROW_LABEL_ON_COMMAND("EditMenu","RowLabelOnCommand");
static ThemeMetric1D<float> ROW_VALUE_X					("EditMenu",ROW_VALUE_X_NAME,NUM_EDIT_MENU_ROWS);
static ThemeMetric<apActorCommands> ROW_VALUE_ON_COMMAND("EditMenu","RowValueOnCommand");
static ThemeMetric1D<float> ROW_Y						("EditMenu",ROW_Y_NAME,NUM_EDIT_MENU_ROWS);


static void GetSongsToShowForGroup( const CString &sGroup, vector<Song*> &vpSongsOut )
{
	vpSongsOut.clear();
	SONGMAN->GetSongs( vpSongsOut, sGroup );
	switch( EDIT_MODE.GetValue() )
	{
	case EDIT_MODE_PRACTICE:
	case EDIT_MODE_HOME:
		for( int i=vpSongsOut.size()-1; i>=0; i-- )
		{
			const Song* pSong = vpSongsOut[i];
			if( UNLOCKMAN->SongIsLocked(pSong)  ||  pSong->IsTutorial() )
				vpSongsOut.erase( vpSongsOut.begin()+i );
		}
		break;
	case EDIT_MODE_FULL:
		break;
	default:
		ASSERT(0);
	}
	SongUtil::SortSongPointerArrayByTitle( vpSongsOut );
}
static void GetGroupsToShow( vector<CString> &vsGroupsOut )
{
	vsGroupsOut.clear();
	SONGMAN->GetGroupNames( vsGroupsOut );
	for( int i = vsGroupsOut.size()-1; i>=0; i-- )
	{
		const CString &sGroup = vsGroupsOut[i];
		vector<Song*> vpSongs;
		GetSongsToShowForGroup( sGroup, vpSongs );
		// strip groups that have no unlocked songs
		if( vpSongs.empty() )
			vsGroupsOut.erase( vsGroupsOut.begin()+i );
	}
}

EditMenu::EditMenu()
{
	LOG->Trace( "EditMenu::EditMenu()" );

	SetName( "EditMenu" );

	for( int i=0; i<2; i++ )
	{
		m_sprArrows[i].Load( THEME->GetPathG("EditMenu",i==0?"left":"right") );
		m_sprArrows[i].SetX( ARROWS_X.GetValue(i) );
		this->AddChild( &m_sprArrows[i] );
	}

	m_SelectedRow = (EditMenuRow)0;

	ZERO( m_iSelection );


	FOREACH_EditMenuRow( r )
	{
		m_textLabel[r].LoadFromFont( THEME->GetPathF("EditMenu","title") );
		m_textLabel[r].SetXY( ROW_LABELS_X, ROW_Y.GetValue(r) );
		m_textLabel[r].SetText( EditMenuRowToThemedString(r) );
		m_textLabel[r].RunCommands( ROW_LABEL_ON_COMMAND );
		m_textLabel[r].SetHorizAlign( Actor::align_left );
		this->AddChild( &m_textLabel[r] );

		m_textValue[r].LoadFromFont( THEME->GetPathF("EditMenu","value") );
		m_textValue[r].SetXY( ROW_VALUE_X.GetValue(r), ROW_Y.GetValue(r) );
		m_textValue[r].SetText( "blah" );
		m_textValue[r].RunCommands( ROW_VALUE_ON_COMMAND );
		this->AddChild( &m_textValue[r] );
	}

	/* Load low-res banners, if needed. */
	BANNERCACHE->Demand();

	m_GroupBanner.SetName( "GroupBanner" );
	SET_XY( m_GroupBanner );
	this->AddChild( &m_GroupBanner );

	m_SongBanner.SetName( "SongBanner" );
	SET_XY( m_SongBanner );
	this->AddChild( &m_SongBanner );

	m_SongTextBanner.SetName( "SongTextBanner" );
	m_SongTextBanner.Load( "TextBanner" );
	SET_XY( m_SongTextBanner );
	this->AddChild( &m_SongTextBanner );
	
	m_Meter.SetName( "Meter" );
	m_Meter.Load( "EditDifficultyMeter" );
	SET_XY( m_Meter );
	this->AddChild( &m_Meter );
	
	m_SourceMeter.SetName( "SourceMeter" );
	m_SourceMeter.Load( "EditDifficultyMeter" );
	SET_XY( m_SourceMeter );
	this->AddChild( &m_SourceMeter );


	m_soundChangeRow.Load( THEME->GetPathS("EditMenu","row") );
	m_soundChangeValue.Load( THEME->GetPathS("EditMenu","value") );


	//
	// fill in data structures
	//
	GetGroupsToShow( m_sGroups );
	m_StepsTypes = STEPS_TYPES_TO_SHOW.GetValue();


	RefreshAll();
}

EditMenu::~EditMenu()
{
	BANNERCACHE->Undemand();
}

void EditMenu::RefreshAll()
{
	ChangeToRow( (EditMenuRow)0 );
	OnRowValueChanged( (EditMenuRow)0 );

	// Select the current song if any
	if( GAMESTATE->m_pCurSong )
	{
		for( unsigned i=0; i<m_sGroups.size(); i++ )
			if( GAMESTATE->m_pCurSong->m_sGroupName == m_sGroups[i] )
				m_iSelection[ROW_GROUP] = i;
		OnRowValueChanged( ROW_GROUP );

		for( unsigned i=0; i<m_pSongs.size(); i++ )
			if( GAMESTATE->m_pCurSong == m_pSongs[i] )
				m_iSelection[ROW_SONG] = i;
		OnRowValueChanged( ROW_SONG );

		// Select the current StepsType and difficulty if any
		if( GAMESTATE->m_pCurSteps[PLAYER_1] )
		{
			for( unsigned i=0; i<m_StepsTypes.size(); i++ )
			{
				if( m_StepsTypes[i] == GAMESTATE->m_pCurSteps[PLAYER_1]->m_StepsType )
				{
					m_iSelection[ROW_STEPS_TYPE] = i;
					OnRowValueChanged( ROW_STEPS_TYPE );
				}
			}

			for( unsigned i=0; i<m_vpSteps.size(); i++ )
			{
				const Steps *pSteps = m_vpSteps[i].pSteps;
				if( pSteps == GAMESTATE->m_pCurSteps[PLAYER_1] )
				{
					m_iSelection[ROW_STEPS] = i;
					OnRowValueChanged( ROW_STEPS );
				}
			}
		}
	}
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
	return m_SelectedRow != NUM_EDIT_MENU_ROWS-1;
}

bool EditMenu::CanGoLeft()
{
	return m_iSelection[m_SelectedRow] != 0;
}

bool EditMenu::CanGoRight()
{
	int num_values[NUM_EDIT_MENU_ROWS] = 
	{
		m_sGroups.size(),
		m_pSongs.size(),
		m_StepsTypes.size(),
		m_vpSteps.size(),
		m_StepsTypes.size(),
		m_vpSourceSteps.size(),
		m_Actions.size()
	};

	return m_iSelection[m_SelectedRow] != (num_values[m_SelectedRow]-1);
}

void EditMenu::Up()
{
	if( CanGoUp() )
	{
		if( GetSelectedSteps() && m_SelectedRow==ROW_ACTION )
			ChangeToRow( ROW_STEPS );
		else
			ChangeToRow( EditMenuRow(m_SelectedRow-1) );
		m_soundChangeRow.PlayRandom();
	}
}

void EditMenu::Down()
{
	if( CanGoDown() )
	{
		if( GetSelectedSteps() && m_SelectedRow==ROW_STEPS )
			ChangeToRow( ROW_ACTION );
		else
			ChangeToRow( EditMenuRow(m_SelectedRow+1) );
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


void EditMenu::ChangeToRow( EditMenuRow newRow )
{
	m_SelectedRow = newRow;

	for( int i=0; i<2; i++ )
		m_sprArrows[i].SetY( ROW_Y.GetValue(newRow) );
	UpdateArrows();
}

void EditMenu::UpdateArrows()
{
	m_sprArrows[0].SetDiffuse( CanGoLeft() ? ARROWS_ENABLED_COLOR : ARROWS_DISABLED_COLOR );
	m_sprArrows[1].SetDiffuse( CanGoRight()? ARROWS_ENABLED_COLOR : ARROWS_DISABLED_COLOR );
	m_sprArrows[0].EnableAnimation( CanGoLeft() );
	m_sprArrows[1].EnableAnimation( CanGoRight() );
}

void EditMenu::OnRowValueChanged( EditMenuRow row )
{
	UpdateArrows();

	switch( row )
	{
	case ROW_GROUP:
		m_textValue[ROW_GROUP].SetText( SONGMAN->ShortenGroupName(GetSelectedGroup()) );
		m_GroupBanner.LoadFromGroup( GetSelectedGroup() );
		m_GroupBanner.ScaleToClipped( GROUP_BANNER_WIDTH, GROUP_BANNER_HEIGHT );
		m_pSongs.clear();
		GetSongsToShowForGroup( GetSelectedGroup(), m_pSongs );
		m_iSelection[ROW_SONG] = 0;
		// fall through
	case ROW_SONG:
		m_textValue[ROW_SONG].SetText( "" );
		m_SongBanner.LoadFromSong( GetSelectedSong() );
		m_SongBanner.ScaleToClipped( SONG_BANNER_WIDTH, SONG_BANNER_HEIGHT );
		m_SongTextBanner.LoadFromSong( GetSelectedSong() );
		// fall through
	case ROW_STEPS_TYPE:
		m_textValue[ROW_STEPS_TYPE].SetText( GAMEMAN->StepsTypeToThemedString(GetSelectedStepsType()) );

		m_vpSteps.clear();
		
		FOREACH_Difficulty( dc )
		{
			if( dc == DIFFICULTY_EDIT )
			{
				vector<Steps*> v;
				GetSelectedSong()->GetSteps( v, GetSelectedStepsType(), DIFFICULTY_EDIT );
				StepsUtil::SortStepsByDescription( v );
				FOREACH_CONST( Steps*, v, p )
					m_vpSteps.push_back( StepsAndDifficulty(*p,dc) );

				switch( EDIT_MODE.GetValue() )
				{
				case EDIT_MODE_PRACTICE:
					break;
				case EDIT_MODE_HOME:
				case EDIT_MODE_FULL:
					m_vpSteps.push_back( StepsAndDifficulty(NULL,dc) );	// "New Edit"
					break;
				default:
					ASSERT(0);
				}
			}
			else
			{
				Steps *pSteps = GetSelectedSong()->GetStepsByDifficulty( GetSelectedStepsType(), dc );
				if( pSteps && UNLOCKMAN->StepsIsLocked( GetSelectedSong(), pSteps ) )
					pSteps = NULL;

				switch( EDIT_MODE.GetValue() )
				{
				case EDIT_MODE_HOME:
					// don't allow selecting of non-edits in HomeMode
					break;
				case EDIT_MODE_PRACTICE:
					// only show this difficulty if steps exist
					if( pSteps )
						m_vpSteps.push_back( StepsAndDifficulty(pSteps,dc) );
					break;
				case EDIT_MODE_FULL:
					// show this difficulty whether or not steps exist.
					m_vpSteps.push_back( StepsAndDifficulty(pSteps,dc) );
					break;
				default:
					ASSERT(0);
				}
			}
		}

		CLAMP( m_iSelection[ROW_STEPS], 0, m_vpSteps.size()-1 );

		// fall through
	case ROW_STEPS:
		{
			CString s;
			Steps *pSteps = GetSelectedSteps();
			if( pSteps  &&  GetSelectedDifficulty() == DIFFICULTY_EDIT )
			{
				if( pSteps->GetDescription().empty() )
					 s += "-no name-";
				else
					s += pSteps->GetDescription();
				s += " (" + DifficultyToThemedString(DIFFICULTY_EDIT) + ")";
			}
			else
			{
				s = DifficultyToThemedString(GetSelectedDifficulty());

				// UGLY.  "Edit" -> "New Edit"
				switch( EDIT_MODE.GetValue() )
				{
				case EDIT_MODE_HOME:
					s = "New " + s;
					break;
				case EDIT_MODE_PRACTICE:
				case EDIT_MODE_FULL:
					break;
				default:
					ASSERT(0);
				}
			}
			m_textValue[ROW_STEPS].SetText( s );
		}
		if( GetSelectedSteps() )
			m_Meter.SetFromSteps( GetSelectedSteps() );
		else
			m_Meter.SetFromMeterAndDifficulty( 0, GetSelectedDifficulty() );
		// fall through
	case ROW_SOURCE_STEPS_TYPE:
		m_textLabel[ROW_SOURCE_STEPS_TYPE].SetHidden( GetSelectedSteps() ? true : false );
		m_textValue[ROW_SOURCE_STEPS_TYPE].SetHidden( GetSelectedSteps() ? true : false );
		m_textValue[ROW_SOURCE_STEPS_TYPE].SetText( GAMEMAN->StepsTypeToThemedString(GetSelectedSourceStepsType()) );

		m_vpSourceSteps.clear();
		m_vpSourceSteps.push_back( StepsAndDifficulty(NULL,DIFFICULTY_INVALID) );	// "blank"
		FOREACH_Difficulty( dc )
		{
			// fill in pSteps
			if( dc != DIFFICULTY_EDIT )
			{
				Steps *pSteps = GetSelectedSong()->GetStepsByDifficulty( GetSelectedSourceStepsType(), dc );
				m_vpSourceSteps.push_back( StepsAndDifficulty(pSteps,dc) );
			}
			else
			{
				vector<Steps*> v;
				GetSelectedSong()->GetSteps( v, GetSelectedSourceStepsType(), dc );
				StepsUtil::SortStepsByDescription( v );
				FOREACH_CONST( Steps*, v, pSteps )
					m_vpSourceSteps.push_back( StepsAndDifficulty(*pSteps,dc) );
				if( v.empty() )
					m_vpSourceSteps.push_back( StepsAndDifficulty(NULL,dc) );
			}
		}
		CLAMP( m_iSelection[ROW_SOURCE_STEPS], 0, m_vpSourceSteps.size()-1 );
		// fall through
	case ROW_SOURCE_STEPS:
		{
			m_textLabel[ROW_SOURCE_STEPS].SetHidden( GetSelectedSteps() ? true : false );
			m_textValue[ROW_SOURCE_STEPS].SetHidden( GetSelectedSteps() ? true : false );
			{
				CString s;
				Steps *pSourceSteps = GetSelectedSourceSteps();
				if( GetSelectedSourceDifficulty() == DIFFICULTY_INVALID )
					s = "Blank";
				else if( pSourceSteps  &&  GetSelectedSourceDifficulty() == DIFFICULTY_EDIT )
					s = pSourceSteps->GetDescription() + " (" + DifficultyToThemedString(DIFFICULTY_EDIT) + ")";
				else
					s = DifficultyToThemedString(GetSelectedSourceDifficulty());
				m_textValue[ROW_SOURCE_STEPS].SetText( s );
			}
			bool bHideMeter = false;
			if( GetSelectedSourceDifficulty() == DIFFICULTY_INVALID )
				bHideMeter = true;
			else if( GetSelectedSourceSteps() )
				m_SourceMeter.SetFromSteps( GetSelectedSourceSteps() );
			else
				m_SourceMeter.SetFromMeterAndDifficulty( 0, GetSelectedSourceDifficulty() );
			m_SourceMeter.SetHidden( (bHideMeter || GetSelectedSteps()) );

			m_Actions.clear();
			if( GetSelectedSteps() )
			{
				m_Actions.push_back( EDIT_MENU_ACTION_EDIT );
				switch( EDIT_MODE.GetValue() )
				{
				case EDIT_MODE_PRACTICE:
					break;
				case EDIT_MODE_HOME:
				case EDIT_MODE_FULL:
					m_Actions.push_back( EDIT_MENU_ACTION_DELETE );
					break;
				default:
					ASSERT(0);
				}
			}
			else
			{
				m_Actions.push_back( EDIT_MENU_ACTION_CREATE );
			}
			m_iSelection[ROW_ACTION] = 0;
		}
		// fall through
	case ROW_ACTION:
		m_textValue[ROW_ACTION].SetText( EditMenuActionToThemedString(GetSelectedAction()) );
		break;
	default:
		ASSERT(0);	// invalid row
	}
}


/*
 * (c) 2001-2004 Chris Danford
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
