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
#include "ActorFrame.h"
#include "Song.h"
#include "BackgroundAnimation.h"



class Background : public ActorFrame
{
public:

	Background();
	virtual bool LoadFromSong( Song *pSong, bool bDisableVisualizations = false );

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void SetSongBeat( const float fSongBeat, const bool bFreeze ) { m_fSongBeat = fSongBeat; m_bFreeze = bFreeze; };

	virtual void SetDiffuseColor( D3DXCOLOR c )
	{
		m_sprSongBackground.SetDiffuseColor( c );
		m_sprVisualizationOverlay.SetDiffuseColor( c );
		m_sprSongBackground.SetDiffuseColor( c );
		m_sprDanger.SetDiffuseColor( c );
		m_sprDangerBackground.SetDiffuseColor( c );
	};
	
	virtual void TurnDangerOn()		{ m_bShowDanger = true; };
	virtual void TurnDangerOff()	{ m_bShowDanger = false; };

	virtual bool IsDangerOn()		{ return m_bShowDanger; };

protected:
	bool DangerVisible();

	CArray<BackgroundAnimation*,BackgroundAnimation*> m_BackgroundAnimations;
	BackgroundAnimation* m_pCurBackgroundAnimation;

	Sprite m_sprVisualizationOverlay;
	Sprite m_sprSongBackground;
	
	Sprite m_sprDanger;
	Sprite m_sprDangerBackground;

	bool m_bShowDanger;

	float m_fSongBeat;
	bool m_bFreeze;
};

