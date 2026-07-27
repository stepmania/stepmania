#include "global.h"
#include "MovieTexture_Generic.h"
#include "PrefsManager.h"
#include "RageDisplay.h"
#include "RageLog.h"
#include "RageSurface.h"
#include "RageTextureManager.h"
#include "RageTextureRenderTarget.h"
#include "RageTimer.h"
#include "RageUtil.h"
#include "Sprite.h"

#include <cmath>
#include <cstdint>

#if defined(WIN32)
#include "archutils/Win32/ErrorStrings.h"
#include <windows.h>
#endif


static Preference<bool> g_bMovieTextureDirectUpdates( "MovieTextureDirectUpdates", true );

MovieTexture_Generic::MovieTexture_Generic( RageTextureID ID, MovieDecoder *pDecoder ):
	RageMovieTexture( ID )
{
	LOG->Trace( "MovieTexture_Generic::MovieTexture_Generic(%s)", ID.filename.c_str() );

	m_pDecoder = pDecoder;

	m_uTexHandle = 0;
	m_pRenderTarget = nullptr;
	m_pTextureIntermediate = nullptr;
	m_bLoop = true;
	m_pSurface = nullptr;
	m_pTextureLock = nullptr;
	m_fRate = 1;
	m_fClock = 0;
	m_pSprite = new Sprite;
}

RString MovieTexture_Generic::Init()
{
	RString sError = m_pDecoder->Open(GetID().filename);
	if (sError != "")
		return sError;

	CreateTexture();
	CreateFrameRects();


	// Draw the first frame immediately, to guarantee that the texture is drawn
	// when this function returns--if possible.
	if (m_pDecoder->DecodeNextFrame() < 0) {
		LOG->Trace("Failure to decode first frame of video file \"%s\"", GetID().filename.c_str());
		m_failure = true;
		return RString("Failure to display movie.");
	};
	UpdateMovie(0);

	decoding_thread = std::make_unique<std::thread>([this]() {
		LOG->Trace("Beginning to decode video file \"%s\"", GetID().filename.c_str());
		auto timer = RageTimer();

		int ret = m_pDecoder->DecodeMovie();
		if (ret == -1) {
			m_failure = true;
		}

		LOG->Trace("Done decoding video file \"%s\", took %f seconds", GetID().filename.c_str(), timer.Ago());
		});

	LOG->Trace("Resolution: %ix%i (%ix%i, %ix%i)",
		m_iSourceWidth, m_iSourceHeight,
		m_iImageWidth, m_iImageHeight, m_iTextureWidth, m_iTextureHeight);

	CHECKPOINT_M("Generic initialization completed. No errors found.");

	return RString();
}

MovieTexture_Generic::~MovieTexture_Generic()
{
	if (m_pDecoder) {
		m_pDecoder->Cancel();
		decoding_thread->join();
		m_pDecoder->Close();
	}

	/* m_pSprite may reference the texture; delete it before DestroyTexture. */
	delete m_pSprite;

	DestroyTexture();

	delete m_pDecoder;
}

/* Delete the surface and texture.  The decoding thread must be stopped, and this
 * is normally done after destroying the decoder. */
void MovieTexture_Generic::DestroyTexture()
{
	delete m_pSurface;
	m_pSurface = nullptr;

	delete m_pTextureLock;
	m_pTextureLock = nullptr;

	if( m_uTexHandle )
	{
		DISPLAY->DeleteTexture( m_uTexHandle );
		m_uTexHandle = 0;
	}

	delete m_pRenderTarget;
	m_pRenderTarget = nullptr;
	delete m_pTextureIntermediate;
	m_pTextureIntermediate = nullptr;
}

class RageMovieTexture_Generic_Intermediate : public RageTexture
{
public:
	RageMovieTexture_Generic_Intermediate( RageTextureID ID, int iWidth, int iHeight,
		int iImageWidth, int iImageHeight, int iTextureWidth, int iTextureHeight,
		RageSurfaceFormat SurfaceFormat, RagePixelFormat pixfmt ):
		RageTexture(ID),
		m_SurfaceFormat( SurfaceFormat )
	{
		m_PixFmt = pixfmt;
		m_iSourceWidth = iWidth;
		m_iSourceHeight = iHeight;

		m_iImageWidth = iImageWidth;
		m_iImageHeight = iImageHeight;
		m_iTextureWidth = iTextureWidth;
		m_iTextureHeight = iTextureHeight;

		CreateFrameRects();

		m_uTexHandle = 0;
		CreateTexture();
	}
	virtual ~RageMovieTexture_Generic_Intermediate()
	{
		if( m_uTexHandle )
		{
			DISPLAY->DeleteTexture( m_uTexHandle );
			m_uTexHandle = 0;
		}
	}

	virtual void Invalidate() { m_uTexHandle = 0; }
	virtual void Reload() { }
	virtual std::uintptr_t GetTexHandle() const
	{
		return m_uTexHandle;
	}

