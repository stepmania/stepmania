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
#include "GameInfo.h"

//#include <stdio.h>
#include <assert.h>

//-----------------------------------------------------------------------------
// RageBitmapTexture constructor
//-----------------------------------------------------------------------------
RageBitmapTexture::RageBitmapTexture( LPRageScreen pScreen, CString sFilePath, DWORD dwHints ) :
	RageTexture( pScreen, sFilePath )
{
//	RageLog( "RageBitmapTexture::RageBitmapTexture()" );

	m_pd3dTexture = NULL;

	Create( dwHints );
	
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


void RageBitmapTexture::Create( DWORD dwHints )
{
	HRESULT hr;

	///////////////////////
	// Figure out which texture format to use
	///////////////////////
	D3DFORMAT fmtTexture;

	// look in the file name for a format hint
	m_sFilePath.MakeLower();
	if( -1 != m_sFilePath.Find("no alpha") )
	{
		fmtTexture = D3DFMT_R5G6B5;
	}
	else if( -1 != m_sFilePath.Find("1 alpha") )
	{
		fmtTexture = D3DFMT_A1R5G5B5;
	}
	else	// no hint, assume full alpha
	{
		fmtTexture = D3DFMT_A4R4G4B4;
	}


	// look in the file name for a dither hint
	bool bDither = (dwHints & HINT_DITHER)  
	           ||  -1 != m_sFilePath.Find("dither");


	bool bCreateMipMaps = !(dwHints & HINT_NOMIPMAPS);

	
	// if the user has requested high color textures, use the higher color
	if( GAMEINFO != NULL
	 && GAMEINFO->m_GameOptions.m_iDisplayColor == 32 
	 && GAMEINFO->m_GameOptions.m_iTextureColor == 32 )
	{
		fmtTexture = D3DFMT_A8R8G8B8;
	}


	/////////////////////
	// Figure out whether the texture can fit into texture memory unscaled
	/////////////////////
	bool bScaleImageToTextureSize;

	D3DXIMAGE_INFO ddii;
	if( FAILED( hr = D3DXGetImageInfoFromFile( 
		m_sFilePath,
		&ddii ) ) )
	{
        RageErrorHr( ssprintf("D3DXGetImageInfoFromFile() failed for file '%s'.", m_sFilePath), hr );
	}

	D3DCAPS8 caps;
	m_pd3dDevice->GetDeviceCaps( &caps );

	bScaleImageToTextureSize = ddii.Width > caps.MaxTextureWidth 
						    || ddii.Height > caps.MaxTextureHeight;
	

	if( FAILED( hr = D3DXCreateTextureFromFileEx( 
		m_pd3dDevice,				// device
		m_sFilePath,				// soure file
		D3DX_DEFAULT, D3DX_DEFAULT,	// width, height 
		bCreateMipMaps ? 4 : 0,		// mip map levels
		0,							// usage (is a render target?)
		fmtTexture,					// our preferred texture format
		D3DPOOL_MANAGED,			// which memory pool
		(bScaleImageToTextureSize ? D3DX_FILTER_BOX : D3DX_FILTER_NONE) | (bDither ? D3DX_FILTER_DITHER : 0),		// filter
		D3DX_DEFAULT | (bDither ? D3DX_FILTER_DITHER : 0),				// mip filter
		0,							// no color key
		&ddii,						// struct to fill with source image info
		NULL,						// no palette
		&m_pd3dTexture ) ) )
	{
        RageErrorHr( ssprintf("D3DXCreateTextureFromFileEx() failed for file '%s'.", m_sFilePath), hr );
	}


	/////////////////////
	// Save information about the texture
	/////////////////////
	m_uSourceWidth = ddii.Width;
	m_uSourceHeight= ddii.Height;

    D3DSURFACE_DESC ddsd;
    if ( FAILED( hr = m_pd3dTexture->GetLevelDesc( 0, &ddsd ) ) ) 
        RageErrorHr( "Could not get level Description of D3DX texture!", hr );

    // save information about the texture
	m_uTextureWidth		= ddsd.Width;
	m_uTextureHeight	= ddsd.Height;
    m_TextureFormat		= ddsd.Format;	

	if( bScaleImageToTextureSize )
	{
		m_uImageWidth	= m_uTextureWidth;
		m_uImageHeight	= m_uTextureHeight;
	}
	else
	{
		m_uImageWidth	= m_uSourceWidth;
		m_uImageHeight	= m_uSourceHeight;
	}
}

