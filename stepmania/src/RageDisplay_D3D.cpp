#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: RageDisplay

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
    Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "RageDisplay.h"
#include "RageDisplay_D3D.h"
#include "D3D8.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageTimer.h"
#include "RageException.h"
#include "RageTexture.h"
#include "RageTextureManager.h"
#include "RageMath.h"
#include "RageTypes.h"
#include "GameConstantsAndTypes.h"
#include "StepMania.h"
#include "RageUtil.h"
#include "D3dx8math.h"
#include "SDL_video.h"	// for SDL_Surface
#include "SDL_utils.h"
#include "Dxerr8.h"

#include "arch/arch.h"

// Static libraries
#ifdef _XBOX
#pragma comment(lib, "D3d8.lib")
#endif
// load Windows D3D8 dynamically
#pragma comment(lib, "D3dx8.lib")
#pragma comment(lib, "Dxerr8.lib")

#include <math.h>
#include <list>


//
// Globals
//
#if !defined(_XBOX)
HMODULE					g_D3D8_Module = NULL;
#endif
LPDIRECT3D8				g_pd3d = NULL;
LPDIRECT3DDEVICE8		g_pd3dDevice = NULL;
D3DCAPS8				g_DeviceCaps;
D3DDISPLAYMODE			g_DesktopMode;
D3DPRESENT_PARAMETERS	g_d3dpp;
int						g_ModelMatrixCnt=0;
bool g_Windowed;
int g_CurrentHeight, g_CurrentWidth, g_CurrentBPP;

/* Direct3D doesn't associate a palette with textures.
 * Instead, we load a palette into a slot.  We need to keep track
 * of which texture's palette is stored in what slot. */
map<unsigned,int>	g_TexResourceToPaletteIndex;
list<int>			g_PaletteIndex;
struct TexturePalette { PALETTEENTRY p[256]; };
map<unsigned,TexturePalette>	g_TexResourceToTexturePalette;

/* Load the palette, if any, for the given texture into a palette slot, and make
 * it current. */
static void SetPalette( unsigned TexResource )
{
	/* If the texture isn't paletted, we have nothing to do. */
	if( g_TexResourceToTexturePalette.find(TexResource) == g_TexResourceToTexturePalette.end() )
		return;

	/* Is the palette already loaded? */
	if( g_TexResourceToPaletteIndex.find(TexResource) == g_TexResourceToPaletteIndex.end() )
	{
		/* It's not.  Grab the least recently used slot. */
		int iPalIndex = g_PaletteIndex.front();

		/* If any other texture is currently using this slot, mark that palette unloaded. */
		for( map<unsigned,int>::iterator i = g_TexResourceToPaletteIndex.begin(); i != g_TexResourceToPaletteIndex.end(); ++i )
		{
			if( i->second != iPalIndex )
				continue;
			g_TexResourceToPaletteIndex.erase(i);
			break;
		}

		/* Load it. */
#ifndef _XBOX
		g_pd3dDevice->SetPaletteEntries( iPalIndex, g_TexResourceToTexturePalette[TexResource].p );
#else
		ASSERT(0);
#endif

		g_TexResourceToPaletteIndex[TexResource] = iPalIndex;
	}
	
	const int iPalIndex = g_TexResourceToPaletteIndex[TexResource];

	/* Find this palette index in the least-recently-used queue and move it to the end. */
	for(list<int>::iterator i = g_PaletteIndex.begin(); i != g_PaletteIndex.end(); ++i)
	{
		if( *i != iPalIndex )
			continue;
		g_PaletteIndex.erase(i);
		g_PaletteIndex.push_back(iPalIndex);
		break;
	}

#ifndef _XBOX
	g_pd3dDevice->SetCurrentTexturePalette( iPalIndex );
#else
	ASSERT(0);
#endif
}

#define D3DFVF_RAGEVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1)	// D3D FVF flags which describe our vertex structure


static const PixelFormatDesc PIXEL_FORMAT_DESC[NUM_PIX_FORMATS] = {
	{
		/* A8B8G8R8 */
		32,
		{ 0x00FF0000,
		  0x0000FF00,
		  0x000000FF,
		  0xFF000000 }
	}, {
		/* A4R4G4B4 */
		16,
		{ 0x0F00,
		  0x00F0,
		  0x000F,
		  0xF000 },
	}, {
		/* A1B5G5R5 */
		16,
		{ 0x7C00,
		  0x03E0,
		  0x001F,
		  0x8000 },
	}, {
		/* X1R5G5R5 */
		16,
		{ 0x7C00,
		  0x03E0,
		  0x001F,
		  0x0000 },
	}, {
		/* B8G8R8 */
		24,
		{ 0xFF0000,
		  0x00FF00,
		  0x0000FF,
		  0x000000 }
	}, {
		/* Paletted */
		8,
		{ 0,0,0,0 } /* N/A */
	}
};

