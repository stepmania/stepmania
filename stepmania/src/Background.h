#ifndef BACKGROUND_H
#define BACKGROUND_H
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
#include "BGAnimation.h"


struct BGSegment	// like a BGChange, but holds index of a background instead of name
{
	BGSegment() {};
	BGSegment( float b, int i, bool f ) { m_fStartBeat = b; m_iBGIndex = i; m_bFade = f; };
	float m_fStartBeat;
	int m_iBGIndex;
	bool m_bFade;
};


class Background : public ActorFrame
{
public:

	Background();
	~Background();

	virtual void LoadFromAniDir( CString sAniDir );
	virtual void LoadFromSong( Song *pSong );
	virtual void Unload();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void FadeIn();
	void FadeOut();
	
	virtual void TurnDangerOn()		{ m_bInDanger = true; };
	virtual void TurnDangerOff()	{ m_bInDanger = false; };

	virtual bool IsInDanger()		{ return m_bInDanger; };

protected:
	bool DangerVisible();
	BGAnimation *GetBGA(const Song *pSong, const BackgroundChange &aniseg, const CString &bgpath) const;
		
	BGAnimation		m_BGADanger;

	// used in all BackgroundModes except OFF
	vector<BGAnimation*> m_BGAnimations;
	vector<BGSegment> m_aBGSegments;
	int m_iCurBGSegment;	// this increases as we move into new segments
	BGAnimation* GetCurBGA() { int index = m_aBGSegments[m_iCurBGSegment].m_iBGIndex; return m_BGAnimations[index]; };
	BGAnimation* m_pCurrentBGA;
	BGAnimation* m_pFadingBGA;
	float m_fSecsLeftInFade;

	Quad m_quadBGBrightness;
	Quad m_quadBorder[4];	// l, t, r, b - cover up the edge of animations that might hang outside of the background rectangle

	bool m_bInDanger;
};


#endif
