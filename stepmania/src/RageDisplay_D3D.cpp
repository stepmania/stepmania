#include "global.h"
#include "RageDisplay.h"
#include "RageDisplay_D3D.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageTimer.h"
#include "RageException.h"
#include "RageTexture.h"
#include "RageTextureManager.h"
#include "RageMath.h"
#include "RageTypes.h"
#include "StepMania.h"
#include "RageSurface.h"
#include "RageSurfaceUtils.h"
#include "EnumHelper.h"
#include "DisplayResolutions.h"

#include <D3D8.h>
#include <D3dx8math.h>
#include <D3DX8Core.h>

#if !defined(XBOX)
#include "archutils/Win32/GraphicsWindow.h"
#else
#include "archutils/Xbox/GraphicsWindow.h"
#include "archutils/Xbox/VirtualMemory.h"
#endif

// Static libraries
// load Windows D3D8 dynamically
#if defined(_MSC_VER) && !defined(_XBOX)
	#pragma comment(lib, "D3dx8.lib")
	#pragma comment(lib, "Dxerr8.lib")
#endif

#include <math.h>
#include <list>


CString GetErrorString( HRESULT hr )
{
	char szError[1024] = "";
	D3DXGetErrorString( hr, szError, sizeof(szError) );
	return szError;
}


//
// Globals
//
#if !defined(XBOX)
HMODULE					g_D3D8_Module = NULL;
#endif
LPDIRECT3D8				g_pd3d = NULL;
LPDIRECT3DDEVICE8		g_pd3dDevice = NULL;
D3DCAPS8				g_DeviceCaps;
D3DDISPLAYMODE			g_DesktopMode;
D3DPRESENT_PARAMETERS	g_d3dpp;
int						g_ModelMatrixCnt=0;
TextureUnit				g_currentTextureUnit = TextureUnit_1;
static int				g_iActualRefreshRateInHz = 60;
static bool				g_bSphereMapping[NUM_TextureUnit] = { false, false };

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
#if !defined(XBOX)
		TexturePalette& pal = g_TexResourceToTexturePalette[TexResource];
		g_pd3dDevice->SetPaletteEntries( iPalIndex, pal.p );
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

#if !defined(XBOX)
	g_pd3dDevice->SetCurrentTexturePalette( iPalIndex );
#else
	ASSERT(0);
#endif
}

#define D3DFVF_RageSpriteVertex (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#define D3DFVF_RageModelVertex (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)


static const RageDisplay::PixelFormatDesc PIXEL_FORMAT_DESC[NUM_PixelFormat] = {
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
	}, {
		/* BGR (N/A; OpenGL only) */
		0, { 0,0,0,0 }
	}, {
		/* ABGR (N/A; OpenGL only) */
		0, { 0,0,0,0 }
	}
};

static D3DFORMAT D3DFORMATS[NUM_PixelFormat] = 
{
	D3DFMT_A8R8G8B8,
	D3DFMT_A4R4G4B4,
	D3DFMT_A1R5G5B5,
	D3DFMT_X1R5G5B5,
#if defined(XBOX)
	D3DFMT_UNKNOWN,  /* no RGB */
#else
	D3DFMT_R8G8B8,
#endif
	D3DFMT_P8,
	D3DFMT_UNKNOWN, /* no BGR */
	D3DFMT_UNKNOWN /* no ABGR */
};

const RageDisplay::PixelFormatDesc *RageDisplay_D3D::GetPixelFormatDesc(PixelFormat pf) const
{
	ASSERT( pf < NUM_PixelFormat );
	return &PIXEL_FORMAT_DESC[pf];
}



RageDisplay_D3D::RageDisplay_D3D()
{

}

#define D3D_NOT_INSTALLED \
	"DirectX 8.1 or greater is not installed.  You can download it from:\n" \
	"http://www.microsoft.com/downloads/details.aspx?FamilyID=a19bed22-0b25-4e5d-a584-6389d8a3dad0&displaylang=en"

CString RageDisplay_D3D::Init( VideoModeParams p )
{
	GraphicsWindow::Initialize( true );

	LOG->Trace( "RageDisplay_D3D::RageDisplay_D3D()" );
	LOG->MapLog("renderer", "Current renderer: Direct3D");

	typedef IDirect3D8 * (WINAPI * Direct3DCreate8_t) (UINT SDKVersion);
	Direct3DCreate8_t pDirect3DCreate8;
#if defined(XBOX)
	pDirect3DCreate8 = Direct3DCreate8;
#else
	g_D3D8_Module = LoadLibrary("D3D8.dll");
	if(!g_D3D8_Module)
		return D3D_NOT_INSTALLED;

	pDirect3DCreate8 = (Direct3DCreate8_t) GetProcAddress(g_D3D8_Module, "Direct3DCreate8");
	if(!pDirect3DCreate8)
	{
		LOG->Trace( "Direct3DCreate8 not found" );
		return D3D_NOT_INSTALLED;
	}
#endif

	g_pd3d = pDirect3DCreate8( D3D_SDK_VERSION );
	if(!g_pd3d)
	{
		LOG->Trace( "Direct3DCreate8 failed" );
		return D3D_NOT_INSTALLED;
	}

	if( FAILED( g_pd3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &g_DeviceCaps) ) )
		return
			"Your system is reporting that Direct3D hardware acceleration is not available.  "
			"Please obtain an updated driver from your video card manufacturer.\n\n";

	D3DADAPTER_IDENTIFIER8	identifier;
	g_pd3d->GetAdapterIdentifier( D3DADAPTER_DEFAULT, 0, &identifier );

	LOG->Trace( 
		"Driver: %s\n"
		"Description: %s\n"
		"Max texture size: %d\n"
		"Alpha in palette: %s\n",
		identifier.Driver, 
		identifier.Description,
		g_DeviceCaps.MaxTextureWidth,
		(g_DeviceCaps.TextureCaps & D3DPTEXTURECAPS_ALPHAPALETTE) ? "yes" : "no" );

	LOG->Trace( "This display adaptor supports the following modes:" );
	D3DDISPLAYMODE mode;
	for( UINT u=0; u<g_pd3d->GetAdapterModeCount(D3DADAPTER_DEFAULT); u++ )
		if( SUCCEEDED( g_pd3d->EnumAdapterModes( D3DADAPTER_DEFAULT, u, &mode ) ) )
			LOG->Trace( "  %ux%u %uHz, format %d", mode.Width, mode.Height, mode.RefreshRate, mode.Format );

	g_PaletteIndex.clear();
	for( int i = 0; i < 256; ++i )
		g_PaletteIndex.push_back(i);

	// Save the original desktop format.
	g_pd3d->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &g_DesktopMode );

	/* Up until now, all we've done is set up g_pd3d and do some queries.  Now,
	 * actually initialize the window.  Do this after as many error conditions
	 * as possible, because if we have to shut it down again we'll flash a window
	 * briefly. */
	bool bIgnore = false;
	return SetVideoMode( p, bIgnore );
}

