#pragma once
/*
-----------------------------------------------------------------------------
 Class: BGAnimation

 Desc: Particles that play in the background of ScreenGameplay

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"

const int MAX_TILES_WIDE = 11;
const int MAX_TILES_HIGH = 8;
const int MAX_SPRITES = MAX_TILES_WIDE*MAX_TILES_HIGH;


class BGAnimationLayer
{
public:
	BGAnimationLayer();
	virtual ~BGAnimationLayer() { }

	void LoadFromStaticGraphic( CString sPath );
	void LoadFromAniLayerFile( CString sPath, CString sSongBGPath );
	void LoadFromMovie( CString sMoviePath, bool bLoop, bool bRewind, bool bFadeSongBG, CString sSongBGPath );
	void LoadFromVisualization( CString sMoviePath );

	virtual void Update( float fDeltaTime );
	virtual void Draw();

	void GainingFocus();
	void LosingFocus();

protected:
	Sprite m_Sprites[MAX_SPRITES];
	int m_iNumSprites;

	bool m_bCycleColor;
	bool m_bCycleAlpha;
	bool m_bRewindMovie;

	enum Effect {
		EFFECT_CENTER,
		EFFECT_STRETCH_STILL,
		EFFECT_STRETCH_SCROLL_LEFT,
		EFFECT_STRETCH_SCROLL_RIGHT,
		EFFECT_STRETCH_SCROLL_UP,
		EFFECT_STRETCH_SCROLL_DOWN,
		EFFECT_STRETCH_WATER,
		EFFECT_STRETCH_BUBBLE,
		EFFECT_STRETCH_TWIST,
		EFFECT_STRETCH_SPIN,
		EFFECT_PARTICLES_SPIRAL_OUT,
		EFFECT_PARTICLES_SPIRAL_IN,
		EFFECT_PARTICLES_FLOAT_UP,
		EFFECT_PARTICLES_FLOAT_DOWN,
		EFFECT_PARTICLES_FLOAT_LEFT,
		EFFECT_PARTICLES_FLOAT_RIGHT,
		EFFECT_PARTICLES_BOUNCE,
		EFFECT_TILE_STILL,
		EFFECT_TILE_SCROLL_LEFT,
		EFFECT_TILE_SCROLL_RIGHT,
		EFFECT_TILE_SCROLL_UP,
		EFFECT_TILE_SCROLL_DOWN,
		EFFECT_TILE_FLIP_X,
		EFFECT_TILE_FLIP_Y,
		EFFECT_TILE_PULSE,
		NUM_EFFECTS		// leave this at the end
	};
	Effect	m_Effect;

	D3DXVECTOR2 m_vHeadings[MAX_SPRITES];	// only used in EFFECT_PARTICLES_BOUNCE

	D3DXVECTOR2 m_vTexCoordVelocity;
	float m_fRotationalVelocity;
};