	bool IsAMovie() const { return true; }
private:
	void CreateTexture()
	{
		if( m_uTexHandle )
			return;

		RageSurface *pSurface = CreateSurfaceFrom( m_iImageWidth, m_iImageHeight,
			m_SurfaceFormat.BitsPerPixel,
			m_SurfaceFormat.Mask[0],
			m_SurfaceFormat.Mask[1],
			m_SurfaceFormat.Mask[2],
			m_SurfaceFormat.Mask[3], nullptr, 1 );

		m_uTexHandle = DISPLAY->CreateTexture( m_PixFmt, pSurface, false );
		delete pSurface;
	}

	std::uintptr_t m_uTexHandle;
	RageSurfaceFormat m_SurfaceFormat;
	RagePixelFormat m_PixFmt;
};

void MovieTexture_Generic::Invalidate()
{
	m_uTexHandle = 0;
	if( m_pTextureIntermediate != nullptr )
		m_pTextureIntermediate->Invalidate();
}

void MovieTexture_Generic::CreateTexture()
{
	if( m_uTexHandle || m_pRenderTarget != nullptr )
		return;

	CHECKPOINT;

	m_iSourceWidth  = m_pDecoder->GetWidth();
	m_iSourceHeight = m_pDecoder->GetHeight();

	/* Adjust m_iSourceWidth to support different source aspect ratios. */
	float fSourceAspectRatio = m_pDecoder->GetSourceAspectRatio();
	if( fSourceAspectRatio < 1 )
		m_iSourceHeight = std::lrint( m_iSourceHeight / fSourceAspectRatio );
	else if( fSourceAspectRatio > 1 )
		m_iSourceWidth = std::lrint( m_iSourceWidth * fSourceAspectRatio );

	/* HACK: Don't cap movie textures to the max texture size, since we
	 * render them onto the texture at the source dimensions.  If we find a
	 * fast way to resize movies, we can change this back. */
	m_iImageWidth = m_iSourceWidth;
	m_iImageHeight = m_iSourceHeight;

	/* Texture dimensions need to be a power of two; jump to the next. */
	m_iTextureWidth = power_of_two( m_iImageWidth );
	m_iTextureHeight = power_of_two( m_iImageHeight );
	MovieDecoderPixelFormatYCbCr fmt = PixelFormatYCbCr_Invalid;
	if( m_pSurface == nullptr )
	{
		ASSERT( m_pTextureLock == nullptr );
		if( g_bMovieTextureDirectUpdates )
			m_pTextureLock = DISPLAY->CreateTextureLock();

		m_pSurface = m_pDecoder->CreateCompatibleSurface( m_iImageWidth, m_iImageHeight,
			TEXTUREMAN->GetPrefs().m_iMovieColorDepth == 32, fmt );
		if( m_pTextureLock != nullptr )
		{
			delete [] m_pSurface->pixels;
			m_pSurface->pixels = nullptr;
		}

	}

	RagePixelFormat pixfmt = DISPLAY->FindPixelFormat( m_pSurface->format->BitsPerPixel,
			m_pSurface->format->Mask[0],
			m_pSurface->format->Mask[1],
			m_pSurface->format->Mask[2],
			m_pSurface->format->Mask[3] );

	if( pixfmt == RagePixelFormat_Invalid )
	{
		/* We weren't given a natively-supported pixel format.  Pick a supported
		 * one.  This is a fallback case, and implies a second conversion. */
		int depth = TEXTUREMAN->GetPrefs().m_iMovieColorDepth;
		switch( depth )
		{
		default:
			FAIL_M(ssprintf("Unsupported movie color depth: %i", depth));
		case 16:
			if( DISPLAY->SupportsTextureFormat(RagePixelFormat_RGB5) )
				pixfmt = RagePixelFormat_RGB5;
			else
				pixfmt = RagePixelFormat_RGBA4;

			break;

		case 32:
			if( DISPLAY->SupportsTextureFormat(RagePixelFormat_RGB8) )
				pixfmt = RagePixelFormat_RGB8;
			else if( DISPLAY->SupportsTextureFormat(RagePixelFormat_RGBA8) )
				pixfmt = RagePixelFormat_RGBA8;
			else if( DISPLAY->SupportsTextureFormat(RagePixelFormat_RGB5) )
				pixfmt = RagePixelFormat_RGB5;
			else
				pixfmt = RagePixelFormat_RGBA4;
			break;
		}
	}

	if( fmt != PixelFormatYCbCr_Invalid )
	{
		SAFE_DELETE( m_pTextureIntermediate );
		m_pSprite->UnloadTexture();

		/* Create the render target.  This will receive the final, converted texture. */
		RenderTargetParam param;
		param.iWidth = m_iImageWidth;
		param.iHeight = m_iImageHeight;

		RageTextureID TargetID( GetID() );
		TargetID.filename += " target";
		m_pRenderTarget = new RageTextureRenderTarget( TargetID, param );

		/* Create the intermediate texture.  This receives the YUV image. */
		RageTextureID IntermedID( GetID() );
		IntermedID.filename += " intermediate";

		m_pTextureIntermediate = new RageMovieTexture_Generic_Intermediate( IntermedID,
			m_pDecoder->GetWidth(), m_pDecoder->GetHeight(),
			m_pSurface->w, m_pSurface->h,
			power_of_two(m_pSurface->w), power_of_two(m_pSurface->h),
			*m_pSurface->format, pixfmt );

		/* Configure the sprite.  This blits the intermediate onto the ifnal render target. */
		m_pSprite->SetHorizAlign( align_left );
		m_pSprite->SetVertAlign( align_top );

		/* Hack: Sprite wants to take ownership of the texture, and will decrement the refcount
		 * when it unloads the texture.  Normally we'd make a "copy", but we can't access
		 * RageTextureManager from here.  Just increment the refcount. */
		++m_pTextureIntermediate->m_iRefCount;
		m_pSprite->SetTexture( m_pTextureIntermediate );
		m_pSprite->SetEffectMode( GetEffectMode(fmt) );

		return;
	}

	m_uTexHandle = DISPLAY->CreateTexture( pixfmt, m_pSurface, false );
}

