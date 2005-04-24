#ifndef STEPSUTIL_H
#define STEPSUTIL_H

#include "GameConstantsAndTypes.h"
#include "Difficulty.h"

class Steps;
class Song;
class Profile;
struct XNode;

namespace StepsUtil
{
	bool CompareNotesPointersByRadarValues(const Steps* pSteps1, const Steps* pSteps2);
	bool CompareNotesPointersByMeter(const Steps *pSteps1, const Steps* pSteps2);
	bool CompareNotesPointersByDifficulty(const Steps *pSteps1, const Steps *pSteps2);
	void SortNotesArrayByDifficulty( vector<Steps*> &arrayNotess );	
	bool CompareStepsPointersByTypeAndDifficulty(const Steps *pStep1, const Steps *pStep2);
	void SortStepsByTypeAndDifficulty( vector<Steps*> &arraySongPointers );
	void SortStepsPointerArrayByNumPlays( vector<Steps*> &vStepsPointers, ProfileSlot slot, bool bDescending );
	void SortStepsPointerArrayByNumPlays( vector<Steps*> &vStepsPointers, const Profile* pProfile, bool bDescending );
	bool CompareStepsPointersByDescription(const Steps *pStep1, const Steps *pStep2);
	void SortStepsByDescription( vector<Steps*> &vStepsPointers );
	void RemoveLockedSteps( const Song *pSong, vector<Steps*> &arrayNotess );	
};

class StepsID
{
	StepsType st;
	Difficulty dc;
	CString sDescription;
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
	CString ToString() const;
	bool IsValid() const;
	static void ClearCache();
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
