#include "stdafx.h"
//-----------------------------------------------------------------------------
// File: RageBitmapTexture.cpp
//
// Desc: 
//
// Copyright (c) 2001 Chris Danford.  All rights reserved.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// In-line Links
//-----------------------------------------------------------------------------
//#pragma comment(lib, "winmm.lib") 
#pragma comment(lib, "dxerr8.lib")
 
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "RageBitmapTexture.h"
#include "dxerr8.h"
#include "DXUtil.h"
#include "RageUtil.h"

//#include <stdio.h>
#include <assert.h>

//-----------------------------------------------------------------------------
// RageBitmapTexture constructor
//-----------------------------------------------------------------------------
RageBitmapTexture::RageBitmapTexture( LPRageScreen pScreen, CString sFilePath ) :
	RageTexture( pScreen, sFilePath )
{
//	RageLog( "RageBitmapTexture::RageBitmapTexture()" );

	Create();
	
	CreateFrameRects();
}

RageBitmapTexture::~RageBitmapTexture()
{
	SAFE_RELEASE(m_pd3dTexture);
}


//-----------------------------------------------------------------------------
// GetTexture
//-----------------------------------------------------------------------------
LPDIRECT3DTEXTURE8 RageBitmapTexture::GetD3DTexture()
{
	return m_pd3dTexture; 
}


VOID RageBitmapTexture::Create()
{
	HRESULT hr;

	D3DXIMAGE_INFO ddii;

	// load texture
	if (FAILED (hr = D3DXCreateTextureFromFileEx( 
						m_pd3dDevice,
						m_sFilePath,
						D3DX_DEFAULT, D3DX_DEFAULT, 
						D3DX_DEFAULT, 
						0, 
						D3DFMT_A4R4G4B4, //D3DFMT_UNKNOWN, // get format from source
						D3DPOOL_MANAGED, 
						D3DX_FILTER_NONE, // don't blow up the image to the texure size
						D3DX_FILTER_NONE,
						0,		// no color key
						&ddii, 
						NULL, // no palette
						&m_pd3dTexture ) ) )
        RageErrorHr( ssprintf("D3DXCreateTextureFromFileEx() failed for file '%s'.", m_sFilePath), hr );


	m_uImageWidth = ddii.Width;
	m_uImageHeight= ddii.Height;


    // D3DXCreateTexture can silently change the parameters on us
    D3DSURFACE_DESC ddsd;
    if ( FAILED( hr = m_pd3dTexture->GetLevelDesc( 0, &ddsd ) ) ) 
        RageErrorHr( "Could not get level Description of D3DX texture!", hr );

	m_uTextureWidth = ddsd.Width;
	m_uTextureHeight = ddsd.Height;
    m_TextureFormat = ddsd.Format;
//    if (m_TextureFormat != D3DFMT_A8R8G8B8 &&
//        m_TextureFormat != D3DFMT_A1R5G5B5) {
//        DXTRACE_ERR(TEXT("Texture is format we can't handle! Format = 0x%x"), m_TextureFormat);
//        return E_FAIL;
//    }
	
}