RageDisplay_D3D::~RageDisplay_D3D()
{
	LOG->Trace( "RageDisplay_D3D::~RageDisplay()" );

	if( g_pd3dDevice )
		g_pd3dDevice->Release();

	if( g_pd3d )
	    g_pd3d->Release();

#if !defined(XBOX)
	if( g_D3D8_Module )
	{
		FreeLibrary( g_D3D8_Module );
		g_D3D8_Module = NULL;
	}
#endif

	GraphicsWindow::Shutdown();
}

void RageDisplay_D3D::GetDisplayResolutions( DisplayResolutions &out ) const
{
	out.s.clear();
	int iCnt = g_pd3d->GetAdapterModeCount( D3DADAPTER_DEFAULT );

	for( int i = 0; i < iCnt; ++i )
	{
		D3DDISPLAYMODE mode;
		g_pd3d->EnumAdapterModes( D3DADAPTER_DEFAULT, i, &mode );

		DisplayResolution res = { mode.Width, mode.Height };
		out.s.insert( res );
	}
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
#if !defined(XBOX)
		vBackBufferFormats.push_back( D3DFMT_R8G8B8 );
#endif
		vBackBufferFormats.push_back( D3DFMT_X8R8G8B8 );
		vBackBufferFormats.push_back( D3DFMT_A8R8G8B8 );
	}
	if( !bWindowed && iBPP != 16 && iBPP != 32 )
	{
		GraphicsWindow::Shutdown();
		RageException::Throw( "Invalid BPP '%i' specified", iBPP );
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
	return D3DFMT_UNKNOWN;
}

CString SetD3DParams( bool &bNewDeviceOut )
{
	if( g_pd3dDevice == NULL )		// device is not yet created.  We need to create it
	{
		bNewDeviceOut = true;
		HRESULT hr = g_pd3d->CreateDevice(
			D3DADAPTER_DEFAULT, 
			D3DDEVTYPE_HAL, 
#if !defined(XBOX)
			GraphicsWindow::GetHwnd(),
			D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
#else
			NULL,
			D3DCREATE_HARDWARE_VERTEXPROCESSING,
#endif
			&g_d3dpp, 
			&g_pd3dDevice );
		if( FAILED(hr) )
		{
			// Likely D3D_ERR_INVALIDCALL.  The driver probably doesn't support this video mode.
			return ssprintf( "CreateDevice failed: '%s'", GetErrorString(hr).c_str() );
		}
	}
	else
	{
		bNewDeviceOut = false;
		//LOG->Warn( "Resetting D3D device" );
		HRESULT hr = g_pd3dDevice->Reset( &g_d3dpp );
		if( FAILED(hr) )
		{
			// Likely D3D_ERR_INVALIDCALL.  The driver probably doesn't support this video mode.
			return ssprintf("g_pd3dDevice->Reset failed: '%s'", GetErrorString(hr).c_str() );
		}
	}

	g_pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );	

	return CString();
}

/* If the given parameters have failed, try to lower them. */
static bool D3DReduceParams( D3DPRESENT_PARAMETERS *pp )
{
	D3DDISPLAYMODE current;
	current.Format = pp->BackBufferFormat;
	current.Height = pp->BackBufferHeight;
	current.Width = pp->BackBufferWidth;
	current.RefreshRate = pp->FullScreen_RefreshRateInHz;

	const int iCnt = g_pd3d->GetAdapterModeCount( D3DADAPTER_DEFAULT );
	int iBest = -1;
	int iBestScore = 0;
	LOG->Trace( "cur: %ux%u %uHz, format %i", current.Width, current.Height, current.RefreshRate, current.Format );
	for( int i = 0; i < iCnt; ++i )
	{
		D3DDISPLAYMODE mode;
		g_pd3d->EnumAdapterModes( D3DADAPTER_DEFAULT, i, &mode );

		/* Never change the format. */
		if( mode.Format != current.Format )
			continue;
		/* Never increase the parameters. */
		if( mode.Height > current.Height || mode.Width > current.Width || mode.RefreshRate > current.RefreshRate )
			continue;

		/* Never go below 640x480 unless we already are. */
		if( (current.Width >= 640 && current.Height >= 480) && (mode.Width < 640 || mode.Height < 480) )
			continue;

		/* Never go below 60Hz. */
		if( mode.RefreshRate && mode.RefreshRate < 60 )
			continue;

		/* If mode.RefreshRate is 0, it means "default".  We don't know what that means;
		 * assume it's 60Hz. */

		/* Higher scores are better. */
		int iScore = 0;
		if( current.RefreshRate >= 70 && mode.RefreshRate < 70 )
		{
			/* Top priority: we really want to avoid dropping to a refresh rate that's
			 * below 70Hz. */
			iScore -= 100000;
		}
		else if( mode.RefreshRate < current.RefreshRate )
		{
			/* Low priority: We're lowering the refresh rate, but not too far.  current.RefreshRate
			 * might be 0, in which case this simply gives points for higher refresh
			 * rates. */
			iScore += (mode.RefreshRate - current.RefreshRate);
		}

		/* Medium priority: */
		int iResolutionDiff = (current.Height - mode.Height) + (current.Width - mode.Width);
		iScore -= iResolutionDiff * 100;

		if( iBest == -1 || iScore > iBestScore )
		{
			iBest = i;
			iBestScore = iScore;
		}

		LOG->Trace( "try: %ux%u %uHz, format %i: score %i", mode.Width, mode.Height, mode.RefreshRate, mode.Format, iScore );
	}

	if( iBest == -1 )
		return false;

	D3DDISPLAYMODE BestMode;
	g_pd3d->EnumAdapterModes( D3DADAPTER_DEFAULT, iBest, &BestMode );
	pp->BackBufferHeight = BestMode.Height;
	pp->BackBufferWidth = BestMode.Width;
	pp->FullScreen_RefreshRateInHz = BestMode.RefreshRate;

	return true;
}


