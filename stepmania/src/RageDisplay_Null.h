#ifndef RAGEDISPLAY_NULL_H
#define RAGEDISPLAY_NULL_H

class RageDisplay_Null: public RageDisplay
{
public:
	RageDisplay_Null( VideoModeParams p );
	void Update( float fDeltaTime ) { }

	bool IsSoftwareRenderer() { return false; }
	void ResolutionChanged() { }
	const PixelFormatDesc *GetPixelFormatDesc(PixelFormat pf) const;

	void BeginFrame() { }
	void EndFrame();
	VideoModeParams GetVideoModeParams() const { return m_Params; }
	void SetBlendMode( BlendMode mode ) { }
	bool SupportsTextureFormat( PixelFormat pixfmt, bool realtime=false ) { return true; }
	bool Supports4BitPalettes() { return true; }
	unsigned CreateTexture( 
		PixelFormat pixfmt, 
		SDL_Surface* img,
		bool bGenerateMipMaps ) { return 1; }
	void UpdateTexture( 
		unsigned uTexHandle, 
		SDL_Surface* img,
		int xoffset, int yoffset, int width, int height 
		) { }
	void DeleteTexture( unsigned uTexHandle ) { }
	void ClearAllTextures() { }
	void SetTexture( int iTextureUnitIndex, RageTexture* pTexture ) { }
	void SetTextureModeModulate() { }
	void SetTextureModeGlow( GlowMode m=GLOW_WHITEN ) { }
	void SetTextureModeAdd() { }
	void SetTextureWrapping( bool b ) { }
	int GetMaxTextureSize() const { return 2048; }
	void SetTextureFiltering( bool b) { }
	bool IsZWriteEnabled() const { return false; }
	bool IsZTestEnabled() const { return false; }
	void SetZWrite( bool b ) { }
	void SetZTest( bool b ) { }
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

	void SetSphereEnironmentMapping( bool b ) { }
	
	RageModelVertexArray* CreateRageModelVertexArray();
	void DeleteRageModelVertexArray( RageModelVertexArray* p );

	void DrawQuads( const RageSpriteVertex v[], int iNumVerts ) { }
	void DrawQuadStrip( const RageSpriteVertex v[], int iNumVerts ) { }
	void DrawFan( const RageSpriteVertex v[], int iNumVerts ) { }
	void DrawStrip( const RageSpriteVertex v[], int iNumVerts ) { }
	void DrawTriangles( const RageSpriteVertex v[], int iNumVerts ) { }
	void DrawIndexedTriangles( const RageModelVertexArray *p ) { }
	void DrawLineStrip( const RageSpriteVertex v[], int iNumVerts, float LineWidth ) { }

protected:
	VideoModeParams m_Params;
	CString TryVideoMode( VideoModeParams params, bool &bNewDeviceOut ) { m_Params = params; return ""; }
	SDL_Surface* CreateScreenshot();
	void SetViewport(int shift_left, int shift_down) { }
	RageMatrix GetOrthoMatrix( float l, float r, float b, float t, float zn, float zf ); 
	bool SupportsSurfaceFormat( PixelFormat pixfmt ) { return true; }
};

#endif
