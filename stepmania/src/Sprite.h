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
#include "RageTexture.h"


class Sprite: public Actor
{
public:
	Sprite();
	virtual ~Sprite();

	virtual void DrawPrimitives();
	virtual void Update( float fDeltaTime );

	/* Just a convenience function; load an image that'll be used in the
	 * background. */
	virtual bool LoadBG( RageTextureID ID );
	virtual bool Load( RageTextureID ID );

	void UnloadTexture();
	RageTexture* GetTexture() { return m_pTexture; };

	virtual void EnableAnimation( bool bEnable );
	virtual void SetState( int iNewState );
	
	virtual int GetNumStates()		{ return m_States.size(); };
	CString	GetTexturePath()	{ return m_pTexture==NULL ? "" : m_pTexture->GetID().filename; };

	void SetCustomTextureCoords( const RectF &newTexCoords );
	const RectF* GetCustomTextureCoords() const;
	void SetCustomImageCoords( const RectF &newImageCoords );	// in image space (not texture space)
	void StopUsingCustomCoords();
	const RectF* GetActiveTextureCoords() const;	// depends on m_bUsingCustomTexCoords
	const RectF* GetCurrentTextureCoords() const;

protected:
	virtual bool LoadFromTexture( RageTextureID ID );
	virtual bool LoadFromSpriteFile( RageTextureID ID );


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
	RectF m_CustomTexCoords;
};

#endif
