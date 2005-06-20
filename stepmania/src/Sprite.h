/* Sprite - A bitmap Actor that animates and moves around. */

#ifndef SPRITE_H
#define SPRITE_H

#include "Actor.h"
#include "RageTextureID.h"

class RageTexture;

class Sprite: public Actor
{
public:
	Sprite();
	virtual ~Sprite();

	void LoadFromNode( const CString& sDir, const XNode* pNode );

	virtual bool EarlyAbortDraw();
	virtual void DrawPrimitives();
	virtual void Update( float fDeltaTime );

	void UpdateAnimationState();	// take m_fSecondsIntoState, and move to a new state

	/* Adjust texture properties for song backgrounds. */
	static RageTextureID SongBGTexture( RageTextureID ID );

	/* Adjust texture properties for song banners. */
	static RageTextureID SongBannerTexture( RageTextureID ID );

	/* Just a convenience function; load an image that'll be used in the
	 * background. */
	virtual bool LoadBG( RageTextureID ID );
	virtual bool Load( RageTextureID ID );

	void UnloadTexture();
	RageTexture* GetTexture() { return m_pTexture; };

	virtual void EnableAnimation( bool bEnable );
	
	virtual int GetNumStates() const;
	virtual void SetState( int iNewState );
	virtual float GetAnimationLengthSeconds() const;
	virtual void SetSecondsIntoAnimation( float fSeconds );
	
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
	void SetPosition( float f );
	void SetLooping( bool b );
	void SetPlaybackRate( float f );


	void SetTexCoordVelocity(float fVelX, float fVelY) { m_fTexCoordVelocityX = fVelX; m_fTexCoordVelocityY = fVelY; }	
	// Scale the Sprite maintaining the aspect ratio so that it fits 
	// within (fWidth,fHeight) and is clipped to (fWidth,fHeight).
	void ScaleToClipped( float fWidth, float fHeight );
	static bool IsDiagonalBanner( int iWidth, int iHeight );

	//
	// Commands
	//
	virtual void PushSelf( lua_State *L );

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
	bool m_bSkipNextUpdate;
	float m_CustomTexCoords[8];     // (x,y) * 4: top left, bottom left, bottom right, top right

	// Remembered clipped dimensions are applied on Load().
	// -1 means no remembered dimensions;
	float	m_fRememberedClipWidth, m_fRememberedClipHeight;

	float m_fTexCoordVelocityX;
	float m_fTexCoordVelocityY;
};

#endif

/*
 * (c) 2001-2004 Chris Danford
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
