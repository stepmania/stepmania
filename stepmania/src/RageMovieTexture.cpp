#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: RageMovieTexture

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


//-----------------------------------------------------------------------------
// In-line Links
//-----------------------------------------------------------------------------
#pragma comment(lib, "winmm.lib") 
#pragma comment(lib, "dxerr8.lib")
 
// Link with the DirectShow base class libraries
#if defined(DEBUG) | defined(_DEBUG)
	#pragma comment(lib, "baseclasses/debug/strmbasd.lib") 
#else
	#pragma comment(lib, "baseclasses/release/strmbase.lib") 
#endif
 
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "RageMovieTexture.h"
#include "dxerr8.h"
#include "DXUtil.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"


#include <stdio.h>

//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// CTextureRenderer constructor
//-----------------------------------------------------------------------------
CTextureRenderer::CTextureRenderer( LPUNKNOWN pUnk, HRESULT *phr )
                                   : CBaseVideoRenderer(__uuidof(CLSID_TextureRenderer), 
                                   NAME("Texture Renderer"), pUnk, phr)
{
    // Store and ARageef the texture for our use.
	m_pTexture = NULL;
	m_bBackBufferLocked = FALSE;
    *phr = S_OK;
}


//-----------------------------------------------------------------------------
// CTextureRenderer destructor
//-----------------------------------------------------------------------------
CTextureRenderer::~CTextureRenderer()
{
    // Do nothing
}


//-----------------------------------------------------------------------------
// CheckMediaType: This method forces the graph to give us an R8G8B8 video
// type, making our copy to texture memory trivial.
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::CheckMediaType(const CMediaType *pmt)
{
    HRESULT   hr = E_FAIL;
    VIDEOINFO *pvi;
    
    // Reject the connection if this is not a video type
    if( *pmt->FormatType() != FORMAT_VideoInfo ) {
        return E_INVALIDARG;
    }
    
    // Only accept RGB24
    pvi = (VIDEOINFO *)pmt->Format();
    if(IsEqualGUID( *pmt->Type(),    MEDIATYPE_Video)  &&
       IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_RGB24))
//       IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_RGB565))
    {
        hr = S_OK;
    }
    
    return hr;
}


//-----------------------------------------------------------------------------
// SetMediaType: Graph connection has been made. 
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::SetMediaType(const CMediaType *pmt)
{
//    HRESULT hr;

    // Retrive the size of this media type
    VIDEOINFO *pviBmp;                      // Bitmap info header
    pviBmp = (VIDEOINFO *)pmt->Format();
    m_lVidWidth  = pviBmp->bmiHeader.biWidth;
    m_lVidHeight = abs(pviBmp->bmiHeader.biHeight);
    m_lVidPitch = (m_lVidWidth * 3 + 3) & ~(3); // We are forcing RGB24


    return S_OK;
}


//-----------------------------------------------------------------------------
// SetRenderTarget: Save all the information we'll need to render to a D3D texture.
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::SetRenderTarget( RageMovieTexture* pTexture )
{
	HRESULT hr;

	LPDIRECT3DTEXTURE8 pD3DTexture = pTexture->GetD3DTexture();

	if (pD3DTexture == NULL) {
        DXTRACE_ERR(TEXT("SetRenderTarget called with a NULL texture!"), 0);
		return E_FAIL;
	}
	m_pTexture = pTexture;


	// get the format of the texture
    D3DSURFACE_DESC ddsd;
    if ( FAILED( hr = pD3DTexture->GetLevelDesc( 0, &ddsd ) ) ) {
        DXTRACE_ERR(TEXT("Could not get level Description of D3DX texture! hr = 0x%x"), hr);
        return hr;
    }

    m_TextureFormat = ddsd.Format;
    if (m_TextureFormat != D3DFMT_A8R8G8B8 &&
        m_TextureFormat != D3DFMT_A1R5G5B5) {
        DXTRACE_ERR(TEXT("Texture is format we can't handle! Format = 0x%x"), m_TextureFormat);
        return E_FAIL;
    }

	return S_OK;
}


