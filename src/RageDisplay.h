/* RageDisplay - Renderer base class. */

#ifndef RAGEDISPLAY_H
#define RAGEDISPLAY_H

#include "RageTypes.h"
#include "ModelTypes.h"

#include <cstddef>
#include <cstdint>
#include <set>
#include <vector>


class DisplaySpec;
typedef std::set<DisplaySpec> DisplaySpecs;

const int REFRESH_DEFAULT = 0;
struct RageSurface;
enum TextureUnit
{
	TextureUnit_1,
	TextureUnit_2,
	TextureUnit_3,
	TextureUnit_4,
	NUM_TextureUnit
};

// RageCompiledGeometry holds vertex data in a format that is most efficient
// for the graphics API.
class RageCompiledGeometry
{
public:
	virtual ~RageCompiledGeometry();

	void Set( const std::vector<msMesh> &vMeshes, bool bNeedsNormals );

	virtual void Allocate( const std::vector<msMesh> &vMeshes ) = 0;	// allocate space
	virtual void Change( const std::vector<msMesh> &vMeshes ) = 0;	// new data must be the same size as was passed to Set()
	virtual void Draw( int iMeshIndex ) const = 0;

protected:
	std::size_t GetTotalVertices() const { if( m_vMeshInfo.empty() ) return 0; return m_vMeshInfo.back().iVertexStart + m_vMeshInfo.back().iVertexCount; }
	std::size_t GetTotalTriangles() const { if( m_vMeshInfo.empty() ) return 0; return m_vMeshInfo.back().iTriangleStart + m_vMeshInfo.back().iTriangleCount; }

	struct MeshInfo
	{
		int iVertexStart;
		int iVertexCount;
		int iTriangleStart;
		int iTriangleCount;
		bool m_bNeedsTextureMatrixScale;
	};
	std::vector<MeshInfo>	m_vMeshInfo;
	bool m_bNeedsNormals;
	bool m_bAnyNeedsTextureMatrixScale;
};

enum RagePixelFormat
{
	RagePixelFormat_RGBA8,
	RagePixelFormat_BGRA8,
	RagePixelFormat_RGBA4,
	RagePixelFormat_RGB5A1,
	RagePixelFormat_RGB5,
	RagePixelFormat_RGB8,
	RagePixelFormat_PAL,
	/* The above formats differ between OpenGL and D3D. These are provided as
	* alternatives for OpenGL that match some format in D3D.  Don't use them
	* directly; they'll be matched automatically by FindPixelFormat. */
	RagePixelFormat_BGR8,
	RagePixelFormat_A1BGR5,
	RagePixelFormat_X1RGB5,
	NUM_RagePixelFormat,
	RagePixelFormat_Invalid
};
const RString& RagePixelFormatToString( RagePixelFormat i );

/** @brief The parameters used for the present Video Mode. */
class VideoModeParams
{
public:
	// Initialize with a constructor so to guarantee all paramters
	// are filled (in case new params are added).
	VideoModeParams(
		bool windowed_,
		RString sDisplayId_,
		int width_,
		int height_,
		int bpp_,
		int rate_,
		bool vsync_,
		bool interlaced_,
		bool bSmoothLines_,
		bool bTrilinearFiltering_,
		bool bAnisotropicFiltering_,
		bool bWindowIsFullscreenBorderless_,
		RString sWindowTitle_,
		RString sIconFile_,
		bool PAL_,
		float fDisplayAspectRatio_
	):
		windowed(windowed_),
		sDisplayId(sDisplayId_),
		width(width_),
		height(height_),
		bpp(bpp_),
		rate(rate_),
		vsync(vsync_),
		interlaced(interlaced_),
		bSmoothLines(bSmoothLines_),
		bTrilinearFiltering(bTrilinearFiltering_),
		bAnisotropicFiltering(bAnisotropicFiltering_),
		bWindowIsFullscreenBorderless(bWindowIsFullscreenBorderless_),
		sWindowTitle(sWindowTitle_),
		sIconFile(sIconFile_),
		PAL(PAL_),
		fDisplayAspectRatio(fDisplayAspectRatio_) {}

