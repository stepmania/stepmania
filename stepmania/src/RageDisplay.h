#ifndef RAGEDISPLAY_H
#define RAGEDISPLAY_H
/*
-----------------------------------------------------------------------------
 Class: RageDisplay

 Desc: Wrapper around a D3D device.  Also holds global vertex and index 
	buffers that dynamic geometry can use to render.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

class RageTexture;
struct RageMatrix;
struct RageVertex;

#include "RageTypes.h"

const int REFRESH_DEFAULT = 0;
struct oglspecs_t;

class RageDisplay
{
	friend class RageTexture;

public:
	RageDisplay( bool windowed, int width, int height, int bpp, int rate, bool vsync );
	~RageDisplay();

	bool IsSoftwareRenderer();

	bool SetVideoMode( bool windowed, int width, int height, int bpp, int rate, bool vsync );

	/* Call this when the resolution has been changed externally: */
	void ResolutionChanged(int width, int height);

	void Clear();
	void Flip();
	float PredictedSecondsUntilNextFlip() const;
	bool IsWindowed() const;
	int GetWidth() const;
	int GetHeight() const;
	int GetBPP() const;
	
	void PushMatrix();
	void PopMatrix();
	void Translate( float x, float y, float z );
	void TranslateLocal( float x, float y, float z );
	void Scale( float x, float y, float z );
	void RotateX( float r );
	void RotateY( float r );
	void RotateZ( float r );
	void smPostMultMatrixf( const RageMatrix &f );

	void SetTexture( RageTexture* pTexture );

	void SetTextureModeModulate();
	void SetTextureModeGlow();
	void SetBlendModeNormal();
	void SetBlendModeAdd();
	bool ZBufferEnabled() const;
	void EnableZBuffer();
	void DisableZBuffer();
	void EnableTextureWrapping();
	void DisableTextureWrapping();

	void DrawQuad( const RageVertex v[4] );	// upper-left, upper-right, lower-left, lower-right
	void DrawQuads( const RageVertex v[], int iNumVerts );
	void DrawFan( const RageVertex v[], int iNumVerts );
	void DrawStrip( const RageVertex v[], int iNumVerts );
	void DrawLoop( const RageVertex v[], int iNumVerts, float LineWidth );
	void DrawLoop_LinesAndPoints( const RageVertex v[], int iNumVerts, float LineWidth );
	void DrawLoop_Polys( const RageVertex v[], int iNumVerts, float LineWidth );
	void FlushQueue();

	int GetMaxTextureSize() const;
	
	void EnterPerspective(float fov, bool preserve_loc = true);
	void ExitPerspective();
	void LookAt(const RageVector3 &Eye, const RageVector3 &At, const RageVector3 &Up);

	/* Statistics */
	int GetFPS() const;
	int GetVPF() const;
	int GetCumFPS() const; /* average FPS since last reset */
	void ResetStats();

	const oglspecs_t &GetSpecs() const { return *m_oglspecs; }
	void DisablePalettedTexture();

	void SaveScreenshot( CString sPath );

protected:
	void AddVerts( const RageVertex v[], int iNumVerts );
	void SetBlendMode(int src, int dst);
	void SetupOpenGL();
	void SetupExtensions();
	void SetViewport(int shift_left, int shift_down);
	oglspecs_t *m_oglspecs;
	/* Don't use this to check extensions; use GetSpecs. */
	bool HasExtension(CString ext) const;
	void RageDisplay::DumpOpenGLDebugInfo();

	void DrawPolyLine(const RageVertex &p1, const RageVertex &p2, float LineWidth );
};


extern RageDisplay*		DISPLAY;	// global and accessable from anywhere in our program

#endif
