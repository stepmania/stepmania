#ifndef HOLDJudgment_H
#define HOLDJudgment_H
/*
-----------------------------------------------------------------------------
 Class: HoldJudgment

 Desc: A graphic displayed in the HoldJudgment during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"
#include "ActorFrame.h"
#include "song.h"
#include "BitmapText.h"
#include "PrefsManager.h"


class HoldJudgment : public ActorFrame
{
public:
	HoldJudgment();
	void SetHoldJudgment( HoldNoteScore hns );
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

protected:
	Sprite		m_sprJudgment;
	float		m_fShowCountdown;
};

#endif
