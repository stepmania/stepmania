#pragma once
/*
-----------------------------------------------------------------------------
 Class: RageMovieTexture

 Desc: Based on the DShowTextures example in the DX8 SDK.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageDisplay.h"
#include "RageTexture.h"
#include <d3dx8.h>
//#include <d3d8types.h>
#include <atlbase.h>

#include "baseclasses/streams.h"

//-----------------------------------------------------------------------------
// RageMovieTexture Class Declarations
//-----------------------------------------------------------------------------
class RageMovieTexture : public RageTexture
{
public:
	RageMovieTexture( 
		RageDisplay* pScreen, 
		const CString &sFilePath, 
		int dwMaxSize = 2048, 
		int dwTextureColorDepth = 16, 
		int iMipMaps = 4,
		int iAlphaBits = 4,
		bool bDither = false,
		bool bStretch = false
		);
	virtual ~RageMovieTexture();

	virtual void Reload( 
		int dwMaxSize, 
		int dwTextureColorDepth,
		int iMipMaps = 4,
		int iAlphaBits = 4,
		bool bDither = false,
		bool bStretch = false
		);

	LPDIRECT3DTEXTURE8 GetD3DTexture();
	virtual void Play();
	virtual void Pause();
	virtual void Stop();
	virtual void SetPosition( float fSeconds );
	virtual bool IsAMovie() const { return true; };
	virtual bool IsPlaying() const;
	void SetLooping(bool looping=true) { m_bLoop = looping; }

	LPDIRECT3DTEXTURE8 GetBackBuffer() { return m_pd3dTexture[!m_iIndexFrontBuffer]; }
	void Flip() { m_iIndexFrontBuffer = !m_iIndexFrontBuffer; }

protected:
	LPDIRECT3DTEXTURE8  m_pd3dTexture[2];	// double buffered
	int m_iIndexFrontBuffer;	// index of the buffer that should be rendered from - either 0 or 1

	virtual void Create();

	virtual HRESULT CreateD3DTexture();
	virtual HRESULT InitDShowTextureRenderer();
	virtual HRESULT PlayMovie();
	virtual void CheckMovieStatus();

	//-----------------------------------------------------------------------------
	// DirectShow pointers
	//-----------------------------------------------------------------------------
	CComPtr<IGraphBuilder>  m_pGB;          // GraphBuilder
	bool					m_bLoop;
	bool					m_bPlaying;
};
