#ifndef HOLDJUDGEMENT_H
#define HOLDJUDGEMENT_H
/*
-----------------------------------------------------------------------------
 Class: HoldJudgement

 Desc: A graphic displayed in the HoldJudgement during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"
#include "ActorFrame.h"
#include "song.h"
#include "BitmapText.h"
#include "PrefsManager.h"


class HoldJudgement : public ActorFrame
{
public:
	HoldJudgement();
	void SetHoldJudgement( HoldNoteScore hns );
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

protected:
	Sprite		m_sprJudgement;
	float		m_fDisplayTime;
	float		m_fDisplayCountdown;
};

#endif