	VideoModeParams(const VideoModeParams &other) = default;

	VideoModeParams(): windowed(false), width(0), height(0),
					bpp(0), rate(0), vsync(false), interlaced(false),
					bSmoothLines(false), bTrilinearFiltering(false),
					bAnisotropicFiltering(false), bWindowIsFullscreenBorderless(false),
					sWindowTitle(RString()), sIconFile(RString()),
					PAL(false), fDisplayAspectRatio(0.0f) {}

	// Subclassing VideoModeParams in ActualVideoModeParams. Make destructor virtual just in case
	// someone tries to delete one of those through a pointer to base...
	virtual ~VideoModeParams() = default;

	bool windowed;
	RString sDisplayId;
	int width;
	int height;
	int bpp;
	int rate;
	bool vsync;
	bool interlaced;
	bool bSmoothLines;
	bool bTrilinearFiltering;
	bool bAnisotropicFiltering;
	bool bWindowIsFullscreenBorderless;
	RString sWindowTitle;
	RString sIconFile;
	bool PAL;
	float fDisplayAspectRatio;
};

/**
 * @brief The _actual_ VideoModeParams determined by the LowLevelWindow implementation.
 * Contains all the attributes of VideoModeParams, plus the actual window width/height determined by
 * LLW
 */
class ActualVideoModeParams: public VideoModeParams
{
public:
	ActualVideoModeParams(): VideoModeParams(), windowWidth(0), windowHeight(0), renderOffscreen(false) {}
	ActualVideoModeParams( const VideoModeParams &params ) : VideoModeParams( params ),
															 windowWidth( params.width ),
															 windowHeight( params.height ),
															 renderOffscreen( false )
	{ }
	ActualVideoModeParams( const VideoModeParams &params, int windowWidth, int windowHeight, bool renderOffscreen ) :
		VideoModeParams( params ), windowWidth( windowWidth ), windowHeight( windowHeight ),
		renderOffscreen( renderOffscreen )
	{ }
	ActualVideoModeParams (const ActualVideoModeParams &other) = default;
	ActualVideoModeParams& operator=(const ActualVideoModeParams& other) = default;


	// If bWindowIsFullscreenBorderless is true,
	// then these properties will differ from width/height (which describe the
	// render size)
	int windowWidth;
	int windowHeight;
	bool renderOffscreen;
};

struct RenderTargetParam
{
	RenderTargetParam():
		iWidth(0),
		iHeight(0),
		bWithDepthBuffer(false),
		bWithAlpha(false),
		bFloat(false)
	{
	}

	// The dimensions of the actual render target, analogous to a window size:
	int iWidth, iHeight;

	bool bWithDepthBuffer;
	bool bWithAlpha;
	bool bFloat;
};

struct RageTextureLock
{
	virtual ~RageTextureLock() { }

	/* Given a surface with a format and no pixel data, lock the texture into the
	 * surface. The data is write-only. */
	virtual void Lock( std::uintptr_t iTexHandle, RageSurface *pSurface ) = 0;

	/* Unlock and update the texture. If bChanged is false, the texture update
	 * may be omitted. */
	virtual void Unlock( RageSurface *pSurface, bool bChanged = true ) = 0;
};

class RageDisplay
{
	friend class RageTexture;

public:

	struct RagePixelFormatDesc {
		int bpp;
		unsigned int masks[4];
	};

	virtual const RagePixelFormatDesc *GetPixelFormatDesc( RagePixelFormat pf ) const = 0;

	RageDisplay();
	virtual ~RageDisplay();

	virtual RString Init( const VideoModeParams &p, bool bAllowUnacceleratedRenderer ) = 0;

	virtual RString GetApiDescription() const = 0;
	virtual void GetDisplaySpecs(DisplaySpecs &out) const = 0;

