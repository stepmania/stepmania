#pragma once
/*
-----------------------------------------------------------------------------
 Class: StageBox

 Desc: bonus meters and max combo number.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "Quad.h"
#include "GameConstantsAndTypes.h"


class StageBox : public ActorFrame
{
public:
	StageBox();

	void SetStageInfo( PlayerNumber p, int iNumStagesCompleted );

protected:

	Sprite			m_sprBox;
	BitmapText		m_textTime;
	BitmapText		m_textNumber;
	BitmapText		m_textST;
};