/* Set the video mode. */
CString RageDisplay_D3D::TryVideoMode( const VideoModeParams &_p, bool &bNewDeviceOut )
{
	VideoModeParams p = _p;

	//LOG->Warn( "RageDisplay_D3D::TryVideoMode( %d, %d, %d, %d, %d, %d )", p.windowed, p.width, p.height, p.bpp, p.rate, p.vsync );

	if( FindBackBufferType( p.windowed, p.bpp ) == D3DFMT_UNKNOWN )	// no possible back buffer formats
		return ssprintf( "FindBackBufferType(%i,%i) failed", p.windowed, p.bpp );	// failed to set mode

#if defined(XBOX)
	p.windowed = false;
#endif

	/* Set up and display the window before setting up D3D.  If we don't do this,
	 * then setting up a fullscreen window (when we're not coming from windowed)
	 * causes all other windows on the system to be resized to the new resolution. */
	GraphicsWindow::ConfigureGraphicsWindow( p );

	ZeroMemory( &g_d3dpp, sizeof(g_d3dpp) );

	g_d3dpp.BackBufferWidth		=	p.width;
	g_d3dpp.BackBufferHeight	=	p.height;
	g_d3dpp.BackBufferFormat	=	FindBackBufferType( p.windowed, p.bpp );
	g_d3dpp.BackBufferCount		=	1;
	g_d3dpp.MultiSampleType		=	D3DMULTISAMPLE_NONE;
	g_d3dpp.SwapEffect			=	D3DSWAPEFFECT_DISCARD;
#if !defined(XBOX)
	g_d3dpp.hDeviceWindow		=	GraphicsWindow::GetHwnd();
#else
	g_d3dpp.hDeviceWindow		=	NULL;
#endif
	g_d3dpp.Windowed				=	p.windowed;
	g_d3dpp.EnableAutoDepthStencil	=	TRUE;
	g_d3dpp.AutoDepthStencilFormat	=	D3DFMT_D16;

	if(p.windowed)
		g_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	else
		g_d3dpp.FullScreen_PresentationInterval = p.vsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;

#if !defined(XBOX)
	g_d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	if( !p.windowed && p.rate != REFRESH_DEFAULT )
		g_d3dpp.FullScreen_RefreshRateInHz = p.rate;
#else
	if( XGetVideoStandard() == XC_VIDEO_STANDARD_PAL_I )
	{
		/* Get supported video flags. */
		DWORD VideoFlags = XGetVideoFlags();
		
		/* Set pal60 if available. */
		if( VideoFlags & XC_VIDEO_FLAGS_PAL_60Hz )
			g_d3dpp.FullScreen_RefreshRateInHz = 60;
		else
			g_d3dpp.FullScreen_RefreshRateInHz = 50;
	}
	else
		g_d3dpp.FullScreen_RefreshRateInHz = 60;
#endif

	g_d3dpp.Flags					=	0;

	LOG->Trace( "Present Parameters: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d", 
		g_d3dpp.BackBufferWidth, g_d3dpp.BackBufferHeight, g_d3dpp.BackBufferFormat,
		g_d3dpp.BackBufferCount,
		g_d3dpp.MultiSampleType, g_d3dpp.SwapEffect, g_d3dpp.hDeviceWindow,
		g_d3dpp.Windowed, g_d3dpp.EnableAutoDepthStencil, g_d3dpp.AutoDepthStencilFormat,
		g_d3dpp.Flags, g_d3dpp.FullScreen_RefreshRateInHz,
		g_d3dpp.FullScreen_PresentationInterval
	);

#if defined(XBOX)
	if( D3D__pDevice )
		g_pd3dDevice = D3D__pDevice;
#endif

	/* Display the window immediately, so we don't display the desktop ... */

	while( 1 )
	{
		/* Try the video mode. */
		CString sErr = SetD3DParams( bNewDeviceOut );
		if( sErr.empty() )
			break;

		/* It failed.  We're probably selecting a video mode that isn't supported.
		 * If we're fullscreen, search the mode list and find the nearest lower
		 * mode. */
		if( p.windowed || !D3DReduceParams( &g_d3dpp ) )
			return sErr;

		/* Store the new settings we're about to try. */
		p.height = g_d3dpp.BackBufferHeight;
		p.width = g_d3dpp.BackBufferWidth;
		if( g_d3dpp.FullScreen_RefreshRateInHz == D3DPRESENT_RATE_DEFAULT )
			p.rate = REFRESH_DEFAULT;
		else
			p.rate = g_d3dpp.FullScreen_RefreshRateInHz;
	}
#ifndef _XBOX

	/* Find the refresh rate. */
	{
		DEVMODE dm;
		ZERO( dm );
		dm.dmSize = sizeof(dm);
		if( EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm) )
		{
			/*
			 * On a nForce 2 IGP on Windows 98, dm.dmDisplayFrequency sometimes 
			 * (but not always) is 0.
			 *
			 * MSDN: When you call the EnumDisplaySettings function, the 
			 * dmDisplayFrequency member may return with the value 0 or 1. 
			 * These values represent the display hardware's default refresh rate. 
			 * This default rate is typically set by switches on a display card or 
			 * computer motherboard, or by a configuration program that does not 
			 * use Win32 display functions such as ChangeDisplaySettings.
			 */
			if( dm.dmDisplayFrequency == 0 || dm.dmDisplayFrequency == 1 )
			{
				g_iActualRefreshRateInHz = 60;
				LOG->Warn( "EnumDisplaySettings doesn't know what the refresh rate is. %d %d %d", dm.dmPelsWidth, dm.dmPelsHeight, dm.dmBitsPerPel );
			}
			else
			{
				g_iActualRefreshRateInHz = dm.dmDisplayFrequency;
			}
		}
		else
		{
			g_iActualRefreshRateInHz = 60;
			LOG->Warn( "%s", werr_ssprintf(GetLastError(), "EnumDisplaySettings failed").c_str() );
		}
	}
