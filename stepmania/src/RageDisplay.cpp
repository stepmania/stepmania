#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: RageDisplay

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageDisplay.h"

#include "RageUtil.h"
#include "RageLog.h"
#include "RageTimer.h"
#include "RageException.h"
#include "RageTexture.h"


RageDisplay*		DISPLAY	= NULL;


RageDisplay::RageDisplay( HWND hWnd )
{
	LOG->Trace( "RageDisplay::RageDisplay()" );

	// Save the window handle
	m_hWnd = hWnd;
	m_pd3d = NULL;
	m_pd3dDevice = NULL;
	m_pVB = NULL;

	m_fLastCheckTime = TIMER->GetTimeSinceStart();
	m_iFramesRenderedSinceLastCheck = 0;
	m_iTrianglesRenderedSinceLastCheck = 0;
	m_iDrawsSinceLastCheck = 0;
	m_iFPS = 0;
	m_iTPF = 0;
	m_iDPF = 0;

	m_iNumVerts = 0;

	try
	{
		// Construct a Direct3D object
		m_pd3d = Direct3DCreate8( D3D_SDK_VERSION );
	}
	catch (...) 
	{
		// Edwin Evans: Catch any exception. It won't be caught by main exception handler.
		throw RageException( "Unknown exception in Direct3DCreate8." );
	}

    if( NULL == m_pd3d )
		throw RageException( "Direct3DCreate8 failed." );

	HRESULT  hr;
	if( FAILED( hr = m_pd3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &m_DeviceCaps) ) )
	{
		throw RageException( 
			"There was an error while initializing your video card.\n\n"
			"Your system is reporting that Direct3D8 hardware acceleration\n"
			"is not available.  In most cases, you can download an updated\n"
			"driver from your card's manufacturer."
		);
	}

	LOG->Trace( 
		"Video card info:\n"
		" - max texture width is %d\n"
		" - max texture height is %d\n"
		" - max texture blend stages is %d\n"
		" - max simultaneous textures is %d\n",
		m_DeviceCaps.MaxTextureWidth,
		m_DeviceCaps.MaxTextureHeight,
		m_DeviceCaps.MaxTextureBlendStages,
		m_DeviceCaps.MaxSimultaneousTextures
		);



	// Enumerate possible display modes
	LOG->Trace( "This display adaptor supports the following modes:" );
	for( UINT u=0; u<m_pd3d->GetAdapterModeCount(D3DADAPTER_DEFAULT); u++ )
	{
		D3DDISPLAYMODE mode;
		if( SUCCEEDED( m_pd3d->EnumAdapterModes( D3DADAPTER_DEFAULT, u, &mode ) ) )
		{
			LOG->Trace( "  %ux%u %uHz, format %d", mode.Width, mode.Height, mode.RefreshRate, mode.Format );
		}
	}

	// Save the original desktop format.
	m_pd3d->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &m_DesktopMode );

	D3DADAPTER_IDENTIFIER8	identifier;
	if( FAILED( hr = m_pd3d->GetAdapterIdentifier( D3DADAPTER_DEFAULT, 0, &identifier ) ) )
		LOG->Trace( hr, "GetAdapterIdentifier failed" );
	else
		LOG->Trace( "Driver: %s.  Description: %s.", 
			identifier.Driver, identifier.Description );
}

unsigned RageDisplay::MaxRefresh(unsigned uWidth, unsigned uHeight, D3DFORMAT fmt) const
{
	UINT mx = D3DPRESENT_RATE_DEFAULT;
	for( UINT u=0; u<m_pd3d->GetAdapterModeCount(D3DADAPTER_DEFAULT); u++ )
	{
		D3DDISPLAYMODE mode;
		if( !SUCCEEDED( m_pd3d->EnumAdapterModes( D3DADAPTER_DEFAULT, u, &mode ) ) )
			continue;

		if(mode.Width != uWidth) continue;
		if(mode.Height != uHeight) continue;
		if(mode.Format != fmt) continue;

		if(mx == D3DPRESENT_RATE_DEFAULT || mode.RefreshRate > mx)
			mx = mode.RefreshRate;
	}
	return mx;
}

