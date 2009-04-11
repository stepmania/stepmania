#include "global.h"
#include "WorkoutManager.h"
#include "SongUtil.h"
#include "SongManager.h"
#include "RageFileManager.h"
#include "Song.h"
#include "FontCharAliases.h"
#include "ProfileManager.h"
#include "LocalizedString.h"
#include "GameState.h"
#include "GameManager.h"
#include "Style.h"
#include "Profile.h"
#include "CourseWriterCRS.h"

int EditCourseUtil::MAX_NAME_LENGTH = 16;
int EditCourseUtil::MAX_PER_PROFILE = 32;
bool EditCourseUtil::s_bNewCourseNeedsName = false;

bool EditCourseUtil::Save( Course *pCourse )
{
	return EditCourseUtil::RenameAndSave( pCourse, pCourse->GetDisplayFullTitle() );
}

bool EditCourseUtil::RenameAndSave( Course *pCourse, RString sNewName )
{
	ASSERT( !sNewName.empty() );

	EditCourseUtil::s_bNewCourseNeedsName = false;

	RString sNewFilePath;
	if( pCourse->IsAnEdit() )
	{
		sNewFilePath = PROFILEMAN->GetProfileDir(ProfileSlot_Machine) + EDIT_COURSES_SUBDIR + sNewName + ".crs";
	}
	else
	{
		RString sDir, sName, sExt;
		splitpath( pCourse->m_sPath, sDir, sName, sExt );
		sNewFilePath = sDir + sNewName + sExt;
	}

	// remove the old file if the name is changing
	if( !pCourse->m_sPath.empty()  &&  sNewFilePath != pCourse->m_sPath )
	{
		FILEMAN->Remove( pCourse->m_sPath );	// not fatal if this fails		
		FlushDirCache();
	}

	pCourse->m_sMainTitle = sNewName;
	pCourse->m_sPath = sNewFilePath;
	return CourseWriterCRS::Write( *pCourse, pCourse->m_sPath, false );
}

bool EditCourseUtil::RemoveAndDeleteFile( Course *pCourse )
{
	if( !FILEMAN->Remove( pCourse->m_sPath ) )
		return false;
	FILEMAN->Remove( pCourse->GetCacheFilePath() );
	FlushDirCache();
	if( pCourse->IsAnEdit() )
	{
		PROFILEMAN->LoadMachineProfile();
	}
	else
	{
		SONGMAN->DeleteCourse( pCourse );
		delete pCourse;
	}
	return true;
}

static LocalizedString YOU_MUST_SUPPLY_NAME	( "WorkoutManager", "You must supply a name for your workout." );
static LocalizedString EDIT_NAME_CONFLICTS	( "WorkoutManager", "The name you chose conflicts with another workout. Please use a different name." );
static LocalizedString EDIT_NAME_CANNOT_CONTAIN	( "WorkoutManager", "The workout name cannot contain any of the following characters: %s" );
bool EditCourseUtil::ValidateEditCourseName( const RString &sAnswer, RString &sErrorOut )
{
	if( sAnswer.empty() )
	{
		sErrorOut = YOU_MUST_SUPPLY_NAME;
		return false;
	}

	static const RString sInvalidChars = "\\/:*?\"<>|";
	if( strpbrk(sAnswer, sInvalidChars) != NULL )
	{
		sErrorOut = ssprintf( EDIT_NAME_CANNOT_CONTAIN.GetValue(), sInvalidChars.c_str() );
		return false;
	}

	// Check for name conflicts
	vector<Course*> vpCourses;
	EditCourseUtil::GetAllEditCourses( vpCourses );
	FOREACH_CONST( Course*, vpCourses, p )
	{
		if( GAMESTATE->m_pCurCourse == *p )
			continue;	// don't comepare name against ourself

		if( (*p)->GetDisplayFullTitle() == sAnswer )
		{
			sErrorOut = EDIT_NAME_CONFLICTS;
			return false;
		}
	}

	return true;
}

void EditCourseUtil::UpdateAndSetTrail()
{
	StepsType st = GAMESTATE->m_pCurStyle->m_StepsType;
	Trail *pTrail = GAMESTATE->m_pCurCourse->GetTrailForceRegenCache( st );
	ASSERT( pTrail );
	GAMESTATE->m_pCurTrail[PLAYER_1].Set( pTrail );
}

void EditCourseUtil::PrepareForPlay()
{
	GAMESTATE->m_pCurSong.Set( NULL );	// CurSong will be set if we back out.  Set it back to NULL so that ScreenStage won't show the last song.
	GAMESTATE->m_PlayMode.Set( PLAY_MODE_ENDLESS );
	GAMESTATE->m_bSideIsJoined[0] = true;

	PROFILEMAN->GetProfile(ProfileSlot_Player1)->m_GoalType = GoalType_Time;
	Course *pCourse = GAMESTATE->m_pCurCourse;
	PROFILEMAN->GetProfile(ProfileSlot_Player1)->m_iGoalSeconds = pCourse->m_fGoalSeconds;
}

void EditCourseUtil::GetAllEditCourses( vector<Course*> &vpCoursesOut )
{
	vector<Course*> vpCoursesTemp;
	SONGMAN->GetAllCourses( vpCoursesTemp, false );
	FOREACH_CONST( Course*, vpCoursesTemp, c )
	{
		if( (*c)->GetLoadedFromProfileSlot() != ProfileSlot_Invalid )
			vpCoursesOut.push_back( *c );
	}
}

void EditCourseUtil::LoadDefaults( Course &out )
{
	out = Course();

	out.m_fGoalSeconds = 0;

	// pick a default name
	// XXX: Make this localizable
	for( int i=0; i<10000; i++ )
	{
		out.m_sMainTitle = ssprintf("Workout %d", i+1);
		bool bNameInUse = false;

		vector<Course*> vpCourses;
		EditCourseUtil::GetAllEditCourses( vpCourses );
		FOREACH_CONST( Course*, vpCourses, p )
		{
			if( out.m_sMainTitle == (*p)->m_sMainTitle )
			{
				bNameInUse = true;
				break;
			}
		}

		if( !bNameInUse )
			break;
	}

	vector<Song*> vpSongs;
	SONGMAN->GetPreferredSortSongs( vpSongs );
	for( int i=0; i<(int)vpSongs.size() && i<6; i++ )
	{
		CourseEntry ce;
		ce.songID.FromSong( vpSongs[i] );
		ce.stepsCriteria.m_difficulty = Difficulty_Easy;
		out.m_vEntries.push_back( ce );
	}
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
