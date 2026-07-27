#include "global.h"

#include "ScreenOptionsEditCourse.h"
#include "ScreenMiniMenu.h"
#include "SongUtil.h"
#include "SongManager.h"
#include "OptionRowHandler.h"
#include "Song.h"
#include "GameState.h"
#include "ScreenPrompt.h"
#include "LocalizedString.h"
#include "CourseUtil.h"
#include "Song.h"
#include "Style.h"
#include "Steps.h"

#include <vector>


static void GetStepsForSong( Song *pSong, std::vector<Steps*> &vpStepsOut )
{
	SongUtil::GetSteps( pSong, vpStepsOut, GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber())->m_StepsType );
	// xxx: If the StepsType isn't valid for the current game, this will cause
	// a crash when changing songs. -aj
	StepsUtil::RemoveLockedSteps( pSong, vpStepsOut );
	StepsUtil::SortNotesArrayByDifficulty( vpStepsOut );
}

// XXX: very similar to OptionRowHandlerSteps
class EditCourseOptionRowHandlerSteps : public OptionRowHandler
{
public:
	void Load( int iEntryIndex )
	{
		m_iEntryIndex = iEntryIndex;
	}
	virtual ReloadChanged Reload()
	{
		m_Def.m_vsChoices.clear();
		m_vpSteps.clear();

		Song *pSong = GAMESTATE->m_pCurSong;
		if( pSong ) // playing a song
		{
			GetStepsForSong( pSong, m_vpSteps );
			for (Steps const *steps : m_vpSteps)
			{
				RString s;
				if( steps->GetDifficulty() == Difficulty_Edit )
					s = steps->GetDescription();
				else
					s = CustomDifficultyToLocalizedString( StepsToCustomDifficulty(steps) );
				s += ssprintf( " %d", steps->GetMeter() );
				m_Def.m_vsChoices.push_back( s );
			}
			m_Def.m_vEnabledForPlayers.clear();
			m_Def.m_vEnabledForPlayers.insert( PLAYER_1 );
		}
		else
		{
			m_Def.m_vsChoices.push_back( "n/a" );
			m_vpSteps.push_back(nullptr);
			m_Def.m_vEnabledForPlayers.clear();
		}


		return RELOAD_CHANGED_ALL;
	}
	virtual void ImportOption( OptionRow *pRow, const std::vector<PlayerNumber> &vpns, std::vector<bool> vbSelectedOut[NUM_PLAYERS] ) const
	{
		Trail *pTrail = GAMESTATE->m_pCurTrail[PLAYER_1];
		Steps *pSteps;
		if( pTrail )
		{
			if( m_iEntryIndex < (int)pTrail->m_vEntries.size() )
				pSteps = pTrail->m_vEntries[ m_iEntryIndex ].pSteps;
		}

		std::vector<Steps*>::const_iterator iter = find( m_vpSteps.begin(), m_vpSteps.end(), pSteps );
		if( iter == m_vpSteps.end() )
		{
			pRow->SetOneSharedSelection( 0 );
		}
		else
		{
			int index = iter - m_vpSteps.begin();
			pRow->SetOneSharedSelection( index );
		}

	}
	virtual int ExportOption( const std::vector<PlayerNumber> &vpns, const std::vector<bool> vbSelected[NUM_PLAYERS] ) const
	{
		return 0;
	}
	Steps *GetSteps( int iStepsIndex ) const
	{
		return m_vpSteps[iStepsIndex];
	}

protected:
	int	m_iEntryIndex;
	std::vector<Steps*> m_vpSteps;
};



const int NUM_SONG_ROWS = 20;

REGISTER_SCREEN_CLASS( ScreenOptionsEditCourse );

enum EditCourseRow
{
	EditCourseRow_Minutes,
	NUM_EditCourseRow
};

enum RowType
{
	RowType_Song,
	RowType_Steps,
	NUM_RowType,
	RowType_Invalid,
};
static int RowToEntryIndex( int iRow )
{
	if( iRow < NUM_EditCourseRow )
		return -1;

	return (iRow-NUM_EditCourseRow)/NUM_RowType;
}
static RowType RowToRowType( int iRow )
{
	if( iRow < NUM_EditCourseRow )
		return RowType_Invalid;
	return (RowType)((iRow-NUM_EditCourseRow) % NUM_RowType);
}
static int EntryIndexAndRowTypeToRow( int iEntryIndex, RowType rowType )
{
	return NUM_EditCourseRow + iEntryIndex*NUM_RowType + rowType;
}