/* Get the maximum refresh rate available in the given size, for any format
 * that fits bpp. */
void RageDisplay::GetHzAtResolution(unsigned width, unsigned height, unsigned bpp, CArray<int,int> &add) const
{
	for( UINT u=0; u<m_pd3d->GetAdapterModeCount(D3DADAPTER_DEFAULT); u++ )
	{
		D3DDISPLAYMODE mode;
		if( !SUCCEEDED( m_pd3d->EnumAdapterModes( D3DADAPTER_DEFAULT, u, &mode ) ) )
			continue;

		if(mode.Width != width) continue;
		if(mode.Height != height) continue;
		if(GetBPP(mode.Format) != bpp) continue;

		add.Add(mode.RefreshRate);
	}
}

RageDisplay::~RageDisplay()
{
	ReleaseVertexBuffer();
	// Release our D3D Device
	SAFE_RELEASE( m_pd3dDevice );
    m_pd3d->Release();
}


HRESULT RageDisplay::SetMode()
{
	HRESULT hr;
	if( m_pd3dDevice == NULL )
	{
		// device is not yet created.  We need to create it
		if( FAILED( hr = m_pd3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, 
											m_hWnd,
											D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
											&m_d3dpp, &m_pd3dDevice) ) )
		{
			return hr;
		}
		LOG->Trace( 
			"Video card info:\n"
			" - available texture mem is %u\n",
			m_pd3dDevice->GetAvailableTextureMem()
		);
		if( m_pVB == NULL )
			CreateVertexBuffer();
	}
	else
	{
		// device is already created.  Just reset it.
		if( FAILED( hr = m_pd3dDevice->Reset( &m_d3dpp ) ) )
			return hr;
	}

	LOG->Trace( "Mode change was successful." );
	return S_OK;
}

D3DFORMAT RageDisplay::FindBackBufferType(bool bWindowed, int iBPP)
{
	HRESULT hr;

	// If windowed, then dwBPP is ignored.  Use whatever works.
    CArray<D3DFORMAT,D3DFORMAT> arrayBackBufferFormats;		// throw all possibilities in here
	
	/* When windowed, add all formats; otherwise add only formats that match dwBPP. */
	if( iBPP == 16 || bWindowed )
	{
		arrayBackBufferFormats.Add( D3DFMT_R5G6B5 );
		arrayBackBufferFormats.Add( D3DFMT_X1R5G5B5 );
		arrayBackBufferFormats.Add( D3DFMT_A1R5G5B5 );
	}
	if( iBPP == 32 || bWindowed )
	{
		arrayBackBufferFormats.Add( D3DFMT_R8G8B8 );
		arrayBackBufferFormats.Add( D3DFMT_X8R8G8B8 );
		arrayBackBufferFormats.Add( D3DFMT_A8R8G8B8 );
	}
	if( !bWindowed && iBPP != 16 && iBPP != 32 )
		throw RageException( ssprintf("Invalid BPP '%u' specified", iBPP) );

	// Test each back buffer format until we find something that works.
	for( int i=0; i < arrayBackBufferFormats.GetSize(); i++ )
	{
		D3DFORMAT fmtBackBuffer = arrayBackBufferFormats[i];

		D3DFORMAT fmtDisplay;
		if( bWindowed )
			fmtDisplay = m_DesktopMode.Format;
		else	// Fullscreen
			fmtDisplay = arrayBackBufferFormats[i];

		LOG->Trace( "Testing format: display %d, back buffer %d, windowed %d...",
					fmtDisplay, fmtBackBuffer, bWindowed );

		hr = m_pd3d->CheckDeviceType( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, 
			fmtDisplay, fmtBackBuffer, bWindowed );

		if( FAILED(hr) ) {
			LOG->Trace( hr, "That won't work.  ");
			continue;
		}

		// done searching
		LOG->Trace( "This will work." );
		return fmtBackBuffer;
	}

	// we didn't find an appropriate format
	return D3DFMT_UNKNOWN;
}


