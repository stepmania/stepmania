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
#include "ModelTypes.h"

const int REFRESH_DEFAULT = 0;
struct SDL_Surface;
const int MAX_TEXTURE_UNITS = 2;

// VertexArray holds vertex data in a format that is most efficient for
// the graphics API.
class RageModelVertexArray
{
public:
	virtual ~RageModelVertexArray() { }
	
	virtual size_t sizeVerts() const = 0;
	virtual void resizeVerts( size_t size ) = 0;
	
	virtual size_t sizeTriangles() const = 0;
	virtual void resizeTriangles( size_t size ) = 0;

	virtual RageVector3&	Position( int index ) = 0;
	virtual RageVector3&	Normal	( int index ) = 0;
	virtual RageVector2&	TexCoord( int index ) = 0;
	virtual Sint8&			Bone	( int index ) = 0;

	virtual msTriangle&		Triangle( int index ) = 0;

	virtual void OnChanged() const = 0;	// call this to re-send data to the video card
	virtual void Draw() const = 0;	// call this to make the video card draw the vertices
};


class RageDisplay
{
	friend class RageTexture;

public:

	struct PixelFormatDesc {
		int bpp;
		unsigned int masks[4];
	};

	enum PixelFormat {
		FMT_RGBA8,
		FMT_RGBA4,
		FMT_RGB5A1,
		FMT_RGB5,
		FMT_RGB8,
		FMT_PAL,
		/* The above formats differ between OpenGL and D3D. These are provided as
		* alternatives for OpenGL that match some format in D3D.  Don't use them
		* directly; they'll be matched automatically by FindPixelFormat. */
		FMT_BGR8,
		FMT_A1BGR5,
		NUM_PIX_FORMATS
	};

	static CString PixelFormatToString( PixelFormat pixfmt );
	virtual const PixelFormatDesc *GetPixelFormatDesc(PixelFormat pf) const = 0;

	struct VideoModeParams
	{
		// Initialize with a constructor so to guarantee all paramters
		// are filled (in case new params are added).
		VideoModeParams( 
			bool windowed_,
			int width_,
			int height_,
			int bpp_,
			int rate_,
			bool vsync_,
			bool interlaced_,
			bool bSmoothLines_,
			CString sWindowTitle_,
			CString sIconFile_
#ifdef _XBOX
			, bool PAL_
#endif
		)
		{
			windowed = windowed_;
			width = width_;
			height = height_;
			bpp = bpp_;
			rate = rate_;
			vsync = vsync_;
			interlaced = interlaced_;
			bSmoothLines = bSmoothLines_;
			sWindowTitle = sWindowTitle_;
			sIconFile = sIconFile_;
#ifdef _XBOX
			PAL = PAL_;
#endif
		}
		VideoModeParams() {}

		bool windowed;
		int width;
		int height;
		int bpp;
		int rate;
		bool vsync;
		bool bSmoothLines;
		bool interlaced;
#ifdef _XBOX
		bool PAL;
#endif
		CString sWindowTitle;
		CString sIconFile;
	};

	/* This is needed or the overridden classes' dtors will not be called. */
	virtual ~RageDisplay() { }

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

	virtual bool SupportsTextureFormat( PixelFormat pixfmt, bool realtime=false ) = 0;

	/* This really indicates whether 4-bit palettes will actually use less memory
	 * than 8-bit ones.  Note that 4-bit palettes are uploaded as 8-bit paletted
	 * surfaces with color index values in the range 0..15; not as 4-bit "packed
	 * indexes". */
	virtual bool Supports4BitPalettes() { return false; }

	/* return 0 if failed or internal texture resource handle 
	 * (unsigned in OpenGL, texture pointer in D3D) */
	virtual unsigned CreateTexture( 
		PixelFormat pixfmt,			// format of img and of texture in video mem
		SDL_Surface* img, 		// must be in pixfmt
		bool bGenerateMipMaps
		) = 0;
	virtual void UpdateTexture( 
		unsigned uTexHandle, 
		SDL_Surface* img,
		int xoffset, int yoffset, int width, int height 
		) = 0;
	virtual void DeleteTexture( unsigned uTexHandle ) = 0;
	virtual void ClearAllTextures() = 0;
	virtual void SetTexture( int iTextureUnitIndex, RageTexture* pTexture ) = 0;
	virtual void SetTextureModeModulate() = 0;
	virtual void SetTextureModeGlow( GlowMode m=GLOW_WHITEN ) = 0;
	virtual void SetTextureModeAdd() = 0;
	virtual void SetTextureWrapping( bool b ) = 0;
	virtual int GetMaxTextureSize() const = 0;
	virtual void SetTextureFiltering( bool b ) = 0;

	virtual bool IsZTestEnabled() const = 0;
	virtual bool IsZWriteEnabled() const = 0;
	virtual void SetZWrite( bool b ) = 0;
	virtual void SetZTest( bool b ) = 0;
	virtual void ClearZBuffer() = 0;
	void SetZBuffer( bool b ) { SetZWrite(b); SetZTest(b); } // shortcut

