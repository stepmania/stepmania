#pragma once
/*
-----------------------------------------------------------------------------
 Class: Background

 Desc: Background behind notes while playing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Sprite.h"
#include "Quad.h"
#include "ActorFrame.h"
#include "Song.h"
#include "BackgroundAnimation.h"


struct AnimSeg
{
	AnimSeg() {};
	AnimSeg( float b, int i ) { m_fStartBeat = b; m_iAnimationIndex = i; };
	float m_fStartBeat;
	int m_iAnimationIndex;
};


class Background : public ActorFrame
{
public:

	Background();
	~Background();

	virtual void LoadFromSong( Song *pSong, bool bDisableVisualizations = false );
	virtual void Unload();	// call this on before calling load

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void FadeIn();
	void FadeOut();
	
	virtual void TurnDangerOn()		{ m_bInDanger = true; };
	virtual void TurnDangerOff()	{ m_bInDanger = false; };

	virtual bool IsInDanger()		{ return m_bInDanger; };

protected:
	bool DangerVisible();

	enum BackgroundMode { MODE_STATIC_BG, MODE_MOVIE_BG, MODE_ANIMATIONS, MODE_MOVIE_VIS, MODE_RANDOMMOVIES };
	BackgroundMode	m_BackgroundMode;
		
	Sprite m_sprDanger;
	Sprite m_sprDangerBackground;


	// used in all BackgroundModes except OFF
	CArray<BackgroundAnimation*,BackgroundAnimation*> m_BackgroundAnimations;
	CArray<AnimSeg,AnimSeg&> m_aAnimSegs;
	int m_iCurAnimSegment;	// this increases as we move into new segments
	BackgroundAnimation* GetCurBGA() { int index = m_aAnimSegs[m_iCurAnimSegment].m_iAnimationIndex; return m_BackgroundAnimations[index]; };


	Quad m_quadBGBrightness;
	Quad m_quadBorder[4];	// l, t, r, b - cover up the edge of animations that might hang outside of the background rectangle

	bool m_bInDanger;
};