//-----------------------------------------------------------------------------
// Name: SwitchDisplayMode()
// Desc:
//-----------------------------------------------------------------------------
bool RageDisplay::SwitchDisplayMode( 
	bool bWindowed, int iWidth, int iHeight, int iBPP, int iFullScreenHz, bool bVsync )
{
	LOG->Trace( "RageDisplay::SwitchDisplayModes( %d, %d, %d, %d, %d, %d )", bWindowed, iWidth, iHeight, iBPP, iFullScreenHz, bVsync );

	if( !bWindowed )
		SetCursor( NULL );


	HRESULT hr;

    // Find a pixel format for the back buffer.
	D3DFORMAT fmtBackBuffer=FindBackBufferType( bWindowed, iBPP );
	if( fmtBackBuffer == D3DFMT_UNKNOWN )
	{
		// we didn't find an appropriate format
		LOG->Trace( "failed to find an appropriate format for %d, %u, %u, %u.", bWindowed, iWidth, iHeight, iBPP );
		return false;
	}

    // Set up presentation parameters for the display
    ZeroMemory( &m_d3dpp, sizeof(m_d3dpp) );
    
	m_d3dpp.BackBufferWidth			=	iWidth;
    m_d3dpp.BackBufferHeight		=	iHeight;
    m_d3dpp.BackBufferFormat		=	fmtBackBuffer;
    m_d3dpp.BackBufferCount			=	1;
    m_d3dpp.MultiSampleType			=	D3DMULTISAMPLE_NONE;
	m_d3dpp.SwapEffect				=	D3DSWAPEFFECT_DISCARD;
	m_d3dpp.hDeviceWindow			=	m_hWnd;
    m_d3dpp.Windowed				=	bWindowed;
    m_d3dpp.EnableAutoDepthStencil	=	TRUE;
    m_d3dpp.AutoDepthStencilFormat	=	D3DFMT_D16;
    m_d3dpp.Flags					=	0;
	m_d3dpp.FullScreen_RefreshRateInHz = bWindowed? D3DPRESENT_RATE_DEFAULT :
									iFullScreenHz == REFRESH_MAX? MaxRefresh(iWidth, iHeight, fmtBackBuffer):
									iFullScreenHz == REFRESH_DEFAULT? D3DPRESENT_RATE_DEFAULT:
										iFullScreenHz;

	/* Windowed must always use D3DPRESENT_INTERVAL_DEFAULT. */
	m_d3dpp.FullScreen_PresentationInterval = 
		bWindowed || bVsync? D3DPRESENT_INTERVAL_DEFAULT:
		D3DPRESENT_INTERVAL_IMMEDIATE;

	LOG->Trace( "Present Parameters: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d", 
		m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight, m_d3dpp.BackBufferFormat,
		m_d3dpp.BackBufferCount,
		m_d3dpp.MultiSampleType, m_d3dpp.SwapEffect, m_d3dpp.hDeviceWindow,
		m_d3dpp.Windowed, m_d3dpp.EnableAutoDepthStencil, m_d3dpp.AutoDepthStencilFormat,
		m_d3dpp.Flags, m_d3dpp.FullScreen_RefreshRateInHz,
		m_d3dpp.FullScreen_PresentationInterval
	);


	if( FAILED( hr=SetMode() ) )
	{
		LOG->Trace( hr, "failed to set device: %d, %u, %u, %u.", bWindowed, iWidth, iHeight, iBPP );
		return false;
	}

	// Clear the back buffer and present it so we don't show the gibberish that was
	// in video memory from the last app.
	BeginFrame();
	EndFrame();
	ShowFrame();

	return true;
}


//-----------------------------------------------------------------------------
// Name: Reset()
// Desc:
//-----------------------------------------------------------------------------
HRESULT RageDisplay::Reset()
{
	return m_pd3dDevice->Reset( &m_d3dpp );
}


