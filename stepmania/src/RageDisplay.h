#pragma once
/*
-----------------------------------------------------------------------------
 Class: RageDisplay

 Desc: Wrapper around a D3D device.  Also holds global vertex and index 
	buffers that dynamic geometry can use to render.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

class RageDisplay;


class RageTexture;
#include <D3DX8.h>

// A structure for our custom vertex type. We added texture coordinates
struct RAGEVERTEX
{
    D3DXVECTOR3 p;			// position
    D3DCOLOR    color;		// diffuse color
	D3DXVECTOR2 t;			// texture coordinates
};

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_RAGEVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)


const int MAX_NUM_QUADS = 2048;
//const int MAX_NUM_INDICIES = MAX_NUM_QUADS*3;	// two triangles per quad
const int MAX_NUM_VERTICIES = MAX_NUM_QUADS*4;	// 4 verticies per quad



// 
// Chris:
// I did a lot of testing, and drawing indexed primitives is SLOWER than duplicating 
// verticies and not indexing.  In fact, drawing indexed primitives is about 30% slower.
// 


class RageDisplay
{
	friend class RageTexture;

public:
	RageDisplay( HWND hWnd );
	~RageDisplay();
	bool SwitchDisplayMode( 
		const bool bWindowed, const int iWidth, const int iHeight, const int iBPP, const int iFullScreenHz );

//	LPDIRECT3D8 GetD3D()					{ return m_pd3d; };
//	inline LPDIRECT3DDEVICE8 GetDevice()	{ return m_pd3dDevice; };
	const D3DCAPS8& GetDeviceCaps()			{ return m_DeviceCaps; };

	HRESULT Reset();
	HRESULT BeginFrame();
	HRESULT EndFrame();
	HRESULT ShowFrame();

	HRESULT Invalidate();
	HRESULT Restore();


	BOOL IsWindowed()	{ return m_d3dpp.Windowed; };
	DWORD GetWidth()	{ return m_d3dpp.BackBufferWidth; };
	DWORD GetHeight()	{ return m_d3dpp.BackBufferHeight; };
	DWORD GetBPP()		
	{ 
		switch( m_d3dpp.BackBufferFormat )
		{
		case D3DFMT_R5G6B5:
		case D3DFMT_X1R5G5B5:
		case D3DFMT_A1R5G5B5:
			return 16;
		case D3DFMT_R8G8B8:
		case D3DFMT_X8R8G8B8:
		case D3DFMT_A8R8G8B8:
			return 32;
		default:
			ASSERT( false );	// unexpected format
			return 0;
		}
	}

	
//	LPDIRECT3DVERTEXBUFFER8 GetVertexBuffer() { return m_pVB; };
	void SetViewTransform( const D3DXMATRIX* pMatrix );
	void SetProjectionTransform( const D3DXMATRIX* pMatrix );
	void GetViewTransform( D3DXMATRIX* pMatrixOut );
	void GetProjectionTransform( D3DXMATRIX* pMatrixOut );

	void ResetMatrixStack();
	void PushMatrix();
	void PopMatrix();
	void Translate( const float x, const float y, const float z );
	void TranslateLocal( const float x, const float y, const float z );
	void Scale( const float x, const float y, const float z );
	void RotateX( const float r );
	void RotateY( const float r );
	void RotateZ( const float r );
//	void RotateYawPitchRoll( const float x, const float y, const float z );

	int GetFPS() { return m_iFPS; };
	int GetTPF() { return m_iTPF; };
	int GetDPF() { return m_iDPF; };

	int MaxRefresh(int iWidth, int iHeight) const;
	void GetHzAtResolution(int width, int height, CArray<int,int> &add) const;

private:
	D3DXMATRIX& GetTopMatrix() { return m_MatrixStack.ElementAt( m_MatrixStack.GetSize()-1 ); };

	HWND m_hWnd;

	// DirectDraw/Direct3D objects
	LPDIRECT3D8			m_pd3d;
	LPDIRECT3DDEVICE8	m_pd3dDevice;
	D3DCAPS8			m_DeviceCaps;


	D3DDISPLAYMODE		m_DesktopMode;
	D3DPRESENT_PARAMETERS	m_d3dpp;

	// a vertex buffer for all to share
	LPDIRECT3DVERTEXBUFFER8 m_pVB;
	void CreateVertexBuffer();
	void ReleaseVertexBuffer();

	// OpenGL-like matrix stack
	CArray<D3DXMATRIX, D3DXMATRIX&>	m_MatrixStack;

	// for performance stats
	float	m_fLastCheckTime;
	int		m_iFramesRenderedSinceLastCheck;
	int		m_iTrianglesRenderedSinceLastCheck;
	int		m_iDrawsSinceLastCheck;
	int		m_iFPS;
	int		m_iTPF;
	int		m_iDPF;


	//
	// Render Queue stuff
	//
protected:
	RAGEVERTEX	m_vertQueue[MAX_NUM_VERTICIES];
	int			m_iNumVerts;

public:
	// TODO:  Elminiate vertex duplication using an index buffer.  Would this work with OpenGL though?
//	void AddTriangle( const RAGEVERTEX v[3] );
	void AddQuad( const RAGEVERTEX v[4] );	// upper-left, upper-right, lower-left, lower-right
	void AddFan( const RAGEVERTEX v[], int iNumPrimitives );
	void AddStrip( const RAGEVERTEX v[], int iNumPrimitives );
	void AddTriangle(
		const D3DXVECTOR3& p0, const D3DCOLOR& c0, const D3DXVECTOR2& t0,
		const D3DXVECTOR3& p1, const D3DCOLOR& c1, const D3DXVECTOR2& t1,
		const D3DXVECTOR3& p2, const D3DCOLOR& c2, const D3DXVECTOR2& t2 );
	void AddQuad(
		const D3DXVECTOR3 &p0, const D3DCOLOR& c0, const D3DXVECTOR2& t0,	// upper-left
		const D3DXVECTOR3 &p1, const D3DCOLOR& c1, const D3DXVECTOR2& t1, 	// upper-right
		const D3DXVECTOR3 &p2, const D3DCOLOR& c2, const D3DXVECTOR2& t2, 	// lower-left
		const D3DXVECTOR3 &p3, const D3DCOLOR& c3, const D3DXVECTOR2& t3 );	// lower-right
	void FlushQueue();
	void SetTexture( RageTexture* pTexture );
	void SetColorTextureMultDiffuse();
	void SetColorDiffuse();
	void SetAlphaTextureMultDiffuse();
	void SetBlendModeNormal();
	void SetBlendModeAdd();
	void EnableZBuffer();
	void DisableZBuffer();
};


extern RageDisplay*		DISPLAY;	// global and accessable from anywhere in our program
