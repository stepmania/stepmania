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
#include "song.h"
#include "BitmapText.h"
#include "PrefsManager.h"


class Judgment : public ActorFrame
{
public:
	Judgment();
	virtual ~Judgment() { }
	void SetJudgment( TapNoteScore score );
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Reset() { m_fShowCountdown = .0f; }

protected:
	Sprite		m_sprJudgment;
	float		m_fShowCountdown;
};

#endif