//-----------------------------------------------------------------------------
// DoRenderSample: A sample has been delivered. Copy it to the texture.
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::DoRenderSample( IMediaSample * pSample )
{
    BYTE  *pBmpBuffer;		// Bitmap buffer
    
    // Get the video bitmap buffer
    pSample->GetPointer( &pBmpBuffer );


	// Find which texture we should render to.  We want to copy to the "back buffer"
	int iIndexBackBuffer = !m_pTexture->m_iIndexFrontBuffer;
	LPDIRECT3DTEXTURE8 pD3DTextureCopyTo = m_pTexture->m_pd3dTexture[iIndexBackBuffer];

    // Lock the Texture
    D3DLOCKED_RECT d3dlr;
    while( FAILED( pD3DTextureCopyTo->LockRect(0, &d3dlr, 0, 0) ) )
	{
        LOG->Warn( "Failed to lock the texture for rendering movie!" );
		// keep trying until we get the lock
	}
    
	m_bBackBufferLocked = TRUE;

    // Get the texture buffer & pitch
	BYTE  *pTxtBuffer = static_cast<byte *>(d3dlr.pBits);
    LONG  lTxtPitch = d3dlr.Pitch;
   
    // Copy the bits    
    // OPTIMIZATION OPPORTUNITY: Use a video and texture
    // format that allows a simpler copy than this one.
    switch( m_TextureFormat )
	{
	case D3DFMT_A8R8G8B8:
		{
			for(int y = 0; y < m_lVidHeight; y++ ) {
				BYTE *pBmpBufferOld = pBmpBuffer;
				BYTE *pTxtBufferOld = pTxtBuffer;   
				for (int x = 0; x < m_lVidWidth; x++) {
					pTxtBuffer[0] = pBmpBuffer[0];
					pTxtBuffer[1] = pBmpBuffer[1];
					pTxtBuffer[2] = pBmpBuffer[2];
					pTxtBuffer[3] = 0xff;
					pTxtBuffer += 4;
					pBmpBuffer += 3;
				}
				pBmpBuffer = pBmpBufferOld + m_lVidPitch;
				pTxtBuffer = pTxtBufferOld + lTxtPitch;
			}
		}
		break;
	case D3DFMT_A1R5G5B5:
		{
			for(int y = 0; y < m_lVidHeight; y++ ) {
				BYTE *pBmpBufferOld = pBmpBuffer;
				BYTE *pTxtBufferOld = pTxtBuffer;   
				for (int x = 0; x < m_lVidWidth; x++) {
					*(WORD *)pTxtBuffer =
						0x8000 +
						((pBmpBuffer[2] & 0xF8) << 7) +
						((pBmpBuffer[1] & 0xF8) << 2) +
						(pBmpBuffer[0] >> 3);
					pTxtBuffer += 2;
					pBmpBuffer += 3;
				}
				pBmpBuffer = pBmpBufferOld + m_lVidPitch;
				pTxtBuffer = pTxtBufferOld + lTxtPitch;
			}
		}
		break;
	default:
		ASSERT(0);
	}

        
    // Unlock the Texture
    if( FAILED( pD3DTextureCopyTo->UnlockRect(0) ) ) 
	{
        LOG->Warn( "Failed to unlock the texture!" );
        return E_FAIL;
	}

	m_bBackBufferLocked = FALSE;
    

	// flip active texture
	m_pTexture->m_iIndexFrontBuffer = !m_pTexture->m_iIndexFrontBuffer;


    return S_OK;
}


//-----------------------------------------------------------------------------
// RageMovieTexture constructor
//-----------------------------------------------------------------------------
RageMovieTexture::RageMovieTexture( 
	RageDisplay* pScreen, 
	const CString &sFilePath, 
	DWORD dwMaxSize, 
	DWORD dwTextureColorDepth,
	int iMipMaps,
	int iAlphaBits,
	bool bDither,
	bool bStretch ) :
  RageTexture( pScreen, sFilePath, dwMaxSize, dwTextureColorDepth, iMipMaps, iAlphaBits, bDither, bStretch )
{
	LOG->Trace( "RageBitmapTexture::RageBitmapTexture()" );

	m_pd3dTexture[0] = m_pd3dTexture[1] = NULL;
	m_iIndexFrontBuffer = 0;

	Create();

	CreateFrameRects();
	// flip all frame rects because movies are upside down
	for( int i=0; i<m_TextureCoordRects.GetSize(); i++ )
	{
		float fTemp = m_TextureCoordRects[i].top;
		m_TextureCoordRects[i].top = m_TextureCoordRects[i].bottom;
		m_TextureCoordRects[i].bottom = fTemp;
	}

	m_bLoop = true;
}