//-----------------------------------------------------------------------------
// Name: BeginFrame()
// Desc:
//-----------------------------------------------------------------------------
HRESULT RageDisplay::BeginFrame()
{
	//////////////////////////////////////////////////////////////
	// Do some fancy testing to make sure the D3D deivce is ready.
	// This is mainly used when the the app is reactivated
	// after the user has Alt-tabed out of full screen mode.
	//////////////////////////////////////////////////////////////

	// Test cooperative level
    HRESULT hr;

    // Test the cooperative level to see if it's okay to render
    if( FAILED( hr = m_pd3dDevice->TestCooperativeLevel() ) )
    {
        // If the device was lost, do not render until we get it back
        if( D3DERR_DEVICELOST == hr )
            return hr;			// not ready to render

        // Check if the device needs to be resized.
        if( D3DERR_DEVICENOTRESET == hr )
	        return hr;

		return hr;
    }


	m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
						 D3DCOLOR_XRGB(0,0,0), 1.0f, 0x00000000 );
	
	
	if ( FAILED( hr  = m_pd3dDevice->BeginScene() ) )
		return E_FAIL;

	// disable culling so backward polys can be drawn
	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

	// Enable Alpha Blending and Testing
	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	//m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE );
	//m_pd3dDevice->SetRenderState( D3DRS_ALPHAREF,         0x08 );
	//m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
            
	m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
	m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,      FALSE );
//	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,      TRUE );
//	m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );


    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );


	// Don't tile texture coords.  This creates ugly wrapping artifacts on textures that have to be rescaled.
	DisableTextureWrapping();
    
	// m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_BORDER   );
    // m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_BORDER   );
    // m_pd3dDevice->SetTextureStageState( 0, D3DTSS_BORDERCOLOR, D3DCOLOR_ARGB(0,0,0,0) );


	m_pd3dDevice->SetVertexShader( D3DFVF_RAGEVERTEX );
	m_pd3dDevice->SetStreamSource( 0, m_pVB, sizeof(RAGEVERTEX) );



	return S_OK;
}


HRESULT RageDisplay::EndFrame()
{
	FlushQueue();


	m_pd3dDevice->EndScene();


	// update stats
	m_iFramesRenderedSinceLastCheck++;

	float fTimeNow = TIMER->GetTimeSinceStart();
	if( fTimeNow - m_fLastCheckTime > 1.0f )	// update stats every 1 sec.
	{
		m_iFPS = m_iFramesRenderedSinceLastCheck;
		m_iFramesRenderedSinceLastCheck = 0;
		m_iTPF = m_iTrianglesRenderedSinceLastCheck / m_iFPS;
		m_iTrianglesRenderedSinceLastCheck = 0;
		m_iDPF = m_iDrawsSinceLastCheck / m_iFPS;
		m_iDrawsSinceLastCheck = 0;
		m_fLastCheckTime = fTimeNow;

		//LOG->Trace( "FPS: %d, TPF: %d, DPF: %d", m_iFPS, m_iTPF, m_iDPF );
	}


	return S_OK;
}

HRESULT RageDisplay::ShowFrame()
{
	if( m_pd3dDevice )
		m_pd3dDevice->Present( 0, 0, 0, 0 );

	return S_OK;
}


HRESULT RageDisplay::Invalidate()
{
	return S_OK;
}


HRESULT RageDisplay::Restore()
{
	return S_OK;
}

