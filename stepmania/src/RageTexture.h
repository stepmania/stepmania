#pragma once
/*
-----------------------------------------------------------------------------
 Class: RageTexture

 Desc: Abstract class for a texture and holds metadata.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/




#include "RageDisplay.h"
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
	RageTexture( 
		RageDisplay* pScreen, 
		const CString &sFilePath, 
		DWORD dwMaxSize = 2048, 
		DWORD dwTextureColorDepth = 16, 
		int iMipMaps = 4,
		int iAlphaBits = 4,
		bool bDither = false,
		bool bStretch = false 
		);
	virtual ~RageTexture() = 0;

	virtual void Reload( 
		DWORD dwMaxSize = 2048, 
		DWORD dwTextureColorDepth = 16, 
		int iMipMaps = 4,
		int iAlphaBits = 4,
		bool bDither = false, 
		bool bStretch = false 
		) = 0;

	virtual LPDIRECT3DTEXTURE8 GetD3DTexture() = 0;
	virtual void Play() {};
	virtual void SetPosition( float fSeconds ) {};
	virtual void Pause() {};
	virtual bool IsAMovie() { return false; };

	int GetSourceWidth()	{return m_iSourceWidth;};
	int GetSourceHeight()	{return m_iSourceHeight;};
	int GetTextureWidth()	{return m_iTextureWidth;};
	int GetTextureHeight()	{return m_iTextureHeight;};
	int GetImageWidth()		{return m_iImageWidth;};
	int GetImageHeight()	{return m_iImageHeight;};

	int GetFramesWide()  {return m_iFramesWide;};
	int GetFramesHigh()  {return m_iFramesHigh;};

	int GetSourceFrameWidth()	{return GetSourceWidth()	/	GetFramesWide();};
	int GetSourceFrameHeight()	{return GetSourceHeight()	/	GetFramesHigh();};
	int GetTextureFrameWidth()	{return GetTextureWidth()	/	GetFramesWide();};
	int GetTextureFrameHeight(){return GetTextureHeight()	/	GetFramesHigh();};
	int GetImageFrameWidth()	{return GetImageWidth()		/	GetFramesWide();};
	int GetImageFrameHeight()	{return GetImageHeight()	/	GetFramesHigh();};
	
	FRECT* GetTextureCoordRect( int uFrameNo ) {return &m_TextureCoordRects[uFrameNo];};
	int   GetNumFrames()	{return m_TextureCoordRects.GetSize();};
	CString GetFilePath()	{return m_sFilePath;};

	INT					m_iRefCount;

protected:
	virtual void CreateFrameRects();
	virtual void GetFrameDimensionsFromFileName( CString sPath, int* puFramesWide, int* puFramesHigh ) const;

	CString				m_sFilePath;
	LPDIRECT3DDEVICE8   m_pd3dDevice;

	int				m_iSourceWidth,		m_iSourceHeight;	// dimensions of the original image loaded from disk
	int				m_iTextureWidth,	m_iTextureHeight;	// dimensions of the texture in memory
	int				m_iImageWidth,		m_iImageHeight;		// dimensions of the image in the texture
	int				m_iFramesWide,		m_iFramesHigh;		// The number of frames of animation in each row and column of this texture
	D3DFORMAT       m_TextureFormat;

	
	// RECTs that hold the bounds of each frame in the bitmap.
	// e.g., if the texture has 4 frames of animation, the SrcRect for each frame would
	// be in m_FrameRects[0..4].
	CArray<FRECT, FRECT&>	m_TextureCoordRects;	
};