RageMovieTexture::~RageMovieTexture()
{
	CleanupDShow();

	SAFE_RELEASE(m_pd3dTexture[0]);
	SAFE_RELEASE(m_pd3dTexture[1]);
}

void RageMovieTexture::Reload( 
	DWORD dwMaxSize, 
	DWORD dwTextureColorDepth,
	int iMipMaps,
	int iAlphaBits,
	bool bDither,
	bool bStretch )
{
	// do nothing
}


//-----------------------------------------------------------------------------
// GetTexture
//-----------------------------------------------------------------------------
LPDIRECT3DTEXTURE8 RageMovieTexture::GetD3DTexture()
{
	CheckMovieStatus();		// restart the movie if we reach the end

	// Return the front buffer because it's guaranteed that CTextureRenderer
	// doesn't have it locked.	
	return m_pd3dTexture[m_iIndexFrontBuffer]; 
}


//-----------------------------------------------------------------------------
// RageMovieTexture::Create()
//-----------------------------------------------------------------------------
VOID RageMovieTexture::Create()
{
	HRESULT hr;

	// Initialize the filter graph find and get information about the
	// video (dimensions, color depth, etc.)
	if( FAILED( hr = InitDShowTextureRenderer() ) )
        throw RageException( hr, "Could not initialize the DirectShow Texture Renderer!" );

	if( FAILED( hr = CreateD3DTexture() ) )
        throw RageException( hr, "Could not create the D3D Texture!" );

	// Pass the D3D texture to our TextureRenderer so it knows 
	// where to render new movie frames to.
	if( FAILED( hr = m_pCTR->SetRenderTarget( this ) ) )
        throw RageException( hr, "RageMovieTexture: SetRenderTarget failed." );

	// Start the graph running
    if( FAILED( hr = PlayMovie() ) )
        throw RageException( hr, "Could not run the DirectShow graph." );

}


void HandleDivXError()
{
	/* Actually, we might need XviD; we might want to look
	 * at the file and try to figure out if it's something
	 * common: DIV3, DIV4, DIV5, XVID, or maybe even MPEG2. */
	throw RageException( 
		"Could not locate the DivX video codec.\n"
		"DivX is required to movie textures and must\n"
		"be installed before running the application.\n\n"
		"Please visit http://www.divx.com to download the latest version."
		);
	exit(1);
}

//-----------------------------------------------------------------------------
// InitDShowTextureRenderer : Create DirectShow filter graph and run the graph
//-----------------------------------------------------------------------------
HRESULT RageMovieTexture::InitDShowTextureRenderer()
{
    HRESULT hr = S_OK;
    CComPtr<IBaseFilter>    pFTR;           // Texture Renderer Filter
    CComPtr<IPin>           pFTRPinIn;      // Texture Renderer Input Pin
    CComPtr<IBaseFilter>    pFSrc;          // Source Filter
    CComPtr<IPin>           pFSrcPinOut;    // Source Filter Output Pin   
    
    // Create the filter graph
    if( FAILED( m_pGB.CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC) ) )
        throw RageException( hr, "Could not create CLSID_FilterGraph!" );
    
    // Create the Texture Renderer object
    m_pCTR = new CTextureRenderer(NULL, &hr);
    if( FAILED(hr) )
        throw RageException( hr, "Could not create texture renderer object!" );
    
    // Get a pointer to the IBaseFilter on the TextureRenderer, add it to graph
    pFTR = m_pCTR;
    if( FAILED( hr = m_pGB->AddFilter(pFTR, L"TEXTURERENDERER" ) ) )
        throw RageException( hr, "Could not add renderer filter to graph!" );
    
    // convert movie file path to wide char string
	WCHAR wFileName[MAX_PATH];

    #ifndef UNICODE
        MultiByteToWideChar(CP_ACP, 0, m_sFilePath, -1, wFileName, MAX_PATH);
    #else
        lstrcpy(wFileName, m_szFilePath);
    #endif


    // Add the source filter
    if( FAILED( hr = m_pGB->AddSourceFilter( wFileName, L"SOURCE", &pFSrc ) ) )		// if this fails, it's probably because the user doesn't have DivX installed
	{
		HandleDivXError();
        throw RageException( hr, "Could not create source filter to graph!" );
	}
    
    // Find the source's output and the renderer's input
    if( FAILED( hr = pFTR->FindPin( L"In", &pFTRPinIn ) ) )
        throw RageException( hr, "Could not find input pin!" );

    if( FAILED( hr = pFSrc->FindPin( L"Output", &pFSrcPinOut ) ) )
        throw RageException( hr, "Could not find output pin!" );
    
    // Connect these two filters
    if( FAILED( hr = m_pGB->Connect( pFSrcPinOut, pFTRPinIn ) ) )
	{
 		HandleDivXError();
		throw RageException( hr, "Could not connect pins!" );
	}
    
    // Get the graph's media control, event & position interfaces
    m_pGB.QueryInterface(&m_pMC);
    m_pGB.QueryInterface(&m_pMP);
    m_pGB.QueryInterface(&m_pME);

    // The graph is built, now get the set the output video width and height.
	// The source and image width will always be the same since we can't scale the video
	m_iSourceWidth  = m_pCTR->GetVidWidth();
	m_iSourceHeight = m_pCTR->GetVidHeight();
	m_iImageWidth   = m_iSourceWidth;
	m_iImageHeight  = m_iSourceHeight;

	
    return S_OK;
}

