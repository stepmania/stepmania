#ifndef STEPSUTIL_H
#define STEPSUTIL_H
/*
-----------------------------------------------------------------------------
 Class: StepsUtil

 Desc: Holds note information for a Song.  A Song may have one or more Notess.
	A Steps can be played by one or more Styles.  See StyleDef.h for more info on
	Styles.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"

class Steps;
class Song;
class Profile;
struct XNode;

namespace StepsUtil
{
	bool CompareNotesPointersByRadarValues(const Steps* pNotes1, const Steps* pNotes2);
	bool CompareNotesPointersByMeter(const Steps *pNotes1, const Steps* pNotes2);
	bool CompareNotesPointersByDifficulty(const Steps *pNotes1, const Steps *pNotes2);
	void SortNotesArrayByDifficulty( vector<Steps*> &arrayNotess );
	bool CompareStepsPointersByTypeAndDifficulty(const Steps *pStep1, const Steps *pStep2);
	void SortStepsByTypeAndDifficulty( vector<Steps*> &arraySongPointers );
	void SortStepsPointerArrayByNumPlays( vector<Steps*> &vStepsPointers, ProfileSlot slot, bool bDescending );
	void SortStepsPointerArrayByNumPlays( vector<Steps*> &vStepsPointers, const Profile* pProfile, bool bDescending );
};

class StepsID
{
	StepsType st;
	Difficulty dc;
	CString sDescription;

public:
	StepsID() { FromSteps(NULL); }
	void FromSteps( const Steps *p );
	Steps *ToSteps( const Song *p, bool bAllowNull ) const;
	bool operator<( const StepsID &other ) const
	{
		return st < other.st || dc < other.dc || sDescription < other.sDescription;
	}

	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );

	bool IsValid() const;
};

#endif
