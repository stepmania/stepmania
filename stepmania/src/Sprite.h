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


	virtual bool LoadFromTexture( CString sTexturePath );
	virtual bool LoadFromSpriteFile( CString sSpritePath );

	virtual void Draw();
	virtual void Update( float fDeltaTime );

	virtual void StartAnimating()	{ m_bIsAnimating = TRUE; };
	virtual void StopAnimating()	{ m_bIsAnimating = FALSE; };
	virtual void SetState( UINT uNewState );
	
	UINT	GetNumStates()		{ return m_uNumStates; };
	CString	GetTexturePath()	{ return m_sTexturePath; };


	void SetCustomSrcRect( FRECT new_texcoord_frect );	// for cropping
	void SetCustomTexCoords( float fTexCoords[8] );

	void TurnShadowOn()		{ m_bHasShadow = true; };
	void TurnShadowOff()	{ m_bHasShadow = false; };

	void SetBlendModeAdd() 		{ m_bBlendAdd = true; }; 
	void SetBlendModeNormal() 	{ m_bBlendAdd = false; };

protected:
	void Init();
	
	bool LoadTexture( CString sTexture );

	CString	m_sSpritePath;
	LPRageTexture m_pTexture;
	CString	m_sTexturePath;

	UINT	m_uFrame[MAX_SPRITE_STATES];	// array of indicies into m_rectBitmapFrames
	float	m_fDelay[MAX_SPRITE_STATES];
	UINT	m_uNumStates;
	UINT	m_uCurState;
	bool	m_bIsAnimating;
	float	m_fSecsIntoState;	// number of seconds that have elapsed since we switched to this frame

	bool m_bUsingCustomTexCoords;
	//FRECT m_CustomTexCoordRect;
	float m_CustomTexCoords[8];	// (x,y) * 4

	bool	m_bHasShadow;

	bool	m_bBlendAdd;
};


#endif