#include "global.h"

/* XXX register thread */
#pragma comment(lib, "winmm.lib") 
 
// Link with the DirectShow base class libraries
#if defined(DEBUG)
	#pragma comment(lib, "baseclasses/debug/strmbasd.lib") 
#else
	#pragma comment(lib, "baseclasses/release/strmbase.lib") 
#endif

#include "MovieTexture_DShowHelper.h"
#include "MovieTexture_DShow.h"

/* for TEXTUREMAN->GetTextureColorDepth() */
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"
#include "RageSurface.h"
#include "arch/Dialog/Dialog.h"

#include <vfw.h> /* for GetVideoCodecDebugInfo */
#pragma comment(lib, "vfw32.lib")

static CString FourCCToString(int fcc)
{
	char c[4];
	c[0] = char((fcc >> 0) & 0xFF);
	c[1] = char((fcc >> 8) & 0xFF);
	c[2] = char((fcc >> 16) & 0xFF);
	c[3] = char((fcc >> 24) & 0xFF);

	CString s;
	for( int i = 0; i < 4; ++i )
		s += clamp( c[i], '\x20', '\x7e' );

	return s;
}

static void CheckCodecVersion( CString codec, CString desc )
{
	if( !codec.CompareNoCase("DIVX") )
	{
		/* "DivX 5.0.5 Codec" */
		Regex GetDivXVersion;

		int major, minor, rev;
		if( sscanf( desc, "DivX %i.%i.%i", &major, &minor, &rev ) != 3 &&
			sscanf( desc, "DivX Pro %i.%i.%i", &major, &minor, &rev ) != 3 )
		{
			LOG->Warn( "Couldn't parse DivX version \"%s\"", desc.c_str() );
			return;
		}

		/* 5.0.0 through 5.0.4 are old and cause crashes. Warn. */
		if( major == 5 && minor == 0 && rev < 5 )
		{
			Dialog::OK(
				ssprintf("The version of DivX installed, %i.%i.%i, is out of date and may\n"
				"cause instability.  Please upgrade to DivX 5.0.5 or newer, available at:\n"
				"\n"
				"http://www.divx.com/", major, minor, rev),
				desc );
			return;
		}
	}
}


static void GetVideoCodecDebugInfo()
{
	ICINFO info = { sizeof(ICINFO) };
	int i;
	LOG->Info("Video codecs:");
	CHECKPOINT;
	for(i=0; ICInfo(ICTYPE_VIDEO, i, &info); ++i)
	{
		CHECKPOINT;
		if( FourCCToString(info.fccHandler) == "ASV1" )
		{
			/* Broken. */
			LOG->Info("%i: %s: skipped", i, FourCCToString(info.fccHandler).c_str());
			continue;
		}

		LOG->Trace( "Scanning codec %s", FourCCToString(info.fccHandler).c_str() );
		CHECKPOINT;
		HIC hic;
		hic = ICOpen(info.fccType, info.fccHandler, ICMODE_DECOMPRESS);
		if(!hic)
		{
			LOG->Info("Couldn't open video codec %s",
				FourCCToString(info.fccHandler).c_str());
			continue;
		}

		CHECKPOINT;
		if (ICGetInfo(hic, &info, sizeof(ICINFO)))
		{
			CheckCodecVersion( FourCCToString(info.fccHandler), WStringToCString(info.szDescription) );
			CHECKPOINT;

			LOG->Info("    %s: %ls (%ls)",
				FourCCToString(info.fccHandler).c_str(), info.szName, info.szDescription);
		}
		else
			LOG->Info("ICGetInfo(%s) failed",
				FourCCToString(info.fccHandler).c_str());

		CHECKPOINT;
		ICClose(hic);
	}

	if(!i)
		LOG->Info("    None found");
}


//-----------------------------------------------------------------------------
// MovieTexture_DShow constructor
//-----------------------------------------------------------------------------
MovieTexture_DShow::MovieTexture_DShow( RageTextureID ID ) :
	RageMovieTexture( ID ),
	buffer_lock( "buffer_lock", 1 ),
	buffer_finished( "buffer_finished", 0 )
{
	LOG->Trace( "MovieTexture_DShow::MovieTexture_DShow()" );

	static bool first = true;
	if( first )
	{
		first = false;
		GetVideoCodecDebugInfo();
	}

	m_bLoop = true;
	m_bPlaying = false;

	m_uTexHandle = 0;
	buffer = NULL;

	Create();
	CreateFrameRects();

	// flip all frame rects because movies are upside down
	for( unsigned i=0; i<m_TextureCoordRects.size(); i++ )
		swap(m_TextureCoordRects[i].top, m_TextureCoordRects[i].bottom);
}