unsigned RageDisplay::GetBPP(D3DFORMAT fmt) const
{ 
	switch( fmt )
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


void RageDisplay::CreateVertexBuffer()
{
	HRESULT hr;
	if( FAILED( hr = m_pd3dDevice->CreateVertexBuffer( 
									MAX_NUM_VERTICIES * sizeof(RAGEVERTEX),
									D3DUSAGE_WRITEONLY, D3DFVF_RAGEVERTEX,
									D3DPOOL_MANAGED, &m_pVB ) ) )
		throw RageException( hr, "Vertex Buffer Could Not Be Created" );
}


void RageDisplay::ReleaseVertexBuffer()
{
	SAFE_RELEASE( m_pVB );
}

/*
void RageDisplay::AddTriangle( const RAGEVERTEX& v[3] )
{
	COPY( &m_vertQueue[m_iNumVerts], v );		// do a big mem copy
	for( int i=0; i<3; i++ )
		D3DXVec3TransformCoord( &m_vertQueue[m_iNumVerts+i].p, v[i].p, &GetTopMatrix() ); 
	m_iNumVerts+=3; 
}
*/

void RageDisplay::AddQuad( const RAGEVERTEX v[4] )	// upper-left, upper-right, lower-left, lower-right
{
	AddQuad( 
		v[0].p, v[0].color, v[0].t,
		v[1].p, v[1].color, v[1].t, 
		v[2].p, v[2].color, v[2].t, 
		v[3].p, v[3].color, v[3].t ); 
}
void RageDisplay::AddFan( const RAGEVERTEX v[], int iNumPrimitives )
{
	// HACK: This function does not take winding order into account.  It will goof if you turn on culling.
	for( int i=0; i<iNumPrimitives; i++ )
	{
		AddTriangle( 
			v[0+0].p, v[0+0].color, v[0+0].t,
			v[i+1].p, v[i+1].color, v[i+1].t,
			v[i+2].p, v[i+2].color, v[i+2].t
			);
	}
}
void RageDisplay::AddStrip( const RAGEVERTEX v[], int iNumPrimitives )
{
	// HACK: This function does not take winding order into account.  It will goof if you turn on culling.
	for( int i=0; i<iNumPrimitives; i++ )
	{
		AddTriangle( 
			v[i+0].p, v[i+0].color, v[i+0].t,
			v[i+1].p, v[i+1].color, v[i+1].t,
			v[i+2].p, v[i+2].color, v[i+2].t
			);
	}
}
void RageDisplay::AddTriangle(
	const D3DXVECTOR3& p0, const D3DCOLOR& c0, const D3DXVECTOR2& t0,
	const D3DXVECTOR3& p1, const D3DCOLOR& c1, const D3DXVECTOR2& t1,
	const D3DXVECTOR3& p2, const D3DCOLOR& c2, const D3DXVECTOR2& t2 )
{
	ASSERT( m_iNumVerts < MAX_NUM_VERTICIES-3 );

	// transform the verticies as we copy
	D3DXVec3TransformCoord( &m_vertQueue[m_iNumVerts].p, &p0, &GetTopMatrix() ); 
	m_vertQueue[m_iNumVerts].color = c0;
	m_vertQueue[m_iNumVerts].t = t0;
	m_iNumVerts++; 
	D3DXVec3TransformCoord( &m_vertQueue[m_iNumVerts].p, &p1, &GetTopMatrix() ); 
	m_vertQueue[m_iNumVerts].color = c1;
	m_vertQueue[m_iNumVerts].t = t1;
	m_iNumVerts++; 
	D3DXVec3TransformCoord( &m_vertQueue[m_iNumVerts].p, &p2, &GetTopMatrix() ); 
	m_vertQueue[m_iNumVerts].color = c2;
	m_vertQueue[m_iNumVerts].t = t2;
	m_iNumVerts++; 

	if( m_iNumVerts > MAX_NUM_VERTICIES-4 )
		FlushQueue();

	m_iTrianglesRenderedSinceLastCheck++;
}

void RageDisplay::AddQuad(
	const D3DXVECTOR3 &p0, const D3DCOLOR& c0, const D3DXVECTOR2& t0,	// upper-left
	const D3DXVECTOR3 &p1, const D3DCOLOR& c1, const D3DXVECTOR2& t1, 	// upper-right
	const D3DXVECTOR3 &p2, const D3DCOLOR& c2, const D3DXVECTOR2& t2, 	// lower-left
	const D3DXVECTOR3 &p3, const D3DCOLOR& c3, const D3DXVECTOR2& t3 )	// lower-right
{
	// trangles must be in clockwise order in case we ever turn on clipping
	AddTriangle( 
		p0, c0, t0,		// upper-left
		p2, c2, t2,		// lower-left
		p3, c3, t3 );	// lower-right
	AddTriangle(  
		p0, c0, t0,		// upper-left
		p3, c3, t3,		// lower-right
		p1, c1, t1 );	// upper-right
}

void RageDisplay::FlushQueue()
{
	if( m_iNumVerts == 0 )
		return;
	ASSERT( (m_iNumVerts % 3) == 0 );

	m_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, m_iNumVerts/3, m_vertQueue, sizeof(RAGEVERTEX) );
	m_iNumVerts = 0;

/*
	RAGEVERTEX* v;
	m_pVB->Lock( 0, 0, (BYTE**)&v, 0 );
	memcpy( v, m_vertQueue, sizeof(RAGEVERTEX)*m_iNumVerts );
	m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, m_iNumVerts/3 );
	m_pVB->Unlock();
	m_iNumVerts = 0;
*/

	m_iDrawsSinceLastCheck++;
}

