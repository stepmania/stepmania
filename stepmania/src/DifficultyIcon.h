#pragma once
/*
-----------------------------------------------------------------------------
 Class: DifficultyIcon

 Desc: Graphical representation of the difficulty class.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"
#include "GameConstantsAndTypes.h"
struct Notes;

class DifficultyIcon : public Sprite
{
public:
	bool Load( CString sFilePath );

	void SetFromNotes( PlayerNumber pn, Notes* pNotes );
};