#endif

	/* Call this again after changing the display mode.  If we're going to a window
	 * from fullscreen, the first call can't set a larger window than the old fullscreen
	 * resolution or set the window position. */
	GraphicsWindow::ConfigureGraphicsWindow( p );

	GraphicsWindow::SetVideoModeParams( p );

	ResolutionChanged();

	this->SetDefaultRenderStates();
	
	/* Palettes were lost by Reset(), so mark them unloaded. */
	g_TexResourceToPaletteIndex.clear();

	return CString();	// mode change successful
}

void RageDisplay_D3D::ResolutionChanged()
{
	//LOG->Warn( "RageDisplay_D3D::ResolutionChanged" );

#if defined(XBOX)
	D3DVIEWPORT8 viewData = { 0,0,640,480, 0.f, 1.f };
	g_pd3dDevice->SetViewport( &viewData );
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
						 D3DCOLOR_XRGB(0,0,0), 1.0f, 0x00000000 );
#endif

	RageDisplay::ResolutionChanged();
}

int RageDisplay_D3D::GetMaxTextureSize() const
{
	return g_DeviceCaps.MaxTextureWidth;
}

bool RageDisplay_D3D::BeginFrame()
{
	GraphicsWindow::Update();

#if !defined(XBOX)
	switch( g_pd3dDevice->TestCooperativeLevel() )
	{
	case D3DERR_DEVICELOST:
		return false;
	case D3DERR_DEVICENOTRESET:
		{
			bool bIgnore = false;
			VideoModeParams params;
			StepMania::GetPreferredVideoModeParams( params );

			CString sError = SetVideoMode( params, bIgnore );
			if( sError != "" )
				RageException::Throw( sError );
			break;
		}
	}
#endif

	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
						 D3DCOLOR_XRGB(0,0,0), 1.0f, 0x00000000 );
	g_pd3dDevice->BeginScene();
	return true;
}

static RageTimer g_LastFrameEndedAt( RageZeroTimer );
void RageDisplay_D3D::EndFrame()
{
	g_pd3dDevice->EndScene();

	FrameLimitBeforeVsync( g_iActualRefreshRateInHz );
	g_pd3dDevice->Present( 0, 0, 0, 0 );
	FrameLimitAfterVsync();

	RageDisplay::EndFrame();
}

bool RageDisplay_D3D::SupportsTextureFormat( PixelFormat pixfmt, bool realtime )
{
#if defined(XBOX)
	// Lazy...  Xbox handles paletted textures completely differently
	// than regular D3D.  It's not worth writing a bunch of code to handle it.
	// Paletted textures result in worse cache efficiency anyway (see "Xbox 
	// Palettized Texture Performance" in XDK).  
	// So, just force 32bit ARGB textures.  -Chris
	// This is also needed for XGSwizzleRect().
	return pixfmt == PixelFormat_RGBA8;
#endif

	// Some cards (Savage) don't support alpha in palettes.
	// Don't allow paletted textures if this is the case.
	if( pixfmt == PixelFormat_PAL  &&  !(g_DeviceCaps.TextureCaps & D3DPTEXTURECAPS_ALPHAPALETTE) )
		return false;

	if(	D3DFORMATS[pixfmt] == D3DFMT_UNKNOWN )
		return false;

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

bool RageDisplay_D3D::SupportsThreadedRendering()
{
	return true;
}

RageSurface* RageDisplay_D3D::CreateScreenshot()
{
#if defined(XBOX)
	return NULL;
#else
	/* Get the back buffer. */
	IDirect3DSurface8* pSurface;
	g_pd3dDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pSurface );

	/* Get the back buffer description. */
	D3DSURFACE_DESC desc;
	pSurface->GetDesc( &desc );

	/* Copy the back buffer into a surface of a type we support. */
	IDirect3DSurface8* pCopy;
	g_pd3dDevice->CreateImageSurface( desc.Width, desc.Height, D3DFMT_A8R8G8B8, &pCopy );

	D3DXLoadSurfaceFromSurface( pCopy, NULL, NULL, pSurface, NULL, NULL, D3DX_DEFAULT, 0 );

	pSurface->Release();

	/* Update desc from the copy. */
	pCopy->GetDesc( &desc );

	D3DLOCKED_RECT lr;

	{
		RECT rect; 
		rect.left = 0;
		rect.top = 0;
		rect.right = desc.Width;
		rect.bottom = desc.Height;
		pCopy->LockRect( &lr, &rect, D3DLOCK_READONLY );
	}

	RageSurface *surface = CreateSurfaceFromPixfmt( PixelFormat_RGBA8, lr.pBits, desc.Width, desc.Height, lr.Pitch);
	ASSERT( surface );

	/* We need to make a copy, since lr.pBits will go away when we call UnlockRect(). */
	RageSurface *SurfaceCopy = 
		CreateSurface( surface->w, surface->h,
			surface->format->BitsPerPixel,
			surface->format->Rmask, surface->format->Gmask,
			surface->format->Bmask, surface->format->Amask );
	RageSurfaceUtils::CopySurface( surface, SurfaceCopy );
	delete surface;

	pCopy->UnlockRect();
	pCopy->Release();

	return SurfaceCopy;
#endif
}

