/*
-----------------------------------------------------------------------------
 File: Sprite.h

 Desc: A bitmap Actor that animates and moves around.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _SPRITE_H_
#define _SPRITE_H_


#include "Actor.h"
#include "RageUtil.h"
#include "RageTexture.h"



#define MAX_SPRITE_STATES 256



class Sprite: public Actor
{
public:
	Sprite();
	virtual ~Sprite();


	enum SpriteEffect { no_effect,
						blinking,	camelion,   glowing,
						wagging,	spinning,
						vibrating,	flickering
						 };
	

	BOOL LoadFromTexture( CString sTexturePath );
	BOOL LoadFromSpriteFile( CString sSpritePath );

	void PrintDebugInfo();

	virtual void Draw();
	virtual void Update( const FLOAT &fDeltaTime );

	void StartAnimating()	{ m_bIsAnimating = TRUE; };
	void StopAnimating()	{ m_bIsAnimating = FALSE; };
	void SetState( UINT uNewState );
	
	UINT	GetNumStates()		{ return m_uNumStates; };
	CString	GetTexturePath()	{ return m_sTexturePath; };


	void SetCustomSrcRect( FRECT new_texcoord_frect );	// for cropping


	void SetEffectNone();
	void SetEffectBlinking( FLOAT fDeltaPercentPerSecond = 2.5,
						    D3DXCOLOR Color  = D3DXCOLOR(0.5f,0.5f,0.5f,1), 
						    D3DXCOLOR Color2 = D3DXCOLOR(1,1,1,1) );
	void SetEffectCamelion( FLOAT fDeltaPercentPerSecond = 2.5,
						    D3DXCOLOR Color  = D3DXCOLOR(0,0,0,1), 
						    D3DXCOLOR Color2 = D3DXCOLOR(1,1,1,1) );
	void SetEffectGlowing( FLOAT fDeltaPercentPerSecond = 2.5,
						   D3DXCOLOR Color  = D3DXCOLOR(0.4f,0.4f,0.4f,0),
						   D3DXCOLOR Color2 = D3DXCOLOR(1.0f,1.0f,1.0f,0) );
	void SetEffectWagging( FLOAT fWagRadians =  0.2,
						   FLOAT fWagPeriod = 2.0 );
	void SetEffectSpinning( FLOAT fRadsPerSpeed = 2.0 );
	void SetEffectVibrating( FLOAT fVibrationDistance = 5.0 );
	void SetEffectFlickering();
	SpriteEffect GetEffect() { return m_Effect; };

protected:
	void Init();
	
	BOOL LoadTexture( CString sTexture );

	CString	m_sSpritePath;
	LPRageTexture m_pTexture;
	CString	m_sTexturePath;

	UINT	m_uFrame[MAX_SPRITE_STATES];	// array of indicies into m_rectBitmapFrames
	FLOAT	m_fDelay[MAX_SPRITE_STATES];
	UINT	m_uNumStates;
	UINT	m_uCurState;
	BOOL	m_bIsAnimating;
	FLOAT	m_fSecsIntoState;	// number of seconds that have elapsed since we switched to this frame

	BOOL m_bUsingCustomTexCoordRect;
	FRECT m_CustomTexCoordRect;

	SpriteEffect m_Effect;

	// Counting variables for sprite effects:
	// camelion and glowing:
//	D3DXCOLOR m_Color2;
	FLOAT m_fPercentBetweenColors;
	BOOL  m_bTweeningTowardEndColor;	// TRUE is fading toward end_color, FALSE if fading toward start_color
	FLOAT m_fDeltaPercentPerSecond;	// percentage change in tweening per second

	// wagging:
	FLOAT m_fWagRadians;
	FLOAT m_fWagPeriod;		// seconds to complete a wag (back and forth)
	FLOAT m_fWagTimer;		// num of seconds into this wag

	// spinning:
	FLOAT m_fSpinSpeed;		// radians per second

	// vibrating:
	FLOAT m_fVibrationDistance;

	// flickering:
	BOOL m_bVisibleThisFrame;

};


#endif