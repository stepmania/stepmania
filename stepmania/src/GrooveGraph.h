#ifndef GrooveGraph_H
#define GrooveGraph_H
/*
-----------------------------------------------------------------------------
 Class: GrooveGraph

 Desc: The song's GrooveGraph displayed in SelectSong.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "Sprite.h"
#include "GameConstantsAndTypes.h"
class Song;


class GrooveGraph : public ActorFrame
{
public:
	GrooveGraph();

	void SetFromSong( Song* pSong );	// NULL means no Song

	void TweenOnScreen();
	void TweenOffScreen();

protected:

	class Mountain : public ActorFrame
	{
	public:
		Mountain();

		virtual void Update( float fDeltaTime );
		virtual void DrawPrimitives();

		void SetValues( float fNewValues[] );

		float m_PercentTowardNew;
		float m_fValuesNew[NUM_DIFFICULTIES];
		float m_fValuesOld[NUM_DIFFICULTIES];
	};

	Mountain m_Mountains[NUM_RADAR_CATEGORIES];
	Sprite m_sprLabels[NUM_RADAR_CATEGORIES];
};

#endif