static D3DFORMAT D3DFORMATS[NUM_PIX_FORMATS] = 
{
	D3DFMT_A8R8G8B8,
	D3DFMT_A4R4G4B4,
	D3DFMT_A1R5G5B5,
	D3DFMT_X1R5G5B5,
#ifndef _XBOX
	D3DFMT_R8G8B8,
#else
	D3DFMT_A8R8G8B8,
#endif
	D3DFMT_P8
};

const PixelFormatDesc *RageDisplay_D3D::GetPixelFormatDesc(PixelFormat pf) const
{
	ASSERT( pf < NUM_PIX_FORMATS );
	return &PIXEL_FORMAT_DESC[pf];
}



RageDisplay_D3D::RageDisplay_D3D( bool windowed, int width, int height, int bpp, int rate, bool vsync, CString sWindowTitle, CString sIconFile )
{
	LOG->Trace( "RageDisplay_D3D::RageDisplay_D3D()" );
	LOG->MapLog("renderer", "Current renderer: Direct3D");

	typedef IDirect3D8 * (WINAPI * Direct3DCreate8_t) (UINT SDKVersion);
	Direct3DCreate8_t pDirect3DCreate8;
#if defined(_XBOX)
	pDirect3DCreate8 = Direct3DCreate8;
#else
	g_D3D8_Module = LoadLibrary("D3D8.dll");
	if(!g_D3D8_Module)
		throw RageException_D3DNotInstalled();

	pDirect3DCreate8 = (Direct3DCreate8_t) GetProcAddress(g_D3D8_Module, "Direct3DCreate8");
#endif

	g_pd3d = pDirect3DCreate8( D3D_SDK_VERSION );

	if( FAILED( g_pd3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &g_DeviceCaps) ) )
	{
	    g_pd3d->Release();

		throw RageException_D3DNoAcceleration();
	}

	D3DADAPTER_IDENTIFIER8	identifier;
	g_pd3d->GetAdapterIdentifier( D3DADAPTER_DEFAULT, 0, &identifier );

	LOG->Trace( 
		"Driver: %s\n"
		"Description: %s\n"
		"Max texture size: %d\n",
		identifier.Driver, 
		identifier.Description,
		g_DeviceCaps.MaxTextureWidth
		);

	LOG->Trace( "This display adaptor supports the following modes:" );
	D3DDISPLAYMODE mode;
	for( UINT u=0; u<g_pd3d->GetAdapterModeCount(D3DADAPTER_DEFAULT); u++ )
		if( SUCCEEDED( g_pd3d->EnumAdapterModes( D3DADAPTER_DEFAULT, u, &mode ) ) )
			LOG->Trace( "  %ux%u %uHz, format %d", mode.Width, mode.Height, mode.RefreshRate, mode.Format );

	/* Up until now, all we've done is set up g_pd3d and do some queries.  Now,
	 * actually initialize the window.  Do this after as many error conditions
	 * as possible, because if we have to shut it down again we'll flash a window
	 * briefly. */
    if(!SDL_WasInit(SDL_INIT_VIDEO))
		SDL_InitSubSystem(SDL_INIT_VIDEO);

	/* By default, ignore all SDL events.  We'll enable them as we need them.
	 * We must not enable any events we don't actually want, since we won't
	 * query for them and they'll fill up the event queue. 
	 *
	 * This needs to be done after we initialize video, since it's really part
	 * of the SDL video system--it'll be reinitialized on us if we do this first. */
	SDL_EventState(0xFF /*SDL_ALLEVENTS*/, SDL_IGNORE);
	SDL_EventState(SDL_VIDEORESIZE, SDL_ENABLE);

	g_Windowed = false;

	g_PaletteIndex.clear();
	for( int i = 0; i < 256; ++i )
		g_PaletteIndex.push_back(i);

	// Save the original desktop format.
	g_pd3d->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &g_DesktopMode );


	// Create the SDL window
	int flags = SDL_RESIZABLE | SDL_SWSURFACE;
	SDL_Surface *screen = SDL_SetVideoMode(width, height, bpp, flags);
	if(!screen)
	{
		SDL_QuitSubSystem(SDL_INIT_VIDEO);	// exit out of full screen.  The ~RageDisplay will not be called!
		RageException::Throw("SDL_SetVideoMode failed: %s", SDL_GetError());
	}


	if( SetVideoMode( windowed, width, height, bpp, rate, vsync, sWindowTitle, sIconFile ) )
		return;
	if( SetVideoMode( false, width, height, bpp, rate, vsync, sWindowTitle, sIconFile ) )
		return;
	if( SetVideoMode( false, width, height, 16, rate, vsync, sWindowTitle, sIconFile ) )
		return;
}

