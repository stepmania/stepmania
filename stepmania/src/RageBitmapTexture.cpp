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
#include "RageHelper.h"


//-----------------------------------------------------------------------------
// RageBitmapTexture constructor
//-----------------------------------------------------------------------------
RageBitmapTexture::RageBitmapTexture( 
	RageScreen* pScreen, 
	const CString &sFilePath, 
	const DWORD dwMaxSize, 
	const DWORD dwTextureColorDepth,
	const DWORD dwHints ) :
	RageTexture( pScreen, sFilePath )
{
//	HELPER.Log( "RageBitmapTexture::RageBitmapTexture()" );

	m_pd3dTexture = NULL;

	Create( dwMaxSize, dwTextureColorDepth, dwHints );
	
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


void RageBitmapTexture::Create( DWORD dwMaxSize, DWORD dwTextureColorDepth, DWORD dwHints )
{
	HRESULT hr;

	///////////////////////
	// Figure out which texture format to use
	///////////////////////
	D3DFORMAT fmtTexture;

	// look in the file name for a format hint
	m_sFilePath.MakeLower();

	switch( dwTextureColorDepth )
	{
	case 16:
		if( -1 != m_sFilePath.Find("no alpha") )
			fmtTexture = D3DFMT_R5G6B5;
		else if( -1 != m_sFilePath.Find("1 alpha") )
			fmtTexture = D3DFMT_A1R5G5B5;
		else
			fmtTexture = D3DFMT_A4R4G4B4;
		break;
	case 32:
		fmtTexture = D3DFMT_A8R8G8B8;
		break;
	default:
		ASSERT( false );	// invalid color depth value
	}


	// look in the file name for a dither hint
	bool bDither = (dwHints & TEXTURE_HINT_DITHER)  
	           ||  -1 != m_sFilePath.Find("dither");


	bool bCreateMipMaps = !(dwHints & TEXTURE_HINT_NOMIPMAPS);

	
	/////////////////////
	// Figure out whether the texture can fit into texture memory unscaled
	/////////////////////
	bool bScaleImageToTextureSize;

	D3DXIMAGE_INFO ddii;
	if( FAILED( hr = D3DXGetImageInfoFromFile( 
		m_sFilePath,
		&ddii ) ) )
	{
        HELPER.FatalErrorHr( hr, "D3DXGetImageInfoFromFile() failed for file '%s'.", m_sFilePath );
	}

	D3DCAPS8 caps;
	m_pd3dDevice->GetDeviceCaps( &caps );
	
	// find out what the min texture size is
	dwMaxSize = min( dwMaxSize, caps.MaxTextureWidth );

	bScaleImageToTextureSize = ddii.Width > dwMaxSize || ddii.Height > dwMaxSize;
	
/*
	// HACK:  The stupid Savage driver will report that it can hold the entire texture, 
	//   then allocate something smaller than the dimensions we need!
	//   after allocating the texture, make sure it's the size we expect.  If not,
	//   load it again with scaling turned on.
*/
	// I'm taking out the Savage hack because it's causing problems.  Tough luck for them.  I think
	// the problem can be worked around by setting MaxTextureSize to 512.
	if( FAILED( hr = D3DXCreateTextureFromFileEx( 
		m_pd3dDevice,				// device
		m_sFilePath,				// soure file
		D3DX_DEFAULT,	// width 
		D3DX_DEFAULT,	// height 
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
		HELPER.FatalErrorHr( hr, "D3DXCreateTextureFromFileEx() failed for file '%s'.", m_sFilePath );
	}

	/////////////////////
	// Save information about the texture
	/////////////////////
	m_iSourceWidth = ddii.Width;
	m_iSourceHeight= ddii.Height;

	D3DSURFACE_DESC ddsd;
	if ( FAILED( hr = m_pd3dTexture->GetLevelDesc( 0, &ddsd ) ) ) 
		HELPER.FatalErrorHr( hr, "Could not get level Description of D3DX texture!" );

	// save information about the texture
	m_iTextureWidth		= ddsd.Width;
	m_iTextureHeight	= ddsd.Height;
	m_TextureFormat		= ddsd.Format;	


	if( bScaleImageToTextureSize )
	{
		m_iImageWidth	= m_iTextureWidth;
		m_iImageHeight	= m_iTextureHeight;
	}
	else
	{
		m_iImageWidth	= m_iSourceWidth;
		m_iImageHeight	= m_iSourceHeight;
	}
}

