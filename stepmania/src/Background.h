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
#include <deque>
class DancingCharacters;


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
	
	DancingCharacters* GetDancingCharacters() { return m_pDancingCharacters; };

protected:
	void LoadFromRandom( float fFirstBeat, float fLastBeat, const TimingData &timing );

	bool IsDangerPlayerVisible( PlayerNumber pn );
	bool IsDangerAllVisible();
	bool IsDeadPlayerVisible( PlayerNumber pn );
	void UpdateCurBGChange();
		
	DancingCharacters*	m_pDancingCharacters;

	BGAnimation		m_DangerPlayer[NUM_PLAYERS];
	BGAnimation		m_DangerAll;

	BGAnimation		m_DeadPlayer[NUM_PLAYERS];

	BGAnimation* CreateSongBGA(const Song *pSong, CString sBGName) const;
	CString CreateRandomBGA();

	map<CString,BGAnimation*> m_BGAnimations;
	deque<CString> m_RandomBGAnimations;
	vector<BackgroundChange> m_aBGChanges;
	int				m_iCurBGChangeIndex;
	BGAnimation* m_pCurrentBGA;
	BGAnimation* m_pFadingBGA;
	float m_fSecsLeftInFade;

	Quad m_quadBGBrightness;
	Quad m_quadBorder[4];	// l, t, r, b - cover up the edge of animations that might hang outside of the background rectangle
};


#endif
