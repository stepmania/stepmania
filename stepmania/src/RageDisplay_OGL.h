#ifndef RAGEDISPLAY_OGL_H
#define RAGEDISPLAY_OGL_H

class RageDisplay_OGL: public RageDisplay
{
public:
	RageDisplay_OGL( VideoModeParams params, bool bAllowUnacceleratedRenderer );
	virtual ~RageDisplay_OGL();
	void Update(float fDeltaTime);

	bool IsSoftwareRenderer();
	void ResolutionChanged();
	const PixelFormatDesc *GetPixelFormatDesc(PixelFormat pf) const;

	void BeginFrame();	
	void EndFrame();
	VideoModeParams GetVideoModeParams() const;
	void SetBlendMode( BlendMode mode );
	bool SupportsTextureFormat( PixelFormat pixfmt, bool realtime=false );
	bool Supports4BitPalettes();
	unsigned CreateTexture( 
		PixelFormat pixfmt, 
		SDL_Surface* img,
		bool bGenerateMipMaps );
	void UpdateTexture( 
		unsigned uTexHandle, 
		SDL_Surface* img,
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
	void SetZTest( bool b );
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
	SDL_Surface* CreateScreenshot();
	void SetViewport(int shift_left, int shift_down);
	RageMatrix GetOrthoMatrix( float l, float r, float b, float t, float zn, float zf ); 
	PixelFormat GetImgPixelFormat( SDL_Surface* &img, bool &FreeImg, int width, int height );
	bool SupportsSurfaceFormat( PixelFormat pixfmt );
	void SendCurrentMatrices();
};

#endif
