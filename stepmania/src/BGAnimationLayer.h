/* BGAnimationLayer - layer elements used by BGAnimation */

#ifndef BGANIMATIONLAYER_H
#define BGANIMATIONLAYER_H

#include "GameConstantsAndTypes.h"
#include "ActorFrame.h"
#include <map>

class BGAnimationLayer : public ActorFrame
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
	void DrawPrimitives();
	bool EarlyAbortDraw();

	float GetMaxTweenTimeLeft() const;
	void GainFocus( float fRate, bool bRewindMovie, bool bLoop );
	void LoseFocus();

	void PlayCommand( const CString &sCommandName );
	void PlayOffCommand() { PlayCommand( "Off" ); }

protected:
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
	float m_fUpdateRate;	// set by GainFocus
	float m_fFOV;	// -1 = no change
	bool m_bLighting;

	CString m_sDrawCond;

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

/*
 * (c) 2001-2004 Ben Nordstrom, Chris Danford, Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
