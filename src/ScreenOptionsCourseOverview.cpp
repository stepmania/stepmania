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


enum CourseOverviewRow
{
	CourseOverviewRow_Play,
	CourseOverviewRow_Edit,
	CourseOverviewRow_Shuffle,
	CourseOverviewRow_Rename,
	CourseOverviewRow_Delete,
	CourseOverviewRow_Save,
	NUM_CourseOverviewRow
};

static bool CurrentCourseIsSaved()
{
	Course *pCourse = GAMESTATE->m_pCurCourse;
	if( pCourse == nullptr )
		return false;
	return !pCourse->m_sPath.empty();
}

static const MenuRowDef g_MenuRows[] =
{
	MenuRowDef( -1,	"Play",		true, EditMode_Practice, true, false, 0, nullptr ),
	MenuRowDef( -1,	"Edit Course",	true, EditMode_Practice, true, false, 0, nullptr ),
	MenuRowDef( -1,	"Shuffle",	true, EditMode_Practice, true, false, 0, nullptr ),
	MenuRowDef( -1,	"Rename",	CurrentCourseIsSaved, EditMode_Practice, true, false, 0, nullptr ),
	MenuRowDef( -1,	"Delete",	CurrentCourseIsSaved, EditMode_Practice, true, false, 0, nullptr ),
	MenuRowDef( -1,	"Save",		true, EditMode_Practice, true, false, 0, nullptr ),
};

REGISTER_SCREEN_CLASS( ScreenOptionsCourseOverview );

static LocalizedString ENTER_COURSE_NAME        ("ScreenOptionsCourseOverview", "Enter a name for the course.");
static LocalizedString ERROR_SAVING_COURSE	("ScreenOptionsCourseOverview", "Error saving course.");
static LocalizedString COURSE_SAVED		("ScreenOptionsCourseOverview", "Course saved successfully.");
static LocalizedString ERROR_RENAMING           ("ScreenOptionsCourseOverview", "Error renaming file.");
static LocalizedString ERROR_DELETING_FILE      ("ScreenOptionsCourseOverview", "Error deleting the file '%s'.");
static LocalizedString COURSE_WILL_BE_LOST      ("ScreenOptionsCourseOverview", "This course will be lost permanently.");
static LocalizedString CONTINUE_WITH_DELETE     ("ScreenOptionsCourseOverview", "Continue with delete?");

AutoScreenMessage( SM_BackFromEnterName );
AutoScreenMessage( SM_BackFromRename );
AutoScreenMessage( SM_BackFromDelete );

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
	FOREACH_ENUM( CourseOverviewRow, rowIndex )
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

void ScreenOptionsCourseOverview::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_GoToPrevScreen )
	{
		// If we're pointing to an unsaved course, it will be inaccessible once we're back on ScreenOptionsManageCourses.
		GAMESTATE->m_pCurCourse.Set(nullptr);
	}
	else if( SM == SM_GoToNextScreen )
	{
		int iRow = m_iCurrentRow[GAMESTATE->GetMasterPlayerNumber()];
		switch( iRow )
		{
		case CourseOverviewRow_Play:
			EditCourseUtil::PrepareForPlay();
			SCREENMAN->SetNewScreen( PLAY_SCREEN );
			return;	// handled
		case CourseOverviewRow_Edit:
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
				SCREENMAN->SystemMessage( COURSE_SAVED );
			}
		}
	}
	else if( SM == SM_BackFromRename )
	{
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			ASSERT( ScreenTextEntry::s_sLastAnswer != "" ); // validate should have assured this

			if( !EditCourseUtil::RenameAndSave(GAMESTATE->m_pCurCourse, ScreenTextEntry::s_sLastAnswer) )
			{
				ScreenPrompt::Prompt( SM_None, ERROR_RENAMING );
				return;
			}

			SCREENMAN->SetNewScreen( this->m_sName ); // reload
		}
	}
	else if( SM == SM_BackFromDelete )
	{
		if( ScreenPrompt::s_LastAnswer == ANSWER_YES )
		{
			if( !EditCourseUtil::RemoveAndDeleteFile(GAMESTATE->m_pCurCourse) )
			{
				ScreenPrompt::Prompt( SM_None, ssprintf(ERROR_DELETING_FILE.GetValue(), GAMESTATE->m_pCurCourse->m_sPath.c_str()) );
				return;
			}

			GAMESTATE->m_pCurCourse.Set(nullptr);
			GAMESTATE->m_pCurTrail[PLAYER_1].Set(nullptr);

			/* Our course is gone, so back out. */
			StartTransitioningScreen( SM_GoToPrevScreen );
		}
	}

	ScreenOptions::HandleScreenMessage( SM );
}

void ScreenOptionsCourseOverview::AfterChangeValueInRow( int iRow, PlayerNumber pn )
{
	ScreenOptions::AfterChangeValueInRow( iRow, pn );
}


void ScreenOptionsCourseOverview::ProcessMenuStart( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return;

	int iRow = m_iCurrentRow[GAMESTATE->GetMasterPlayerNumber()];
	switch( iRow )
	{
	case CourseOverviewRow_Play:
	case CourseOverviewRow_Edit:
		SCREENMAN->PlayStartSound();
		this->BeginFadingOut();
		return;	// handled
	case CourseOverviewRow_Shuffle:
		{
			Course *pCourse = GAMESTATE->m_pCurCourse;
			std::shuffle( pCourse->m_vEntries.begin(), pCourse->m_vEntries.end(), g_RandomNumberGenerator );
			Trail *pTrail = pCourse->GetTrailForceRegenCache( GAMESTATE->GetCurrentStyle(input.pn)->m_StepsType );
			GAMESTATE->m_pCurTrail[PLAYER_1].Set( pTrail );
			SCREENMAN->PlayStartSound();
			MESSAGEMAN->Broadcast("CurrentCourseChanged");
		}
		return;	// handled
	case CourseOverviewRow_Rename:
		ScreenTextEntry::TextEntry(
				SM_BackFromRename,
				ENTER_COURSE_NAME,
				GAMESTATE->m_pCurCourse->GetDisplayFullTitle(),
				EditCourseUtil::MAX_NAME_LENGTH,
				EditCourseUtil::ValidateEditCourseName );
		break;
	case CourseOverviewRow_Delete:
		ScreenPrompt::Prompt( SM_BackFromDelete, COURSE_WILL_BE_LOST.GetValue()+"\n\n"+CONTINUE_WITH_DELETE.GetValue(), PROMPT_YES_NO, ANSWER_NO );
		break;
	case CourseOverviewRow_Save:
		{
			bool bPromptForName = EditCourseUtil::s_bNewCourseNeedsName;
			if( bPromptForName )
			{
				ScreenTextEntry::TextEntry(
					SM_BackFromEnterName,
					ENTER_COURSE_NAME,
					GAMESTATE->m_pCurCourse->GetDisplayFullTitle(),
					EditCourseUtil::MAX_NAME_LENGTH,
					EditCourseUtil::ValidateEditCourseName );
			}
			else
			{
				if( EditCourseUtil::Save( GAMESTATE->m_pCurCourse ) )
				{
					m_soundSave.Play(true);
					SCREENMAN->SystemMessage( COURSE_SAVED );
				}
				else
				{
					SCREENMAN->PlayInvalidSound();
					SCREENMAN->SystemMessage( ERROR_SAVING_COURSE );
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
