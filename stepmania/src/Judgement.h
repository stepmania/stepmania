/*
-----------------------------------------------------------------------------
 File: Judgement.h

 Desc: A graphic displayed in the Judgement during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef JUDGEMENT_H
#define JUDGEMENT_H


#include "Sprite.h"
#include "ActorFrame.h"
#include "song.h"
#include "BitmapText.h"
#include "PrefsManager.h"


class Judgement : public ActorFrame
{
public:
	Judgement();
	virtual ~Judgement() { }
	void SetJudgement( TapNoteScore score );
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Reset() { m_fDisplayCountdown = .0f; }

protected:
	Sprite		m_sprJudgement;
	float		m_fDisplayCountdown;
};

#endif