void RageDisplay::SetViewTransform( const D3DXMATRIX* pMatrix )
{
	FlushQueue();
	m_pd3dDevice->SetTransform( D3DTS_VIEW, pMatrix );
}
void RageDisplay::SetProjectionTransform( const D3DXMATRIX* pMatrix )
{
	FlushQueue();
	m_pd3dDevice->SetTransform( D3DTS_PROJECTION, pMatrix );
}
void RageDisplay::GetViewTransform( D3DXMATRIX* pMatrixOut )
{
	m_pd3dDevice->GetTransform( D3DTS_VIEW, pMatrixOut );
}
void RageDisplay::GetProjectionTransform( D3DXMATRIX* pMatrixOut )
{
	m_pd3dDevice->GetTransform( D3DTS_PROJECTION, pMatrixOut );
}


void RageDisplay::ResetMatrixStack() 
{ 
	m_MatrixStack.SetSize( 1, 20 );
	D3DXMatrixIdentity( &GetTopMatrix() );
}

void RageDisplay::PushMatrix() 
{ 
	m_MatrixStack.Add( GetTopMatrix() );	
	ASSERT(m_MatrixStack.GetSize()<20);		// check for infinite loop
}

void RageDisplay::PopMatrix() 
{ 
	m_MatrixStack.RemoveAt( m_MatrixStack.GetSize()-1 ); 
}

void RageDisplay::Translate( float x, float y, float z )
{
	D3DXMATRIX matTemp;
	D3DXMatrixTranslation( &matTemp, x, y, z );
	D3DXMATRIX& matTop = GetTopMatrix();
	matTop = matTemp * matTop;
}

void RageDisplay::TranslateLocal( float x, float y, float z )
{
	D3DXMATRIX matTemp;
	D3DXMatrixTranslation( &matTemp, x, y, z );
	D3DXMATRIX& matTop = GetTopMatrix();
	matTop = matTop * matTemp;
}

void RageDisplay::Scale( float x, float y, float z )
{
	D3DXMATRIX matTemp;
	D3DXMatrixScaling( &matTemp, x, y, z );
	D3DXMATRIX& matTop = GetTopMatrix();
	matTop = matTemp * matTop;
}

void RageDisplay::RotateX( float r )
{
	D3DXMATRIX matTemp;
	D3DXMatrixRotationX( &matTemp, r );
	D3DXMATRIX& matTop = GetTopMatrix();
	matTop = matTemp * matTop;
}

void RageDisplay::RotateY( float r )
{
	D3DXMATRIX matTemp;
	D3DXMatrixRotationY( &matTemp, r );
	D3DXMATRIX& matTop = GetTopMatrix();
	matTop = matTemp * matTop;
}

void RageDisplay::RotateZ( float r )
{
	D3DXMATRIX matTemp;
	D3DXMatrixRotationZ( &matTemp, r );
	D3DXMATRIX& matTop = GetTopMatrix();
	matTop = matTemp * matTop;
}

/*
void RageDisplay::RotateYawPitchRoll( const float x, const float y, const float z )
{
	D3DXMATRIX matTemp;
	D3DXMatrixRotationYawPitchRoll( &matTemp, x, y, z );
	D3DXMATRIX& matTop = GetTopMatrix();
	matTop = matTemp * matTop;
}
*/