void RageDisplay_D3D::Update(float fDeltaTime)
{
	SDL_Event event;
	while(SDL_GetEvent(event, SDL_VIDEORESIZEMASK))
	{
		switch(event.type)
		{
		case SDL_VIDEORESIZE:
			g_CurrentWidth = event.resize.w;
			g_CurrentHeight = event.resize.h;

			/* Let DISPLAY know that our resolution has changed. */
			ResolutionChanged();
			break;
		}
	}
}

bool RageDisplay_D3D::IsSoftwareRenderer()
{
	return false;
}

RageDisplay_D3D::~RageDisplay_D3D()
{
	LOG->Trace( "RageDisplay_D3D::~RageDisplay()" );

	SDL_EventState(SDL_VIDEORESIZE, SDL_IGNORE);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);

	g_pd3dDevice->Release();
    g_pd3d->Release();
}

D3DFORMAT FindBackBufferType(bool bWindowed, int iBPP)
{
	HRESULT hr;

	// If windowed, then bpp is ignored.  Use whatever works.
    vector<D3DFORMAT> vBackBufferFormats;		// throw all possibilities in here
	
	/* When windowed, add all formats; otherwise add only formats that match dwBPP. */
	if( iBPP == 16 || bWindowed )
	{
		vBackBufferFormats.push_back( D3DFMT_R5G6B5 );
		vBackBufferFormats.push_back( D3DFMT_X1R5G5B5 );
		vBackBufferFormats.push_back( D3DFMT_A1R5G5B5 );
	}
	if( iBPP == 32 || bWindowed )
	{
#ifndef _XBOX
		vBackBufferFormats.push_back( D3DFMT_R8G8B8 );
#endif
		vBackBufferFormats.push_back( D3DFMT_X8R8G8B8 );
		vBackBufferFormats.push_back( D3DFMT_A8R8G8B8 );
	}
	if( !bWindowed && iBPP != 16 && iBPP != 32 )
	{
		SDL_QuitSubSystem(SDL_INIT_VIDEO);	// exit out of full screen.  The ~RageDisplay will not be called!
		throw RageException( ssprintf("Invalid BPP '%u' specified", iBPP) );
	}

	// Test each back buffer format until we find something that works.
	for( unsigned i=0; i < vBackBufferFormats.size(); i++ )
	{
		D3DFORMAT fmtBackBuffer = vBackBufferFormats[i];

		D3DFORMAT fmtDisplay;
		if( bWindowed )
			fmtDisplay = g_DesktopMode.Format;
		else	// Fullscreen
			fmtDisplay = vBackBufferFormats[i];

		LOG->Trace( "Testing format: display %d, back buffer %d, windowed %d...",
					fmtDisplay, fmtBackBuffer, bWindowed );

		hr = g_pd3d->CheckDeviceType( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, 
			fmtDisplay, fmtBackBuffer, bWindowed );

		if( FAILED(hr) )
			continue;	// skip

		// done searching
		LOG->Trace( "This will work." );
		return fmtBackBuffer;
	}

	LOG->Trace( "Couldn't find an appropriate back buffer format." );
	return (D3DFORMAT)-1;
}

#ifndef _XBOX
HWND GetHwnd()
{
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if( SDL_GetWMInfo(&info) < 0 ) 
	{
		SDL_QuitSubSystem(SDL_INIT_VIDEO);	// exit out of full screen.  The ~RageDisplay will not be called!
		RageException::Throw( "SDL_GetWMInfo failed" );
	}
	return info.window;
}
#endif