	// Don't override this.  Override TryVideoMode() instead.
	// This will set the video mode to be as close as possible to params.
	// Return true if device was re-created and we need to reload textures.
	RString SetVideoMode( VideoModeParams p, bool &bNeedReloadTextures );

	// Call this when the resolution has been changed externally:
	virtual void ResolutionChanged();

	virtual bool BeginFrame();
	virtual void EndFrame();
	virtual ActualVideoModeParams GetActualVideoModeParams() const = 0;
	bool IsWindowed() const { return this->GetActualVideoModeParams().windowed; }

	virtual void SetBlendMode( BlendMode mode ) = 0;

	virtual bool SupportsTextureFormat( RagePixelFormat pixfmt, bool realtime=false ) = 0;
	virtual bool SupportsThreadedRendering() { return false; }
	virtual bool SupportsPerVertexMatrixScale() = 0;

	// If threaded rendering is supported, these will be called from the
	// rendering thread before and after rendering.
	virtual void BeginConcurrentRenderingMainThread() { }
	virtual void EndConcurrentRenderingMainThread() { }
	virtual void BeginConcurrentRendering();
	virtual void EndConcurrentRendering() { }

	/* return 0 if failed or internal texture resource handle
	 * (unsigned in OpenGL, texture pointer in D3D) */
	virtual std::uintptr_t CreateTexture(
		RagePixelFormat pixfmt,		// format of img and of texture in video mem
		RageSurface* img,		// must be in pixfmt
		bool bGenerateMipMaps
		) = 0;
	virtual void UpdateTexture(
		std::uintptr_t iTexHandle,
		RageSurface* img,
		int xoffset, int yoffset, int width, int height
		) = 0;
	virtual void DeleteTexture( std::uintptr_t iTexHandle ) = 0;
	/* Return an object to lock pixels for streaming. If not supported, returns nullptr.
	 * Delete the object normally. */
	virtual RageTextureLock *CreateTextureLock() { return nullptr; }
	virtual void ClearAllTextures() = 0;
	virtual int GetNumTextureUnits() = 0;
	virtual void SetTexture( TextureUnit, std::uintptr_t /* iTexture */ ) = 0;
	virtual void SetTextureMode( TextureUnit, TextureMode ) = 0;
	virtual void SetTextureWrapping( TextureUnit, bool ) = 0;
	virtual int GetMaxTextureSize() const = 0;
	virtual void SetTextureFiltering( TextureUnit, bool ) = 0;
	virtual void SetEffectMode( EffectMode ) { }
	virtual bool IsEffectModeSupported( EffectMode effect ) { return effect == EffectMode_Normal; }

	virtual bool SupportsRenderToTexture() const { return false; }
	virtual bool SupportsFullscreenBorderlessWindow() const { return false; }

	/* Create a render target, returning a texture handle. In addition to normal
	 * texture functions, this can be passed to SetRenderTarget. Delete with
	 * DeleteTexture. (UpdateTexture is not permitted.) Returns 0 if render-to-
	 * texture is unsupported.
	 */
	virtual std::uintptr_t CreateRenderTarget( const RenderTargetParam &, int & /* iTextureWidthOut */, int & /* iTextureHeightOut */ ) { return 0; }

	virtual std::uintptr_t GetRenderTarget()	{ return 0; }

	/* Set the render target, or 0 to resume rendering to the framebuffer. An active render
	 * target may not be used as a texture. If bPreserveTexture is true, the contents
	 * of the texture will be preserved from the previous call; otherwise, cleared.  If
	 * bPreserveTexture is true the first time a render target is used, behave as if
	 * bPreserveTexture was false.
	 */
	virtual void SetRenderTarget( std::uintptr_t /* iHandle */, bool /* bPreserveTexture */ = true ) { }