void RageDisplay::SetTexture( RageTexture* pTexture )
{
	LPDIRECT3DBASETEXTURE8 pNewD3DTexture = pTexture ? pTexture->GetD3DTexture() : NULL;

	LPDIRECT3DBASETEXTURE8 pOldD3DTexture;
	m_pd3dDevice->GetTexture( 0, &pOldD3DTexture );

	if( pOldD3DTexture != pNewD3DTexture )
		FlushQueue();

	if(pOldD3DTexture)
		pOldD3DTexture->Release();
	
	m_pd3dDevice->SetTexture( 0, pNewD3DTexture );
}
void RageDisplay::SetColorTextureMultDiffuse()
{
	DWORD dw0, dw1, dw2;
	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_COLORARG1, &dw0 );
	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_COLORARG2, &dw1 );
	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_COLOROP,   &dw2 );

	if( dw0!=D3DTA_TEXTURE || dw1!=D3DTA_DIFFUSE || dw2!=D3DTOP_MODULATE )
		FlushQueue();

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
}
void RageDisplay::SetColorDiffuse()
{
	DWORD dw0, dw1, dw2;
	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_COLORARG1, &dw0 );
	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_COLORARG2, &dw1 );
	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_COLOROP,   &dw2 );

	if( dw0!=D3DTA_TEXTURE || dw1!=D3DTA_DIFFUSE || dw2!=D3DTOP_SELECTARG2 )
		FlushQueue();

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
}
void RageDisplay::SetAlphaTextureMultDiffuse()
{
	DWORD dw0, dw1, dw2;
	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_ALPHAARG1, &dw0 );
	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_ALPHAARG2, &dw1 );
	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_ALPHAOP,   &dw2 );

	if( dw0!=D3DTA_TEXTURE || dw1!=D3DTA_DIFFUSE || dw2!=D3DTOP_MODULATE )
		FlushQueue();

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
}
void RageDisplay::SetBlendModeNormal()
{
	DWORD dw0, dw1;
	m_pd3dDevice->GetRenderState( D3DRS_SRCBLEND, &dw0 );
	m_pd3dDevice->GetRenderState( D3DRS_DESTBLEND, &dw1 );

	if( dw0!=D3DBLEND_SRCALPHA || dw1!=D3DBLEND_INVSRCALPHA )
		FlushQueue();

	m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
	m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
}
void RageDisplay::SetBlendModeAdd()
{
	DWORD dw0, dw1;
	m_pd3dDevice->GetRenderState( D3DRS_SRCBLEND, &dw0 );
	m_pd3dDevice->GetRenderState( D3DRS_DESTBLEND, &dw1 );

	if( dw0!=D3DBLEND_ONE || dw1!=D3DBLEND_ONE )
		FlushQueue();

	m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ONE );
	m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
}
void RageDisplay::EnableZBuffer()
{
	DWORD dw0, dw1;
	m_pd3dDevice->GetRenderState( D3DRS_ZENABLE, &dw0 );
	m_pd3dDevice->GetRenderState( D3DRS_ZWRITEENABLE, &dw1 );

	if( dw0!=TRUE || dw1!=TRUE )
		FlushQueue();

	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,      TRUE );
	m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
}
void RageDisplay::DisableZBuffer()
{
	DWORD dw0, dw1;
	m_pd3dDevice->GetRenderState( D3DRS_ZENABLE, &dw0 );
	m_pd3dDevice->GetRenderState( D3DRS_ZWRITEENABLE, &dw1 );

	if( dw0!=FALSE || dw1!=FALSE )
		FlushQueue();

	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,      FALSE );
	m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
}
void RageDisplay::EnableTextureWrapping()
{
	DWORD dw0, dw1;
	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_ADDRESSU, &dw0 );
	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_ADDRESSV, &dw1 );

	if( dw0!=D3DTADDRESS_WRAP || dw1!=D3DTADDRESS_WRAP )
		FlushQueue();

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP );
}
void RageDisplay::DisableTextureWrapping()
{
	DWORD dw0, dw1;
	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_ADDRESSU, &dw0 );
	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_ADDRESSV, &dw1 );

	if( dw0!=D3DTADDRESS_CLAMP  || dw1!=D3DTADDRESS_CLAMP  )
		FlushQueue();

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP  );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP  );
}