/* Set the video mode. */
bool RageDisplay_D3D::SetVideoMode( bool windowed, int width, int height, int bpp, int rate, bool vsync, CString sWindowTitle, CString sIconFile )
{
	HRESULT hr;

	if( FindBackBufferType( windowed, bpp ) == -1 )	// no possible back buffer formats
		return false;

	/* Set SDL window title and icon -before- creating the window */
	SDL_WM_SetCaption(sWindowTitle, "");
	mySDL_WM_SetIcon( sIconFile );


	/* Round to the nearest valid fullscreen resolution */
	if( !windowed )
	{
		if(      width <= 320 )		width = 320;
		else if( width <= 400 )		width = 400;
		else if( width <= 512 )		width = 512;
		else if( width <= 640 )		width = 640;
		else if( width <= 800 )		width = 800;
		else if( width <= 1024 )	width = 1024;
		else if( width <= 1280 )	width = 1280;

		switch( width )
		{
		case 320:	height = 240;	break;
		case 400:	height = 300;	break;
		case 512:	height = 384;	break;
		case 640:	height = 480;	break;
		case 800:	height = 600;	break;
		case 1024:	height = 768;	break;
		case 1280:	height = 1024;	break;
		default:	ASSERT(0);
		}
	}

	
	// HACK: On Windows 98, we can't call SDL_SetVideoMode while D3D is full screen.
	// It will result in "SDL_SetVideoMode  failed: DirectDraw2::CreateSurface(PRIMARY): 
	// Not in exclusive access mode".  So, we'll Reset the D3D device, then resize the 
	// SDL window only if we're not fullscreen.

	g_Windowed = windowed;
	g_CurrentWidth = width;
	g_CurrentHeight = height;
	g_CurrentBPP = bpp;

	SDL_ShowCursor( windowed );

    ZeroMemory( &g_d3dpp, sizeof(g_d3dpp) );
	g_d3dpp.BackBufferWidth			=	width;
    g_d3dpp.BackBufferHeight		=	height;
    g_d3dpp.BackBufferFormat		=	FindBackBufferType( windowed, bpp );
    g_d3dpp.BackBufferCount			=	1;
    g_d3dpp.MultiSampleType			=	D3DMULTISAMPLE_NONE;
	g_d3dpp.SwapEffect				=	D3DSWAPEFFECT_DISCARD;
	g_d3dpp.hDeviceWindow			=	NULL;
    g_d3dpp.Windowed				=	windowed;
    g_d3dpp.EnableAutoDepthStencil	=	TRUE;
    g_d3dpp.AutoDepthStencilFormat	=	D3DFMT_D16;
    g_d3dpp.Flags					=	0;
	g_d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	/* Windowed must always use D3DPRESENT_INTERVAL_DEFAULT. */
	g_d3dpp.FullScreen_PresentationInterval = 
		(windowed || vsync) ? D3DPRESENT_INTERVAL_DEFAULT : D3DPRESENT_INTERVAL_IMMEDIATE;

	LOG->Trace( "Present Parameters: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d", 
		g_d3dpp.BackBufferWidth, g_d3dpp.BackBufferHeight, g_d3dpp.BackBufferFormat,
		g_d3dpp.BackBufferCount,
		g_d3dpp.MultiSampleType, g_d3dpp.SwapEffect, g_d3dpp.hDeviceWindow,
		g_d3dpp.Windowed, g_d3dpp.EnableAutoDepthStencil, g_d3dpp.AutoDepthStencilFormat,
		g_d3dpp.Flags, g_d3dpp.FullScreen_RefreshRateInHz,
		g_d3dpp.FullScreen_PresentationInterval
	);

	bool bCreateNewDevice = g_pd3dDevice == NULL;

	if( bCreateNewDevice )		// device is not yet created.  We need to create it
	{
		hr = g_pd3d->CreateDevice(
			D3DADAPTER_DEFAULT, 
			D3DDEVTYPE_HAL, 
#ifndef _XBOX
			GetHwnd(),
			D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
#else
			NULL,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING,
#endif
			&g_d3dpp, 
			&g_pd3dDevice );
		if( FAILED(hr) )
		{
			SDL_QuitSubSystem(SDL_INIT_VIDEO);	// exit out of full screen.  The ~RageDisplay will not be called!
			RageException::Throw( "CreateDevice failed: '%s'", DXGetErrorString8(hr) );
		}
	}
	else
	{
		hr = g_pd3dDevice->Reset( &g_d3dpp );
		if( FAILED(hr) )
		{
			SDL_QuitSubSystem(SDL_INIT_VIDEO);	// exit out of full screen.  The ~RageDisplay will not be called!
			RageException::Throw( "g_pd3dDevice->Reset failed: '%s'", DXGetErrorString8(hr) );
		}
	}
	
	if( this->IsWindowed() )
	{
		int flags = SDL_RESIZABLE | SDL_SWSURFACE;
		
		// Don't use SDL to change the video mode.  This will cause a 
		// D3DERR_DRIVERINTERNALERROR on TNTs, V3s, and probably more.
	//	if( !windowed )
	//		flags |= SDL_FULLSCREEN;

		SDL_ShowCursor( g_Windowed );

		SDL_Surface *screen = SDL_SetVideoMode(width, height, bpp, flags);
		if(!screen)
		{
			SDL_QuitSubSystem(SDL_INIT_VIDEO);	// exit out of full screen.  The ~RageDisplay will not be called!
			RageException::Throw("SDL_SetVideoMode failed: %s", SDL_GetError());
		}
	}

	ResolutionChanged();

	this->SetDefaultRenderStates();
	
	/* Palettes were lost by Reset(), so mark them unloaded. */
	g_TexResourceToPaletteIndex.clear();

	return bCreateNewDevice;
}

