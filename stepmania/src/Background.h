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

	virtual bool LoadFromSong( Song *pSong, bool bDisableVisualizations = false );

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void SetSongBeat( const float fSongBeat, const bool bFreeze, const float fMusicSeconds );

	void FadeIn();
	void FadeOut();
	
	virtual void TurnDangerOn()		{ m_bInDanger = true; };
	virtual void TurnDangerOff()	{ m_bInDanger = false; };

	virtual bool IsInDanger()		{ return m_bInDanger; };

protected:
	bool DangerVisible();

	Song* m_pSong;

	enum BackgroundMode { MODE_STATIC_BG, MODE_MOVIE_BG, MODE_ANIMATIONS, MODE_MOVIE_VIS };
	BackgroundMode	m_BackgroundMode;
	
	Sprite m_sprSongBackground;
	
	Sprite m_sprDanger;
	Sprite m_sprDangerBackground;

	// for movie BG
	Sprite m_sprMovieBackground;

	// for animations
	CArray<BackgroundAnimation*,BackgroundAnimation*> m_BackgroundAnimations;
	CArray<AnimSeg,AnimSeg&> m_aAnimSegs;
	BackgroundAnimation* GetCurBGA()
	{
		if( m_aAnimSegs.GetSize() == 0 )
			return NULL;

		if( m_fSongBeat < m_pSong->m_fFirstBeat  ||  m_fSongBeat > m_pSong->m_fLastBeat )
			return NULL;

		for( int i=0; i<m_aAnimSegs.GetSize()-1; i++ )
			if( m_aAnimSegs[i+1].m_fStartBeat > m_fSongBeat )
				break;
		int iIndex = m_aAnimSegs[i].m_iAnimationIndex;
		return m_BackgroundAnimations[iIndex];
	};


	// for movie vis
	Sprite m_sprMovieVis;

	Quad m_quadBGBrightness;

	bool m_bInDanger;

	float m_fSongBeat;
	bool m_bFreeze;
	float m_fMusicSeconds;
	bool m_bStartedBGMovie;
};