void ScreenOptionsEditCourse::Init()
{
	ScreenOptions::Init();

	SongCriteria sc;
	sc.m_Selectable = SongCriteria::Selectable_Yes;
	sc.m_Tutorial = SongCriteria::Tutorial_No;
	sc.m_Locked = SongCriteria::Locked_Unlocked;

	SongUtil::FilterSongs( sc, SONGMAN->GetAllSongs(), m_vpSongs, true );

	SongUtil::SortSongPointerArrayByTitle( m_vpSongs );
}

const MenuRowDef g_MenuRows[] =
{
	MenuRowDef( -1,	"Max Minutes",	true, EditMode_Practice, true, false, 0, nullptr ),
};

static LocalizedString EMPTY	("ScreenOptionsEditCourse","-Empty-");
static LocalizedString SONG	("ScreenOptionsEditCourse","Song");
static LocalizedString STEPS	("ScreenOptionsEditCourse","Steps");
static LocalizedString MINUTES	("ScreenOptionsEditCourse","minutes");

static RString MakeMinutesString( int mins )
{
	if( mins == 0 )
		return "No Cut-off";
	return ssprintf( "%d", mins ) + " " + MINUTES.GetValue();
}

void ScreenOptionsEditCourse::BeginScreen()
{
	std::vector<OptionRowHandler*> vHands;

	FOREACH_ENUM( EditCourseRow, rowIndex )
	{
		const MenuRowDef &mr = g_MenuRows[rowIndex];
		OptionRowHandler *pHand = OptionRowHandlerUtil::MakeSimple( mr );

		pHand->m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
		pHand->m_Def.m_vsChoices.clear();

		switch( rowIndex )
		{
		DEFAULT_FAIL(rowIndex);
		case EditCourseRow_Minutes:
			pHand->m_Def.m_vsChoices.push_back( MakeMinutesString(0) );
			for( int i=EditCourseUtil::MIN_WORKOUT_MINUTES; i<=20; i+=2 )
				pHand->m_Def.m_vsChoices.push_back( MakeMinutesString(i) );
			for( int i=20; i<=EditCourseUtil::MAX_WORKOUT_MINUTES; i+=5 )
				pHand->m_Def.m_vsChoices.push_back( MakeMinutesString(i) );
			break;
		}

		pHand->m_Def.m_bExportOnChange = true;
		vHands.push_back( pHand );
	}



	for( int i=0; i<NUM_SONG_ROWS; i++ )
	{
		{
			MenuRowDef mrd = MenuRowDef( -1, "---", true, EditMode_Practice, true, false, 0, EMPTY.GetValue() );
			for (Song const *s : m_vpSongs)
				mrd.choices.push_back( s->GetDisplayFullTitle() );
			mrd.sName = ssprintf(SONG.GetValue() + " %d",i+1);
			OptionRowHandler *pHand = OptionRowHandlerUtil::MakeSimple( mrd );
			pHand->m_Def.m_bAllowThemeTitle = false;	// already themed
			pHand->m_Def.m_sExplanationName = "Choose Song";
			pHand->m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
			pHand->m_Def.m_bExportOnChange = true;
			vHands.push_back( pHand );
		}

		{
			EditCourseOptionRowHandlerSteps *pHand = new EditCourseOptionRowHandlerSteps;
			pHand->Load( i );
			pHand->m_Def.m_vsChoices.push_back( "n/a" );
			pHand->m_Def.m_sName = ssprintf(STEPS.GetValue() + " %d",i+1);
			pHand->m_Def.m_bAllowThemeTitle = false;	// already themed
			pHand->m_Def.m_bAllowThemeItems = false;	// already themed
			pHand->m_Def.m_sExplanationName = "Choose Steps";
			pHand->m_Def.m_bOneChoiceForAllPlayers = true;
			pHand->m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
			pHand->m_Def.m_bExportOnChange = true;
			vHands.push_back( pHand );
		}

	}

	ScreenOptions::InitMenu( vHands );

	ScreenOptions::BeginScreen();


	for( int i=0; i<(int)m_pRows.size(); i++ )
	{
		OptionRow *pRow = m_pRows[i];
		m_iCurrentRow[PLAYER_1] = i;
		this->SetCurrentSong();
		pRow->Reload();
	}
	m_iCurrentRow[PLAYER_1] = 0;

	this->SetCurrentSong();

	//this->AfterChangeRow( PLAYER_1 );
}

