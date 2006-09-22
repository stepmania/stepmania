#ifndef WorkoutManager_H
#define WorkoutManager_H

struct lua_State;
class Song;
class Workout;
class Course;

const int MAX_WORKOUT_NAME_LENGTH = 16;
const int MAX_WORKOUTS_PER_PROFILE = 32;

class WorkoutManager
{
public:
	WorkoutManager();
	~WorkoutManager();

	void LoadAllFromDisk();

	void LoadDefaults( Workout &out );
	bool RenameAndSave( Workout *pToRename, RString sNewName );
	bool Save( Workout *pToRename );
	bool RemoveAndDeleteFile( Workout *pToDelete );
	
	static bool ValidateWorkoutName( const RString &sAnswer, RString &sErrorOut );

	void GetWorkoutSongsForGenres( const vector<RString> &vsSongGenres, vector<Song*> &vpSongsOut );

	vector<Workout*> m_vpAllWorkouts;
	Workout *m_pCurWorkout;

	Course *m_pTempCourse;

	// Lua
	void PushSelf( lua_State *L );
};

extern WorkoutManager*	WORKOUTMAN;	// global and accessable from anywhere in our program

#endif

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
