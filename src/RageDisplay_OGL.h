/* RageDisplay_Legacy: OpenGL renderer. */

#ifndef RAGE_DISPLAY_OGL_H
#define RAGE_DISPLAY_OGL_H

#include "RageDisplay.h"
#include "RageVector3.hpp"
#include "RageTextureRenderTarget.h"
#include "Sprite.h"

/* Making an OpenGL call doesn't also flush the error state; if we happen
 * to have an error from a previous call, then the assert below will fail.
 * Flush it. */
#define FlushGLErrors() do { } while( glGetError() != GL_NO_ERROR )
#define AssertNoGLError() \
{ \
	GLenum error = glGetError(); \
	ASSERT_M( error == GL_NO_ERROR, RageDisplay_Legacy_Helpers::GLToString(error) ); \
}

#if defined(DEBUG) || !defined(GL_GET_ERROR_IS_SLOW)
#define DebugFlushGLErrors() FlushGLErrors()
#define DebugAssertNoGLError() AssertNoGLError()
#else
#define DebugFlushGLErrors()
#define DebugAssertNoGLError()
#endif

class RageDisplay_Legacy: public RageDisplay
{
public:
	RageDisplay_Legacy();
	virtual ~RageDisplay_Legacy();
	virtual std::string Init( const VideoModeParams &p, bool bAllowUnacceleratedRenderer );

	virtual std::string GetApiDescription() const { return "OpenGL"; }
	virtual void GetDisplaySpecs(DisplaySpecs &out) const;
	void ResolutionChanged();
	const RagePixelFormatDesc *GetPixelFormatDesc(RagePixelFormat pf) const;

	bool BeginFrame();
	void EndFrame();
	ActualVideoModeParams GetActualVideoModeParams() const;
	void SetBlendMode( BlendMode mode );
	bool SupportsTextureFormat( RagePixelFormat pixfmt, bool realtime=false );
	bool SupportsPerVertexMatrixScale();
	unsigned CreateTexture(
		RagePixelFormat pixfmt,
		RageSurface* img,
		bool bGenerateMipMaps );
	void UpdateTexture(
		unsigned iTexHandle,
		RageSurface* img,
		int xoffset, int yoffset, int width, int height
		);
	void DeleteTexture( unsigned iTexHandle );
	bool UseOffscreenRenderTarget();
	RageSurface *GetTexture( unsigned iTexture );
	RageTextureLock *CreateTextureLock();

	void ClearAllTextures();
	int GetNumTextureUnits();
	void SetTexture( TextureUnit tu, unsigned iTexture );
	void SetTextureMode( TextureUnit tu, TextureMode tm );
	void SetTextureWrapping( TextureUnit tu, bool b );
	int GetMaxTextureSize() const;
	void SetTextureFiltering( TextureUnit tu, bool b );
	void SetEffectMode( EffectMode effect );
	void set_color_key_shader(Rage::Color const& color, unsigned int tex_handle);
	bool IsEffectModeSupported( EffectMode effect );
	bool SupportsRenderToTexture() const;
	bool SupportsFullscreenBorderlessWindow() const;
	unsigned CreateRenderTarget( const RenderTargetParam &param, int &iTextureWidthOut, int &iTextureHeightOut );
	unsigned GetRenderTarget();
	void SetRenderTarget( unsigned iHandle, bool bPreserveTexture );
	bool IsZWriteEnabled() const;
	bool IsZTestEnabled() const;
	void SetZWrite( bool b );
	void SetZBias( float f );
	void SetZTestMode( ZTestMode mode );
	void ClearZBuffer();
	void SetCullMode( CullMode mode );
	void SetAlphaTest( bool b );
	void SetMaterial(
		const Rage::Color &emissive,
		const Rage::Color &ambient,
		const Rage::Color &diffuse,
		const Rage::Color &specular,
		float shininess
		);
	void SetLighting( bool b );
	void SetLightOff( int index );
	void SetLightDirectional(
		int index,
		const Rage::Color &ambient,
		const Rage::Color &diffuse,
		const Rage::Color &specular,
		const Rage::Vector3 &dir );

	void SetSphereEnvironmentMapping( TextureUnit tu, bool b );
	void SetCelShaded( int stage );

	RageCompiledGeometry* CreateCompiledGeometry();
	void DeleteCompiledGeometry( RageCompiledGeometry* p );

	// hacks for cell-shaded models
	virtual void SetPolygonMode( PolygonMode pm );
	virtual void SetLineWidth( float fWidth );

	std::string GetTextureDiagnostics( unsigned id ) const;

protected:
	void DrawQuadsInternal( const Rage::SpriteVertex v[], int iNumVerts );
	void DrawQuadStripInternal( const Rage::SpriteVertex v[], int iNumVerts );
	void DrawFanInternal( const Rage::SpriteVertex v[], int iNumVerts );
	void DrawStripInternal( const Rage::SpriteVertex v[], int iNumVerts );
	void DrawTrianglesInternal( const Rage::SpriteVertex v[], int iNumVerts );
	void DrawCompiledGeometryInternal( const RageCompiledGeometry *p, int iMeshIndex );
	void DrawLineStripInternal( const Rage::SpriteVertex v[], int iNumVerts, float LineWidth );
	void DrawSymmetricQuadStripInternal( const Rage::SpriteVertex v[], int iNumVerts );

	std::string TryVideoMode( const VideoModeParams &p, bool &bNewDeviceOut );
	RageSurface* CreateScreenshot();
	RagePixelFormat GetImgPixelFormat( RageSurface* &img, bool &FreeImg, int width, int height, bool bPalettedTexture );
	bool SupportsSurfaceFormat( RagePixelFormat pixfmt );

	void SendCurrentMatrices();

private:
	RageTextureRenderTarget *offscreenRenderTarget = nullptr;
};

#endif

/*
 * Copyright (c) 2001-2011 Chris Danford, Glenn Maynard, Colby Klein
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

