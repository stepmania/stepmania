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

	bool IsSoftwareRenderer();
	void ResolutionChanged();
	const PixelFormatDesc *GetPixelFormatDesc(PixelFormat pf) const;

	void BeginFrame();	
	void EndFrame();
	VideoModeParams GetVideoModeParams() const;
	void SetBlendMode( BlendMode mode );
	bool SupportsTextureFormat( PixelFormat pixfmt, bool realtime=false );
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

	RageModelVertexArray* CreateRageModelVertexArray();
	void DeleteRageModelVertexArray( RageModelVertexArray* p );

	void DrawQuads( const RageSpriteVertex v[], int iNumVerts );
	void DrawFan( const RageSpriteVertex v[], int iNumVerts );
	void DrawStrip( const RageSpriteVertex v[], int iNumVerts );
	void DrawTriangles( const RageSpriteVertex v[], int iNumVerts );
	void DrawIndexedTriangles( const RageModelVertexArray *p );
//	void DrawLineStrip( const RageSpriteVertex v[], int iNumVerts, float LineWidth );

protected:
	CString TryVideoMode( VideoModeParams params, bool &bNewDeviceOut );
	SDL_Surface* CreateScreenshot();
	void SetViewport(int shift_left, int shift_down);
	RageMatrix GetOrthoMatrix( float l, float r, float b, float t, float zn, float zf ); 
};

#endif