void RageDisplay_D3D::ResolutionChanged()
{
	// no need to clear because D3D uses an overlay
//	SetViewport(0,0);
//
//	/* Clear any junk that's in the framebuffer. */
//	Clear();
//	Flip();
}

void RageDisplay_D3D::SetViewport(int shift_left, int shift_down)
{
	/* left and down are on a 0..SCREEN_WIDTH, 0..SCREEN_HEIGHT scale.
	 * Scale them to the actual viewport range. */
	shift_left = int( shift_left * float(g_CurrentWidth) / SCREEN_WIDTH );
	shift_down = int( shift_down * float(g_CurrentHeight) / SCREEN_HEIGHT );

	D3DVIEWPORT8 viewData = { shift_left, -shift_down, g_CurrentWidth, g_CurrentHeight, 0.f, 1.f };
	g_pd3dDevice->SetViewport( &viewData );
}

int RageDisplay_D3D::GetMaxTextureSize() const
{
	return g_DeviceCaps.MaxTextureWidth;
}

void RageDisplay_D3D::BeginFrame()
{
	if( g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET )
		SetVideoMode( g_Windowed, g_CurrentWidth, g_CurrentHeight, g_CurrentBPP, 0, 0, "", "" );	// FIXME: preserve prefs

	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
						 D3DCOLOR_XRGB(0,0,0), 1.0f, 0x00000000 );
	g_pd3dDevice->BeginScene();
}

void RageDisplay_D3D::EndFrame()
{
	g_pd3dDevice->EndScene();
	g_pd3dDevice->Present( 0, 0, 0, 0 );
	ProcessStatsOnFlip();
}

bool RageDisplay_D3D::SupportsTextureFormat( PixelFormat pixfmt )
{
	D3DFORMAT d3dfmt = D3DFORMATS[pixfmt];
	HRESULT hr = g_pd3d->CheckDeviceFormat( 
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		g_d3dpp.BackBufferFormat,
		0,
		D3DRTYPE_TEXTURE,
		d3dfmt);
    return SUCCEEDED( hr );
}

void RageDisplay_D3D::SaveScreenshot( CString sPath )
{
#ifndef _XBOX
	IDirect3DSurface8* pSurface;
	g_pd3dDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pSurface );
	D3DXSaveSurfaceToFile( sPath, D3DXIFF_BMP, pSurface, 0, NULL );
	pSurface->Release();
#endif
}


bool RageDisplay_D3D::IsWindowed() const { return g_Windowed; }
int RageDisplay_D3D::GetWidth() const { return g_CurrentWidth; }
int RageDisplay_D3D::GetHeight() const { return g_CurrentHeight; }
int RageDisplay_D3D::GetBPP() const { return g_CurrentBPP; }

#define SEND_CURRENT_MATRICES \
	g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)GetProjection() ); \
	RageMatrix m; \
	/* Convert to OpenGL-style "pixel-centered" coords */ \
	RageMatrixTranslation( &m, -0.5f, -0.5f, 0 ); \
	RageMatrixMultiply( &m, &m, GetModelViewTop() ); \
	g_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&m );


