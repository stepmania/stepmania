#ifndef RAGEDISPLAY_D3D_H
#define RAGEDISPLAY_D3D_H

class RageException_D3DNotInstalled: public exception
{
		const char *what() const { return
				"DirectX 8.1 or greater is not installed.  You can download it from:\n"
				"http://www.microsoft.com/downloads/details.aspx?FamilyID=a19bed22-0b25-4e5d-a584-6389d8a3dad0&displaylang=en"; }
};

class RageException_D3DNoAcceleration: public exception
{
		const char *what() const { return
				"Your system is reporting that Direct3D hardware acceleration is not available.  "
				"Please obtain an updated driver from your video card manufacturer.\n\n"; }
};
				

class RageDisplay_D3D: public RageDisplay
{
public:
	RageDisplay_D3D( VideoModeParams params );
	~RageDisplay_D3D();
	void Update(float fDeltaTime);

	void ResolutionChanged();
	const PixelFormatDesc *GetPixelFormatDesc(PixelFormat pf) const;

	bool BeginFrame();	
	void EndFrame();
	VideoModeParams GetVideoModeParams() const;
	void SetBlendMode( BlendMode mode );
	bool SupportsTextureFormat( PixelFormat pixfmt, bool realtime=false );
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
	void SetTexture( int iTextureUnitIndex, RageTexture* pTexture );
	void SetTextureModeModulate();
	void SetTextureModeGlow( GlowMode m=GLOW_WHITEN );
	void SetTextureModeAdd();
	void SetTextureWrapping( bool b );
	int GetMaxTextureSize() const;
	void SetTextureFiltering( bool b);
	bool IsZWriteEnabled() const;
	bool IsZTestEnabled() const;
	void SetZWrite( bool b );
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

	void SetSphereEnironmentMapping( bool b );

	RageCompiledGeometry* CreateCompiledGeometry();
	void DeleteCompiledGeometry( RageCompiledGeometry* p );

protected:
	void DrawQuadsInternal( const RageSpriteVertex v[], int iNumVerts );
	void DrawQuadStripInternal( const RageSpriteVertex v[], int iNumVerts );
	void DrawFanInternal( const RageSpriteVertex v[], int iNumVerts );
	void DrawStripInternal( const RageSpriteVertex v[], int iNumVerts );
	void DrawTrianglesInternal( const RageSpriteVertex v[], int iNumVerts );
	void DrawCompiledGeometryInternal( const RageCompiledGeometry *p, int iMeshIndex );

	CString TryVideoMode( VideoModeParams params, bool &bNewDeviceOut );
	RageSurface* CreateScreenshot();
	void SetViewport(int shift_left, int shift_down);
	RageMatrix GetOrthoMatrix( float l, float r, float b, float t, float zn, float zf ); 

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