ScreenOptionsEditCourse::~ScreenOptionsEditCourse()
{

}

void ScreenOptionsEditCourse::ImportOptions( int iRow, const std::vector<PlayerNumber> &vpns )
{
	OptionRow &row = *m_pRows[iRow];
	if( row.GetRowType() == OptionRow::RowType_Exit )
		return;

	switch( iRow )
	{
	case EditCourseRow_Minutes:
		row.SetOneSharedSelection( 0 );
		row.SetOneSharedSelectionIfPresent( MakeMinutesString(static_cast<int>(GAMESTATE->m_pCurCourse->m_fGoalSeconds)/60) );
		break;
	default:
		{
			int iEntryIndex = RowToEntryIndex( iRow );
			RowType rowType = RowToRowType( iRow );

			switch( rowType )
			{
			DEFAULT_FAIL( rowType );
			case RowType_Song:
				{
					Song *pSong = nullptr;
					if( iEntryIndex < (int)GAMESTATE->m_pCurCourse->m_vEntries.size() )
						pSong = GAMESTATE->m_pCurCourse->m_vEntries[iEntryIndex].songID.ToSong();

					std::vector<Song*>::iterator iter = find( m_vpSongs.begin(), m_vpSongs.end(), pSong );
					if( iter == m_vpSongs.end() )
						row.SetOneSharedSelection( 0 );
					else
						row.SetOneSharedSelection( 1 + iter - m_vpSongs.begin() );
				}
				break;
			case RowType_Steps:
				// the OptionRowHandler does its own importing
				break;
			}
		}
		break;
	}
}

void ScreenOptionsEditCourse::ExportOptions( int iRow, const std::vector<PlayerNumber> &vpns )
{
	FOREACH_ENUM( EditCourseRow, i )
	{
		OptionRow &row = *m_pRows[i];
		int iIndex = row.GetOneSharedSelection( true );
		RString sValue;
		if( iIndex >= 0 )
			sValue = row.GetRowDef().m_vsChoices[ iIndex ];

		switch( i )
		{
		DEFAULT_FAIL(i);
		case EditCourseRow_Minutes:
			GAMESTATE->m_pCurCourse->m_fGoalSeconds = 0;
			int mins;
			if( sscanf( sValue, "%d", &mins ) == 1 )
				GAMESTATE->m_pCurCourse->m_fGoalSeconds = float(mins * 60);
			break;
		}
	}

	GAMESTATE->m_pCurCourse->m_vEntries.clear();

	for( int i=NUM_EditCourseRow; i<(int)m_pRows.size(); i++ )
	{
		OptionRow &row = *m_pRows[i];
		if( row.GetRowType() == OptionRow::RowType_Exit )
			continue;

		RowType rowType = RowToRowType( i );
		int iEntryIndex = RowToEntryIndex( i );

		switch( rowType )
		{
		case RowType_Song:
			{
				Song *pSong = this->GetSongForEntry( iEntryIndex );
				if( pSong )
				{
					Steps *pSteps = this->GetStepsForEntry( iEntryIndex );
					ASSERT_M( pSteps != nullptr, "No Steps for this Song!" );
					CourseEntry ce;
					ce.songID.FromSong( pSong );
					ce.stepsCriteria.m_difficulty = pSteps->GetDifficulty();
					GAMESTATE->m_pCurCourse->m_vEntries.push_back( ce );
				}
			}
			break;
		case RowType_Steps:
			// push each CourseEntry when we handle each RowType_Song above
			break;
		default:
			break;
		}
	}

	EditCourseUtil::UpdateAndSetTrail();
}

void ScreenOptionsEditCourse::GoToNextScreen()
{
}

void ScreenOptionsEditCourse::GoToPrevScreen()
{
}

void ScreenOptionsEditCourse::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_ExportOptions )
	{
		//g_Workout.m_vpSongs.clear();
	}

	ScreenOptions::HandleScreenMessage( SM );
}

