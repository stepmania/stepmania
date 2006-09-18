#include "global.h"
#include "MovieTexture_DShowHelper.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "archutils/Win32/DirectXHelpers.h"

//-----------------------------------------------------------------------------
// Define GUID for Texture Renderer
// {71771540-2017-11cf-AE26-0020AFD79767}
//-----------------------------------------------------------------------------
struct __declspec(uuid("{71771540-2017-11cf-ae26-0020afd79767}")) CLSID_TextureRenderer;

static HRESULT CBV_ret;
CTextureRenderer::CTextureRenderer():
	CBaseVideoRenderer(__uuidof(CLSID_TextureRenderer), 
	NAME("Texture Renderer"), NULL, &CBV_ret),
	m_OneFrameDecoded( "m_OneFrameDecoded", 0 )
{
	if( FAILED(CBV_ret) )
		RageException::Throw( hr_ssprintf(CBV_ret, "Could not create texture renderer object!") );

	m_pTexture = NULL;
}

CTextureRenderer::~CTextureRenderer()
{
}


HRESULT CTextureRenderer::CheckMediaType(const CMediaType *pmt)
{
	VIDEOINFO *pvi;

	// Reject the connection if this is not a video type
	if( *pmt->FormatType() != FORMAT_VideoInfo )
		return E_INVALIDARG;

	/* Force the graph to R8G8B8.  DirectShow won't generate a FMT_RGB5 that OpenGL
	 * can handle.  It's faster to generate FMT8 and let OpenGL convert on the fly
	 * than to generate FMT_RGB5 and convert it ourself. */
	pvi = (VIDEOINFO *)pmt->Format();
	if( IsEqualGUID( *pmt->Type(), MEDIATYPE_Video)  &&
		IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_RGB24) )
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


void CTextureRenderer::SetRenderTarget( MovieTexture_DShow* pTexture )
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

void CTextureRenderer::OnReceiveFirstSample( IMediaSample * pSample )
{
	/* If the main thread is in MovieTexture_DShow::Create, kick: */
	if( m_OneFrameDecoded.GetValue() == 0 )
		m_OneFrameDecoded.Post();

	DoRenderSample( pSample );
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
