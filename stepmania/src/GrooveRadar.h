#pragma once
/*
-----------------------------------------------------------------------------
 Class: GrooveRadar

 Desc: The song's GrooveRadar displayed in SelectSong.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "Sprite.h"
#include "Song.h"


const int NUM_RADAR_CATEGORIES	=	5;

class GrooveRadar : public ActorFrame
{
public:
	GrooveRadar();

	virtual void Update( float fDeltaTime );
	virtual void RenderPrimitives();

	void SetFromNoteMetadata( PlayerNumber p, NoteMetadata* pNoteMetadata );	// NULL means no Song

	void TweenOnScreen();
	void TweenOffScreen();

protected:

	bool m_bValuesVisible[NUM_PLAYERS];
	float m_PercentTowardNew[NUM_PLAYERS];
	float m_fValuesNew[NUM_PLAYERS][NUM_RADAR_CATEGORIES];
	float m_fValuesOld[NUM_PLAYERS][NUM_RADAR_CATEGORIES];

	Sprite m_sprRadarBase;
	Sprite m_sprRadarLabels[NUM_RADAR_CATEGORIES];
};
