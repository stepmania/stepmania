#include "global.h"
#include "RageMovieTextureHelper.h"
#include "RageUtil.h"
#include "RageLog.h"

//-----------------------------------------------------------------------------
// Define GUID for Texture Renderer
// {71771540-2017-11cf-AE26-0020AFD79767}
//-----------------------------------------------------------------------------
struct __declspec(uuid("{71771540-2017-11cf-ae26-0020afd79767}")) CLSID_TextureRenderer;

static HRESULT CBV_ret;
CTextureRenderer::CTextureRenderer()
                                   : CBaseVideoRenderer(__uuidof(CLSID_TextureRenderer), 
                                   NAME("Texture Renderer"), NULL, &CBV_ret)
{
    if( FAILED(CBV_ret) )
        RageException::Throw( hr_ssprintf(CBV_ret, "Could not create texture renderer object!") );

    // Store and ARageef the texture for our use.
	m_pTexture = NULL;
}

CTextureRenderer::~CTextureRenderer()
{
    // Do nothing
}


HRESULT CTextureRenderer::CheckMediaType(const CMediaType *pmt)
{
    VIDEOINFO *pvi;
    
    // Reject the connection if this is not a video type
    if( *pmt->FormatType() != FORMAT_VideoInfo )
        return E_INVALIDARG;
    
	// Force the graph to R8G8B8.
    pvi = (VIDEOINFO *)pmt->Format();
    if(IsEqualGUID( *pmt->Type(),    MEDIATYPE_Video)  &&
       IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_RGB24))
        return S_OK;
    
    return E_FAIL;
}


// SetMediaType: Graph connection has been made. 
HRESULT CTextureRenderer::SetMediaType(const CMediaType *pmt)
{
    // Retrive the size of this media type
    VIDEOINFO *pviBmp;                      // Bitmap info header
    pviBmp = (VIDEOINFO *)pmt->Format();
    m_lVidWidth  = pviBmp->bmiHeader.biWidth;
    m_lVidHeight = abs(pviBmp->bmiHeader.biHeight);
    m_lVidPitch = (m_lVidWidth * 3 + 3) + ~3; // We are forcing RGB24

    return S_OK;
}


void CTextureRenderer::SetRenderTarget( RageMovieTexture* pTexture )
{
	m_pTexture = pTexture;
}

// DoRenderSample: A sample has been delivered. Copy it.
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

    // Copy the bits    
	m_pTexture->NewData((char *) pBmpBuffer);

    return S_OK;
}