VideoModeParams RageDisplay_D3D::GetActualVideoModeParams() const 
{
	VideoModeParams p; 
	GraphicsWindow::GetParams( p ); 
	return p; 
}

void RageDisplay_D3D::SendCurrentMatrices()
{
	RageMatrix projection;
	RageMatrixMultiply( &projection, GetCentering(), GetProjectionTop() );
	g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&projection );

	g_pd3dDevice->SetTransform( D3DTS_VIEW, (D3DMATRIX*)GetViewTop() );

	/* Convert to OpenGL-style "pixel-centered" coords */
	RageMatrix m;
	RageMatrixTranslation( &m, -0.5f, -0.5f, 0 );
	RageMatrixMultiply( &m, &m, GetWorldTop() );
	g_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&m );
	

	FOREACH_ENUM2( TextureUnit, tu )
	{
		// Optimization opportunity: Turn off texture transform if not using texture coords.
		g_pd3dDevice->SetTextureStageState( tu, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
		
		// If no texture is set for this texture unit, don't bother setting it up.
		IDirect3DBaseTexture8* pTexture = NULL;
		g_pd3dDevice->GetTexture( g_currentTextureUnit, &pTexture );
		if( pTexture == NULL )
			 continue;
		pTexture->Release();


		if( g_bSphereMapping[tu] )
		{
			static const RageMatrix tex = RageMatrix
			(
				0.5f,   0.0f,  0.0f, 0.0f,
				0.0f,  -0.5f,  0.0f, 0.0f,
				0.0f,   0.0f,  0.0f, 0.0f,
				0.5f,  -0.5f,  0.0f, 1.0f
			);
			g_pd3dDevice->SetTransform( (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+tu), (D3DMATRIX*)&tex );

			// Tell D3D to use transformed reflection vectors as texture co-ordinate 0
			// and then transform this coordinate by the specified texture matrix.
			g_pd3dDevice->SetTextureStageState( tu, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR );    
		}
		else
		{
			/*
			 * Direct3D is expecting a 3x3 matrix loaded into the 4x4 in order to transform
			 * the 2-component texture coordinates.  We currently only use translate and scale,
			 * and ignore the z component entirely, so convert the texture matrix from
			 * 4x4 to 3x3 by dropping z.
			 */

			const RageMatrix &tex1 = *GetTextureTop();
			const RageMatrix tex2 = RageMatrix
			(
				tex1.m[0][0], tex1.m[0][1],  tex1.m[0][3],	0,
				tex1.m[1][0], tex1.m[1][1],  tex1.m[1][3],	0,
				tex1.m[3][0], tex1.m[3][1],  tex1.m[3][3],	0,
				0,				0,			0,		0
			);
			g_pd3dDevice->SetTransform( D3DTRANSFORMSTATETYPE(D3DTS_TEXTURE0+tu), (D3DMATRIX*)&tex2 );

			g_pd3dDevice->SetTextureStageState( tu, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU );
		}
	}
}

class RageCompiledGeometrySWD3D : public RageCompiledGeometry
{
public:
	void Allocate( const vector<msMesh> &vMeshes )
	{
		m_vVertex.resize( GetTotalVertices() );
		m_vTriangles.resize( GetTotalTriangles() );
	}
	void Change( const vector<msMesh> &vMeshes )
	{
		for( unsigned i=0; i<vMeshes.size(); i++ )
		{
			const MeshInfo& meshInfo = m_vMeshInfo[i];
			const msMesh& mesh = vMeshes[i];
			const vector<RageModelVertex> &Vertices = mesh.Vertices;
			const vector<msTriangle> &Triangles = mesh.Triangles;

			for( unsigned j=0; j<Vertices.size(); j++ )
				m_vVertex[meshInfo.iVertexStart+j] = Vertices[j];

			for( unsigned j=0; j<Triangles.size(); j++ )
				for( unsigned k=0; k<3; k++ )
					m_vTriangles[meshInfo.iTriangleStart+j].nVertexIndices[k] = (uint16_t) meshInfo.iVertexStart + Triangles[j].nVertexIndices[k];
		}
	}
	void Draw( int iMeshIndex ) const
	{
		const MeshInfo& meshInfo = m_vMeshInfo[iMeshIndex];

		if( meshInfo.m_bNeedsTextureMatrixScale )
		{
			// Kill the texture translation.
			// XXX: Change me to scale the translation by the TextureTranslationScale of the first vertex.
			RageMatrix m;
			g_pd3dDevice->GetTransform( D3DTS_TEXTURE0, (D3DMATRIX*)&m );

			m.m[2][0] = 0;
			m.m[2][1] = 0;

			g_pd3dDevice->SetTransform( D3DTS_TEXTURE0, (D3DMATRIX*)&m );
		}

		g_pd3dDevice->SetVertexShader( D3DFVF_RageModelVertex );
		g_pd3dDevice->DrawIndexedPrimitiveUP(
			D3DPT_TRIANGLELIST,			// PrimitiveType
			meshInfo.iVertexStart,		// MinIndex
			meshInfo.iVertexCount,		// NumVertices
			meshInfo.iTriangleCount,	// PrimitiveCount,
			&m_vTriangles[0]+meshInfo.iTriangleStart,// pIndexData,
			D3DFMT_INDEX16,				// IndexDataFormat,
			&m_vVertex[0],				// pVertexStreamZeroData,
			sizeof(m_vVertex[0])		// VertexStreamZeroStride
		);
	}

protected:
	vector<RageModelVertex> m_vVertex;
	vector<msTriangle>		m_vTriangles;
};

RageCompiledGeometry* RageDisplay_D3D::CreateCompiledGeometry()
{
	return new RageCompiledGeometrySWD3D;
}

void RageDisplay_D3D::DeleteCompiledGeometry( RageCompiledGeometry* p )
{
	delete p;
}

