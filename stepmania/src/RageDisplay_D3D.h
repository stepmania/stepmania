#ifndef RAGEDISPLAY_D3D_H
#define RAGEDISPLAY_D3D_H

class RageException_D3DNotInstalled: public exception 
{
	const char *what() { return 
		"DirectX 8.1 or greater is not installed.  You can download it from:\n"
		"http://search.microsoft.com/gomsuri.asp?n=1&c=rp_BestBets&siteid=us&target=http://www.microsoft.com/downloads/details.aspx?FamilyID=a19bed22-0b25-4e5d-a584-6389d8a3dad0&displaylang=en"; }
};
class RageException_D3DNoAcceleration: public exception
{
	const char *what() { return 
		"Your system is reporting that Direct3D hardware acceleration is not available.  "
		"Please obtain an updated driver from your video card manufacturer.\n\n"; }
};
class RageException_D3DNoPalettedAlpha: public exception
{
	const char *what() { return 
		"Your video card doesn'thardware acceleration is not available.  "
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
	bool SupportsTextureFormat( PixelFormat pixfmt );
	unsigned CreateTexture( PixelFormat pixfmt, SDL_Surface*& img );
	void UpdateTexture( 
		unsigned uTexHandle, 
		PixelFormat pixfmt,	// this must be the same as what was passed to CreateTexture
		SDL_Surface*& img,
		int xoffset, int yoffset, int width, int height 
		);
	void DeleteTexture( unsigned uTexHandle );
	void SetTexture( RageTexture* pTexture );
	void SetTextureModeModulate();
	void SetTextureModeGlow( GlowMode m=GLOW_WHITEN );
	void SetTextureWrapping( bool b );
	int GetMaxTextureSize() const;
	void SetTextureFiltering( bool b);
	bool IsZBufferEnabled() const;
	void SetZBuffer( bool b );
	void ClearZBuffer();
	void SetBackfaceCull( bool b );
	void SetAlphaTest( bool b );
	void SetMaterial( 
		float emissive[4],
		float ambient[4],
		float diffuse[4],
		float specular[4],
		float shininess
		);
	void SetLighting( bool b );
	void SetLightOff( int index );
	void SetLightDirectional( 
		int index, 
		RageColor ambient, 
		RageColor diffuse, 
		RageColor specular, 
		RageVector3 dir );

	void DrawQuads( const RageVertex v[], int iNumVerts );
	void DrawFan( const RageVertex v[], int iNumVerts );
	void DrawStrip( const RageVertex v[], int iNumVerts );
	void DrawTriangles( const RageVertex v[], int iNumVerts );
	void DrawIndexedTriangles( const RageVertex v[], int iNumVerts, const Uint16* pIndices, int iNumIndices );
//	void DrawLineStrip( const RageVertex v[], int iNumVerts, float LineWidth );

	void SaveScreenshot( CString sPath );
protected:
	bool TryVideoMode( VideoModeParams params, bool &bNewDeviceOut );
	void SetViewport(int shift_left, int shift_down);
	RageMatrix GetOrthoMatrix( float l, float r, float b, float t, float zn, float zf ); 

};

#endif