void RageDisplay_D3D::DrawQuads( const RageVertex v[], int iNumVerts )
{
	ASSERT( (iNumVerts%4) == 0 );

	if(iNumVerts == 0)
		return;

	// there isn't a quad primitive in D3D, so we have to fake it with indexed triangles
	int iNumQuads = iNumVerts/4;
	int iNumTriangles = iNumQuads*2;
	int iNumIndices = iNumTriangles*3;

	// make a temporary index buffer
	static vector<Uint16> vIndices;
	unsigned uOldSize = vIndices.size();
	unsigned uNewSize = max(uOldSize,(unsigned)iNumIndices);
	vIndices.resize( uNewSize );
	for( Uint16 i=(Uint16)uOldSize/6; i<(Uint16)iNumQuads; i++ )
	{
		vIndices[i*6+0] = i*4+0;
		vIndices[i*6+1] = i*4+1;
		vIndices[i*6+2] = i*4+2;
		vIndices[i*6+3] = i*4+2;
		vIndices[i*6+4] = i*4+3;
		vIndices[i*6+5] = i*4+0;
	}

	g_pd3dDevice->SetVertexShader( D3DFVF_RAGEVERTEX );
	SEND_CURRENT_MATRICES;
	g_pd3dDevice->DrawIndexedPrimitiveUP(
		D3DPT_TRIANGLELIST, // PrimitiveType
		0, // MinIndex
		iNumVerts, // NumVertices
		iNumTriangles, // PrimitiveCount,
		&vIndices[0], // pIndexData,
		D3DFMT_INDEX16, // IndexDataFormat,
		v, // pVertexStreamZeroData,
		sizeof(RageVertex) // VertexStreamZeroStride
	);

	StatsAddVerts( iNumVerts );
}
void RageDisplay_D3D::DrawFan( const RageVertex v[], int iNumVerts )
{
	ASSERT( iNumVerts >= 3 );
	g_pd3dDevice->SetVertexShader( D3DFVF_RAGEVERTEX );
	SEND_CURRENT_MATRICES;
	g_pd3dDevice->DrawPrimitiveUP(
		D3DPT_TRIANGLEFAN, // PrimitiveType
		iNumVerts-2, // PrimitiveCount,
		v, // pVertexStreamZeroData,
		sizeof(RageVertex)
	);
	StatsAddVerts( iNumVerts );
}

void RageDisplay_D3D::DrawStrip( const RageVertex v[], int iNumVerts )
{
	ASSERT( iNumVerts >= 3 );
	g_pd3dDevice->SetVertexShader( D3DFVF_RAGEVERTEX );
	SEND_CURRENT_MATRICES;
	g_pd3dDevice->DrawPrimitiveUP(
		D3DPT_TRIANGLESTRIP, // PrimitiveType
		iNumVerts-2, // PrimitiveCount,
		v, // pVertexStreamZeroData,
		sizeof(RageVertex)
	);
	StatsAddVerts( iNumVerts );
}

void RageDisplay_D3D::DrawTriangles( const RageVertex v[], int iNumVerts )
{
	if( iNumVerts == 0 )
		return;
	ASSERT( iNumVerts >= 3 );
	g_pd3dDevice->SetVertexShader( D3DFVF_RAGEVERTEX );
	SEND_CURRENT_MATRICES;
	g_pd3dDevice->DrawPrimitiveUP(
		D3DPT_TRIANGLELIST, // PrimitiveType
		iNumVerts/3, // PrimitiveCount,
		v, // pVertexStreamZeroData,
		sizeof(RageVertex)
	);
	StatsAddVerts( iNumVerts );
}

void RageDisplay_D3D::DrawIndexedTriangles( const RageVertex v[], const Uint16 pIndices[], int iNumIndices )
{
	if( iNumIndices == 0 )
		return;
	g_pd3dDevice->SetVertexShader( D3DFVF_RAGEVERTEX );
	SEND_CURRENT_MATRICES;
	g_pd3dDevice->DrawIndexedPrimitiveUP(
		D3DPT_TRIANGLELIST, // PrimitiveType
		0, // MinIndex
		iNumIndices, // NumVertices
		iNumIndices/3, // PrimitiveCount,
		pIndices, // pIndexData,
		D3DFMT_INDEX16, // IndexDataFormat,
		v, // pVertexStreamZeroData,
		sizeof(RageVertex) // VertexStreamZeroStride
	);
	StatsAddVerts( iNumIndices );
}

/* Use the default poly-based implementation.  D3D lines apparently don't support
 * AA with greater-than-one widths. */
