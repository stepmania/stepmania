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



#define MAX_SPRITE_STATES 256



class Sprite: public Actor
{
public:
	Sprite();
	virtual ~Sprite();

	virtual bool Load( CString sFilePath ) { return Load( sFilePath, RageTexturePrefs() ); }
	virtual bool Load( CString sFilePath, RageTexturePrefs prefs )
	{
		ASSERT( sFilePath != "" );
		if( sFilePath.Right(7) == ".sprite" )
			return LoadFromSpriteFile( sFilePath, prefs );
		else 
			return LoadFromTexture( sFilePath, prefs );
	};
	void UnloadTexture();
	RageTexture* GetTexture() { return m_pTexture; };

	virtual void DrawPrimitives();
	virtual void Update( float fDeltaTime );

	virtual void StartAnimating()	{ m_bIsAnimating = true; if(m_pTexture) m_pTexture->Play(); };
	virtual void StopAnimating()	{ m_bIsAnimating = false; if(m_pTexture) m_pTexture->Pause(); };
	virtual void SetState( int iNewState );
	
	int		GetNumStates()		{ return m_iNumStates; };
	CString	GetTexturePath()	{ return m_sTexturePath; };


	void SetCustomTextureRect( const RectF &new_texcoord_frect );
	void SetCustomTextureCoords( float fTexCoords[8] );
	void GetCustomTextureCoords( float fTexCoordsOut[8] );
	void SetCustomSourceRect( const RectF &rectSourceCoords );	// in source pixel space
	void SetCustomImageRect( RectF rectImageCoords );	// in image pixel space
	void SetCustomImageCoords( float fImageCoords[8] );
	void StopUsingCustomCoords();

protected:
	virtual bool LoadFromTexture( CString sTexturePath, RageTexturePrefs prefs );
	virtual bool LoadFromSpriteFile( CString sSpritePath, RageTexturePrefs prefs );


	CString	m_sSpritePath;
	RageTexture* m_pTexture;
	bool	m_bDrawIfTextureNull;
	CString	m_sTexturePath;

	int		m_iStateToFrame[MAX_SPRITE_STATES];	// array of indicies into m_rectBitmapFrames
	float	m_fDelay[MAX_SPRITE_STATES];
	int		m_iNumStates;
	int		m_iCurState;
	bool	m_bIsAnimating;
	float	m_fSecsIntoState;	// number of seconds that have elapsed since we switched to this frame

	bool m_bUsingCustomTexCoords;
	//FRECT m_CustomTexCoordRect;
	float m_CustomTexCoords[8];	// (x,y) * 4
};

#endif
