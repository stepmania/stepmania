/*
-----------------------------------------------------------------------------
 File: HoldJudgement.h

 Desc: A graphic displayed in the HoldJudgement during Dancing.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _HoldJudgement_H_
#define _HoldJudgement_H_


#include "Sprite.h"
#include "ActorFrame.h"
#include "Song.h"
#include "BitmapText.h"
#include "ThemeManager.h"


class HoldJudgement : public ActorFrame
{
public:
	HoldJudgement();
	void SetHoldJudgement( HoldNoteResult result );
	virtual void Update( float fDeltaTime );
	virtual void RenderPrimitives();

protected:
	Sprite		m_sprJudgement;
	float		m_fDisplayCountdown;
};

#endif