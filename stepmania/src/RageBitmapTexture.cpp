#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: RageBitmapTexture.h

 Desc: Holder for a static texture with metadata.  Can load just about any image format.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
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
#include "RageLog.h"
#include "RageException.h"



//-----------------------------------------------------------------------------
// RageBitmapTexture constructor
//-----------------------------------------------------------------------------
RageBitmapTexture::RageBitmapTexture( 
	RageDisplay* pScreen, 
	const CString &sFilePath, 
	DWORD dwMaxSize, 
	DWORD dwTextureColorDepth,
	int iMipMaps,
	int iAlphaBits,
	bool bDither,
	bool bStretch
	) :
	RageTexture( pScreen, sFilePath, dwMaxSize, dwTextureColorDepth, iMipMaps, iAlphaBits, bDither, bStretch )
{
//	LOG->WriteLine( "RageBitmapTexture::RageBitmapTexture()" );

	m_pd3dTexture = NULL;

	//if( !LoadFromCacheFile() )
		Create( dwMaxSize, dwTextureColorDepth, iMipMaps, iAlphaBits, bDither, bStretch );
	
	//SaveToCache();

	CreateFrameRects();
}

RageBitmapTexture::~RageBitmapTexture()
{
	SAFE_RELEASE(m_pd3dTexture);
}

void RageBitmapTexture::Reload( 	
	DWORD dwMaxSize, 
	DWORD dwTextureColorDepth,
	int iMipMaps,
	int iAlphaBits,
	bool bDither,
	bool bStretch
	)
{
	SAFE_RELEASE(m_pd3dTexture);
	Create( dwMaxSize, dwTextureColorDepth, iMipMaps, iAlphaBits, bDither, bStretch );
	// leave m_iRefCount alone!
	CreateFrameRects();
}

//-----------------------------------------------------------------------------
// GetTexture
//-----------------------------------------------------------------------------
LPDIRECT3DTEXTURE8 RageBitmapTexture::GetD3DTexture()
{
	return m_pd3dTexture; 
}


void RageBitmapTexture::Create( 
	DWORD dwMaxSize, 
	DWORD dwTextureColorDepth, 
	int iMipMaps,
	int iAlphaBits,
	bool bDither,
	bool bStretch
	)
{
	HRESULT hr;

	// look in the file name for a format hints
	m_sFilePath.MakeLower();

	if( -1 != m_sFilePath.Find("no alpha") )
		iAlphaBits = 0;
	else if( -1 != m_sFilePath.Find("1 alpha") )
		iAlphaBits = 1;
	if( -1 != m_sFilePath.Find("dither") )
		bDither = true; 

	///////////////////////
	// Figure out which texture format to use
	///////////////////////
	D3DFORMAT fmtTexture;
	switch( dwTextureColorDepth )
	{
	case 16:
		switch( iAlphaBits )
		{
		case 0:		fmtTexture = D3DFMT_R5G6B5;		break;
		case 1:		fmtTexture = D3DFMT_A1R5G5B5;	break;
		default:	fmtTexture = D3DFMT_A4R4G4B4;	break;
		}
		break;
	case 32:
		fmtTexture = D3DFMT_A8R8G8B8;
		break;
	default:
		throw RageException( "Invalid color depth: %d bits", dwTextureColorDepth );
	}

	
	/////////////////////
	// Figure out whether the texture can fit into texture memory unscaled
	/////////////////////
	D3DXIMAGE_INFO ddii;
	if( FAILED( hr = D3DXGetImageInfoFromFile( 
		m_sFilePath,
		&ddii ) ) )
	{
        throw RageException( hr, "D3DXGetImageInfoFromFile() failed for file '%s'.", m_sFilePath );
	}

	// find out what the min texture size is
	dwMaxSize = min( dwMaxSize, DISPLAY->GetDeviceCaps().MaxTextureWidth );

	bStretch |= ddii.Width > dwMaxSize || ddii.Height > dwMaxSize;
	
/*
	// HACK:  On a Voodoo3 and Win98, D3DXCreateTextureFromFileEx sometimes fails for no good reason.
	// So, we'll try the call 2x in a row in case the first one fails
*/
	// I'm taking out the Savage hack because it's causing problems.  Tough luck for them.  I think
	// the problem can be worked around by setting MaxTextureSize to 512.
	for( int i=0; i<2; i++ )
	{
		if( FAILED( hr = D3DXCreateTextureFromFileEx( 
			m_pd3dDevice,				// device
			m_sFilePath,				// soure file
			bStretch ? dwMaxSize : D3DX_DEFAULT,	// width 
			bStretch ? dwMaxSize : D3DX_DEFAULT,	// height 
			iMipMaps,					// mip map levels
			0,							// usage (is a render target?)
			fmtTexture,					// our preferred texture format
			D3DPOOL_MANAGED,			// which memory pool
			(bStretch ? D3DX_FILTER_BOX : D3DX_FILTER_NONE) | (bDither ? D3DX_FILTER_DITHER : 0),		// filter
			D3DX_FILTER_BOX | (bDither ? D3DX_FILTER_DITHER : 0),				// mip filter
			0,							// no color key
			&ddii,						// struct to fill with source image info
			NULL,						// no palette
			&m_pd3dTexture ) ) )
		{
			if( i==0 )
			{
				LOG->WriteLine( "WARNING! D3DXCreateTextureFromFileEx failed.  Sleep and try one more time..." );
				::Sleep( 10 );
				continue;
			}
			throw RageException( hr, "D3DXCreateTextureFromFileEx() failed for file '%s'.", m_sFilePath );
		}
		else
			break;
	}

	/////////////////////
	// Save information about the texture
	/////////////////////
	m_iSourceWidth = ddii.Width;
	m_iSourceHeight= ddii.Height;

	D3DSURFACE_DESC ddsd;
	if ( FAILED( hr = m_pd3dTexture->GetLevelDesc( 0, &ddsd ) ) ) 
		throw RageException( hr, "Could not get level Description of D3DX texture!" );

	// save information about the texture
	m_iTextureWidth		= ddsd.Width;
	m_iTextureHeight	= ddsd.Height;
	m_TextureFormat		= ddsd.Format;	


	if( bStretch )
	{
		m_iImageWidth	= m_iTextureWidth;
		m_iImageHeight	= m_iTextureHeight;
	}
	else
	{
		m_iImageWidth	= m_iSourceWidth;
		m_iImageHeight	= m_iSourceHeight;
	}

	LOG->WriteLine( "RageBitmapTexture: Loaded '%s' (%ux%u) from disk.  bStretch = %d, source %d,%d;  image %d,%d.", 
		m_sFilePath, 
		GetTextureWidth(), 
		GetTextureHeight(),
		bStretch,
		m_iSourceWidth,
		m_iSourceHeight,
		m_iImageWidth,
		m_iImageHeight
		);
}

