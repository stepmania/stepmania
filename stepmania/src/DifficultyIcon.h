#ifndef DIFFICULTYICON_H
#define DIFFICULTYICON_H
/*
-----------------------------------------------------------------------------
 Class: DifficultyIcon

 Desc: Graphical representation of the difficulty class.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"
#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"
class Steps;


class DifficultyIcon : public Sprite
{
	bool m_bBlank;

public:
	DifficultyIcon();
	virtual bool EarlyAbortDraw() { return m_bBlank || Sprite::EarlyAbortDraw(); }

	bool Load( CString sFilePath );

	void SetFromNotes( PlayerNumber pn, Steps* pNotes );
	void SetFromDifficulty( PlayerNumber pn, Difficulty dc );
	void SetFromCourseDifficulty( PlayerNumber pn, CourseDifficulty cd );
};

#endif
