#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: RageBitmapTexture.h

 Desc: Holder for a static texture with metadata.  Can load just about any image format.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


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
						D3DFMT_A4R4G4B4, // this is our preferred format
						D3DPOOL_MANAGED, 
						D3DX_DEFAULT,
						D3DX_DEFAULT,
						0,		// no color key
						&ddii, 
						NULL, // no palette
						&m_pd3dTexture ) ) )
        RageErrorHr( ssprintf("D3DXCreateTextureFromFileEx() failed for file '%s'.", m_sFilePath), hr );

	// save the source image's width and height
	m_uSourceWidth = ddii.Width;
	m_uSourceHeight= ddii.Height;

	
	//RageLog( "info.Width = %d, info.Height = %d, devCaps.MaxTextureWidth = %d, devCaps.MaxTextureHeight = %d",
	//	info.Width, info.Height, devCaps.MaxTextureWidth, devCaps.MaxTextureHeight );


    D3DSURFACE_DESC ddsd;
    if ( FAILED( hr = m_pd3dTexture->GetLevelDesc( 0, &ddsd ) ) ) 
        RageErrorHr( "Could not get level Description of D3DX texture!", hr );

    // save information about the texture
	m_uTextureWidth		= ddsd.Width;
	m_uTextureHeight	= ddsd.Height;
    m_TextureFormat		= ddsd.Format;	
}