/* Hold buffer_lock.  If it's held, then the decoding thread is waiting
 * for us to process a frame; do so. */
void MovieTexture_DShow::SkipUpdates()
{
	while( buffer_lock.TryWait() )
		CheckFrame();
}

void MovieTexture_DShow::StopSkippingUpdates()
{
	buffer_lock.Post();
}

MovieTexture_DShow::~MovieTexture_DShow()
{
	LOG->Trace("MovieTexture_DShow::~MovieTexture_DShow");
	LOG->Flush();

	SkipUpdates();

	/* Shut down the graph.  We can't call Stop() here, since that will
	 * call SkipUpdates again, which will deadlock if we call it twice
	 * in a row. */
    if (m_pGB) {
		LOG->Trace("MovieTexture_DShow: shutdown");
	LOG->Flush();
		CComPtr<IMediaControl> pMC;
		m_pGB.QueryInterface(&pMC);

		HRESULT hr;
		if( FAILED( hr = pMC->Stop() ) )
			RageException::Throw( hr_ssprintf(hr, "Could not stop the DirectShow graph.") );

//		Stop();
		m_pGB.Release();
	}
	LOG->Trace("MovieTexture_DShow: shutdown ok");
	LOG->Flush();
	if(m_uTexHandle)
		DISPLAY->DeleteTexture( m_uTexHandle );
}

void MovieTexture_DShow::Reload()
{
	// do nothing
}

/* If there's a frame waiting in the buffer, then the decoding thread put it there
 * and is waiting for us to do something with it. */
void MovieTexture_DShow::CheckFrame()
{
	if(buffer == NULL)
		return;

	CHECKPOINT;

	/* Just in case we were invalidated: */
	CreateTexture();

	// DirectShow feeds us in BGR8
	RageSurface *fromDShow = CreateSurfaceFrom(
		m_iSourceWidth, m_iSourceHeight,
		24, 
		0xFF0000,
		0x00FF00,
		0x0000FF,
		0x000000,
		(uint8_t *) buffer, m_iSourceWidth*3 );

	/* Optimization notes:
	 *
	 * With D3D, this surface can be anything; it'll convert it on the fly.  If
	 * it happens to exactly match the texture, it'll copy a little faster.
	 *
	 * With OpenGL, it's best that this be a real, supported texture format, though
	 * it doesn't need to be that of the actual texture.  If it isn't, it'll have
	 * to do a very slow conversion.  Both RGB8 and BGR8 are both (usually) valid
	 * formats.
	 */
	CHECKPOINT;
	DISPLAY->UpdateTexture(
		m_uTexHandle, 
		fromDShow,
		0, 0,
		m_iImageWidth, m_iImageHeight );
	CHECKPOINT;

	delete fromDShow;

	buffer = NULL;

	CHECKPOINT;

	/* Start the decoding thread again. */
	buffer_finished.Post();

	CHECKPOINT;
}

void MovieTexture_DShow::Update(float fDeltaTime)
{
	CHECKPOINT;

	// restart the movie if we reach the end
	if(m_bLoop)
	{
		// Check for completion events
		CComPtr<IMediaEvent>    pME;
		m_pGB.QueryInterface(&pME);

		long lEventCode, lParam1, lParam2;
		pME->GetEvent( &lEventCode, &lParam1, &lParam2, 0 );
		if( lEventCode == EC_COMPLETE )
			SetPosition(0);
	}

	CHECKPOINT;

	CheckFrame();
}

void PrintCodecError(HRESULT hr, CString s)
{
	/* Actually, we might need XviD; we might want to look
	 * at the file and try to figure out if it's something
	 * common: DIV3, DIV4, DIV5, XVID, or maybe even MPEG2. */
	CString err = hr_ssprintf(hr, "%s", s.c_str());
	LOG->Warn( 
		ssprintf(
		"There was an error initializing a movie: %s.\n"
		"Could not locate the DivX video codec.\n"
		"DivX is required to movie textures and must\n"
		"be installed before running the application.\n\n"
		"Please visit http://www.divx.com to download the latest version.",
		err.c_str())
		);
}

