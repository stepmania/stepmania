/* RageDisplay_Null - No-op diagnostic renderer. */

#ifndef RAGE_DISPLAY_NULL_H
#define RAGE_DISPLAY_NULL_H

class RageDisplay_Null: public RageDisplay
{
public:
	RageDisplay_Null();
	virtual RString Init( const VideoModeParams &p, bool bAllowUnacceleratedRenderer );

	virtual RString GetApiDescription() const { return "Null"; }
	virtual void GetDisplayResolutions( DisplayResolutions &out ) const;
	virtual float GetMonitorAspectRatio() const { return 4/3.f; }
	const PixelFormatDesc *GetPixelFormatDesc(PixelFormat pf) const;

	bool BeginFrame() { return true; }
	void EndFrame();
	VideoModeParams GetActualVideoModeParams() const { return m_Params; }
	void SetBlendMode( BlendMode mode ) { }
	bool SupportsTextureFormat( PixelFormat pixfmt, bool realtime=false ) { return true; }
	bool SupportsPerVertexMatrixScale() { return false; }
	unsigned CreateTexture( 
		PixelFormat pixfmt, 
		RageSurface* img,
		bool bGenerateMipMaps ) { return 1; }
	void UpdateTexture( 
		unsigned iTexHandle, 
		RageSurface* img,
		int xoffset, int yoffset, int width, int height 
		) { }
	void DeleteTexture( unsigned iTexHandle ) { }
	void ClearAllTextures() { }
	int GetNumTextureUnits() { return 1; }
	void SetTexture( TextureUnit tu, unsigned iTexture ) { }
	void SetTextureMode( TextureUnit tu, TextureMode tm ) { }
	void SetTextureWrapping( TextureUnit tu, bool b ) { }
	int GetMaxTextureSize() const { return 2048; }
	void SetTextureFiltering( TextureUnit tu, bool b ) { }
	bool IsZWriteEnabled() const { return false; }
	bool IsZTestEnabled() const { return false; }
	void SetZWrite( bool b ) { }
	void SetZBias( float f ) { }
	void SetZTestMode( ZTestMode mode ) { }
	void ClearZBuffer() { }
	void SetCullMode( CullMode mode ) { }
	void SetAlphaTest( bool b ) { }
	void SetMaterial( 
		const RageColor &emissive,
		const RageColor &ambient,
		const RageColor &diffuse,
		const RageColor &specular,
		float shininess
		) { }
	void SetLighting( bool b ) { }
	void SetLightOff( int index ) { }
	void SetLightDirectional( 
		int index, 
		const RageColor &ambient, 
		const RageColor &diffuse, 
		const RageColor &specular, 
		const RageVector3 &dir ) { }

	void SetSphereEnvironmentMapping( TextureUnit tu, bool b ) { }
	
	RageCompiledGeometry* CreateCompiledGeometry();
	void DeleteCompiledGeometry( RageCompiledGeometry* p );

protected:
	void DrawQuadsInternal( const RageSpriteVertex v[], int iNumVerts ) { }
	void DrawQuadStripInternal( const RageSpriteVertex v[], int iNumVerts ) { }
	void DrawFanInternal( const RageSpriteVertex v[], int iNumVerts ) { }
	void DrawStripInternal( const RageSpriteVertex v[], int iNumVerts ) { }
	void DrawTrianglesInternal( const RageSpriteVertex v[], int iNumVerts ) { }
	void DrawCompiledGeometryInternal( const RageCompiledGeometry *p, int iMeshIndex ) { }
	void DrawLineStripInternal( const RageSpriteVertex v[], int iNumVerts, float LineWidth ) { }
	void DrawSymmetricQuadStripInternal( const RageSpriteVertex v[], int iNumVerts ) { }

	VideoModeParams m_Params;
	RString TryVideoMode( const VideoModeParams &p, bool &bNewDeviceOut ) { m_Params = p; return RString(); }
	RageSurface* CreateScreenshot();
	RageMatrix GetOrthoMatrix( float l, float r, float b, float t, float zn, float zf ); 
	bool SupportsSurfaceFormat( PixelFormat pixfmt ) { return true; }
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

