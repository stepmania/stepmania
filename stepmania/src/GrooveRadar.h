#pragma once
/*
-----------------------------------------------------------------------------
 Class: GrooveRadar

 Desc: The song's GrooveRadar displayed in SelectSong.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
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

	void SetFromNotes( PlayerNumber p, Notes* pNotes )	// NULL means no Song
	{
		m_GrooveRadarValueMap.SetFromNotes( p, pNotes );
	}

	void TweenOnScreen();
	void TweenOffScreen();

protected:

	// the value map must be a separate Actor so we can tween it separately from the labels
	class GrooveRadarValueMap : public ActorFrame
	{
	public:
		GrooveRadarValueMap();

		virtual void Update( float fDeltaTime );
		virtual void DrawPrimitives();

		void SetFromNotes( PlayerNumber p, Notes* pNotes );	// NULL means no Song

		void TweenOnScreen();
		void TweenOffScreen();

		bool m_bValuesVisible[NUM_PLAYERS];
		float m_PercentTowardNew[NUM_PLAYERS];
		float m_fValuesNew[NUM_PLAYERS][NUM_RADAR_CATEGORIES];
		float m_fValuesOld[NUM_PLAYERS][NUM_RADAR_CATEGORIES];

		Sprite m_sprRadarBase;
	};

	GrooveRadarValueMap m_GrooveRadarValueMap;
	Sprite m_sprRadarLabels[NUM_RADAR_CATEGORIES];
};
