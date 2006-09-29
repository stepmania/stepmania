#include "global.h"
#include "WorkoutManager.h"
#include "SongUtil.h"
#include "SongManager.h"
#include "RageFileManager.h"
#include "Workout.h"
#include "song.h"
#include "FontCharAliases.h"
#include "ProfileManager.h"
#include "LocalizedString.h"

static const RString WORKOUTS_SUBDIR = "Workouts/";

WorkoutManager*	WORKOUTMAN = NULL;	// global and accessable from anywhere in our program

WorkoutManager::WorkoutManager()
{
	m_pCurWorkout = NULL;
	m_pTempCourse = new Course;

	// Register with Lua.
	{
		Lua *L = LUA->Get();
		lua_pushstring( L, "WORKOUTMAN" );
		this->PushSelf( L );
		lua_settable(L, LUA_GLOBALSINDEX);
		LUA->Release( L );
	}
}

WorkoutManager::~WorkoutManager()
{
	// Unregister with Lua.
	LUA->UnsetGlobal( "WORKOUTMAN" );

	FOREACH( Workout*, m_vpAllWorkouts, p )
		SAFE_DELETE( *p );
	m_vpAllWorkouts.clear();

	SAFE_DELETE( m_pTempCourse );
}

void WorkoutManager::LoadAllFromDisk()
{
	m_vpAllWorkouts.clear();
	vector<RString> vsFiles;
	GetDirListing( PROFILEMAN->GetProfileDir(ProfileSlot_Machine) + WORKOUTS_SUBDIR + "*.xml", vsFiles, false, true );
	FOREACH_CONST( RString, vsFiles, s )
	{
		Workout *p = new Workout;
		if( p->LoadFromFile( *s ) )
			m_vpAllWorkouts.push_back( p );
	}
}

void WorkoutManager::LoadDefaults( Workout &out )
{
	out = Workout();
	SongUtil::GetAllSongGenres( out.m_vsSongGenres );
}

bool WorkoutManager::RenameAndSave( Workout *pWorkout, RString sNewName )
{
	ASSERT( !sNewName.empty() );
	vector<Workout*>::iterator iter = find( m_vpAllWorkouts.begin(), m_vpAllWorkouts.end(), pWorkout );
	if( iter == m_vpAllWorkouts.end() )
		return false;
	pWorkout->m_sName = sNewName;
	FILEMAN->Remove( pWorkout->m_sFile );
	FlushDirCache();	// not fatal if this fails
	pWorkout->m_sFile = PROFILEMAN->GetProfileDir(ProfileSlot_Machine) + WORKOUTS_SUBDIR + pWorkout->m_sName + ".xml";
	return pWorkout->SaveToFile( pWorkout->m_sFile );
}

bool WorkoutManager::Save( Workout *pWorkout )
{
	ASSERT( !pWorkout->m_sFile.empty() );
	return pWorkout->SaveToFile( pWorkout->m_sFile );
}

bool WorkoutManager::RemoveAndDeleteFile( Workout *pToDelete )
{
	vector<Workout*>::iterator iter = find( m_vpAllWorkouts.begin(), m_vpAllWorkouts.end(), pToDelete );
	if( iter == m_vpAllWorkouts.end() )
		return false;
	if( !FILEMAN->Remove( pToDelete->m_sFile ) )
		return false;
	FlushDirCache();
	return true;
}

static LocalizedString YOU_MUST_SUPPLY_NAME	( "WorkoutManager", "You must supply a name for your workout." );
static LocalizedString EDIT_NAME_CONFLICTS	( "WorkoutManager", "The name you chose conflicts with another workout. Please use a different name." );
static LocalizedString EDIT_NAME_CANNOT_CONTAIN	( "WorkoutManager", "The workout name cannot contain any of the following characters: %s" );
bool WorkoutManager::ValidateWorkoutName( const RString &sAnswer, RString &sErrorOut )
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

	Workout *pWorkout = WORKOUTMAN->m_pCurWorkout;

	// Steps name must be unique for this song.
	FOREACH_CONST( Workout*, WORKOUTMAN->m_vpAllWorkouts, p )
	{
		if( pWorkout == *p )
			continue;	// don't comepare name against ourself

		if( (*p)->m_sName == sAnswer )
		{
			sErrorOut = EDIT_NAME_CONFLICTS;
			return false;
		}
	}

	return true;
}

void WorkoutManager::GetWorkoutSongsForGenres( const vector<RString> &vsSongGenres, vector<Song*> &vpSongsOut )
{
	SongCriteria soc;
	soc.m_Selectable = SongCriteria::Selectable_Yes;
	soc.m_bUseSongGenreAllowedList = true;
	soc.m_vsSongGenreAllowedList = vsSongGenres;
	SongUtil::FilterSongs( soc, SONGMAN->GetAllSongs(), vpSongsOut );
}

static LocalizedString SONGS_ENABLED( "WorkoutManager", "%d/%d songs enabled" );
static RString GetWorkoutSongsOverview()
{
	SongCriteria soc;

	Workout defaultWorkout;
	WORKOUTMAN->LoadDefaults( defaultWorkout );
	soc.m_Selectable = SongCriteria::Selectable_Yes;
	vector<Song*> vpAllSongs;
	WORKOUTMAN->GetWorkoutSongsForGenres( defaultWorkout.m_vsSongGenres, vpAllSongs );

	vector<Song*> vpSelectedSongs;
	WORKOUTMAN->GetWorkoutSongsForGenres( WORKOUTMAN->m_pCurWorkout->m_vsSongGenres, vpSelectedSongs );

	return ssprintf( SONGS_ENABLED.GetValue(), (int)vpSelectedSongs.size(), (int)vpAllSongs.size() );
}

static RString GetWorkoutSongTitleText()
{
	vector<RString> vs;
	FOREACH_CONST( RString, WORKOUTMAN->m_pCurWorkout->m_vsSongGenres, s )
	{
		RString s2 = *s;
		s2.Replace( " ", "&nbsp;" );
		vs.push_back( s2 );

		// show max N to avoid frame rate slowdown
		if( vs.size() >= 40 )
			break;
	}

	RString sReturn = join( ",   ", vs );
	FontCharAliases::ReplaceMarkers( sReturn );
	return sReturn;
}


// lua start
#include "LuaBinding.h"

class LunaWorkoutManager: public Luna<WorkoutManager>
{
public:
	static int GetCurrentWorkout( T* p, lua_State *L )		{ if(p->m_pCurWorkout) p->m_pCurWorkout->PushSelf(L); else lua_pushnil(L); return 1; }
	static int GetWorkoutSongsOverview( T* p, lua_State *L )	{ lua_pushstring( L, ::GetWorkoutSongsOverview() ); return 1; }
	static int GetWorkoutSongTitleText( T* p, lua_State *L )	{ lua_pushstring( L, ::GetWorkoutSongTitleText() ); return 1; }

	LunaWorkoutManager()
	{
		ADD_METHOD( GetCurrentWorkout );
		ADD_METHOD( GetWorkoutSongsOverview );
		ADD_METHOD( GetWorkoutSongTitleText );
	}
};

LUA_REGISTER_CLASS( WorkoutManager )
// lua end

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