/*
void RageDisplay_D3D::DrawLineStrip( const RageVertex v[], int iNumVerts, float LineWidth )
{
	ASSERT( iNumVerts >= 2 );
	g_pd3dDevice->SetRenderState( D3DRS_POINTSIZE, *((DWORD*)&LineWidth) );	// funky cast.  See D3DRENDERSTATETYPE doc
	g_pd3dDevice->SetVertexShader( D3DFVF_RAGEVERTEX );
	SEND_CURRENT_MATRICES;
	g_pd3dDevice->DrawPrimitiveUP(
		D3DPT_LINESTRIP, // PrimitiveType
		iNumVerts-1, // PrimitiveCount,
		v, // pVertexStreamZeroData,
		sizeof(RageVertex)
	);
	StatsAddVerts( iNumVerts );
}
*/
void RageDisplay_D3D::SetTexture( RageTexture* pTexture )
{
	if( pTexture == NULL )
	{
		g_pd3dDevice->SetTexture( 0, NULL );
		return;
	}

	unsigned uTexHandle = pTexture->GetTexHandle();
	IDirect3DTexture8* pTex = (IDirect3DTexture8*)uTexHandle;
	g_pd3dDevice->SetTexture( 0, pTex );
	
	// Set palette (if any)
	SetPalette(uTexHandle);
}
void RageDisplay_D3D::SetTextureModeModulate()
{
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
}

void RageDisplay_D3D::SetTextureModeGlow(GlowMode m)
{
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );

}

void RageDisplay_D3D::SetTextureFiltering( bool b )
{
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, b ? D3DTEXF_LINEAR : D3DTEXF_POINT );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, b ? D3DTEXF_LINEAR : D3DTEXF_POINT );
}

void RageDisplay_D3D::SetBlendMode( BlendMode mode )
{
	g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	switch( mode )
	{
	case BLEND_NORMAL:
		g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
		g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		break;
	case BLEND_ADD:
		g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ONE );
		g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		break;
	case BLEND_NO_EFFECT:
		g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ZERO );
		g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		break;
	default:
		ASSERT(0);
	}
}

bool RageDisplay_D3D::IsZBufferEnabled() const
{
	DWORD b;
	g_pd3dDevice->GetRenderState( D3DRS_ZENABLE, &b );
	return b!=0;
}

void RageDisplay_D3D::SetZBuffer( bool b )
{
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE,      b ? D3DZB_TRUE : D3DZB_FALSE );
	g_pd3dDevice->SetRenderState( D3DRS_ZFUNC,		  D3DCMP_LESSEQUAL );
	g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, b );
}
void RageDisplay_D3D::ClearZBuffer()
{
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,0), 1.0f, 0x00000000 );
}

void RageDisplay_D3D::SetTextureWrapping( bool b )
{
	int mode = b ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP;
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU, mode );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV, mode );
}

void RageDisplay_D3D::SetMaterial( 
	float emissive[4],
	float ambient[4],
	float diffuse[4],
	float specular[4],
	float shininess
	)
{
	D3DMATERIAL8 mat;
	memcpy( &mat.Diffuse, diffuse, sizeof(diffuse) );
	memcpy( &mat.Ambient, ambient, sizeof(ambient) );
	memcpy( &mat.Specular, specular, sizeof(specular) );
	memcpy( &mat.Emissive, emissive, sizeof(emissive) );
	mat.Power = shininess;
	g_pd3dDevice->SetMaterial( &mat );
}

void RageDisplay_D3D::SetLighting( bool b )
{
	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, b );
}

void RageDisplay_D3D::SetLightOff( int index )
{
	g_pd3dDevice->LightEnable( index, false );
}
void RageDisplay_D3D::SetLightDirectional( 
	int index, 
	RageColor ambient, 
	RageColor diffuse, 
	RageColor specular, 
	RageVector3 dir )
{
	g_pd3dDevice->LightEnable( index, true );

	D3DLIGHT8 light;
	ZERO( light );
	memcpy( &light.Direction, &dir, sizeof(dir) );
	memcpy( &light.Direction, &dir, sizeof(dir) );
	memcpy( &light.Diffuse, diffuse, sizeof(diffuse) );
	memcpy( &light.Ambient, ambient, sizeof(ambient) );
	memcpy( &light.Specular, specular, sizeof(specular) );

	g_pd3dDevice->SetLight( index, &light );
}

void RageDisplay_D3D::SetBackfaceCull( bool b )
{
	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, b ? D3DCULL_CW : D3DCULL_NONE );
}

