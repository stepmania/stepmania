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

enum RefreshRateMode { REFRESH_DEFAULT, REFRESH_MAX, REFRESH_MIN };

class RageDisplay
{
	friend class RageTexture;

public:
	RageDisplay( bool windowed, int width, int height, int bpp, RefreshRateMode rate, bool vsync );
	~RageDisplay();

	void SetVideoMode( bool windowed, int width, int height, int bpp, RefreshRateMode rate, bool vsync );

	void Clear();
	void Flip();
	bool IsWindowed() const;
	int GetWidth() const;
	int GetHeight() const;
	int GetBPP() const;
	
	void SetViewTransform( const RageMatrix* pMatrix );
	void SetProjectionTransform( const RageMatrix* pMatrix );
	void GetViewTransform( RageMatrix* pMatrixOut );
	void GetProjectionTransform( RageMatrix* pMatrixOut );

	void ResetMatrixStack();
	void PushMatrix();
	void PopMatrix();
	void Translate( float x, float y, float z );
	void TranslateLocal( float x, float y, float z );
	void Scale( float x, float y, float z );
	void RotateX( float r );
	void RotateY( float r );
	void RotateZ( float r );

	void SetTexture( RageTexture* pTexture );

	void SetTextureModeModulate();
	void SetTextureModeGlow();
	void SetBlendModeNormal();
	void SetBlendModeAdd();
	void EnableZBuffer();
	void DisableZBuffer();
	void EnableTextureWrapping();
	void DisableTextureWrapping();

	void DrawQuad( const RageVertex v[4] );	// upper-left, upper-right, lower-left, lower-right
	void DrawQuads( const RageVertex v[], int iNumVerts );
	void DrawFan( const RageVertex v[], int iNumVerts );
	void DrawStrip( const RageVertex v[], int iNumVerts );
	void FlushQueue();

	void GetHzAtResolution(int width, int height, int bpp, CArray<int,int> &add) const;
	int GetMaxTextureSize() const;

	/* Statistics */
	int GetFPS() const;
	int GetVPF() const;
	int GetDPF() const;

protected:
	void AddVerts( const RageVertex v[], int iNumVerts );
	void SetBlendMode(int src, int dst);
};


extern RageDisplay*		DISPLAY;	// global and accessable from anywhere in our program

#endif