void ScreenOptionsEditCourse::SetCurrentSong()
{
	int iRow = m_iCurrentRow[PLAYER_1];
	OptionRow &row = *m_pRows[iRow];

	if( row.GetRowType() == OptionRow::RowType_Exit )
	{
		GAMESTATE->m_pCurSong.Set(nullptr);
		GAMESTATE->m_pCurSteps[PLAYER_1].Set(nullptr);
	}
	else
	{
		iRow = m_iCurrentRow[PLAYER_1];
		int iEntryIndex = RowToEntryIndex( iRow );
		Song *pSong = nullptr;
		if( iEntryIndex != -1 )
		{
			int iCurrentSongRow = EntryIndexAndRowTypeToRow(iEntryIndex,RowType_Song);
			OptionRow &oRow = *m_pRows[ iCurrentSongRow ];
			int index = oRow.GetOneSelection(PLAYER_1);
			if( index != 0 )
				pSong = m_vpSongs[ index - 1 ];
		}
		if ( pSong != nullptr )
		{
			GAMESTATE->m_pCurSong.Set( pSong );
		}
	}
}

void ScreenOptionsEditCourse::SetCurrentSteps()
{
	Song *pSong = GAMESTATE->m_pCurSong;
	if( pSong )
	{
		int iRow = m_iCurrentRow[PLAYER_1];
		int iEntryIndex = RowToEntryIndex( iRow );
		OptionRow &row = *m_pRows[ EntryIndexAndRowTypeToRow(iEntryIndex, RowType_Steps) ];
		int iStepsIndex = row.GetOneSharedSelection();
		const EditCourseOptionRowHandlerSteps *pHand = dynamic_cast<const EditCourseOptionRowHandlerSteps *>( row.GetHandler() );
		ASSERT( pHand != nullptr );
		Steps *pSteps = pHand->GetSteps( iStepsIndex );
		GAMESTATE->m_pCurSteps[PLAYER_1].Set( pSteps );
	}
	else
	{
		GAMESTATE->m_pCurSteps[PLAYER_1].Set(nullptr);
	}
}

Song *ScreenOptionsEditCourse::GetSongForEntry( int iEntryIndex )
{
	int iRow = EntryIndexAndRowTypeToRow( iEntryIndex, RowType_Song );
	OptionRow &row = *m_pRows[iRow];

	int index = row.GetOneSharedSelection();
	if( index == 0 )
		return nullptr;
	return m_vpSongs[ index - 1 ];
}

Steps *ScreenOptionsEditCourse::GetStepsForEntry( int iEntryIndex )
{
	int iRow = EntryIndexAndRowTypeToRow( iEntryIndex, RowType_Steps );
	OptionRow &row = *m_pRows[iRow];
	int index = row.GetOneSharedSelection();
	Song *pSong = GetSongForEntry( iEntryIndex );
	std::vector<Steps*> vpSteps;
	GetStepsForSong( pSong, vpSteps );
	return vpSteps[index];
}

void ScreenOptionsEditCourse::AfterChangeRow( PlayerNumber pn )
{
	ScreenOptions::AfterChangeRow( pn );

	const int iCurRow = m_iCurrentRow[pn];
	// only do this if it's not the first row. -aj
	if( iCurRow > 0 )
	{
		SetCurrentSong();
		SetCurrentSteps();
	}
}

void ScreenOptionsEditCourse::AfterChangeValueInRow( int iRow, PlayerNumber pn )
{
	ScreenOptions::AfterChangeValueInRow( iRow, pn );

	int iEntryIndex = RowToEntryIndex( iRow );
	RowType rowType = RowToRowType( iRow );
	switch( rowType )
	{
	case RowType_Song:
		{
			SetCurrentSong();
			OptionRow &row = *m_pRows[ EntryIndexAndRowTypeToRow(iEntryIndex, RowType_Steps) ];
			row.Reload();
		}
		break;
	case RowType_Steps:
		SetCurrentSteps();
		break;
	default:
		break;
	}
}

const int MIN_ENABLED_SONGS = 2;

static LocalizedString MUST_ENABLE_AT_LEAST("ScreenOptionsEditCourse","You must enable at least %d songs.");
void ScreenOptionsEditCourse::ProcessMenuStart( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return;

	int iRow = m_iCurrentRow[GAMESTATE->GetMasterPlayerNumber()];

	unsigned iSongCount = GAMESTATE->m_pCurCourse->m_vEntries.size();

	if( m_pRows[iRow]->GetRowType() == OptionRow::RowType_Exit  &&  iSongCount < unsigned(MIN_ENABLED_SONGS) )
	{
		ScreenPrompt::Prompt( SM_None, ssprintf(MUST_ENABLE_AT_LEAST.GetValue(),MIN_ENABLED_SONGS) );
		return;
	}

	ScreenOptions::ProcessMenuStart( input );
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
