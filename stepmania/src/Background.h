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
#include "BGAnimation.h"
#include "song.h"


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
	bool IsDangerVisible();
		
	BGAnimation		m_BGADanger;

	BGAnimation* CreateSongBGA(const Song *pSong, CString sBGName) const;
	BGAnimation* CreateRandomBGA() const;

	map<CString,BGAnimation*> m_BGAnimations;
	vector<BackgroundChange> m_aBGChanges;
	int				m_iCurBGChangeIndex;
	BGAnimation* m_pCurrentBGA;
	BGAnimation* m_pFadingBGA;
	float m_fSecsLeftInFade;

	Quad m_quadBGBrightness;
	Quad m_quadBorder[4];	// l, t, r, b - cover up the edge of animations that might hang outside of the background rectangle

	bool m_bInDanger;
};


#endif