void RageDisplay_D3D::DrawQuadsInternal( const RageSpriteVertex v[], int iNumVerts )
{
	// there isn't a quad primitive in D3D, so we have to fake it with indexed triangles
	int iNumQuads = iNumVerts/4;
	int iNumTriangles = iNumQuads*2;
	int iNumIndices = iNumTriangles*3;

	// make a temporary index buffer
	static vector<uint16_t> vIndices;
	unsigned uOldSize = vIndices.size();
	unsigned uNewSize = max(uOldSize,(unsigned)iNumIndices);
	vIndices.resize( uNewSize );
	for( uint16_t i=(uint16_t)uOldSize/6; i<(uint16_t)iNumQuads; i++ )
	{
		vIndices[i*6+0] = i*4+0;
		vIndices[i*6+1] = i*4+1;
		vIndices[i*6+2] = i*4+2;
		vIndices[i*6+3] = i*4+2;
		vIndices[i*6+4] = i*4+3;
		vIndices[i*6+5] = i*4+0;
	}

	g_pd3dDevice->SetVertexShader( D3DFVF_RageSpriteVertex );
	SendCurrentMatrices();
	g_pd3dDevice->DrawIndexedPrimitiveUP(
		D3DPT_TRIANGLELIST, // PrimitiveType
		0, // MinIndex
		iNumVerts, // NumVertices
		iNumTriangles, // PrimitiveCount,
		&vIndices[0], // pIndexData,
		D3DFMT_INDEX16, // IndexDataFormat,
		v, // pVertexStreamZeroData,
		sizeof(RageSpriteVertex) // VertexStreamZeroStride
	);
}

void RageDisplay_D3D::DrawQuadStripInternal( const RageSpriteVertex v[], int iNumVerts )
{
	// there isn't a quad strip primitive in D3D, so we have to fake it with indexed triangles
	int iNumQuads = (iNumVerts-2)/2;
	int iNumTriangles = iNumQuads*2;
	int iNumIndices = iNumTriangles*3;

	// make a temporary index buffer
	static vector<uint16_t> vIndices;
	unsigned uOldSize = vIndices.size();
	unsigned uNewSize = max(uOldSize,(unsigned)iNumIndices);
	vIndices.resize( uNewSize );
	for( uint16_t i=(uint16_t)uOldSize/6; i<(uint16_t)iNumQuads; i++ )
	{
		vIndices[i*6+0] = i*2+0;
		vIndices[i*6+1] = i*2+1;
		vIndices[i*6+2] = i*2+2;
		vIndices[i*6+3] = i*2+1;
		vIndices[i*6+4] = i*2+2;
		vIndices[i*6+5] = i*2+3;
	}

	g_pd3dDevice->SetVertexShader( D3DFVF_RageSpriteVertex );
	SendCurrentMatrices();
	g_pd3dDevice->DrawIndexedPrimitiveUP(
		D3DPT_TRIANGLELIST, // PrimitiveType
		0, // MinIndex
		iNumVerts, // NumVertices
		iNumTriangles, // PrimitiveCount,
		&vIndices[0], // pIndexData,
		D3DFMT_INDEX16, // IndexDataFormat,
		v, // pVertexStreamZeroData,
		sizeof(RageSpriteVertex) // VertexStreamZeroStride
	);
}

void RageDisplay_D3D::DrawFanInternal( const RageSpriteVertex v[], int iNumVerts )
{
	g_pd3dDevice->SetVertexShader( D3DFVF_RageSpriteVertex );
	SendCurrentMatrices();
	g_pd3dDevice->DrawPrimitiveUP(
		D3DPT_TRIANGLEFAN, // PrimitiveType
		iNumVerts-2, // PrimitiveCount,
		v, // pVertexStreamZeroData,
		sizeof(RageSpriteVertex)
	);
}

void RageDisplay_D3D::DrawStripInternal( const RageSpriteVertex v[], int iNumVerts )
{
	g_pd3dDevice->SetVertexShader( D3DFVF_RageSpriteVertex );
	SendCurrentMatrices();
	g_pd3dDevice->DrawPrimitiveUP(
		D3DPT_TRIANGLESTRIP, // PrimitiveType
		iNumVerts-2, // PrimitiveCount,
		v, // pVertexStreamZeroData,
		sizeof(RageSpriteVertex)
	);
}

void RageDisplay_D3D::DrawTrianglesInternal( const RageSpriteVertex v[], int iNumVerts )
{
	g_pd3dDevice->SetVertexShader( D3DFVF_RageSpriteVertex );
	SendCurrentMatrices();
	g_pd3dDevice->DrawPrimitiveUP(
		D3DPT_TRIANGLELIST, // PrimitiveType
		iNumVerts/3, // PrimitiveCount,
		v, // pVertexStreamZeroData,
		sizeof(RageSpriteVertex)
	);
}

void RageDisplay_D3D::DrawCompiledGeometryInternal( const RageCompiledGeometry *p, int iMeshIndex )
{
	SendCurrentMatrices();

	/* If lighting is off, then the current material will have no effect.
	 * We want to still be able to color models with lighting off,
	 * so shove the material color in texture factor and modify the 
	 * texture stage to use it instead of the vertex color (our models
	 * don't have vertex coloring anyway). 
	 */
	DWORD bLighting;
	g_pd3dDevice->GetRenderState( D3DRS_LIGHTING, &bLighting );

	if( !bLighting )
	{
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR );
	}

	p->Draw( iMeshIndex );

	if( !bLighting )
	{
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	}
}

/* Use the default poly-based implementation.  D3D lines apparently don't support
 * AA with greater-than-one widths. */
/*
void RageDisplay_D3D::DrawLineStrip( const RageSpriteVertex v[], int iNumVerts, float LineWidth )
{
	ASSERT( iNumVerts >= 2 );
	g_pd3dDevice->SetRenderState( D3DRS_POINTSIZE, *((DWORD*)&LineWidth) );	// funky cast.  See D3DRENDERSTATETYPE doc
	g_pd3dDevice->SetVertexShader( D3DFVF_RageSpriteVertex );
	SendCurrentMatrices();
	g_pd3dDevice->DrawPrimitiveUP(
		D3DPT_LINESTRIP, // PrimitiveType
		iNumVerts-1, // PrimitiveCount,
		v, // pVertexStreamZeroData,
		sizeof(RageSpriteVertex)
	);
	StatsAddVerts( iNumVerts );
}
*/

