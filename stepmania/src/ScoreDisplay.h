#pragma once
/*
-----------------------------------------------------------------------------
 Class: ScoreDisplay

 Desc: A graphic displayed in the ScoreDisplay during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"
#include "Song.h"
#include "ActorFrame.h"
#include "BitmapText.h"
#include "PlayerOptions.h"



class ScoreDisplay : public BitmapText
{
public:
	virtual void Init( PlayerNumber pn, PlayerOptions po, int iOriginalNumNotes, int iNotesMeter ) = 0;

	virtual void SetScore( float fNewScore ) = 0;
	virtual int GetScore() = 0;
	virtual void AddToScore( TapNoteScore score, int iCurCombo ) = 0;

	virtual void Update( float fDeltaTime ) = 0;
	virtual void Draw() = 0;

protected:
};
