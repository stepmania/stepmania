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

	for( int i=0; i<2; i++ )
	{
		m_sprArrows[i].Load( THEME->GetPathG("EditMenu",i==0?"left":"right") );
		m_sprArrows[i].SetX( ARROWS_X(i) );
		this->AddChild( &m_sprArrows[i] );
	}

	m_SelectedRow = (Row)0;

	ZERO( m_iSelection );

	
	// start out on easy, not beginner
	m_iSelection[ROW_STEPS] = DIFFICULTY_EASY;
	m_iSelection[ROW_SOURCE_STEPS] = DIFFICULTY_EASY;



	for( int i=0; i<NUM_ROWS; i++ )
	{
		m_textLabel[i].LoadFromFont( THEME->GetPathF("EditMenu","title") );
		m_textLabel[i].SetXY( ROW_LABELS_X, ROW_Y(i) );
		m_textLabel[i].SetText( RowToString((Row)i) );
		m_textLabel[i].SetZoom( 0.8f );
		m_textLabel[i].SetHorizAlign( Actor::align_left );
		this->AddChild( &m_textLabel[i] );

		m_textValue[i].LoadFromFont( THEME->GetPathF("EditMenu","value") );
		m_textValue[i].SetXY( ROW_VALUE_X(i), ROW_Y(i) );
		m_textValue[i].SetText( "blah" );
		m_textValue[i].SetZoom( 0.8f );
		this->AddChild( &m_textValue[i] );
	}

	m_GroupBanner.SetXY( GROUP_BANNER_X, GROUP_BANNER_Y );
	this->AddChild( &m_GroupBanner );

	m_SongBanner.SetXY( SONG_BANNER_X, SONG_BANNER_Y );
	this->AddChild( &m_SongBanner );

	m_SongTextBanner.SetName( "TextBanner" );
	m_SongTextBanner.SetXY( SONG_TEXT_BANNER_X, SONG_TEXT_BANNER_Y );
	this->AddChild( &m_SongTextBanner );
	
	m_Meter.SetName( "DifficultyMeter" );
	m_Meter.SetXY( METER_X, METER_Y );
	m_Meter.Load( "EditDifficultyMeter" );
	this->AddChild( &m_Meter );
	
	m_SourceMeter.SetName( "DifficultyMeter" );
	m_SourceMeter.SetXY( SOURCE_METER_X, SOURCE_METER_Y );
	m_SourceMeter.Load( "EditDifficultyMeter" );
	this->AddChild( &m_SourceMeter );
	

	m_soundChangeRow.Load( THEME->GetPathS("EditMenu","row") );
	m_soundChangeValue.Load( THEME->GetPathS("EditMenu","value") );


	// fill in data structures
	SONGMAN->GetGroupNames( m_sGroups );
	GAMEMAN->GetStepsTypesForGame( GAMESTATE->m_pCurGame, m_StepsTypes );
	
	ChangeToRow( (Row)0 );
	OnRowValueChanged( (Row)0 );

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

			vector<Steps*>::const_iterator iter = find( m_vpSteps.begin(), m_vpSteps.end(), GAMESTATE->m_pCurSteps[PLAYER_1] );
			if( iter != m_vpSteps.end() )
			{
				m_iSelection[ROW_STEPS] = m_vpSteps.begin() - iter;
				OnRowValueChanged( ROW_STEPS );
			}
		}
	}
}

EditMenu::~EditMenu()
{

}

void EditMenu::RefreshSteps()
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
			ChangeToRow( Row(m_SelectedRow-1) );
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
	case ROW_STEPS_TYPE:
		m_textValue[ROW_STEPS_TYPE].SetText( GAMEMAN->StepsTypeToThemedString(GetSelectedStepsType()) );
		CLAMP( m_iSelection[ROW_STEPS], 0, NUM_DIFFICULTIES-1 );	// jump back to the slot for DIFFICULTY_EDIT
		m_vpSteps.clear();
		GetSelectedSong()->GetSteps( m_vpSteps, GetSelectedStepsType() );
		StepsUtil::SortStepsByDescription( m_vpSteps );
		StepsUtil::SortStepsByTypeAndDifficulty( m_vpSteps );
		FOREACH_Difficulty( dc )
		{
			Steps *pSteps = dc < m_vpSteps.size() ? m_vpSteps[dc] : NULL;
			if( pSteps == NULL || pSteps->GetDifficulty() != dc )
				m_vpSteps.insert( m_vpSteps.begin()+dc, NULL );
		}
		if( GetSelectedSong()->HasEdits(GetSelectedStepsType()) )
			m_vpSteps.push_back( NULL );	// for "New Edit"
		// fall through
	case ROW_STEPS:
		{
			CString s = DifficultyToThemedString(GetSelectedDifficulty());
			Steps *pSteps = GetSelectedSteps();
			if( GetSelectedDifficulty() == DIFFICULTY_EDIT )
				s += " - " + (pSteps ? pSteps->GetDescription() : "New" );
			m_textValue[ROW_STEPS].SetText( s );
			m_Meter.SetFromSteps( GetSelectedSteps() );
		}
		// fall through
	case ROW_SOURCE_STEPS_TYPE:
		m_textLabel[ROW_SOURCE_STEPS_TYPE].SetHidden( GetSelectedSteps() ? true : false );
		m_textValue[ROW_SOURCE_STEPS_TYPE].SetHidden( GetSelectedSteps() ? true : false );
		m_textValue[ROW_SOURCE_STEPS_TYPE].SetText( GAMEMAN->StepsTypeToThemedString(GetSelectedSourceStepsType()) );

		CLAMP( m_iSelection[ROW_SOURCE_STEPS], 0, NUM_DIFFICULTIES-1 );	// jump back to the slot for DIFFICULTY_EDIT
		m_vpSourceSteps.clear();
		GetSelectedSong()->GetSteps( m_vpSourceSteps, GetSelectedSourceStepsType() );
		StepsUtil::SortStepsByDescription( m_vpSourceSteps );
		StepsUtil::SortStepsByTypeAndDifficulty( m_vpSourceSteps );
		FOREACH_Difficulty( dc )
		{
			Steps *pSteps = dc < m_vpSourceSteps.size() ? m_vpSourceSteps[dc] : NULL;
			if( pSteps == NULL || pSteps->GetDifficulty() != dc )
				m_vpSourceSteps.insert( m_vpSourceSteps.begin()+dc, NULL );
		}

		// fall through
	case ROW_SOURCE_STEPS:
		m_textLabel[ROW_SOURCE_STEPS].SetHidden( GetSelectedSteps() ? true : false );
		m_textValue[ROW_SOURCE_STEPS].SetHidden( GetSelectedSteps() ? true : false );
		m_textValue[ROW_SOURCE_STEPS].SetText( DifficultyToThemedString(GetSelectedSourceDifficulty()) );
		m_SourceMeter.SetFromSteps( GetSelectedSourceSteps() );
		m_SourceMeter.SetHidden( GetSelectedSteps() ? true : false );

		m_Actions.clear();
		if( GetSelectedSteps() )
		{
			m_Actions.push_back( ACTION_EDIT );
			m_Actions.push_back( ACTION_DELETE );
		}
		else if( GetSelectedSourceSteps() )
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