void RageDisplay_D3D::ClearAllTextures()
{
	FOREACH_ENUM2( TextureUnit, i )
		SetTexture( i, NULL );
	g_currentTextureUnit = TextureUnit_1;
}

int RageDisplay_D3D::GetNumTextureUnits()
{
	return g_DeviceCaps.MaxSimultaneousTextures;
}

void RageDisplay_D3D::SetTexture( TextureUnit tu, RageTexture* pTexture )
{
	g_currentTextureUnit = tu;

//	g_DeviceCaps.MaxSimultaneousTextures = 1;
	if( g_currentTextureUnit >= (int) g_DeviceCaps.MaxSimultaneousTextures )	// not supported
		return;

	if( pTexture == NULL )
	{
		g_pd3dDevice->SetTexture( g_currentTextureUnit, NULL );
		g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	}
	else
	{
		unsigned uTexHandle = pTexture->GetTexHandle();
		IDirect3DTexture8* pTex = (IDirect3DTexture8*)uTexHandle;
		g_pd3dDevice->SetTexture( g_currentTextureUnit, pTex );
		
		g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_COLOROP,   D3DTOP_MODULATE );

		// Set palette (if any)
		SetPalette(uTexHandle);
	}
}
void RageDisplay_D3D::SetTextureModeModulate()
{
	if( g_currentTextureUnit >= (int) g_DeviceCaps.MaxSimultaneousTextures )	// not supported
		return;

	// Use D3DTA_CURRENT instead of diffuse so that multitexturing works 
	// properly.  For stage 0, D3DTA_CURRENT is the diffuse color.

	g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_COLORARG2, D3DTA_CURRENT );
	g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
}

void RageDisplay_D3D::SetTextureModeGlow()
{
	if( g_currentTextureUnit >= (int) g_DeviceCaps.MaxSimultaneousTextures )	// not supported
		return;

	g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_COLORARG2, D3DTA_CURRENT );
	g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
	g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
}

void RageDisplay_D3D::SetTextureModeAdd()
{
	if( g_currentTextureUnit >= (int) g_DeviceCaps.MaxSimultaneousTextures )	// not supported
		return;

	g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_COLORARG2, D3DTA_CURRENT );
	g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_COLOROP,   D3DTOP_ADD );
	g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
}

void RageDisplay_D3D::SetTextureFiltering( bool b )
{
	if( g_currentTextureUnit >= (int) g_DeviceCaps.MaxSimultaneousTextures )	// not supported
		return;

	g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_MINFILTER, b ? D3DTEXF_LINEAR : D3DTEXF_POINT );
	g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_MAGFILTER, b ? D3DTEXF_LINEAR : D3DTEXF_POINT );
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
		g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
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

bool RageDisplay_D3D::IsZWriteEnabled() const
{
	DWORD b;
	g_pd3dDevice->GetRenderState( D3DRS_ZWRITEENABLE, &b );
	return b!=0;
}

void RageDisplay_D3D::SetZBias( float f )
{
	D3DVIEWPORT8 viewData;
	g_pd3dDevice->GetViewport( &viewData );
	viewData.MinZ = SCALE( f, 0.0f, 1.0f, 0.05f, 0.0f );
	viewData.MaxZ = SCALE( f, 0.0f, 1.0f, 1.0f, 0.95f );
	g_pd3dDevice->SetViewport( &viewData );
}


bool RageDisplay_D3D::IsZTestEnabled() const
{
	DWORD b;
	g_pd3dDevice->GetRenderState( D3DRS_ZFUNC, &b );
	return b!=D3DCMP_ALWAYS;
}

void RageDisplay_D3D::SetZWrite( bool b )
{
	g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, b );
}

void RageDisplay_D3D::SetZTestMode( ZTestMode mode )
{
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE,      D3DZB_TRUE );
	DWORD dw;
	switch( mode )
	{
	case ZTEST_OFF:				dw = D3DCMP_ALWAYS;		break;
	case ZTEST_WRITE_ON_PASS:	dw = D3DCMP_LESSEQUAL;	break;
	case ZTEST_WRITE_ON_FAIL:	dw = D3DCMP_GREATER;	break;
	default:	ASSERT( 0 );
	}
	g_pd3dDevice->SetRenderState( D3DRS_ZFUNC, dw );
}

void RageDisplay_D3D::ClearZBuffer()
{
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,0), 1.0f, 0x00000000 );
}

void RageDisplay_D3D::SetTextureWrapping( bool b )
{
	if( g_currentTextureUnit >= (int) g_DeviceCaps.MaxSimultaneousTextures )	// not supported
		return;

	int mode = b ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP;
    g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_ADDRESSU, mode );
    g_pd3dDevice->SetTextureStageState( g_currentTextureUnit, D3DTSS_ADDRESSV, mode );
}