CString MovieTexture_DShow::GetActiveFilterList()
{
	CString ret;
	
	IEnumFilters *pEnum = NULL;
	HRESULT hr = m_pGB->EnumFilters(&pEnum);
	if (FAILED(hr))
		return hr_ssprintf(hr, "EnumFilters");

	IBaseFilter *pF = NULL;
	while (S_OK == pEnum->Next(1, &pF, 0))
	{
		FILTER_INFO FilterInfo;
		pF->QueryFilterInfo(&FilterInfo);

		if(ret != "")
			ret += ", ";
		ret += WStringToCString(FilterInfo.achName);

		if( FilterInfo.pGraph )
			FilterInfo.pGraph->Release();
		pF->Release();
	}
	pEnum->Release();
	return ret;
}

void MovieTexture_DShow::Create()
{
	RageTextureID actualID = GetID();

    HRESULT hr;
    
	actualID.iAlphaBits = 0;

    if( FAILED( hr=CoInitialize(NULL) ) )
        RageException::Throw( hr_ssprintf(hr, "Could not CoInitialize") );

    // Create the filter graph
    if( FAILED( hr=m_pGB.CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC) ) )
        RageException::Throw( hr_ssprintf(hr, "Could not create CLSID_FilterGraph!") );
    
    // Create the Texture Renderer object
    CTextureRenderer *pCTR = new CTextureRenderer;
    
    /* Get a pointer to the IBaseFilter on the TextureRenderer, and add it to the
	 * graph.  When m_pGB is released, it will free pFTR. */
    CComPtr<IBaseFilter> pFTR = pCTR;
    if( FAILED( hr = m_pGB->AddFilter(pFTR, L"TEXTURERENDERER" ) ) )
        RageException::Throw( hr_ssprintf(hr, "Could not add renderer filter to graph!") );

    // Add the source filter
    CComPtr<IBaseFilter>    pFSrc;          // Source Filter
    wstring wFileName = CStringToWstring(actualID.filename);

	// if this fails, it's probably because the user doesn't have DivX installed
	/* No, it also happens if the movie can't be opened for some reason; for example,
	 * if another program has it open and locked.  Missing codecs probably won't
	 * show up until Connect(). */
    if( FAILED( hr = m_pGB->AddSourceFilter( wFileName.c_str(), wFileName.c_str(), &pFSrc ) ) )
	{
		PrintCodecError(hr, "Could not create source filter to graph!");
		return;	// survive and don't Run the graph.
	}
    
    // Find the source's output and the renderer's input
    CComPtr<IPin>           pFTRPinIn;      // Texture Renderer Input Pin
    if( FAILED( hr = pFTR->FindPin( L"In", &pFTRPinIn ) ) )
        RageException::Throw( hr_ssprintf(hr, "Could not find input pin!") );

    CComPtr<IPin>           pFSrcPinOut;    // Source Filter Output Pin   
    if( FAILED( hr = pFSrc->FindPin( L"Output", &pFSrcPinOut ) ) )
        RageException::Throw( hr_ssprintf(hr, "Could not find output pin!") );

    // Connect these two filters
    if( FAILED( hr = m_pGB->Connect( pFSrcPinOut, pFTRPinIn ) ) )
	{
 		PrintCodecError(hr, "Could not connect pins!");
		return;	// survive and don't Run the graph.
	}

	LOG->Trace( "Filters: %s", GetActiveFilterList().c_str() );

	// Pass us to our TextureRenderer.
	pCTR->SetRenderTarget(this);

	/* Cap the max texture size to the hardware max. */
	actualID.iMaxSize = min( actualID.iMaxSize, DISPLAY->GetMaxTextureSize() );

    // The graph is built, now get the set the output video width and height.
	// The source and image width will always be the same since we can't scale the video
	m_iSourceWidth  = pCTR->GetVidWidth();
	m_iSourceHeight = pCTR->GetVidHeight();

	/* image size cannot exceed max size */
	m_iImageWidth = min( m_iSourceWidth, actualID.iMaxSize );
	m_iImageHeight = min( m_iSourceHeight, actualID.iMaxSize );

	/* Texture dimensions need to be a power of two; jump to the next. */
	m_iTextureWidth = power_of_two(m_iImageWidth);
	m_iTextureHeight = power_of_two(m_iImageHeight);

	/* We've set up the movie, so we know the dimensions we need.  Set
	 * up the texture. */
	CreateTexture();

	/* Pausing the graph will cause only one frame to be rendered.  Do that, then
	 * wait for the frame to be rendered, to guarantee that the texture is set
	 * when this function returns. */
	Pause();

	CHECKPOINT;
	pCTR->m_OneFrameDecoded.Wait();
	CHECKPOINT;
	CheckFrame();
	CHECKPOINT;

	// Start the graph running
    Play();
}


