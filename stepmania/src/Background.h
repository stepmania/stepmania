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


struct BGSegment	// like a BGChange, but holds index of a background instead of name
{
	BGSegment() {};
	BGSegment( float b, int i ) { m_fStartBeat = b; m_iBGIndex = i; };
	float m_fStartBeat;
	int m_iBGIndex;
};


class Background : public ActorFrame
{
public:

	Background();
	~Background();

	virtual void LoadFromSong( Song *pSong );
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
	CArray<BGSegment,BGSegment&> m_aBGSegments;
	int m_iCurBGSegment;	// this increases as we move into new segments
	BackgroundAnimation* GetCurBGA() { int index = m_aBGSegments[m_iCurBGSegment].m_iBGIndex; return m_BackgroundAnimations[index]; };


	Quad m_quadBGBrightness;
	Quad m_quadBorder[4];	// l, t, r, b - cover up the edge of animations that might hang outside of the background rectangle

	bool m_bInDanger;
};

