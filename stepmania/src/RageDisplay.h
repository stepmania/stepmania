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

CString PixelFormatToString( PixelFormat pixfmt );

struct PixelFormatDesc {
	int bpp;
	unsigned int masks[4];
};

class RageDisplay
{
	friend class RageTexture;

public:

	struct VideoModeParams
	{
		// Initialize with a constructor so to guarantee all paramters
		// are filled (in case new params are added).
		VideoModeParams( 
			bool _windowed,
			int _width,
			int _height,
			int _bpp,
			int _rate,
			bool _vsync,
			bool _bAntiAliasing,
			CString _sWindowTitle,
			CString _sIconFile )
		{
			windowed = _windowed;
			width = _width;
			height = _height;
			bpp = _bpp;
			rate = _rate;
			vsync = _vsync;
			bAntiAliasing = _bAntiAliasing;
			sWindowTitle = _sWindowTitle;
			sIconFile = _sIconFile;
		}
		VideoModeParams() {}

		bool windowed;
		int width;
		int height;
		int bpp;
		int rate;
		bool vsync;
		bool bAntiAliasing;
		CString sWindowTitle;
		CString sIconFile;
	};

	virtual const PixelFormatDesc *GetPixelFormatDesc(PixelFormat pf) const = 0;

	virtual void Update(float fDeltaTime) { }

	virtual bool IsSoftwareRenderer() = 0;

	// Don't override this.  Override TryVideoMode() instead.
	// This will set the video mode to be as close as possible to params.
	// Return true if device was re-created and we need to reload textures.
	bool SetVideoMode( VideoModeParams params );

	/* Call this when the resolution has been changed externally: */
	virtual void ResolutionChanged() { }

	virtual void BeginFrame() = 0;	
	virtual void EndFrame() = 0;
	virtual VideoModeParams GetVideoModeParams() const = 0;
	bool IsWindowed() const { return this->GetVideoModeParams().windowed; }
	
	virtual void SetBlendMode( BlendMode mode ) = 0;

	virtual bool SupportsTextureFormat( PixelFormat pixfmt ) = 0;
	/* return 0 if failed or internal texture resource handle 
	 * (unsigned in OpenGL, texture pointer in D3D) */
	virtual unsigned CreateTexture( 
		PixelFormat pixfmt,			// format of img and of texture in video mem
		SDL_Surface*& img 		// must be in pixfmt
		) = 0;
	virtual void UpdateTexture( 
		unsigned uTexHandle, 
		PixelFormat pixfmt,	// this must be the same as what was passed to CreateTexture
		SDL_Surface*& img,
		int xoffset, int yoffset, int width, int height 
		) = 0;
	virtual void DeleteTexture( unsigned uTexHandle ) = 0;
	virtual void SetTexture( RageTexture* pTexture ) = 0;
	virtual void SetTextureModeModulate() = 0;
	virtual void SetTextureModeGlow( GlowMode m=GLOW_WHITEN ) = 0;
	virtual void SetTextureWrapping( bool b ) = 0;
	virtual int GetMaxTextureSize() const = 0;
	virtual void SetTextureFiltering( bool b ) = 0;

	virtual bool IsZBufferEnabled() const = 0;
	virtual void SetZBuffer( bool b ) = 0;
	virtual void ClearZBuffer() = 0;

	virtual void SetBackfaceCull( bool b ) = 0;
	
	virtual void SetAlphaTest( bool b ) = 0;
	
	virtual void SetMaterial( 
		float emissive[4],
		float ambient[4],
		float diffuse[4],
		float specular[4],
		float shininess
		) = 0;

	virtual void SetLighting( bool b ) = 0;
	virtual void SetLightOff( int index ) = 0;
	virtual void SetLightDirectional( 
		int index, 
		RageColor ambient, 
		RageColor diffuse, 
		RageColor specular, 
		RageVector3 dir ) = 0;


	void DrawQuad( const RageVertex v[] ) { DrawQuads(v,4); } /* alias. upper-left, upper-right, lower-left, lower-right */
	virtual void DrawQuads( const RageVertex v[], int iNumVerts ) = 0;
	virtual void DrawFan( const RageVertex v[], int iNumVerts ) = 0;
	virtual void DrawStrip( const RageVertex v[], int iNumVerts ) = 0;
	virtual void DrawTriangles( const RageVertex v[], int iNumVerts ) = 0;
	virtual void DrawIndexedTriangles( const RageVertex v[], const Uint16* pIndices, int iNumIndices ) = 0;
	virtual void DrawLineStrip( const RageVertex v[], int iNumVerts, float LineWidth );

	void DrawCircle( const RageVertex &v, float radius );

	virtual void SaveScreenshot( CString sPath ) = 0;

protected:
	// Return true if mode change was successful.
	// bNewDeviceOut is set true if a new device was created and textures
	// need to be reloaded.
	virtual bool TryVideoMode( VideoModeParams params, bool &bNewDeviceOut ) = 0;

	virtual void SetViewport(int shift_left, int shift_down) = 0;

	void DrawPolyLine(const RageVertex &p1, const RageVertex &p2, float LineWidth );

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

	virtual RageMatrix GetOrthoMatrix( float l, float r, float b, float t, float zn, float zf ) = 0; 
	RageMatrix GetFrustrumMatrix(
		float left,    
		float right,   
		float bottom,  
		float top,     
		float znear,   
		float zfar );
	RageMatrix GetPerspectiveMatrix(float fovy, float aspect, float zNear, float zFar);

protected:
	const RageMatrix* GetProjection();
	const RageMatrix* GetModelViewTop();

};


extern RageDisplay*		DISPLAY;	// global and accessable from anywhere in our program

#endif
