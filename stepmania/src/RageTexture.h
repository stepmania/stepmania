/*
-----------------------------------------------------------------------------
 File: RageTexture.h

 Desc: Abstract class for a texture with metadata.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

class RageTexture;
typedef RageTexture* LPRageTexture;


#ifndef _RAGETEXTURE_H_
#define _RAGETEXTURE_H_


#include "RageScreen.h"
#include <d3dx8.h>
#include <assert.h>
//#include <d3d8types.h>
	

struct FRECT
{	
public:
	FRECT() {};
	FRECT(float l, float t, float r, float b) {
		left = l; top = t; right = r; bottom = b;
	};

	float    left, top, right, bottom;
};


//-----------------------------------------------------------------------------
// RageTexture Class Declarations
//-----------------------------------------------------------------------------
class RageTexture
{
public:
	RageTexture( LPRageScreen pScreen, CString sFilePath );
	virtual ~RageTexture() PURE;

	virtual LPDIRECT3DTEXTURE8 GetD3DTexture() PURE;

	UINT GetSourceWidth()	{return m_uSourceWidth;};
	UINT GetSourceHeight()	{return m_uSourceHeight;};
	UINT GetTextureWidth()	{return m_uTextureWidth;};
	UINT GetTextureHeight()	{return m_uTextureHeight;};

	UINT   GetFramesWide()  {return m_uFramesWide;};
	UINT   GetFramesHigh()  {return m_uFramesHigh;};
	UINT   GetSourceFrameWidth()  {return m_uSourceFrameWidth;};
	UINT   GetSourceFrameHeight() {return m_uSourceFrameHeight;};
	FRECT* GetTextureCoordRect( UINT uFrameNo ) {return &m_TextureCoordRects[uFrameNo];};
	UINT   GetNumFrames()	{return m_TextureCoordRects.GetSize();};
	CString GetFilePath()	{return m_sFilePath;};

	INT					m_iRefCount;

protected:
	virtual void CreateFrameRects();
	virtual void GetFrameDimensionsFromFileName( CString sPath, UINT* puFramesWide, UINT* puFramesHigh ) const;

	CString				m_sFilePath;
	LPDIRECT3DDEVICE8   m_pd3dDevice;
	LPDIRECT3DTEXTURE8  m_pd3dTexture;

	UINT				m_uSourceWidth,		m_uSourceHeight;	// dimensions of the original image
	UINT				m_uTextureWidth,	m_uTextureHeight;	// dimensions of the texture holding the image
	D3DFORMAT			m_TextureFormat; 

	// The number of frames of animation in each row and column of this texture.
	UINT				m_uFramesWide, m_uFramesHigh;
	UINT				m_uSourceFrameWidth, m_uSourceFrameHeight;
	

	// RECTs that hold the bounds of each frame in the bitmap.
	// e.g., if the texture has 4 frames of animation, the SrcRect for each frame would
	// be in m_FrameRects[0..4].
	CArray<FRECT, FRECT&>	m_TextureCoordRects;	

};


#endif
