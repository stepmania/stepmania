/* RageDisplay_Null - No-op diagnostic renderer. */

#ifndef RAGE_DISPLAY_NULL_H
#define RAGE_DISPLAY_NULL_H

class RageDisplay_Null: public RageDisplay
{
public:
	RageDisplay_Null();
	virtual std::string Init( const VideoModeParams &p, bool bAllowUnacceleratedRenderer );

	virtual std::string GetApiDescription() const { return "Null"; }
	virtual void GetDisplaySpecs(DisplaySpecs &out) const;
	const RagePixelFormatDesc *GetPixelFormatDesc(RagePixelFormat pf) const;

	bool BeginFrame() { return true; }
	void EndFrame();
	ActualVideoModeParams GetActualVideoModeParams() const { return m_Params; }
	void SetBlendMode( BlendMode ) { }
	bool SupportsTextureFormat( RagePixelFormat, bool /* realtime */ =false ) { return true; }
	bool SupportsPerVertexMatrixScale() { return false; }
	unsigned CreateTexture(
		RagePixelFormat,
		RageSurface* /* img */,
		bool /* bGenerateMipMaps */ ) { return 1; }
	void UpdateTexture(
		unsigned /* iTexHandle */,
		RageSurface* /* img */,
		int /* xoffset */, int /* yoffset */, int /* width */, int /* height */
		) { }
	void DeleteTexture( unsigned /* iTexHandle */ ) { }
	void ClearAllTextures() { }
	int GetNumTextureUnits() { return 1; }
	void SetTexture( TextureUnit, unsigned /* iTexture */ ) { }
	void SetTextureMode( TextureUnit, TextureMode ) { }
	void SetTextureWrapping( TextureUnit, bool ) { }
	int GetMaxTextureSize() const { return 2048; }
	void SetTextureFiltering( TextureUnit, bool ) { }
	bool IsZWriteEnabled() const { return false; }
	bool IsZTestEnabled() const { return false; }
	void SetZWrite( bool ) { }
	void SetZBias( float ) { }
	void SetZTestMode( ZTestMode ) { }
	void ClearZBuffer() { }
	void SetCullMode( CullMode ) { }
	void SetAlphaTest( bool ) { }
	void SetMaterial(
		const Rage::Color & /* unreferenced: emissive */,
		const Rage::Color & /* unreferenced: ambient */,
		const Rage::Color & /* unreferenced: diffuse */,
		const Rage::Color & /* unreferenced: specular */,
		float /* unreferenced: shininess */
		) { }
	void SetLighting( bool ) { }
	void SetLightOff( int /* index */ ) { }
	void SetLightDirectional( 
		int /* index */, 
		const Rage::Color & /* unreferenced: ambient */, 
		const Rage::Color & /* unreferenced: diffuse */, 
		const Rage::Color & /* unreferenced: specular */, 
		const Rage::Vector3 & /* unreferenced: dir */ ) { }

	void SetSphereEnvironmentMapping( TextureUnit /* tu */, bool /* b */ ) { }
	void SetCelShaded( int /* stage */ ) { }

	RageCompiledGeometry* CreateCompiledGeometry();
	void DeleteCompiledGeometry( RageCompiledGeometry* );

protected:
	void DrawQuadsInternal( const Rage::SpriteVertex [], int /* iNumVerts */ ) { }
	void DrawQuadStripInternal( const Rage::SpriteVertex [], int /* iNumVerts */ ) { }
	void DrawFanInternal( const Rage::SpriteVertex [], int /* iNumVerts */ ) { }
	void DrawStripInternal( const Rage::SpriteVertex [], int /* iNumVerts */ ) { }
	void DrawTrianglesInternal( const Rage::SpriteVertex [], int /* iNumVerts */ ) { }
	void DrawCompiledGeometryInternal( const RageCompiledGeometry *, int /* iMeshIndex */ ) { }
	void DrawLineStripInternal( const Rage::SpriteVertex [], int /* iNumVerts */, float /* LineWidth */ ) { }
	void DrawSymmetricQuadStripInternal( const Rage::SpriteVertex [], int /* iNumVerts */ ) { }

	VideoModeParams m_Params;
	std::string TryVideoMode( const VideoModeParams &p, bool & /* bNewDeviceOut */ ) { m_Params = p; return std::string(); }
	RageSurface* CreateScreenshot();
	Rage::Matrix GetOrthoMatrix( float l, float r, float b, float t, float zn, float zf ); 
	bool SupportsSurfaceFormat( RagePixelFormat ) { return true; }
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