	virtual void SetCullMode( CullMode mode ) = 0;
	
	virtual void SetAlphaTest( bool b ) = 0;
	
	virtual void SetMaterial( 
		const RageColor &emissive,
		const RageColor &ambient,
		const RageColor &diffuse,
		const RageColor &specular,
		float shininess
		) = 0;

	virtual void SetLighting( bool b ) = 0;
	virtual void SetLightOff( int index ) = 0;
	virtual void SetLightDirectional( 
		int index, 
		const RageColor &ambient, 
		const RageColor &diffuse, 
		const RageColor &specular, 
		const RageVector3 &dir ) = 0;

	virtual void SetSphereEnironmentMapping( bool b ) = 0;

	virtual RageModelVertexArray* CreateRageModelVertexArray() = 0;
	virtual void DeleteRageModelVertexArray( RageModelVertexArray* p ) = 0;

	void DrawQuads( const RageSpriteVertex v[], int iNumVerts );
	void DrawQuadStrip( const RageSpriteVertex v[], int iNumVerts );
	void DrawFan( const RageSpriteVertex v[], int iNumVerts );
	void DrawStrip( const RageSpriteVertex v[], int iNumVerts );
	void DrawTriangles( const RageSpriteVertex v[], int iNumVerts );
	void DrawIndexedTriangles( const RageModelVertexArray *p );
	void DrawLineStrip( const RageSpriteVertex v[], int iNumVerts, float LineWidth );
	void DrawCircle( const RageSpriteVertex &v, float radius );

	void DrawQuad( const RageSpriteVertex v[] ) { DrawQuads(v,4); } /* alias. upper-left, upper-right, lower-left, lower-right */

	enum GraphicsFileFormat
	{
		SAVE_LOSSLESS, // bmp
		SAVE_LOSSY_LOW_QUAL, // jpg
		SAVE_LOSSY_HIGH_QUAL // jpg
	};
	bool SaveScreenshot( CString sPath, GraphicsFileFormat format );

	virtual CString GetTextureDiagnostics( unsigned id ) const { return ""; }

protected:
	virtual void DrawQuadsInternal( const RageSpriteVertex v[], int iNumVerts ) = 0;
	virtual void DrawQuadStripInternal( const RageSpriteVertex v[], int iNumVerts ) = 0;
	virtual void DrawFanInternal( const RageSpriteVertex v[], int iNumVerts ) = 0;
	virtual void DrawStripInternal( const RageSpriteVertex v[], int iNumVerts ) = 0;
	virtual void DrawTrianglesInternal( const RageSpriteVertex v[], int iNumVerts ) = 0;
	virtual void DrawIndexedTrianglesInternal( const RageModelVertexArray *p ) = 0;
	virtual void DrawLineStripInternal( const RageSpriteVertex v[], int iNumVerts, float LineWidth );
	virtual void DrawCircleInternal( const RageSpriteVertex &v, float radius );

	// Return "" if mode change was successful, an error message otherwise.
	// bNewDeviceOut is set true if a new device was created and textures
	// need to be reloaded.
	virtual CString TryVideoMode( VideoModeParams params, bool &bNewDeviceOut ) = 0;
	virtual SDL_Surface* CreateScreenshot() = 0;	// allocates a surface.  Caller must SDL_FreeSurface it.

	virtual void SetViewport(int shift_left, int shift_down) = 0;

	void DrawPolyLine(const RageSpriteVertex &p1, const RageSpriteVertex &p2, float LineWidth );


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

	/* World matrix stack functions. */
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

	/* Projection and View matrix stack functions. */
	void CameraPushMatrix();
	void CameraPopMatrix();
	void LoadMenuPerspective( float fovDegrees, float fVanishPointX, float fVanishPointY );
	void LoadLookAt( float fov, const RageVector3 &Eye, const RageVector3 &At, const RageVector3 &Up );

	/* Centering matrix */
	void ChangeCentering( int trans_x, int trans_y, float scale_x, float scale_y );

	SDL_Surface *CreateSurfaceFromPixfmt( PixelFormat pixfmt, void *pixels, int width, int height, int pitch );
	PixelFormat FindPixelFormat( int bpp, int Rmask, int Gmask, int Bmask, int Amask, bool realtime=false );

protected:
	RageMatrix GetPerspectiveMatrix(float fovy, float aspect, float zNear, float zFar);

	// Different for D3D and OpenGL.  Not sure why they're not compatible. -Chris
	virtual RageMatrix GetOrthoMatrix( float l, float r, float b, float t, float zn, float zf ) = 0; 
	virtual RageMatrix GetFrustumMatrix( float l, float r, float b, float t, float zn, float zf ); 

	//
	// Matrix that adjusts position and scale of image on the screen
	//
	RageMatrix m_Centering;

	// Called by the RageDisplay derivitives
	const RageMatrix* GetCentering() { return &m_Centering; }
	const RageMatrix* GetProjectionTop();
	const RageMatrix* GetViewTop();
	const RageMatrix* GetWorldTop();
};


extern RageDisplay*		DISPLAY;	// global and accessable from anywhere in our program

#endif
