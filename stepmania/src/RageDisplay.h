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



class RageDisplay
{
public:
	RageDisplay( HWND hWnd );
	~RageDisplay();
	bool SwitchDisplayMode( 
		const bool bWindowed, const int iWidth, const int iHeight, const int iBPP, const int iFullScreenHz );

	LPDIRECT3D8 GetD3D()					{ return m_pd3d; };
	inline LPDIRECT3DDEVICE8 GetDevice()	{ return m_pd3dDevice; };
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

	
	LPDIRECT3DVERTEXBUFFER8 GetVertexBuffer() { return m_pVB; };

	inline void ResetMatrixStack() 
	{ 
		m_MatrixStack.SetSize( 1, 20 );
		D3DXMatrixIdentity( &GetTopMatrix() );
		
		m_pd3dDevice->SetTransform( D3DTS_WORLD, &m_MatrixStack[m_MatrixStack.GetSize()-1] );
	};
	inline void PushMatrix() 
	{ 
		m_MatrixStack.Add( GetTopMatrix() );	
		ASSERT(m_MatrixStack.GetSize()<30);		// check for infinite loop
		m_pd3dDevice->SetTransform( D3DTS_WORLD, &m_MatrixStack[m_MatrixStack.GetSize()-1] );
	};
	inline void PopMatrix() 
	{ 
		m_MatrixStack.RemoveAt( m_MatrixStack.GetSize()-1 ); 
		m_pd3dDevice->SetTransform( D3DTS_WORLD, &m_MatrixStack[m_MatrixStack.GetSize()-1] );
	};
	inline void Translate( const float x, const float y, const float z )
	{
		D3DXMATRIX matTemp;
		D3DXMatrixTranslation( &matTemp, x, y, z );
		D3DXMATRIX& matTop = GetTopMatrix();
		matTop = matTemp * matTop;
		m_pd3dDevice->SetTransform( D3DTS_WORLD, &matTop ); 
	};
	inline void TranslateLocal( const float x, const float y, const float z )
	{
		D3DXMATRIX matTemp;
		D3DXMatrixTranslation( &matTemp, x, y, z );
		D3DXMATRIX& matTop = GetTopMatrix();
		matTop = matTop * matTemp;
		m_pd3dDevice->SetTransform( D3DTS_WORLD, &matTop ); 
	};
	inline void Scale( const float x, const float y, const float z )
	{
		D3DXMATRIX matTemp;
		D3DXMatrixScaling( &matTemp, x, y, z );
		D3DXMATRIX& matTop = GetTopMatrix();
		matTop = matTemp * matTop;
		m_pd3dDevice->SetTransform( D3DTS_WORLD, &matTop ); 
	};
	inline void RotateX( const float r )
	{
		D3DXMATRIX matTemp;
		D3DXMatrixRotationX( &matTemp, r );
		D3DXMATRIX& matTop = GetTopMatrix();
		matTop = matTemp * matTop;
		m_pd3dDevice->SetTransform( D3DTS_WORLD, &matTop ); 
	};
	inline void RotateY( const float r )
	{
		D3DXMATRIX matTemp;
		D3DXMatrixRotationY( &matTemp, r );
		D3DXMATRIX& matTop = GetTopMatrix();
		matTop = matTemp * matTop;
		m_pd3dDevice->SetTransform( D3DTS_WORLD, &matTop ); 
	};
	inline void RotateZ( const float r )
	{
		D3DXMATRIX matTemp;
		D3DXMatrixRotationZ( &matTemp, r );
		D3DXMATRIX& matTop = GetTopMatrix();
		matTop = matTemp * matTop;
		m_pd3dDevice->SetTransform( D3DTS_WORLD, &matTop ); 
	};
	inline void RotateYawPitchRoll( const float x, const float y, const float z )
	{
		D3DXMATRIX matTemp;
		D3DXMatrixRotationYawPitchRoll( &matTemp, x, y, z );
		D3DXMATRIX& matTop = GetTopMatrix();
		matTop = matTemp * matTop;
		m_pd3dDevice->SetTransform( D3DTS_WORLD, &matTop ); 
	};

	float GetFPS() { return m_fFPS; };

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
	float m_fLastUpdateTime;
	int m_iFramesRenderedSinceLastCheck;
	float m_fFPS;


	//
	// Render Queue stuff
	//
protected:
	RAGEVERTEX	m_vertQueue[MAX_NUM_VERTICIES];
	D3DCOLOR	m_addColors[MAX_NUM_VERTICIES];
	int			m_iNumVerts;

public:
	void AddTriangle(
		const D3DXVECTOR3& p0, const D3DCOLOR& c0, const D3DXVECTOR2& t0, const D3DCOLOR& a0,
		const D3DXVECTOR3& p1, const D3DCOLOR& c1, const D3DXVECTOR2& t1, const D3DCOLOR& a1,
		const D3DXVECTOR3& p2, const D3DCOLOR& c2, const D3DXVECTOR2& t2, const D3DCOLOR& a2 );
	void AddQuad(
		const D3DXVECTOR3 &p0, const D3DCOLOR& c0, const D3DXVECTOR2& t0, const D3DCOLOR& a0,	// upper-left
		const D3DXVECTOR3 &p1, const D3DCOLOR& c1, const D3DXVECTOR2& t1, const D3DCOLOR& a1, 	// upper-right
		const D3DXVECTOR3 &p2, const D3DCOLOR& c2, const D3DXVECTOR2& t2, const D3DCOLOR& a2, 	// lower-left
		const D3DXVECTOR3 &p3, const D3DCOLOR& c3, const D3DXVECTOR2& t3, const D3DCOLOR& a3  );// lower-right
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
