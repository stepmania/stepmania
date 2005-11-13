/* RageDisplay_OGL: OpenGL renderer. */

#ifndef RAGEDISPLAY_OGL_H
#define RAGEDISPLAY_OGL_H

class RageDisplay_OGL: public RageDisplay
{
public:
	RageDisplay_OGL();
	virtual ~RageDisplay_OGL();
	CString Init( VideoModeParams p, bool bAllowUnacceleratedRenderer );

	virtual CString GetApiDescription() { return "OpenGL"; }
	void ResolutionChanged();
	const PixelFormatDesc *GetPixelFormatDesc(PixelFormat pf) const;

	bool BeginFrame();	
	void EndFrame();
	VideoModeParams GetActualVideoModeParams() const;
	void SetBlendMode( BlendMode mode );
	bool SupportsTextureFormat( PixelFormat pixfmt, bool realtime=false );
	bool SupportsPerVertexMatrixScale();
	unsigned CreateTexture( 
		PixelFormat pixfmt, 
		RageSurface* img,
		bool bGenerateMipMaps );
	void UpdateTexture( 
		unsigned uTexHandle, 
		RageSurface* img,
		int xoffset, int yoffset, int width, int height 
		);
	void DeleteTexture( unsigned uTexHandle );
	void ClearAllTextures();
	int GetNumTextureUnits();
	void SetTexture( TextureUnit tu, RageTexture* pTexture );
	void SetTextureModeModulate();
	void SetTextureModeGlow();
	void SetTextureModeAdd();
	void SetTextureWrapping( bool b );
	int GetMaxTextureSize() const;
	void SetTextureFiltering( bool b);
	bool IsZWriteEnabled() const;
	bool IsZTestEnabled() const;
	void SetZWrite( bool b );
	void SetZBias( float f );
	void SetZTestMode( ZTestMode mode );
	void ClearZBuffer();
	void SetCullMode( CullMode mode );
	void SetAlphaTest( bool b );
	void SetMaterial( 
		const RageColor &emissive,
		const RageColor &ambient,
		const RageColor &diffuse,
		const RageColor &specular,
		float shininess
		);
	void SetLighting( bool b );
	void SetLightOff( int index );
	void SetLightDirectional( 
		int index, 
		const RageColor &ambient, 
		const RageColor &diffuse, 
		const RageColor &specular, 
		const RageVector3 &dir );

	void SetSphereEnvironmentMapping( bool b );

	RageCompiledGeometry* CreateCompiledGeometry();
	void DeleteCompiledGeometry( RageCompiledGeometry* p );

	// hacks for cell-shaded models
	virtual void SetPolygonMode( PolygonMode pm );
	virtual void SetLineWidth( float fWidth );

	CString GetTextureDiagnostics( unsigned id ) const;

protected:
	void DrawQuadsInternal( const RageSpriteVertex v[], int iNumVerts );
	void DrawQuadStripInternal( const RageSpriteVertex v[], int iNumVerts );
	void DrawFanInternal( const RageSpriteVertex v[], int iNumVerts );
	void DrawStripInternal( const RageSpriteVertex v[], int iNumVerts );
	void DrawTrianglesInternal( const RageSpriteVertex v[], int iNumVerts );
	void DrawCompiledGeometryInternal( const RageCompiledGeometry *p, int iMeshIndex );
	void DrawLineStripInternal( const RageSpriteVertex v[], int iNumVerts, float LineWidth );

	CString TryVideoMode( VideoModeParams params, bool &bNewDeviceOut );
	RageSurface* CreateScreenshot();
	void SetViewport(int shift_left, int shift_down);
	PixelFormat GetImgPixelFormat( RageSurface* &img, bool &FreeImg, int width, int height, bool bPalettedTexture );
	bool SupportsSurfaceFormat( PixelFormat pixfmt );
	
	void SendCurrentMatrices();
};

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

