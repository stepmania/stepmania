#ifndef Judgment_H
#define Judgment_H
/*
-----------------------------------------------------------------------------
 File: Judgment.h

 Desc: A graphic displayed in the Judgment during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"
#include "ActorFrame.h"
#include "GameConstantsAndTypes.h"
#include "BitmapText.h"


class Judgment : public ActorFrame
{
public:
	Judgment();

	void Reset();
	void SetJudgment( TapNoteScore score );

protected:
	Sprite		m_sprJudgment;
};

#endif
