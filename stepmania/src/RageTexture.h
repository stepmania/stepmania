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


//-----------------------------------------------------------------------------
// RageTexture Class Declarations
//-----------------------------------------------------------------------------
class RageTexture
{
public:
	RageTexture( LPRageScreen pScreen, CString sFilePath );
	virtual ~RageTexture() PURE;

	virtual LPDIRECT3DTEXTURE8 GetD3DTexture() PURE;

	UINT GetImageWidth()	{return m_uImageWidth;};
	UINT GetImageHeight()	{return m_uImageHeight;};
	UINT GetTextureWidth()	{return m_uTextureWidth;};
	UINT GetTextureHeight()	{return m_uTextureHeight;};

	UINT   GetFramesWide()  {return m_uFramesWide;};
	UINT   GetFramesHigh()  {return m_uFramesHigh;};
	UINT   GetImageFrameWidth()  {return m_uImageFrameWidth;};
	UINT   GetImageFrameHeight() {return m_uImageFrameHeight;};
	UINT   GetTextureFrameWidth()  {return m_uTextureFrameWidth;};
	UINT   GetTextureFrameHeight() {return m_uTextureFrameHeight;};
	float  GetWidthCorrectionRatio()  {return m_fWidthCorrectionRatio;};
	float  GetHeightCorrectionRatio() {return m_fHeightCorrectionRatio;};
	LPRECT GetFrameRect( UINT uFrameNo ) {return &m_FrameRects[uFrameNo];};
	UINT   GetNumFrames()	{return m_FrameRects.GetSize();};
	CString GetFilePath()	{return m_sFilePath;};

	INT					m_iRefCount;

protected:
	virtual VOID CreateFrameRects();
	virtual VOID GetFrameDimensionsFromFileName( CString sPath, UINT &uFramesWide, UINT &uFramesHigh ) const;

	CString				m_sFilePath;
	LPDIRECT3DDEVICE8   m_pd3dDevice;
	LPDIRECT3DTEXTURE8  m_pd3dTexture;

	UINT				m_uImageWidth,   m_uImageHeight;
	UINT				m_uTextureWidth, m_uTextureHeight;
	D3DFORMAT			m_TextureFormat; 

	// The number of frames of animation in each row and column of this texture.
	UINT				m_uFramesWide, m_uFramesHigh;
	UINT				m_uImageFrameWidth, m_uImageFrameHeight;
	UINT				m_uTextureFrameWidth, m_uTextureFrameHeight;

	float				m_fWidthCorrectionRatio;
	float				m_fHeightCorrectionRatio;

	// RECTs that hold the bounds of each frame in the bitmap.
	// e.g., if the texture has 4 frames of animation, the SrcRect for each frame would
	// be in m_FrameRects[0..4].
	CArray<RECT, RECT&>	m_FrameRects;	

};


#endif
