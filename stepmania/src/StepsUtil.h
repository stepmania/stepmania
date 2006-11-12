#ifndef STEPS_UTIL_H
#define STEPS_UTIL_H

#include "GameConstantsAndTypes.h"
#include "Difficulty.h"

class Steps;
class Song;
class Profile;
class XNode;
class SongCriteria;

class StepsCriteria
{
public:
	Difficulty m_difficulty;	// don't filter if Difficulty_Invalid
	int m_iLowMeter;		// don't filter if -1
	int m_iHighMeter;		// don't filter if -1
	StepsType m_st;			// don't filter if StepsType_Invalid
	enum Locked { Locked_Locked, Locked_Unlocked, Locked_DontCare } m_Locked;

	StepsCriteria()
	{
		m_difficulty = Difficulty_Invalid;
		m_iLowMeter = -1;
		m_iHighMeter = -1;
		m_st = StepsType_Invalid;
		m_Locked = Locked_DontCare;
	}

	bool Matches( const Song *pSong, const Steps *pSteps ) const;
	bool operator==( const StepsCriteria &other ) const
	{
#define X(x) (x == other.x)
		return X(m_difficulty) && X(m_iLowMeter) && X(m_iHighMeter) && X(m_st) && X(m_Locked);
#undef X
	}
	bool operator!=( const StepsCriteria &other ) const { return !operator==( other ); }
};

class SongAndSteps
{
public:
	Song *pSong;
	Steps *pSteps;
	SongAndSteps() : pSong(NULL), pSteps(NULL) { }
	SongAndSteps( Song *pSong_, Steps *pSteps_ ) : pSong(pSong_), pSteps(pSteps_) { }
	bool operator==( const SongAndSteps& other ) const { return pSong==other.pSong && pSteps==other.pSteps; }
	bool operator<( const SongAndSteps& other ) const { return pSong<=other.pSong && pSteps<=other.pSteps; }
};

namespace StepsUtil
{
	void GetAllMatching( const SongCriteria &soc, const StepsCriteria &stc, vector<SongAndSteps> &out );	// look up in SONGMAN
	void GetAllMatching( Song *pSong, const StepsCriteria &stc, vector<SongAndSteps> &out );

	bool CompareNotesPointersByRadarValues(const Steps* pSteps1, const Steps* pSteps2);
	bool CompareNotesPointersByMeter(const Steps *pSteps1, const Steps* pSteps2);
	bool CompareNotesPointersByDifficulty(const Steps *pSteps1, const Steps *pSteps2);
	void SortNotesArrayByDifficulty( vector<Steps*> &vpStepsInOut );	
	bool CompareStepsPointersByTypeAndDifficulty(const Steps *pStep1, const Steps *pStep2);
	void SortStepsByTypeAndDifficulty( vector<Steps*> &vpStepsInOut );
	void SortStepsPointerArrayByNumPlays( vector<Steps*> &vpStepsInOut, ProfileSlot slot, bool bDescending );
	void SortStepsPointerArrayByNumPlays( vector<Steps*> &vpStepsInOut, const Profile* pProfile, bool bDescending );
	bool CompareStepsPointersByDescription(const Steps *pStep1, const Steps *pStep2);
	void SortStepsByDescription( vector<Steps*> &vpStepsInOut );
	void RemoveLockedSteps( const Song *pSong, vector<Steps*> &vpStepsInOut );	
};

class StepsID
{
	StepsType st;
	Difficulty dc;
	RString sDescription;
	unsigned uHash;

public:
	StepsID() { Unset(); }
	void Unset() { FromSteps(NULL); }
	void FromSteps( const Steps *p );
	Steps *ToSteps( const Song *p, bool bAllowNull, bool bUseCache = true ) const;
	bool operator<( const StepsID &rhs ) const;
	bool MatchesStepsType( StepsType s ) const { return st == s; }

	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );
	RString ToString() const;
	bool IsValid() const;
	static void ClearCache();
	
	StepsType GetStepsType() const { return st; }
};

#endif

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
