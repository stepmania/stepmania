#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: RageMovieTexture

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/


#pragma comment(lib, "winmm.lib") 
 
// Link with the DirectShow base class libraries
#if defined(DEBUG) | defined(_DEBUG)
	#pragma comment(lib, "baseclasses/debug/strmbasd.lib") 
#else
	#pragma comment(lib, "baseclasses/release/strmbase.lib") 
#endif

#include "RageMovieTextureHelper.h"
 
#include "RageMovieTexture.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"

#include <stdio.h>

#define NO_SDL_GLEXT
#define __glext_h_ /* try harder to stop glext.h from being forced on us by someone else */
#include "SDL_opengl.h"
#include "glext.h"

//-----------------------------------------------------------------------------
// RageMovieTexture constructor
//-----------------------------------------------------------------------------
RageMovieTexture::RageMovieTexture( RageTextureID ID ) :
	RageTexture( ID )
{
	LOG->Trace( "RageBitmapTexture::RageBitmapTexture()" );

	buffer_changed = false;

	m_uGLTextureID = 0;
	buffer = NULL;
 
	Create();

	CreateFrameRects();
	// flip all frame rects because movies are upside down
	for( unsigned i=0; i<m_TextureCoordRects.size(); i++ )
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

	delete [] buffer;

	if(m_uGLTextureID)
		glDeleteTextures(1, &m_uGLTextureID);
}

void RageMovieTexture::Reload()
{
	// do nothing
}


unsigned int RageMovieTexture::GetGLTextureID()
{
	CheckMovieStatus();		// restart the movie if we reach the end
	return m_uGLTextureID;
}

void RageMovieTexture::Update(float fDeltaTime)
{
	LockMutex L(buffer_mutex);

	if(!buffer_changed)
		return;

	buffer_changed = false;

	DISPLAY->SetTexture(this);
	glPixelStorei(GL_UNPACK_SWAP_BYTES, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	/* XXX: We should use m_lVidPitch; we might be padded.  However, I can't
	 * find any codec that don't force the width to a multiple of at least
	 * 4 anyway, so I can't test it, so I'll leave it like this for now. */
	glPixelStorei(GL_UNPACK_ROW_LENGTH, m_iSourceWidth);

	glTexSubImage2D(GL_TEXTURE_2D, 0,
		0, 0,
		min(m_iSourceWidth, m_iTextureWidth),
		min(m_iSourceHeight, m_iTextureHeight),
		GL_BGR, GL_UNSIGNED_BYTE, buffer);

	/* Must unset PixelStore when we're done! */
	glPixelStorei(GL_UNPACK_SWAP_BYTES, 0);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glFlush();
}

void HandleDivXError(HRESULT hr, CString s)
{
	/* Actually, we might need XviD; we might want to look
	 * at the file and try to figure out if it's something
	 * common: DIV3, DIV4, DIV5, XVID, or maybe even MPEG2. */
	CString err = hr_ssprintf(hr, "%s", s.GetString());
	RageException::Throw( 
		ssprintf(
		"There was an error initializing a movie: %s.\n"
		"Could not locate the DivX video codec.\n"
		"DivX is required to movie textures and must\n"
		"be installed before running the application.\n\n"
		"Please visit http://www.divx.com to download the latest version.",
		err.GetString())
		);
}

void RageMovieTexture::Create()
{
    HRESULT hr;
    
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
    WCHAR wFileName[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, m_ActualID.filename.GetString(), -1, wFileName, MAX_PATH);

	// if this fails, it's probably because the user doesn't have DivX installed
    if( FAILED( hr = m_pGB->AddSourceFilter( wFileName, L"SOURCE", &pFSrc ) ) )
	{
		HandleDivXError(hr, "Could not create source filter to graph!");
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
 		HandleDivXError(hr, "Could not connect pins!");
	}

	// Pass us to our TextureRenderer.
	pCTR->SetRenderTarget(this);

    // The graph is built, now get the set the output video width and height.
	// The source and image width will always be the same since we can't scale the video
	m_iSourceWidth  = pCTR->GetVidWidth();
	m_iSourceHeight = pCTR->GetVidHeight();

	/* We've set up the movie, so we know the dimensions we need.  Set
	 * up the texture. */
	CreateTexture();

	// Start the graph running
    if( !PlayMovie() )
        RageException::Throw( "Could not run the DirectShow graph." );
}

void RageMovieTexture::NewData(char *data)
{
	LockMutex L(buffer_mutex);
	buffer_changed = true;
	memcpy(buffer, data, m_iSourceWidth * m_iSourceHeight * 3);
}

bool RageMovieTexture::CreateTexture()
{
	/* image size cannot exceed max size */
	m_iImageWidth = min( m_iSourceWidth, m_ActualID.iMaxSize );
	m_iImageHeight = min( m_iSourceHeight, m_ActualID.iMaxSize );

	/* Texture dimensions need to be a power of two; jump to the next. */
	m_iTextureWidth = power_of_two(m_iImageWidth);
	m_iTextureHeight = power_of_two(m_iImageHeight);

	buffer = new char[m_iSourceWidth * m_iSourceHeight * 3];
	if(!m_uGLTextureID)
		glGenTextures(1, &m_uGLTextureID);

	DISPLAY->SetTexture(this);

	/* Initialize the texture and set it to black. */
	string buf(m_iTextureWidth*m_iTextureHeight*3, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8,
		m_iTextureWidth, m_iTextureHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, buf.data());

	return true;
}

bool RageMovieTexture::PlayMovie()
{
	LOG->Trace("RageMovieTexture::PlayMovie()");
	CComPtr<IMediaControl> pMC;
    m_pGB.QueryInterface(&pMC);

    // Start the graph running;
	HRESULT hr;
    if( FAILED( hr = pMC->Run() ) )
        RageException::Throw( hr_ssprintf(hr, "Could not run the DirectShow graph.") );

	m_bPlaying = true;

    return true;
}


//-----------------------------------------------------------------------------
// CheckMovieStatus: If the movie has ended, rewind to beginning
//-----------------------------------------------------------------------------
void RageMovieTexture::CheckMovieStatus()
{
	if(!m_bLoop) return;

    // Check for completion events
	CComPtr<IMediaEvent>    pME;
    m_pGB.QueryInterface(&pME);

	long lEventCode, lParam1, lParam2;
    pME->GetEvent( &lEventCode, &lParam1, &lParam2, 0 );
    if( EC_COMPLETE == lEventCode )
		SetPosition(0);
}

void RageMovieTexture::Play()
{
	PlayMovie();
}

void RageMovieTexture::Pause()
{
	CComPtr<IMediaControl> pMC;
    m_pGB.QueryInterface(&pMC);

	HRESULT hr;
	if( FAILED( hr = pMC->Pause() ) )
        RageException::Throw( hr_ssprintf(hr, "Could not pause the DirectShow graph.") );
}

void RageMovieTexture::Stop()
{
	CComPtr<IMediaControl> pMC;
    m_pGB.QueryInterface(&pMC);

	HRESULT hr;
	if( FAILED( hr = pMC->Stop() ) )
        RageException::Throw( hr_ssprintf(hr, "Could not stop the DirectShow graph.") );

	m_bPlaying = false;
}

void RageMovieTexture::SetPosition( float fSeconds )
{
	CComPtr<IMediaPosition> pMP;
    m_pGB.QueryInterface(&pMP);
    pMP->put_CurrentPosition(0);
}

bool RageMovieTexture::IsPlaying() const
{
	return m_bPlaying;
}