/*
 * Returns:
 *  <= 0 if it's time for the next frame to display
 *   > 0 (seconds) if it's not yet time to display
 */
float MovieTexture_Generic::CheckFrameTime()
{
	if (m_fRate == 0) {
		return 1;	// "a long time until the next frame"
	}
	return (m_pDecoder->GetTimestamp() - m_fClock) / m_fRate;
}

void MovieTexture_Generic::UpdateMovie(float fSeconds)
{
	// Quick exit in case we failed to decode the movie.
	if (m_failure) {
		return;
	}
	m_fClock += fSeconds * m_fRate;

	// If the frame isn't ready, don't update. This does mean the video
	// will "speed up" to catch up when decoding does outpace display.
	//
	// In practice, display should rarely, if ever, outpace decoding.
	if (m_pDecoder->IsCurrentFrameReady() && CheckFrameTime() <= 0) {
		UpdateFrame();
		return;
	}
}

void MovieTexture_Generic::UpdateFrame()
{
	/* Just in case we were invalidated: */
	CreateTexture();

	if (m_pDecoder->SkipNextFrame()) {
		return;
	}

	if(m_pTextureLock != nullptr)
	{
		std::uintptr_t iHandle = m_pTextureIntermediate != nullptr ? m_pTextureIntermediate->GetTexHandle(): this->GetTexHandle();
		m_pTextureLock->Lock(iHandle, m_pSurface);
	}

	/* Are we looping? */
	if (m_pDecoder->GetFrame(m_pSurface) && m_bLoop) {
		LOG->Trace("File \"%s\" looping", GetID().filename.c_str());
		m_pDecoder->Rewind();
		// There's a gap in the audio when the music preview loops. This value
		// is dynamic based on the ending and starting beats (see
		// GameSoundManager.cpp::StartMusic).
		//
		// This means that the video will be off-sync during the loop, since
		// the movie texture doesn't have access to the SoundManager's offset.
		// Until it does, we can either freeze at the end of the video banner,
		// or give it a best effort approximation (0.5 seconds).
		m_fClock = 0.5;
	};

	if (m_pTextureLock != nullptr) {
		m_pTextureLock->Unlock(m_pSurface, true);
	}

	if (m_pRenderTarget != nullptr)
	{
		CHECKPOINT_M("About to upload the texture.");

		/* If we have no m_pTextureLock, we still have to upload the texture. */
		if (m_pTextureLock == nullptr) {
			DISPLAY->UpdateTexture(
				m_pTextureIntermediate->GetTexHandle(),
				m_pSurface,
				0, 0,
				m_pSurface->w, m_pSurface->h);
		}
		m_pRenderTarget->BeginRenderingTo(false);
		m_pSprite->Draw();
		m_pRenderTarget->FinishRenderingTo();
	}
	else {
		if (m_pTextureLock == nullptr) {
			DISPLAY->UpdateTexture(
				m_uTexHandle,
				m_pSurface,
				0, 0,
				m_iImageWidth, m_iImageHeight);
		}
	}
}

static EffectMode EffectModes[] =
{
	EffectMode_YUYV422,
};
static_assert( ARRAYLEN(EffectModes) == NUM_PixelFormatYCbCr );

EffectMode MovieTexture_Generic::GetEffectMode( MovieDecoderPixelFormatYCbCr fmt )
{
	ASSERT( fmt != PixelFormatYCbCr_Invalid );
	return EffectModes[fmt];
}

void MovieTexture_Generic::Reload()
{
}

void MovieTexture_Generic::SetPosition(float fSeconds)
{
	// In theory, we can math out fSeconds and frame counts to seek the video,
	// but there's likely no practical use case of this.
	if (fSeconds != 0)
	{
		LOG->Warn("MovieTexture_Generic::SetPosition(%f): non-0 seeking unsupported; ignored", fSeconds);
		return;
	}

	LOG->Trace("Seek to %f", fSeconds);
	m_fClock = 0;
	m_pDecoder->Rewind();
}

std::uintptr_t MovieTexture_Generic::GetTexHandle() const
{
	if( m_pRenderTarget != nullptr )
		return m_pRenderTarget->GetTexHandle();

	return m_uTexHandle;
}

/*
 * (c) 2003-2005 Glenn Maynard
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
