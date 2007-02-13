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
	Sprite( const Sprite &cpy );
	virtual ~Sprite();

	void LoadFromNode( const XNode* pNode );
	virtual Sprite *Copy() const;

	virtual bool EarlyAbortDraw() const;
	virtual void DrawPrimitives();
	virtual void Update( float fDeltaTime );

	void UpdateAnimationState();	// take m_fSecondsIntoState, and move to a new state

	/* Adjust texture properties for song backgrounds. */
	static RageTextureID SongBGTexture( RageTextureID ID );

	/* Adjust texture properties for song banners. */
	static RageTextureID SongBannerTexture( RageTextureID ID );

	virtual void Load( RageTextureID ID );

	void UnloadTexture();
	RageTexture* GetTexture() { return m_pTexture; };

	virtual void EnableAnimation( bool bEnable );
	
	virtual int GetNumStates() const;
	virtual void SetState( int iNewState );
	virtual float GetAnimationLengthSeconds() const;
	virtual void SetSecondsIntoAnimation( float fSeconds );
	
	RString	GetTexturePath() const;

	void SetCustomTextureRect( const RectF &new_texcoord_frect );
	void SetCustomTextureCoords( float fTexCoords[8] );
	void SetCustomImageRect( RectF rectImageCoords );	// in image pixel space
	void SetCustomImageCoords( float fImageCoords[8] );
	const RectF *GetCurrentTextureCoordRect() const;
	void StopUsingCustomCoords();
	void GetActiveTextureCoords(float fTexCoordsOut[8]) const;
	void StretchTexCoords( float fX, float fY );
	void AddImageCoords( float fX, float fY ); // in image pixel space

	void SetTexCoordVelocity(float fVelX, float fVelY);
	// Scale the Sprite maintaining the aspect ratio so that it fits 
	// within (fWidth,fHeight) and is clipped to (fWidth,fHeight).
	void ScaleToClipped( float fWidth, float fHeight );
	static bool IsDiagonalBanner( int iWidth, int iHeight );

	//
	// Commands
	//
	virtual void PushSelf( lua_State *L );

protected:
	void LoadFromTexture( RageTextureID ID );
	void LoadStatesFromTexture();

	void DrawTexture( const TweenState *state );

	RageTexture* m_pTexture;
	bool	m_bDrawIfTextureNull;

	struct State
	{
		RectF rect;
		float fDelay;	// "seconds to show"
	};
	vector<State> m_States;
	int		m_iCurState;
	float	m_fSecsIntoState;	// number of seconds that have elapsed since we switched to this frame

	bool m_bUsingCustomTexCoords;
	bool m_bSkipNextUpdate;
	float m_CustomTexCoords[8];	// (x,y) * 4: top left, bottom left, bottom right, top right

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
