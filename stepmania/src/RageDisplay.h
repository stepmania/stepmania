#ifndef RAGEDISPLAY_H
#define RAGEDISPLAY_H
/*
-----------------------------------------------------------------------------
 Class: RageDisplay

 Desc: Wrapper around a graphics device.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "SDL_types.h"
#include "RageTypes.h"

const int REFRESH_DEFAULT = 0;
struct SDL_Surface;

// VertexArray holds vertex data in a format that is most efficient for
// the graphics API.
/*struct VertexArray
{
	VertexArray();
	~VertexArray();
	unsigned size();
	void resize( unsigned new_size );
	RageVector2& TexCoord( int index );
	RageColor& Color( int index );
	RageVector3& Normal( int index );
	RageVector3& Position( int index );
	// convenience.  Remove this later!
	void Set( int index, const RageVertex& v );

	struct Impl;
	Impl* pImpl;
};
*/

enum PixelFormat {
	FMT_RGBA8,
	FMT_RGBA4,
	FMT_RGB5A1,
	FMT_RGB5,
	FMT_RGB8,
	FMT_PAL,
	NUM_PIX_FORMATS
};
inline CString PixelFormatToString( PixelFormat pixfmt )
{
	const CString s[NUM_PIX_FORMATS] = {
		"FMT_RGBA8",
		"FMT_RGBA4",
		"FMT_RGB5A1",
		"FMT_RGB5",
		"FMT_RGB8",
		"FMT_PAL" };
	return s[pixfmt];
};
struct PixelFormatDesc {
	int bpp;
	unsigned int masks[4];
};
/* This is info is different for OGL and D3D. */
extern const PixelFormatDesc PIXEL_FORMAT_DESC[NUM_PIX_FORMATS];

class RageDisplay
{
	friend class RageTexture;

public:
	RageDisplay( bool windowed, int width, int height, int bpp, int rate, bool vsync );
	~RageDisplay();
	void Update(float fDeltaTime);

	bool IsSoftwareRenderer();

	bool SetVideoMode( bool windowed, int width, int height, int bpp, int rate, bool vsync );

	/* Call this when the resolution has been changed externally: */
	void ResolutionChanged();

	void BeginFrame();	
	void EndFrame();
	bool IsWindowed() const;
	int GetWidth() const;
	int GetHeight() const;
	int GetBPP() const;
	
	void SetBlendMode( BlendMode mode );

	bool SupportsTextureFormat( PixelFormat pixfmt );
	/* return 0 if failed or internal texture resource handle 
	 * (unsigned in OpenGL, texture pointer in D3D) */
	unsigned CreateTexture( 
		PixelFormat pixfmt,			// format of img and of texture in video mem
		SDL_Surface*& img 		// must be in pixfmt
		);
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


	void DrawQuad( const RageVertex v[] ) { DrawQuads(v,4); } /* alias. upper-left, upper-right, lower-left, lower-right */
	void DrawQuads( const RageVertex v[], int iNumVerts );
	void DrawFan( const RageVertex v[], int iNumVerts );
	void DrawStrip( const RageVertex v[], int iNumVerts );
	void DrawTriangles( const RageVertex v[], int iNumVerts );
	void DrawIndexedTriangles( const RageVertex v[], const Uint16* pIndices, int iNumIndices );
	void DrawLineStrip( const RageVertex v[], int iNumVerts, float LineWidth );

	void SaveScreenshot( CString sPath );

protected:
	void SetViewport(int shift_left, int shift_down);

	// Stuff in RageDisplay.cpp
	void SetDefaultRenderStates();

public:
	/* Statistics */
	int GetFPS() const;
	int GetVPF() const;
	int GetCumFPS() const; /* average FPS since last reset */
	void ResetStats();
	void ProcessStatsOnFlip();
	void StatsAddVerts( int iNumVertsRendered );

	/* Statistics */

	/* Statistics */
	void PushMatrix();
	void PopMatrix();
	void Translate( float x, float y, float z );
	void TranslateWorld( float x, float y, float z );
	void Scale( float x, float y, float z );
	void RotateX( float deg );
	void RotateY( float deg );
	void RotateZ( float deg );
	void MultMatrix( const RageMatrix &f ) { this->PostMultMatrix(f); } /* alias */
	void PostMultMatrix( const RageMatrix &f );
	void PreMultMatrix( const RageMatrix &f );
	void LoadIdentity();
	void LoadMenuPerspective(float fovDegrees);
	void EnterPerspective(float fov, bool preserve_loc = true, float znear = 1, float zfar = 1000);
	void ExitPerspective();
	void LookAt(const RageVector3 &Eye, const RageVector3 &At, const RageVector3 &Up);

protected:
	const RageMatrix* GetProjection();
	const RageMatrix* GetModelViewTop();

};


extern RageDisplay*		DISPLAY;	// global and accessable from anywhere in our program

#endif