void RageDisplay_D3D::SetMaterial( 
	const RageColor &emissive,
	const RageColor &ambient,
	const RageColor &diffuse,
	const RageColor &specular,
	float shininess
	)
{
	/* If lighting is off, then the current material will have no effect.
	 * We want to still be able to color models with lighting off,
	 * so shove the material color in texture factor and modify the 
	 * texture stage to use it instead of the vertex color (our models
	 * don't have vertex coloring anyway). 
	 */
	DWORD bLighting;
	g_pd3dDevice->GetRenderState( D3DRS_LIGHTING, &bLighting );

	if( bLighting )
	{
		D3DMATERIAL8 mat;
		memcpy( &mat.Diffuse, diffuse, sizeof(float)*4 );
		memcpy( &mat.Ambient, ambient, sizeof(float)*4 );
		memcpy( &mat.Specular, specular, sizeof(float)*4 );
		memcpy( &mat.Emissive, emissive, sizeof(float)*4 );
		mat.Power = shininess;
		g_pd3dDevice->SetMaterial( &mat );
	}
	else
	{
		RageColor c = diffuse;
		c.r += emissive.r + ambient.r;
		c.g += emissive.g + ambient.g;
		c.b += emissive.b + ambient.b;
		RageVColor c2 = c;
		DWORD c3 = *(DWORD*)&c2;
		g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, c3 );
	}
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
	const RageColor &ambient, 
	const RageColor &diffuse, 
	const RageColor &specular, 
	const RageVector3 &dir )
{
	g_pd3dDevice->LightEnable( index, true );

	D3DLIGHT8 light;
	ZERO( light );
	light.Type = D3DLIGHT_DIRECTIONAL;

	/* Z for lighting is flipped for D3D compared to OpenGL.
	 * XXX: figure out exactly why this is needed.  Our transforms 
	 * are probably goofed up, but the Z test is the same for both
	 * API's, so I'm not sure why we don't see other weirdness. -Chris */
	float position[] = { dir.x, dir.y, -dir.z };
	memcpy( &light.Direction, position, sizeof(position) );
	memcpy( &light.Diffuse, diffuse, sizeof(diffuse) );
	memcpy( &light.Ambient, ambient, sizeof(ambient) );
	memcpy( &light.Specular, specular, sizeof(specular) );
	
	// Same as OpenGL defaults.  Not used in directional lights.
//	light.Attenuation0 = 1;
//	light.Attenuation1 = 0;
//	light.Attenuation2 = 0;

	g_pd3dDevice->SetLight( index, &light );
}

void RageDisplay_D3D::SetCullMode( CullMode mode )
{
	switch( mode )
	{
	case CULL_BACK:
		g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
		break;
	case CULL_FRONT:
		g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
		break;
	case CULL_NONE:
		g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		break;
	default:
		ASSERT(0);
	}
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
	RageSurface* img,
	bool bGenerateMipMaps )
{
	// texture must be power of two
	ASSERT( img->w == power_of_two(img->w) );
	ASSERT( img->h == power_of_two(img->h) );


	HRESULT hr;
	IDirect3DTexture8* pTex;
	hr = g_pd3dDevice->CreateTexture( img->w, img->h, 1, 0, D3DFORMATS[pixfmt], D3DPOOL_MANAGED, &pTex );

#if defined(XBOX)
	while(hr == E_OUTOFMEMORY)
	{
		if(!vmem_Manager.DecommitLRU())
			break;
		hr = g_pd3dDevice->CreateTexture( img->w, img->h, 1, 0, D3DFORMATS[pixfmt], D3DPOOL_MANAGED, &pTex );
	}
#endif

	if( FAILED(hr) )
		RageException::Throw( "CreateTexture(%i,%i,%s) failed: %s", 
		img->w, img->h, PixelFormatToString(pixfmt).c_str(), GetErrorString(hr).c_str() );

	unsigned uTexHandle = (unsigned)pTex;

	if( pixfmt == PixelFormat_PAL )
	{
		// Save palette
		TexturePalette pal;
		memset( pal.p, 0, sizeof(pal.p) );
		for( int i=0; i<img->format->palette->ncolors; i++ )
		{
			RageSurfaceColor &c = img->format->palette->colors[i];
			pal.p[i].peRed = c.r;
			pal.p[i].peGreen = c.g;
			pal.p[i].peBlue = c.b;
			pal.p[i].peFlags = c.a;
		}

		ASSERT( g_TexResourceToTexturePalette.find(uTexHandle) == g_TexResourceToTexturePalette.end() );
		g_TexResourceToTexturePalette[uTexHandle] = pal;
	}

	UpdateTexture( uTexHandle, img, 0, 0, img->w, img->h );

	return uTexHandle;
}

void RageDisplay_D3D::UpdateTexture( 
	unsigned uTexHandle, 
	RageSurface* img,
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
	
	D3DSURFACE_DESC desc;
	pTex->GetLevelDesc(0, &desc);
	ASSERT( xoffset+width <= int(desc.Width) );
	ASSERT( yoffset+height <= int(desc.Height) );

	//
	// Copy bits
	//
#if defined(XBOX)
	// Xbox textures need to be swizzled
	XGSwizzleRect(
		img->pixels,	// pSource, 
		img->pitch,		// Pitch,
		NULL,	// pRect,
		lr.pBits,	// pDest,
		img->w,	// Width,
		img->h,	// Height,
		NULL,	// pPoint,
		img->format->BytesPerPixel ); //BytesPerPixel
#else
	int texpixfmt;
	for(texpixfmt = 0; texpixfmt < NUM_PixelFormat; ++texpixfmt)
		if(D3DFORMATS[texpixfmt] == desc.Format) break;
	ASSERT( texpixfmt != NUM_PixelFormat );

	RageSurface *Texture = CreateSurfaceFromPixfmt(PixelFormat(texpixfmt), lr.pBits, width, height, lr.Pitch);
	ASSERT( Texture );
	RageSurfaceUtils::Blit( img, Texture, width, height );

	delete Texture;
#endif

	pTex->UnlockRect( 0 );
}

void RageDisplay_D3D::SetAlphaTest( bool b )
{
	g_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  b );
	g_pd3dDevice->SetRenderState( D3DRS_ALPHAREF,         0 );
	g_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );
}

RageMatrix RageDisplay_D3D::GetOrthoMatrix( float l, float r, float b, float t, float zn, float zf )
{
	RageMatrix m = RageDisplay::GetOrthoMatrix( l, r, b, t, zn, zf );

	/* Convert from OpenGL's [-1,+1] Z values to D3D's [0,+1]. */
	RageMatrix tmp;
	RageMatrixScaling( &tmp, 1, 1, 0.5f );
	RageMatrixMultiply( &m, &tmp, &m );

	RageMatrixTranslation( &tmp, 0, 0, 0.5f );
	RageMatrixMultiply( &m, &tmp, &m );

	return m;
}

void RageDisplay_D3D::SetSphereEnvironmentMapping( bool b )
{
	g_bSphereMapping[g_currentTextureUnit] = b;
}

/*
 * Copyright (c) 2001-2004 Chris Danford, Glenn Maynard
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
