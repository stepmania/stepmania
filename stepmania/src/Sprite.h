#ifndef SPRITE_H
#define SPRITE_H
/*
-----------------------------------------------------------------------------
 File: Sprite.h

 Desc: A bitmap Actor that animates and moves around.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Actor.h"
#include "RageUtil.h"
#include "RageTextureID.h"

class RageTexture;

class Sprite: public Actor
{
public:
	Sprite();
	virtual ~Sprite();

	virtual bool EarlyAbortDraw();
	virtual void DrawPrimitives();
	virtual void Update( float fDeltaTime );
	void UpdateAnimationState();	// take m_fSecondsIntoState, and move to a new state

	/* Just a convenience function; load an image that'll be used in the
	 * background. */
	virtual bool LoadBG( RageTextureID ID );
	virtual bool Load( RageTextureID ID );

	void UnloadTexture();
	RageTexture* GetTexture() { return m_pTexture; };

	virtual void EnableAnimation( bool bEnable );
	virtual void SetState( int iNewState );
	virtual void SetSecondsIntoAnimation( float fSeconds );
	
	virtual int GetNumStates()		{ return m_States.size(); };
	CString	GetTexturePath() const;

	void SetCustomTextureRect( const RectF &new_texcoord_frect );
	void SetCustomTextureCoords( float fTexCoords[8] );
	void SetCustomSourceRect( const RectF &rectSourceCoords );	// in source pixel space
	void SetCustomImageRect( RectF rectImageCoords );	// in image pixel space
	void SetCustomImageCoords( float fImageCoords[8] );
	const RectF *GetCurrentTextureCoordRect() const;
	void StopUsingCustomCoords();
	void GetActiveTextureCoords(float fTexCoordsOut[8]) const;
	void StretchTexCoords( float fX, float fY );


	void SetTexCoordVelocity(float fVelX, float fVelY) { m_fTexCoordVelocityX = fVelX; m_fTexCoordVelocityY = fVelY; }	
	// Scale the Sprite maintaining the aspect ratio so that it fits 
	// within (fWidth,fHeight) and is clipped to (fWidth,fHeight).
	void ScaleToClipped( float fWidth, float fHeight );
	static bool IsDiagonalBanner( int iWidth, int iHeight );

	virtual void HandleCommand( const ParsedCommand &command );

protected:
	virtual bool LoadFromTexture( RageTextureID ID );
	virtual bool LoadFromSpriteFile( RageTextureID ID );

	void DrawTexture( const TweenState *state );

	CString	m_sSpritePath;
	RageTexture* m_pTexture;
	bool	m_bDrawIfTextureNull;

	struct State
	{
		int iFrameIndex;
		float fDelay;	// "seconds to show"
	};
	vector<State> m_States;
	int		m_iCurState;
	float	m_fSecsIntoState;	// number of seconds that have elapsed since we switched to this frame

	bool m_bUsingCustomTexCoords;
	float m_CustomTexCoords[8];     // (x,y) * 4: top left, bottom left, bottom right, top right

	// Remembered clipped dimensions are applied on Load().
	// -1 means no remembered dimensions;
	float	m_fRememberedClipWidth, m_fRememberedClipHeight;

	float m_fTexCoordVelocityX;
	float m_fTexCoordVelocityY;
};

#endif
