/*
-----------------------------------------------------------------------------
 File: Judgement.h

 Desc: A graphic displayed in the Judgement during Dancing.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _Judgement_H_
#define _Judgement_H_


#include "Sprite.h"
#include "ActorFrame.h"
#include "Song.h"
#include "BitmapText.h"
#include "ThemeManager.h"


class Judgement : public ActorFrame
{
public:
	Judgement();
	void SetJudgement( TapNoteScore score );
	virtual void Update( float fDeltaTime );
	virtual void RenderPrimitives();

protected:
	Sprite		m_sprJudgement;
	float		m_fDisplayCountdown;
};

#endif