#include "global.h"

#include "ScreenOptionsCourseOverview.h"
#include "ScreenManager.h"
#include "RageUtil.h"
#include "GameState.h"
#include "OptionRowHandler.h"
#include "ProfileManager.h"
#include "ScreenMiniMenu.h"
#include "LocalizedString.h"
#include "SongManager.h"
#include "SongUtil.h"
#include "ScreenTextEntry.h"
#include "GameManager.h"
#include "Profile.h"
#include "ScreenPrompt.h"
#include "PlayerState.h"
#include "Style.h"
#include "PrefsManager.h"

#include <vector>


enum ReviewWorkoutRow
{
	ReviewWorkoutRow_Play,
	ReviewWorkoutRow_Edit,
	ReviewWorkoutRow_Shuffle,
	ReviewWorkoutRow_Save,
	NUM_ReviewWorkoutRow
};

static const MenuRowDef g_MenuRows[] =
{
	MenuRowDef( -1,	"Play",		true, EditMode_Practice, true, false, 0, nullptr ),
	MenuRowDef( -1,	"Edit Workout",	true, EditMode_Practice, true, false, 0, nullptr ),
	MenuRowDef( -1,	"Shuffle",	true, EditMode_Practice, true, false, 0, nullptr ),
	MenuRowDef( -1,	"Save",		true, EditMode_Practice, true, false, 0, nullptr ),
};

REGISTER_SCREEN_CLASS( ScreenOptionsCourseOverview );

AutoScreenMessage( SM_BackFromEnterName )

void ScreenOptionsCourseOverview::Init()
{
	if( PREFSMAN->m_iArcadeOptionsNavigation )
		SetNavigation( NAV_THREE_KEY_MENU );

	ScreenOptions::Init();

	m_soundSave.Load( THEME->GetPathS(m_sName,"Save") );
	PLAY_SCREEN.Load(m_sName,"PlayScreen");
	EDIT_SCREEN.Load(m_sName,"EditScreen");
}

void ScreenOptionsCourseOverview::BeginScreen()
{
	std::vector<OptionRowHandler*> vHands;
	FOREACH_ENUM( ReviewWorkoutRow, rowIndex )
	{
		const MenuRowDef &mr = g_MenuRows[rowIndex];
		OptionRowHandler *pHand = OptionRowHandlerUtil::MakeSimple( mr );
		vHands.push_back( pHand );
	}

	ScreenOptions::InitMenu( vHands );

	ScreenOptions::BeginScreen();

	// clear the current song in case it's set when we back out from gameplay
	GAMESTATE->m_pCurSong.Set(nullptr);
}

ScreenOptionsCourseOverview::~ScreenOptionsCourseOverview()
{

}

void ScreenOptionsCourseOverview::ImportOptions( int iRow, const std::vector<PlayerNumber> &vpns )
{
	//OptionRow &row = *m_pRows[iRow];
}

void ScreenOptionsCourseOverview::ExportOptions( int iRow, const std::vector<PlayerNumber> &vpns )
{
	OptionRow &row = *m_pRows[iRow];
	int iIndex = row.GetOneSharedSelection( true );
	RString sValue;
	if( iIndex >= 0 )
		sValue = row.GetRowDef().m_vsChoices[ iIndex ];
}

static LocalizedString ERROR_SAVING_WORKOUT	( "ScreenOptionsCourseOverview", "Error saving workout." );
static LocalizedString WORKOUT_SAVED		( "ScreenOptionsCourseOverview", "Workout saved successfully." );
void ScreenOptionsCourseOverview::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_GoToNextScreen )
	{
		int iRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
		switch( iRow )
		{
		case ReviewWorkoutRow_Play:
			EditCourseUtil::PrepareForPlay();
			SCREENMAN->SetNewScreen( PLAY_SCREEN );
			return;	// handled
		case ReviewWorkoutRow_Edit:
			SCREENMAN->SetNewScreen( EDIT_SCREEN );
			return;	// handled
		}
	}
	else if( SM == SM_BackFromEnterName )
	{
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			ASSERT( ScreenTextEntry::s_sLastAnswer != "" );	// validate should have assured this

			if( EditCourseUtil::RenameAndSave( GAMESTATE->m_pCurCourse, ScreenTextEntry::s_sLastAnswer ) )
			{
				m_soundSave.Play(true);
				SCREENMAN->SystemMessage( WORKOUT_SAVED );
			}
		}
	}

	ScreenOptions::HandleScreenMessage( SM );
}

void ScreenOptionsCourseOverview::AfterChangeValueInRow( int iRow, PlayerNumber pn )
{
	ScreenOptions::AfterChangeValueInRow( iRow, pn );
}


static LocalizedString ENTER_WORKOUT_NAME	( "ScreenOptionsCourseOverview", "Enter a name for the workout." );
void ScreenOptionsCourseOverview::ProcessMenuStart( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return;

	int iRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
	switch( iRow )
	{
	case ReviewWorkoutRow_Play:
	case ReviewWorkoutRow_Edit:
		SCREENMAN->PlayStartSound();
		this->BeginFadingOut();
		return;	// handled
	case ReviewWorkoutRow_Shuffle:
		{
			Course *pCourse = GAMESTATE->m_pCurCourse;
			std::shuffle( pCourse->m_vEntries.begin(), pCourse->m_vEntries.end(), g_RandomNumberGenerator );
			Trail *pTrail = pCourse->GetTrailForceRegenCache( GAMESTATE->m_pCurStyle->m_StepsType );
			GAMESTATE->m_pCurTrail[PLAYER_1].Set( pTrail );
			SCREENMAN->PlayStartSound();
			MESSAGEMAN->Broadcast("CurrentCourseChanged");
		}
		return;	// handled
	case ReviewWorkoutRow_Save:
		{
			bool bPromptForName = EditCourseUtil::s_bNewCourseNeedsName;
			if( bPromptForName )
			{
				ScreenTextEntry::TextEntry(
					SM_BackFromEnterName,
					ENTER_WORKOUT_NAME,
					GAMESTATE->m_pCurCourse->GetDisplayFullTitle(),
					EditCourseUtil::MAX_NAME_LENGTH,
					EditCourseUtil::ValidateEditCourseName );
			}
			else
			{
				if( EditCourseUtil::Save( GAMESTATE->m_pCurCourse ) )
				{
					m_soundSave.Play(true);
					SCREENMAN->SystemMessage( WORKOUT_SAVED );
				}
				else
				{
					SCREENMAN->PlayInvalidSound();
					SCREENMAN->SystemMessage( ERROR_SAVING_WORKOUT );
				}
			}
		}
		return;	// handled
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