void MovieTexture_DShow::NewData(const char *data)
{
	ASSERT(data);
	
	/* Try to lock. */
	if( buffer_lock.TryWait() )
	{
		/* The main thread is doing something uncommon, such as pausing.
		 * Drop this frame. */
		return;
	}

	buffer = data;

	buffer_finished.Wait();

	ASSERT(buffer == NULL);

	buffer_lock.Post();
}


void MovieTexture_DShow::CreateTexture()
{
	if(m_uTexHandle)
		return;

	RageDisplay::PixelFormat pixfmt;
	switch( TEXTUREMAN->GetPrefs().m_iMovieColorDepth )
	{
	default:
		ASSERT(0);
	case 16:
		if( DISPLAY->SupportsTextureFormat(RageDisplay::FMT_RGB5) )
			pixfmt = RageDisplay::FMT_RGB5;
		else
			pixfmt = RageDisplay::FMT_RGBA4;	// everything supports RGBA4
		break;
	case 32:
		if( DISPLAY->SupportsTextureFormat(RageDisplay::FMT_RGB8) )
			pixfmt = RageDisplay::FMT_RGB8;
		else if( DISPLAY->SupportsTextureFormat(RageDisplay::FMT_RGBA8) )
			pixfmt = RageDisplay::FMT_RGBA8;
		else if( DISPLAY->SupportsTextureFormat(RageDisplay::FMT_RGB5) )
			pixfmt = RageDisplay::FMT_RGB5;
		else
			pixfmt = RageDisplay::FMT_RGBA4;	// everything supports RGBA4
		break;
	}


	const RageDisplay::PixelFormatDesc *pfd = DISPLAY->GetPixelFormatDesc(pixfmt);
	RageSurface *img = CreateSurface( m_iTextureWidth, m_iTextureHeight,
		pfd->bpp, pfd->masks[0], pfd->masks[1], pfd->masks[2], pfd->masks[3] );

	m_uTexHandle = DISPLAY->CreateTexture( pixfmt, img, false );

	delete img;
}


void MovieTexture_DShow::Play()
{
	SkipUpdates();

	LOG->Trace("MovieTexture_DShow::Play()");
	CComPtr<IMediaControl> pMC;
    m_pGB.QueryInterface(&pMC);

    // Start the graph running;
	HRESULT hr;
    if( FAILED( hr = pMC->Run() ) )
        RageException::Throw( hr_ssprintf(hr, "Could not run the DirectShow graph.") );

	m_bPlaying = true;

	StopSkippingUpdates();
}

void MovieTexture_DShow::Pause()
{
	SkipUpdates();

	CComPtr<IMediaControl> pMC;
    m_pGB.QueryInterface(&pMC);

	HRESULT hr;
	/* Use Pause(), so we'll get a still frame in CTextureRenderer::OnReceiveFirstSample. */
	if( FAILED( hr = pMC->Pause() ) )
        RageException::Throw( hr_ssprintf(hr, "Could not pause the DirectShow graph.") );

	StopSkippingUpdates();
}


void MovieTexture_DShow::SetPosition( float fSeconds )
{
	SkipUpdates();

	CComPtr<IMediaPosition> pMP;
    m_pGB.QueryInterface(&pMP);
    pMP->put_CurrentPosition(0);

	StopSkippingUpdates();
}

void MovieTexture_DShow::SetPlaybackRate( float fRate )
{
	if( fRate == 0 )
	{
		this->Pause();
		return;
	}

	SkipUpdates();

	CComPtr<IMediaPosition> pMP;
	m_pGB.QueryInterface(&pMP);
	HRESULT hr = pMP->put_Rate(fRate);	// fails on many codecs

	StopSkippingUpdates();

	if( FAILED(hr) )
	{
		this->Pause();
		return;
	}
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