void RageDisplay_D3D::DeleteTexture( unsigned uTexHandle )
{
	IDirect3DTexture8* pTex = (IDirect3DTexture8*) uTexHandle;
	pTex->Release();

	// Delete palette (if any)
	if( g_TexResourceToPaletteIndex.find(uTexHandle) != g_TexResourceToPaletteIndex.end() )
		g_TexResourceToPaletteIndex.erase( g_TexResourceToPaletteIndex.find(uTexHandle) );
	if( g_TexResourceToTexturePalette.find(uTexHandle) != g_TexResourceToTexturePalette.end() )
		g_TexResourceToTexturePalette.erase( g_TexResourceToTexturePalette.find(uTexHandle) );
}


unsigned RageDisplay_D3D::CreateTexture( 
	PixelFormat pixfmt,
	SDL_Surface*& img )
{
	// texture must be power of two
	ASSERT( img->w == power_of_two(img->w) );
	ASSERT( img->h == power_of_two(img->h) );


	HRESULT hr;
	IDirect3DTexture8* pTex;
	hr = g_pd3dDevice->CreateTexture( img->w, img->h, 1, 0, D3DFORMATS[pixfmt], D3DPOOL_MANAGED, &pTex );
	if( FAILED(hr) )
		RageException::Throw( "CreateTexture(%i,%i,pixfmt=%i) failed: %s", 
		img->w, img->h, pixfmt, DXGetErrorString8(hr) );

	unsigned uTexHandle = (unsigned)pTex;

	if( pixfmt == FMT_PAL )
	{
		// Save palette
		TexturePalette pal;
		memset( pal.p, 0, sizeof(pal.p) );
		for( int i=0; i<img->format->palette->ncolors; i++ )
		{
			SDL_Color c = img->format->palette->colors[i];
			pal.p[i].peRed = c.r;
			pal.p[i].peGreen = c.g;
			pal.p[i].peBlue = c.b;
			bool bIsColorKey = img->flags & SDL_SRCCOLORKEY && (unsigned)i == img->format->colorkey;
			pal.p[i].peFlags = bIsColorKey ? 0x00 : 0xFF;
		}

		ASSERT( g_TexResourceToTexturePalette.find(uTexHandle) == g_TexResourceToTexturePalette.end() );
		g_TexResourceToTexturePalette[uTexHandle] = pal;
	}

	UpdateTexture( uTexHandle, pixfmt, img, 0, 0, img->w, img->h );

	return uTexHandle;
}

void RageDisplay_D3D::UpdateTexture( 
	unsigned uTexHandle, 
	PixelFormat pixfmt,
	SDL_Surface*& img,
	int xoffset, int yoffset, int width, int height )
{
	IDirect3DTexture8* pTex = (IDirect3DTexture8*)uTexHandle;
	ASSERT( pTex != NULL );
	
	RECT rect; 
	rect.left = xoffset;
	rect.top = yoffset;
	rect.right = width - xoffset;
	rect.bottom = height - yoffset;

	D3DLOCKED_RECT lr;

	
	
	pTex->LockRect( 0, &lr, &rect, 0 );
	
	// copy each row
	int bytes_per_pixel = img->format->BytesPerPixel;
	for( int y=rect.top; y<rect.bottom; y++ )
	{
		char* src = (char*)img->pixels + y*img->pitch + rect.left*bytes_per_pixel;
		char* dst = (char*)lr.pBits    + y*lr.Pitch   + rect.left*bytes_per_pixel;
		memcpy( dst, src, (rect.right-rect.left)*bytes_per_pixel );
	}
	pTex->UnlockRect( 0 );

//	SDL_SaveBMP( (SDL_Surface*)img, "testing-SDL.bmp" );

//	D3DXSaveTextureToFile(
//		"testing-D3D.bmp",
//		D3DXIFF_BMP,
//		pTex,
//		NULL );
}

void RageDisplay_D3D::SetAlphaTest( bool b )
{
	g_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  b );
	g_pd3dDevice->SetRenderState( D3DRS_ALPHAREF,         0 );
	g_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );
}

RageMatrix RageDisplay_D3D::GetOrthoMatrix( float l, float r, float b, float t, float zn, float zf )
{
	// D3DXMatrixOrthoOffCenterRH
	RageMatrix m(
		2/(r-l),      0,            0,           0,
		0,            2/(t-b),      0,           0,
		0,            0,            -1/(zf-zn),  0,
		-(r+l)/(r-l), -(t+b)/(t-b), -zn/(zf-zn),  1 );
	return m;
}
