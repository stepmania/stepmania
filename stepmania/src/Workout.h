#ifndef Workout_H
#define Workout_H

#include "SongUtil.h"

enum WorkoutProgram
{
	WorkoutProgram_FatBurn,
	WorkoutProgram_FitnessTest,
	WorkoutProgram_Intermediate,
	WorkoutProgram_Interval,
	WorkoutProgram_Runner,
	WorkoutProgram_Flat,
	NUM_WorkoutProgram,
	WorkoutProgram_Invalid
};
const RString& WorkoutProgramToLocalizedString( WorkoutProgram i );
const RString& WorkoutProgramToLocalizedString( WorkoutProgram i );
WorkoutProgram StringToWorkoutProgram( const RString& str );

enum WorkoutStepsType
{
	WorkoutStepsType_NormalSteps,
	WorkoutStepsType_WorkoutSteps,
	NUM_WorkoutStepsType,
	WorkoutStepsType_Invalid
};
const RString& WorkoutStepsTypeToLocalizedString( WorkoutStepsType i );
WorkoutStepsType StringToWorkoutStepsType( const RString& str );
LuaDeclareType( WorkoutStepsType );


const int MIN_WORKOUT_MINUTES = 4;
const int MAX_WORKOUT_MINUTES = 90;


struct lua_State;
class Course;
class Song;
class Steps;

class Workout
{
public:
	Workout();

	RString m_sFile;
	RString m_sName;
	WorkoutProgram m_WorkoutProgram;
	int m_iMinutes;
	int m_iAverageMeter;
	WorkoutStepsType m_WorkoutStepsType;
	vector<RString> m_vsSongGenres;

	int GetEstimatedNumSongs() const;
	static int GetEstimatedNumSongsFromSeconds( float fSeconds );
	void GetEstimatedMeters( int iNumSongs, vector<int> &viMetersOut );
	void GenerateCourse( Course &out );
	bool LoadFromFile( RString sFile );
	bool SaveToFile( RString sFile );

	// Lua
	void PushSelf( lua_State *L );
};

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