	virtual bool IsZTestEnabled() const = 0;
	virtual bool IsZWriteEnabled() const = 0;
	virtual void SetZWrite( bool ) = 0;
	virtual void SetZTestMode( ZTestMode ) = 0;
	virtual void SetZBias( float ) = 0;
	virtual void ClearZBuffer() = 0;

	virtual void SetCullMode( CullMode mode ) = 0;

	virtual void SetAlphaTest( bool b ) = 0;

	virtual void SetMaterial(
		const RageColor &emissive,
		const RageColor &ambient,
		const RageColor &diffuse,
		const RageColor &specular,
		float shininess
		) = 0;

	virtual void SetLighting( bool b ) = 0;
	virtual void SetLightOff( int index ) = 0;
	virtual void SetLightDirectional(
		int index,
		const RageColor &ambient,
		const RageColor &diffuse,
		const RageColor &specular,
		const RageVector3 &dir ) = 0;

	virtual void SetSphereEnvironmentMapping( TextureUnit tu, bool b ) = 0;
	virtual void SetCelShaded( int stage ) = 0;

	virtual RageCompiledGeometry* CreateCompiledGeometry() = 0;
	virtual void DeleteCompiledGeometry( RageCompiledGeometry* p ) = 0;

	void DrawQuads( const RageSpriteVertex v[], int iNumVerts );
	void DrawQuadStrip( const RageSpriteVertex v[], int iNumVerts );
	void DrawFan( const RageSpriteVertex v[], int iNumVerts );
	void DrawStrip( const RageSpriteVertex v[], int iNumVerts );
	void DrawTriangles( const RageSpriteVertex v[], int iNumVerts );
	void DrawCompiledGeometry( const RageCompiledGeometry *p, int iMeshIndex, const std::vector<msMesh> &vMeshes );
	void DrawLineStrip( const RageSpriteVertex v[], int iNumVerts, float LineWidth );
	void DrawSymmetricQuadStrip( const RageSpriteVertex v[], int iNumVerts );
	void DrawCircle( const RageSpriteVertex &v, float radius );

	void DrawQuad( const RageSpriteVertex v[] ) { DrawQuads(v,4); } /* alias. upper-left, upper-right, lower-left, lower-right */

	// hacks for cell-shaded models
	virtual void SetPolygonMode( PolygonMode ) {}
	virtual void SetLineWidth( float ) {}

	enum GraphicsFileFormat
	{
		SAVE_LOSSLESS,			// bmp
		SAVE_LOSSLESS_SENSIBLE,	// png
		SAVE_LOSSY_LOW_QUAL,	// jpg
		SAVE_LOSSY_HIGH_QUAL	// jpg
	};
	bool SaveScreenshot( RString sPath, GraphicsFileFormat format );

	virtual RString GetTextureDiagnostics( std::uintptr_t /* id */ ) const { return RString(); }
	virtual RageSurface* CreateScreenshot() = 0;	// allocates a surface.  Caller must delete it.
	virtual RageSurface *GetTexture( std::uintptr_t /* iTexture */ ) { return nullptr; } // allocates a surface.  Caller must delete it.

protected:
	virtual void DrawQuadsInternal( const RageSpriteVertex v[], int iNumVerts ) = 0;
	virtual void DrawQuadStripInternal( const RageSpriteVertex v[], int iNumVerts ) = 0;
	virtual void DrawFanInternal( const RageSpriteVertex v[], int iNumVerts ) = 0;
	virtual void DrawStripInternal( const RageSpriteVertex v[], int iNumVerts ) = 0;
	virtual void DrawTrianglesInternal( const RageSpriteVertex v[], int iNumVerts ) = 0;
	virtual void DrawCompiledGeometryInternal( const RageCompiledGeometry *p, int iMeshIndex ) = 0;
	virtual void DrawLineStripInternal( const RageSpriteVertex v[], int iNumVerts, float LineWidth );
	virtual void DrawSymmetricQuadStripInternal( const RageSpriteVertex v[], int iNumVerts ) = 0;
	virtual void DrawCircleInternal( const RageSpriteVertex &v, float radius );

