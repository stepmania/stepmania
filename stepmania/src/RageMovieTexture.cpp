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
// Define GUID for Texture Renderer
// {71771540-2017-11cf-AE26-0020AFD79767}
//-----------------------------------------------------------------------------
struct __declspec(uuid("{71771540-2017-11cf-ae26-0020afd79767}")) CLSID_TextureRenderer;

 
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
    CTextureRenderer();
    ~CTextureRenderer();

	// overridden methods
    HRESULT CheckMediaType(const CMediaType *pmt );     // Format acceptable?
    HRESULT SetMediaType(const CMediaType *pmt );       // Video format notification
    HRESULT DoRenderSample(IMediaSample *pMediaSample); // New video sample

	// new methods
	long GetVidWidth() const { return m_lVidWidth; }
	long GetVidHeight() const { return m_lVidHeight; }
	HRESULT SetRenderTarget( RageMovieTexture* pTexture );

protected:
	// Video width, height, and pitch.
	long m_lVidWidth, m_lVidHeight, m_lVidPitch;

	RageMovieTexture*	m_pTexture;	// the video surface we will copy new frames to
	D3DFORMAT			m_TextureFormat; // Texture format
};




//-----------------------------------------------------------------------------
// CTextureRenderer constructor
//-----------------------------------------------------------------------------
static HRESULT CBV_ret;
CTextureRenderer::CTextureRenderer()
                                   : CBaseVideoRenderer(__uuidof(CLSID_TextureRenderer), 
                                   NAME("Texture Renderer"), NULL, &CBV_ret)
{
    if( FAILED(CBV_ret) )
        throw RageException( CBV_ret, "Could not create texture renderer object!" );

    // Store and ARageef the texture for our use.
	m_pTexture = NULL;
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

	m_pTexture = pTexture;

	if( m_pTexture == NULL )
		return S_OK;
	
	LPDIRECT3DTEXTURE8 pD3DTexture = pTexture->GetD3DTexture();

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
	if( m_pTexture == NULL )
	{
		LOG->Warn( "DoRenderSample called while m_pTexture was NULL!" );
		return S_OK;
	}

    BYTE  *pBmpBuffer;		// Bitmap buffer

    // Get the video bitmap buffer
    pSample->GetPointer( &pBmpBuffer );

	/* m_pTexture is getting NULLed out somewhere.  I'd like to put a bunch
	 * of traces in here, but this is called 20 times per frame and I don't
	 * want to send some 200 traces/sec during gameplay, so let's just try
	 * to narrow it down.  */
	 
	// Find which texture we should render to.  We want to copy to the "back buffer"
	LPDIRECT3DTEXTURE8 pD3DTextureCopyTo = m_pTexture->GetBackBuffer();

    // Lock the Texture
    D3DLOCKED_RECT d3dlr;
    while( FAILED( pD3DTextureCopyTo->LockRect(0, &d3dlr, 0, 0) ) )
	{
        LOG->Warn( "Failed to lock the texture for rendering movie!" );
		// keep trying until we get the lock
	}
    
    // Get the texture buffer & pitch
	byte  *pTxtBuffer = static_cast<byte *>(d3dlr.pBits);
    long  lTxtPitch = d3dlr.Pitch;
   
	ASSERT( pTxtBuffer != NULL );

	/* I had the crash occur down at the flip, meaning m_pTexture is valid when
	 * we got the backbuffer.  Let's get some data here, in the middle. */
	LOG->Trace("movie: %p, %p, pit %i, %ix%i, %i, %i", m_pTexture, pTxtBuffer, lTxtPitch,
		m_lVidHeight, m_lVidWidth, m_TextureFormat, m_lVidPitch);
    // Copy the bits    
    // OPTIMIZATION OPPORTUNITY: Use a video and texture
    // format that allows a simpler copy than this one.
    switch( m_TextureFormat )
	{
	case D3DFMT_A8R8G8B8:
		{
			for(int y = 0; y < m_pTexture->GetImageHeight(); y++ ) {
				BYTE *pBmpBufferOld = pBmpBuffer;
				BYTE *pTxtBufferOld = pTxtBuffer;   
				for (int x = 0; x < m_pTexture->GetImageWidth(); x++) {
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
			for(int y = 0; y < m_pTexture->GetImageHeight(); y++ ) {
				BYTE *pBmpBufferOld = pBmpBuffer;
				BYTE *pTxtBufferOld = pTxtBuffer;   
				for (int x = 0; x < m_pTexture->GetImageWidth(); x++) {
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

	ASSERT( m_pTexture != NULL );
        
    // Unlock the Texture
    if( FAILED( pD3DTextureCopyTo->UnlockRect(0) ) ) 
	{
        LOG->Warn( "Failed to unlock the texture!" );
        return E_FAIL;
	}

	ASSERT( m_pTexture != NULL );

	// flip active texture
	/* Hmm.  GetD3DTexture says "Return the front buffer because it's guaranteed
	 * that CTextureRenderer doesn't have it locked."  But what happens if we somehow
	 * get two movie frames while whoever called GetD3DTexture is still working with
	 * it?  Maybe we should triple buffer, just to be sure. */
	m_pTexture->Flip();

    return S_OK;
}


//-----------------------------------------------------------------------------
// RageMovieTexture constructor
//-----------------------------------------------------------------------------
RageMovieTexture::RageMovieTexture( 
	RageDisplay* pScreen, 
	const CString &sFilePath, 
	int dwMaxSize, 
	int dwTextureColorDepth,
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
	m_bPlaying = false;
}

RageMovieTexture::~RageMovieTexture()
{
	LOG->Trace("RageMovieTexture::~RageMovieTexture");

	// Shut down the graph
    if (m_pGB) {
		Stop();
		m_pGB.Release ();
	}

	SAFE_RELEASE(m_pd3dTexture[0]);
	SAFE_RELEASE(m_pd3dTexture[1]);
}

void RageMovieTexture::Reload( 
	int dwMaxSize, 
	int dwTextureColorDepth,
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
void RageMovieTexture::Create()
{
	HRESULT hr;

	// Initialize the filter graph find and get information about the
	// video (dimensions, color depth, etc.)
	if( FAILED( hr = InitDShowTextureRenderer() ) )
        throw RageException( hr, "Could not initialize the DirectShow Texture Renderer!" );

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
}

//-----------------------------------------------------------------------------
// InitDShowTextureRenderer : Create DirectShow filter graph and run the graph
//-----------------------------------------------------------------------------
HRESULT RageMovieTexture::InitDShowTextureRenderer()
{
    HRESULT hr = S_OK;
    
    // Create the filter graph
    if( FAILED( m_pGB.CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC) ) )
        throw RageException( hr, "Could not create CLSID_FilterGraph!" );
    
    // Create the Texture Renderer object
    CTextureRenderer *pCTR = new CTextureRenderer;
    
    /* Get a pointer to the IBaseFilter on the TextureRenderer, and add it to the
	 * graph.  When m_pGB is released, it will free pFTR. */
    CComPtr<IBaseFilter> pFTR = pCTR;
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
    CComPtr<IBaseFilter>    pFSrc;          // Source Filter
    if( FAILED( hr = m_pGB->AddSourceFilter( wFileName, L"SOURCE", &pFSrc ) ) )		// if this fails, it's probably because the user doesn't have DivX installed
	{
		HandleDivXError();
        throw RageException( hr, "Could not create source filter to graph!" );
	}
    
    // Find the source's output and the renderer's input
    CComPtr<IPin>           pFTRPinIn;      // Texture Renderer Input Pin
    if( FAILED( hr = pFTR->FindPin( L"In", &pFTRPinIn ) ) )
        throw RageException( hr, "Could not find input pin!" );

    CComPtr<IPin>           pFSrcPinOut;    // Source Filter Output Pin   
    if( FAILED( hr = pFSrc->FindPin( L"Output", &pFSrcPinOut ) ) )
        throw RageException( hr, "Could not find output pin!" );
    
    // Connect these two filters
    if( FAILED( hr = m_pGB->Connect( pFSrcPinOut, pFTRPinIn ) ) )
	{
 		HandleDivXError();
		throw RageException( hr, "Could not connect pins!" );
	}
    
    // The graph is built, now get the set the output video width and height.
	// The source and image width will always be the same since we can't scale the video
	m_iSourceWidth  = pCTR->GetVidWidth();
	m_iSourceHeight = pCTR->GetVidHeight();
	m_iImageWidth   = m_iSourceWidth;
	m_iImageHeight  = m_iSourceHeight;

	/* We've set up the movie, so we know the dimensions we need.  Set
	 * up the texture. */
	if( FAILED( hr = CreateD3DTexture() ) )
        throw RageException( hr, "Could not create the D3D Texture!" );

	// Pass the D3D texture to our TextureRenderer so it knows 
	// where to render new movie frames to.
	if( FAILED( hr = pCTR->SetRenderTarget( this ) ) )
        throw RageException( hr, "RageMovieTexture: SetRenderTarget failed." );

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

	if( m_iTextureWidth < m_iImageWidth || m_iTextureHeight < m_iImageHeight )
	{
		/* Gack.  We got less than we asked for; probably on a Voodoo.
		 * We really need to scale the image down; hopefully DShow has some
		 * way to do that efficiently.  For now, we'll just have to
		 * chop it off. */
		m_iImageWidth = min(m_iImageWidth, m_iTextureWidth);
		m_iImageHeight = min(m_iImageHeight, m_iTextureHeight);
	}

    if( ddsd.Format != D3DFMT_A8R8G8B8 &&
		ddsd.Format != D3DFMT_A1R5G5B5 )
        throw RageException( "Texture is format we can't handle! Format = 0x%x!", ddsd.Format );


	return S_OK;
}

HRESULT RageMovieTexture::PlayMovie()
{
	LOG->Trace("RageMovieTexture::PlayMovie()");
	CComPtr<IMediaControl> pMC;
    m_pGB.QueryInterface(&pMC);

    // Start the graph running;
	HRESULT hr;
    if( FAILED( hr = pMC->Run() ) )
        throw RageException( hr, "Could not run the DirectShow graph." );

	m_bPlaying = true;

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
	CComPtr<IMediaEvent>    pME;
    m_pGB.QueryInterface(&pME);
    pME->GetEvent( &lEventCode, &lParam1, &lParam2, 0 );
    if( EC_COMPLETE == lEventCode  && m_bLoop )
		SetPosition(0);
}

void RageMovieTexture::Play()
{
	PlayMovie();
}

void RageMovieTexture::Pause()
{
	LOG->Trace("RageMovieTexture::Pause()");
	CComPtr<IMediaControl> pMC;
    m_pGB.QueryInterface(&pMC);

	HRESULT hr;
	if( FAILED( hr = pMC->Pause() ) )
        throw RageException( hr, "Could not pause the DirectShow graph." );

}

void RageMovieTexture::Stop()
{
	LOG->Trace("RageMovieTexture::Stop()");
	CComPtr<IMediaControl> pMC;
    m_pGB.QueryInterface(&pMC);

	HRESULT hr;
	if( FAILED( hr = pMC->Stop() ) )
        throw RageException( hr, "Could not stop the DirectShow graph." );

	m_bPlaying = false;
}

void RageMovieTexture::SetPosition( float fSeconds )
{
	LOG->Trace("RageMovieTexture::Stop()");
	CComPtr<IMediaPosition> pMP;
    m_pGB.QueryInterface(&pMP);
    pMP->put_CurrentPosition(0);
}

bool RageMovieTexture::IsPlaying() const
{
	return m_bPlaying;
}

