//-----------------------------------------------------------------------------
// File: RageMovieTexture.h
//
// Desc: Based on the DShowTextures example in the DX8 SDK.
//
// Copyright (c) 2001 Chris Danford.  All rights reserved.
//-----------------------------------------------------------------------------

#ifndef _RAGEMOVIETEXTURE_H_
#define _RAGEMOVIETEXTURE_H_


#include "RageScreen.h"
#include <d3dx8.h>
//#include <d3d8types.h>
#include <atlbase.h>

#include <streams.h>

//-----------------------------------------------------------------------------
// Define GUID for Texture Renderer
// {71771540-2017-11cf-AE26-0020AFD79767}
//-----------------------------------------------------------------------------
struct __declspec(uuid("{71771540-2017-11cf-ae26-0020afd79767}")) CLSID_TextureRenderer;


//-----------------------------------------------------------------------------
// CTextureRenderer Class Declarations
//
//	Usage: 1) CheckMediaType is called by the graph
//         2) SetMediaType is called by the graph
//         3) call GetVidWidth and GetVidHeight to get texture information
//         4) call SetRenderTarget
//         5) Do RenderSample is called by the graph
//-----------------------------------------------------------------------------
class CTextureRenderer : public CBaseVideoRenderer
{
public:
    CTextureRenderer(LPUNKNOWN pUnk,HRESULT *phr);
    ~CTextureRenderer();

public:
	// overwritten methods
    HRESULT CheckMediaType(const CMediaType *pmt );     // Format acceptable?
    HRESULT SetMediaType(const CMediaType *pmt );       // Video format notification
    HRESULT DoRenderSample(IMediaSample *pMediaSample); // New video sample

	// new methods
	LONG GetVidWidth() {return m_lVidWidth;};
	LONG GetVidHeight(){return m_lVidHeight;};
	HRESULT SetRenderTarget(LPDIRECT3DTEXTURE8 pTexture);
	BOOL IsLocked() { return m_bLocked; };

protected:
	LONG m_lVidWidth;	// Video width
	LONG m_lVidHeight;	// Video Height
	LONG m_lVidPitch;	// Video Pitch

	LPDIRECT3DTEXTURE8	m_pTexture;	// the video surface we will copy new frames to
	D3DFORMAT			m_TextureFormat; // Texture format
	BOOL				m_bLocked;	// Is the texture currently locked while we 
									// copy the movie frame to it?
};


//-----------------------------------------------------------------------------
// RageMovieTexture Class Declarations
//-----------------------------------------------------------------------------
class RageMovieTexture : public RageTexture
{
public:
	RageMovieTexture( LPRageScreen pScreen, CString sFilePath );
	virtual ~RageMovieTexture();

	LPDIRECT3DTEXTURE8 GetD3DTexture();

protected:
	virtual VOID Create();

	virtual HRESULT CreateD3DTexture();
	virtual HRESULT InitDShowTextureRenderer();
	virtual HRESULT PlayMovie();
	virtual void CheckMovieStatus();
	virtual void CleanupDShow();

	//-----------------------------------------------------------------------------
	// DirectShow pointers
	//-----------------------------------------------------------------------------
	CComPtr<IGraphBuilder>  m_pGB;          // GraphBuilder
	CComPtr<IMediaControl>  m_pMC;          // Media Control
	CComPtr<IMediaPosition> m_pMP;          // Media Postion
	CComPtr<IMediaEvent>    m_pME;          // Media Event
    CTextureRenderer        *m_pCTR;        // DShow Texture renderer
};

typedef RageMovieTexture* LPRageMovieTexture;


#endif