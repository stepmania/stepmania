#ifndef BGANIMATIONLAYER_H
#define BGANIMATIONLAYER_H
/*
-----------------------------------------------------------------------------
 Class: BGAnimation

 Desc: Particles that play in the background of ScreenGameplay

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageTypes.h"
#include "Sprite.h"
#include "GameConstantsAndTypes.h"

#define MAX_TILES_WIDE (SCREEN_WIDTH/32+2)
#define MAX_TILES_HIGH (SCREEN_HEIGHT/32+2)
#define MAX_SPRITES  16
// (MAX_TILES_WIDE*MAX_TILES_HIGH)

class BGAnimationLayer
{
public:
	BGAnimationLayer();
	~BGAnimationLayer();
	void Init();
	void Unload();

	void LoadFromStaticGraphic( CString sPath );
	void LoadFromAniLayerFile( CString sPath );
	void LoadFromMovie( CString sMoviePath );
	void LoadFromVisualization( CString sMoviePath );
	void LoadFromIni( CString sDir, CString sLayer );

	virtual void Update( float fDeltaTime );
	virtual void Draw();

	virtual void SetDiffuse( RageColor c );

	float GetMaxTweenTimeLeft() const;
	void GainingFocus( float fRate, bool bRewindMovie, bool bLoop );
	void LosingFocus();

protected:
	vector<Sprite *> m_Sprites;
	RageVector3 m_vParticleVelocity[MAX_SPRITES];

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
		NUM_EFFECTS,		// leave this at the end
		EFFECT_INVALID
	};

	enum Type
	{
		TYPE_SPRITE,
		TYPE_STRETCH,
		TYPE_PARTICLES,
		TYPE_TILES,
		NUM_TYPES,
		TYPE_INVALID
	} m_Type;



	//
	// loaded prefs
	//
	
	// common stuff
	CString m_sCommand;

	// stretch stuff
	float m_fStretchTexCoordVelocityX;
	float m_fStretchTexCoordVelocityY;

	// particle and tile stuff
	float m_fZoomMin, m_fZoomMax;
	float m_fVelocityXMin, m_fVelocityXMax;
	float m_fVelocityYMin, m_fVelocityYMax;
	float m_fVelocityZMin, m_fVelocityZMax;
	float m_fOverrideSpeed;		// 0 means don't override speed

	// particles stuff
	int m_iNumParticles;
	bool  m_bParticlesBounce;

	// tiles stuff
	int m_iNumTilesWide;
	int m_iNumTilesHigh;
	float m_fTilesStartX;
	float m_fTilesStartY;
	float m_fTilesSpacingX;
	float m_fTilesSpacingY;
	float m_fTileVelocityX;
	float m_fTileVelocityY;


};

#endif