HRESULT RageMovieTexture::CreateD3DTexture()
{
	HRESULT hr;

	//////////////////////////////////////////////////
    // Create the texture that maps to this media type
	//////////////////////////////////////////////////
    if( FAILED( hr = D3DXCreateTexture(m_pd3dDevice,
                    m_iSourceWidth, m_iSourceHeight,
                    1, 0, 
                    D3DFMT_A1R5G5B5, D3DPOOL_MANAGED, &m_pd3dTexture[0] ) ) )
        throw RageException( hr, "Could not create the D3DX texture!" );

    if( FAILED( hr = D3DXCreateTexture(m_pd3dDevice,
                    m_iSourceWidth, m_iSourceHeight,
                    1, 0, 
                    D3DFMT_A1R5G5B5, D3DPOOL_MANAGED, &m_pd3dTexture[1] ) ) )
        throw RageException( hr, "Could not create the D3DX texture!" );

    // D3DXCreateTexture can silently change the parameters on us
    D3DSURFACE_DESC ddsd;
    if ( FAILED( hr = m_pd3dTexture[0]->GetLevelDesc( 0, &ddsd ) ) )
        throw RageException( hr, "Could not get level Description of D3DX texture!" );

	m_iTextureWidth = ddsd.Width;
	m_iTextureHeight = ddsd.Height;
	m_TextureFormat = ddsd.Format;
    if( m_TextureFormat != D3DFMT_A8R8G8B8 &&
		m_TextureFormat != D3DFMT_A1R5G5B5 )
        throw RageException( "Texture is format we can't handle! Format = 0x%x!", m_TextureFormat );


	return S_OK;
}

HRESULT RageMovieTexture::PlayMovie()
{
	HRESULT hr;

    // Start the graph running;
    if( FAILED( hr = m_pMC->Run() ) )
        throw RageException( hr, "Could not run the DirectShow graph." );

    return S_OK;
}


//-----------------------------------------------------------------------------
// CheckMovieStatus: If the movie has ended, rewind to beginning
//-----------------------------------------------------------------------------
void RageMovieTexture::CheckMovieStatus()
{
    long lEventCode;
    long lParam1;
    long lParam2;

    // Check for completion events
    m_pME->GetEvent( &lEventCode, &lParam1, &lParam2, 0 );
    if( EC_COMPLETE == lEventCode  &&  m_bLoop )
        m_pMP->put_CurrentPosition(0);
}


//-----------------------------------------------------------------------------
// CleanupDShow
//-----------------------------------------------------------------------------
void RageMovieTexture::CleanupDShow()
{
    // Shut down the graph
    if (m_pMC) m_pMC->Stop();
    if (m_pGB) m_pGB.Release ();
}
    
void RageMovieTexture::Play()
{
	PlayMovie();
}

void RageMovieTexture::Pause()
{
	HRESULT hr;
	if( FAILED( hr = m_pMC->Pause() ) )
        throw RageException( hr, "Could not pause the DirectShow graph." );

}

void RageMovieTexture::Stop()
{
	HRESULT hr;
	if( FAILED( hr = m_pMC->Stop() ) )
        throw RageException( hr, "Could not stop the DirectShow graph." );
}

void RageMovieTexture::TurnLoopOn()
{
	m_bLoop = true;
}

void RageMovieTexture::TurnLoopOff()
{
	m_bLoop = false;
}

void RageMovieTexture::SetPosition( float fSeconds )
{
     m_pMP->put_CurrentPosition(0);
}

