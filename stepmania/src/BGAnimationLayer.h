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

class BGAnimationLayer
{
public:
	BGAnimationLayer( bool Generic );
	~BGAnimationLayer();
	void Init();
	void Unload();

	void LoadFromStaticGraphic( CString sPath );
	void LoadFromAniLayerFile( CString sPath );
	void LoadFromMovie( CString sMoviePath );
	void LoadFromVisualization( CString sMoviePath );
	void LoadFromIni( CString sDir, CString sLayer );

	void Update( float fDeltaTime );
	void Draw();

	void SetDiffuse( RageColor c );

	float GetMaxTweenTimeLeft() const;
	void FinishTweening();
	void GainingFocus( float fRate, bool bRewindMovie, bool bLoop );
	void LosingFocus();

	void PlayCommand( CString cmd );
	void PlayOffCommand() { PlayCommand( "Off" ); }

protected:
	vector<Actor*> m_pActors;
	vector<RageVector3> m_vParticleVelocity;

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
		TYPE_PARTICLES,
		TYPE_TILES,
		NUM_TYPES,
	} m_Type;



	//
	// loaded prefs
	//
	
	// common stuff
	bool m_bGeneric;
	map<CString, CString> m_asCommands;
	float m_fRepeatCommandEverySeconds;	// -1 = no repeat
	float m_fSecondsUntilNextCommand;
	float m_fUpdateRate;	// set by GainingFocus
	float m_fFOV;	// -1 = no change
	bool m_bLighting;

	// stretch stuff
	float m_fTexCoordVelocityX;
	float m_fTexCoordVelocityY;

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