	// return RString() if mode change was successful, an error message otherwise.
	// bNewDeviceOut is set true if a new device was created and textures
	// need to be reloaded.
	virtual RString TryVideoMode( const VideoModeParams &p, bool &bNewDeviceOut ) = 0;

	void DrawPolyLine( const RageSpriteVertex &p1, const RageSpriteVertex &p2, float LineWidth );

	// Stuff in RageDisplay.cpp
	void SetDefaultRenderStates();

public:
	// Statistics
	int GetFPS() const;
	int GetVPF() const;
	int GetCumFPS() const; // average FPS since last reset
	virtual void ResetStats();
	virtual void ProcessStatsOnFlip();
	virtual RString GetStats() const;
	void StatsAddVerts( int iNumVertsRendered );

	// World matrix stack functions.
	void PushMatrix();
	void PopMatrix();
	void Translate( float x, float y, float z );
	void TranslateWorld( float x, float y, float z );
	void Scale( float x, float y, float z );
	void RotateX( float deg );
	void RotateY( float deg );
	void RotateZ( float deg );
	void SkewX( float fAmount );
	void SkewY( float fAmount );
	void MultMatrix( const RageMatrix &f ) { this->PostMultMatrix(f); } /* alias */
	void PostMultMatrix( const RageMatrix &f );
	void PreMultMatrix( const RageMatrix &f );
	void LoadIdentity();

	// Texture matrix functions
	void TexturePushMatrix();
	void TexturePopMatrix();
	void TextureTranslate( float x, float y );
	void TextureTranslate( const RageVector2 &v ) { this->TextureTranslate( v.x, v.y ); }

	// Projection and View matrix stack functions.
	void CameraPushMatrix();
	void CameraPopMatrix();
	void LoadMenuPerspective( float fFOVDegrees, float fWidth, float fHeight, float fVanishPointX, float fVanishPointY );
	void LoadLookAt( float fov, const RageVector3 &Eye, const RageVector3 &At, const RageVector3 &Up );

	// Centering matrix
	void CenteringPushMatrix();
	void CenteringPopMatrix();
	void ChangeCentering( int trans_x, int trans_y, int add_width, int add_height );

	RageSurface *CreateSurfaceFromPixfmt( RagePixelFormat pixfmt, void *pixels, int width, int height, int pitch );
	RagePixelFormat FindPixelFormat( int bpp, unsigned Rmask, unsigned Gmask, unsigned Bmask, unsigned Amask, bool realtime=false );

	// Lua
	void PushSelf( lua_State *L );

protected:
	RageMatrix GetPerspectiveMatrix( float fovy, float aspect, float zNear, float zFar );

	// Different for D3D and OpenGL. Not sure why they're not compatible. -Chris
	virtual RageMatrix GetOrthoMatrix( float l, float r, float b, float t, float zn, float zf );
	virtual RageMatrix GetFrustumMatrix( float l, float r, float b, float t, float zn, float zf );

	// Matrix that adjusts position and scale of image on the screen
	RageMatrix GetCenteringMatrix( float fTranslateX, float fTranslateY, float fAddWidth, float fAddHeight ) const;
	void UpdateCentering();

	// Called by the RageDisplay derivitives
	const RageMatrix* GetCentering() const;
	const RageMatrix* GetProjectionTop() const;
	const RageMatrix* GetViewTop() const;
	const RageMatrix* GetWorldTop() const;
	const RageMatrix* GetTextureTop() const;

	// To limit the framerate, call FrameLimitBeforeVsync before waiting
	// for vsync and FrameLimitAfterVsync after.
	void FrameLimitBeforeVsync( int iFPS );
	void FrameLimitAfterVsync();
};


extern RageDisplay*		DISPLAY;	// global and accessible from anywhere in our program

#endif
/*
 * Copyright (c) 2001-2004 Chris Danford, Glenn Maynard
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
