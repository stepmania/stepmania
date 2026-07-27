#include "global.h"
#include "RageDisplay.h"
#include "RageDisplay_D3D.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageTimer.h"
#include "RageException.h"
#include "RageMath.h"
#include "RageTypes.h"
#include "RageSurface.h"
#include "RageSurfaceUtils.h"
#include "EnumHelper.h"
#include "DisplaySpec.h"
#include "LocalizedString.h"

#include <d3d9.h>

#include "archutils/Win32/GraphicsWindow.h"
#include "archutils/Win32/DirectXHelpers.h"

// Static libraries
// load Windows D3D9 dynamically
#if defined(_MSC_VER)
	#pragma comment(lib, "d3d9.lib")
#endif

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <list>
#include <vector>


// Globals
HMODULE				g_D3D9_Module = nullptr;
LPDIRECT3D9			g_pd3d = nullptr;
LPDIRECT3DDEVICE9	g_pd3dDevice = nullptr;
D3DCAPS9			g_DeviceCaps;
D3DDISPLAYMODE		g_DesktopMode;
D3DPRESENT_PARAMETERS	g_d3dpp;
int					g_ModelMatrixCnt=0;
static bool		g_bSphereMapping[NUM_TextureUnit] = { false, false };

// TODO: Instead of defining this here, enumerate the possible formats and select whatever one we want to use. This format should
// be fine for the uses of this application though.
const D3DFORMAT g_DefaultAdapterFormat = D3DFMT_X8R8G8B8;

/* Direct3D doesn't associate a palette with textures. Instead, we load a
 * palette into a slot. We need to keep track of which texture's palette is
 * stored in what slot. */
std::map<std::uintptr_t, std::size_t>		g_TexResourceToPaletteIndex;
std::list<std::size_t>			g_PaletteIndex;
struct TexturePalette { PALETTEENTRY p[256]; };
std::map<std::uintptr_t, TexturePalette>	g_TexResourceToTexturePalette;

// Load the palette, if any, for the given texture into a palette slot, and make it current.
static void SetPalette( std::uintptr_t TexResource )
{
	// If the texture isn't paletted, we have nothing to do.
	if( g_TexResourceToTexturePalette.find(TexResource) == g_TexResourceToTexturePalette.end() )
		return;

	// Is the palette already loaded?
	if( g_TexResourceToPaletteIndex.find(TexResource) == g_TexResourceToPaletteIndex.end() )
	{
		// It's not. Grab the least recently used slot.
		UINT iPalIndex = static_cast<UINT>(g_PaletteIndex.front());

		// If any other texture is currently using this slot, mark that palette unloaded.
		for( std::map<std::uintptr_t, std::size_t>::iterator i = g_TexResourceToPaletteIndex.begin(); i != g_TexResourceToPaletteIndex.end(); ++i )
		{
			if( i->second != iPalIndex )
				continue;
			g_TexResourceToPaletteIndex.erase(i);
			break;
		}

		// Load it.
		TexturePalette& pal = g_TexResourceToTexturePalette[TexResource];
		g_pd3dDevice->SetPaletteEntries( iPalIndex, pal.p );

		g_TexResourceToPaletteIndex[TexResource] = iPalIndex;
	}

	const int iPalIndex = g_TexResourceToPaletteIndex[TexResource];

	// Find this palette index in the least-recently-used queue and move it to the end.
	for(std::list<std::size_t>::iterator i = g_PaletteIndex.begin(); i != g_PaletteIndex.end(); ++i)
	{
		if( *i != iPalIndex )
			continue;
		g_PaletteIndex.erase(i);
		g_PaletteIndex.push_back(iPalIndex);
		break;
	}

	g_pd3dDevice->SetCurrentTexturePalette( iPalIndex );
}

#define D3DFVF_RageSpriteVertex (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#define D3DFVF_RageModelVertex (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)


static const RageDisplay::RagePixelFormatDesc PIXEL_FORMAT_DESC[NUM_RagePixelFormat] = {
	{
		/* A8B8G8R8 */
		32,
		{ 0x00FF0000,
		  0x0000FF00,
		  0x000000FF,
		  0xFF000000 }
	}, {
		0, { 0,0,0,0 }
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
		/* X1R5G5B5 */
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
	}, {
		/* X1R5G5B5 */
		0, { 0,0,0,0 }
	}
};

static D3DFORMAT D3DFORMATS[NUM_RagePixelFormat] =
{
	D3DFMT_A8R8G8B8,
	D3DFMT_UNKNOWN,
	D3DFMT_A4R4G4B4,
	D3DFMT_A1R5G5B5,
	D3DFMT_X1R5G5B5,
	D3DFMT_R8G8B8,
	D3DFMT_P8,
	D3DFMT_UNKNOWN, // no BGR
	D3DFMT_UNKNOWN, // no ABGR
	D3DFMT_UNKNOWN, // X1R5G5B5
};

const RageDisplay::RagePixelFormatDesc *RageDisplay_D3D::GetPixelFormatDesc(RagePixelFormat pf) const
{
	ASSERT( pf < NUM_RagePixelFormat );
	return &PIXEL_FORMAT_DESC[pf];
}


RageDisplay_D3D::RageDisplay_D3D()
{

}

static LocalizedString D3D_NOT_INSTALLED ( "RageDisplay_D3D", "DirectX 9.0c or greater is not installed.  You can download it from:" );
const RString D3D_URL = "http://www.microsoft.com/en-us/download/details.aspx?id=8109";
static LocalizedString HARDWARE_ACCELERATION_NOT_AVAILABLE ( "RageDisplay_D3D",
	"Your system is reporting that Direct3D hardware acceleration is not available.  Please obtain an updated driver from your video card manufacturer." );
RString RageDisplay_D3D::Init( const VideoModeParams &p, bool /* bAllowUnacceleratedRenderer */ )
{
	GraphicsWindow::Initialize( true );

	LOG->Trace( "RageDisplay_D3D::RageDisplay_D3D()" );
	LOG->MapLog("renderer", "Current renderer: Direct3D");

	g_pd3d = Direct3DCreate9(D3D_SDK_VERSION);
	if(!g_pd3d)
	{
		LOG->Trace( "Direct3DCreate9 failed" );
		return D3D_NOT_INSTALLED.GetValue();
	}

	if( FAILED( g_pd3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &g_DeviceCaps) ) )
		return HARDWARE_ACCELERATION_NOT_AVAILABLE.GetValue();


	D3DADAPTER_IDENTIFIER9	identifier;
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

	UINT modeCount = g_pd3d->GetAdapterModeCount(D3DADAPTER_DEFAULT, g_DefaultAdapterFormat);

	for( UINT u=0; u < modeCount; u++ )
		if( SUCCEEDED( g_pd3d->EnumAdapterModes( D3DADAPTER_DEFAULT, g_DefaultAdapterFormat, u, &mode ) ) )
			LOG->Trace( "  %ux%u %uHz, format %d", mode.Width, mode.Height, mode.RefreshRate, mode.Format );

	g_PaletteIndex.clear();
	for( int i = 0; i < 256; ++i )
		g_PaletteIndex.push_back(i);

	// Save the original desktop format.
	g_pd3d->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &g_DesktopMode );

	/* Up until now, all we've done is set up g_pd3d and do some queries. Now,
	 * actually initialize the window. Do this after as many error conditions as
	 * possible, because if we have to shut it down again we'll flash a window briefly. */
	bool bIgnore = false;
	return SetVideoMode( p, bIgnore );
}

RageDisplay_D3D::~RageDisplay_D3D()
{
	LOG->Trace( "RageDisplay_D3D::~RageDisplay()" );

	GraphicsWindow::Shutdown();

	if( g_pd3dDevice )
	{
		g_pd3dDevice->Release();
		g_pd3dDevice = nullptr;
	}

	if( g_pd3d )
	{
		g_pd3d->Release();
		g_pd3d = nullptr;
	}

	/* Even after we call Release(), D3D may still affect our window. It seems
	 * to subclass the window, and never release it. Free the DLL after
	 * destroying the window. */
	if( g_D3D9_Module )
	{
		FreeLibrary( g_D3D9_Module );
		g_D3D9_Module = nullptr;
	}
}

void RageDisplay_D3D::GetDisplaySpecs( DisplaySpecs &out ) const
{
	out.clear();
	int iCnt = g_pd3d->GetAdapterModeCount( D3DADAPTER_DEFAULT, g_DefaultAdapterFormat );

	std::set<DisplayMode> modes;
	D3DDISPLAYMODE mode;
	for ( int i = 0; i < iCnt; ++i )
	{
		g_pd3d->EnumAdapterModes( D3DADAPTER_DEFAULT, g_DefaultAdapterFormat, i, &mode );
		modes.insert( { mode.Width, mode.Height, static_cast<double> (mode.RefreshRate) } );
	}

	// Get the current display mode
	if ( g_pd3d->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &mode ) == D3D_OK )
	{
		D3DADAPTER_IDENTIFIER9 ID;
		g_pd3d->GetAdapterIdentifier( D3DADAPTER_DEFAULT, 0, &ID );
		DisplayMode active = { mode.Width, mode.Height, static_cast<double> (mode.RefreshRate) };
		RectI bounds( 0, 0, active.width, active.height );
		out.insert( DisplaySpec( "", "Fullscreen", modes, active, bounds ) );
	}
	else
	{
		LOG->Warn( "Could not find active mode for default D3D adapter" );
		if ( !modes.empty() )
		{
			const DisplayMode &m = *modes.begin();
			RectI bounds( 0, 0, m.width, m.height );
			out.insert( DisplaySpec( "", "Fullscreen", modes, m, bounds ) );
		}
	}

}

D3DFORMAT FindBackBufferType(bool bWindowed, int iBPP)
{
	HRESULT hr;

	// If windowed, then bpp is ignored.  Use whatever works.
	std::vector<D3DFORMAT> vBackBufferFormats; // throw all possibilities in here

	// When windowed, add all formats; otherwise add only formats that match dwBPP.
	if( iBPP == 32 || bWindowed )
	{
		vBackBufferFormats.push_back( D3DFMT_R8G8B8 );
		vBackBufferFormats.push_back( D3DFMT_X8R8G8B8 );
		vBackBufferFormats.push_back( D3DFMT_A8R8G8B8 );
	}
	if( iBPP == 16 || bWindowed )
	{
		vBackBufferFormats.push_back( D3DFMT_R5G6B5 );
		vBackBufferFormats.push_back( D3DFMT_X1R5G5B5 );
		vBackBufferFormats.push_back( D3DFMT_A1R5G5B5 );
	}


	if( !bWindowed && iBPP != 16 && iBPP != 32 )
	{
		GraphicsWindow::Shutdown();
		RageException::Throw( "Invalid BPP '%i' specified", iBPP );
	}

	// Test each back buffer format until we find something that works.
	for( std::size_t i=0; i < vBackBufferFormats.size(); i++ )
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
			continue; // skip

		// done searching
		LOG->Trace( "This will work." );
		return fmtBackBuffer;
	}

	LOG->Trace( "Couldn't find an appropriate back buffer format." );
	return D3DFMT_UNKNOWN;
}

RString SetD3DParams( bool &bNewDeviceOut )
{
	if( g_pd3dDevice == nullptr ) // device is not yet created. We need to create it
	{
		bNewDeviceOut = true;
		HRESULT hr = g_pd3d->CreateDevice(
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			GraphicsWindow::GetHwnd(),
			D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
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

	// Palettes were lost by Reset(), so mark them unloaded.
	g_TexResourceToPaletteIndex.clear();

	return RString();
}

// If the given parameters have failed, try to lower them.
static bool D3DReduceParams( D3DPRESENT_PARAMETERS *pp )
{
	D3DDISPLAYMODE current;
	current.Format = pp->BackBufferFormat;
	current.Height = pp->BackBufferHeight;
	current.Width = pp->BackBufferWidth;
	current.RefreshRate = pp->FullScreen_RefreshRateInHz;

	const int iCnt = g_pd3d->GetAdapterModeCount( D3DADAPTER_DEFAULT, g_DefaultAdapterFormat );
	int iBest = -1;
	int iBestScore = 0;
	LOG->Trace( "cur: %ux%u %uHz, format %i", current.Width, current.Height, current.RefreshRate, current.Format );
	for( int i = 0; i < iCnt; ++i )
	{
		D3DDISPLAYMODE mode;
		g_pd3d->EnumAdapterModes( D3DADAPTER_DEFAULT, g_DefaultAdapterFormat, i, &mode );

		// Never change the format.
		if( mode.Format != current.Format )
			continue;
		// Never increase the parameters.
		if( mode.Height > current.Height || mode.Width > current.Width || mode.RefreshRate > current.RefreshRate )
			continue;

		// Never go below 640x480 unless we already are.
		if( (current.Width >= 640 && current.Height >= 480) && (mode.Width < 640 || mode.Height < 480) )
			continue;

		// Never go below 60Hz.
		if( mode.RefreshRate && mode.RefreshRate < 60 )
			continue;

		/* If mode.RefreshRate is 0, it means "default". We don't know what
		 * that means; assume it's 60Hz. */

		// Higher scores are better.
		int iScore = 0;
		if( current.RefreshRate >= 70 && mode.RefreshRate < 70 )
		{
			/* Top priority: we really want to avoid dropping to a refresh rate
			 * that's below 70Hz. */
			iScore -= 100000;
		}
		else if( mode.RefreshRate < current.RefreshRate )
		{
			/* Low priority: We're lowering the refresh rate, but not too far.
			 * current.RefreshRate might be 0, in which case this simply gives
			 * points for higher refresh rates. */
			iScore += (mode.RefreshRate - current.RefreshRate);
		}

		// Medium priority:
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
	g_pd3d->EnumAdapterModes( D3DADAPTER_DEFAULT, g_DefaultAdapterFormat, iBest, &BestMode );
	pp->BackBufferHeight = BestMode.Height;
	pp->BackBufferWidth = BestMode.Width;
	pp->FullScreen_RefreshRateInHz = BestMode.RefreshRate;

	return true;
}


static void SetPresentParametersFromVideoModeParams( const VideoModeParams &p, D3DPRESENT_PARAMETERS *pD3Dpp )
{
	ZERO( *pD3Dpp );

	pD3Dpp->BackBufferWidth		= p.width;
	pD3Dpp->BackBufferHeight	= p.height;
	pD3Dpp->BackBufferFormat	= FindBackBufferType( p.windowed, p.bpp );
	pD3Dpp->BackBufferCount		= 1;
	pD3Dpp->MultiSampleType		= D3DMULTISAMPLE_NONE;
	pD3Dpp->SwapEffect		= D3DSWAPEFFECT_DISCARD;
	pD3Dpp->hDeviceWindow		= GraphicsWindow::GetHwnd();
	pD3Dpp->Windowed		= p.windowed;
	pD3Dpp->EnableAutoDepthStencil	= TRUE;
	pD3Dpp->AutoDepthStencilFormat	= D3DFMT_D16;

	if( p.windowed )
		pD3Dpp->PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	else
		pD3Dpp->PresentationInterval = p.vsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;

	pD3Dpp->FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	if( !p.windowed && p.rate != REFRESH_DEFAULT )
		pD3Dpp->FullScreen_RefreshRateInHz = p.rate;

	pD3Dpp->Flags = 0;

	LOG->Trace( "Present Parameters: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
		pD3Dpp->BackBufferWidth, pD3Dpp->BackBufferHeight, pD3Dpp->BackBufferFormat,
		pD3Dpp->BackBufferCount,
		pD3Dpp->MultiSampleType, pD3Dpp->SwapEffect, pD3Dpp->hDeviceWindow,
		pD3Dpp->Windowed, pD3Dpp->EnableAutoDepthStencil, pD3Dpp->AutoDepthStencilFormat,
		pD3Dpp->Flags, pD3Dpp->FullScreen_RefreshRateInHz,
		pD3Dpp->PresentationInterval
	);
}

// Set the video mode.
RString RageDisplay_D3D::TryVideoMode( const VideoModeParams &_p, bool &bNewDeviceOut )
{
	VideoModeParams p = _p;
	LOG->Warn( "RageDisplay_D3D::TryVideoMode( %d, %d, %d, %d, %d, %d )", p.windowed, p.width, p.height, p.bpp, p.rate, p.vsync );

	if( FindBackBufferType( p.windowed, p.bpp ) == D3DFMT_UNKNOWN )	// no possible back buffer formats
		return ssprintf( "FindBackBufferType(%i,%i) failed", p.windowed, p.bpp );	// failed to set mode

	/* Set up and display the window before setting up D3D. If we don't do this,
	 * then setting up a fullscreen window (when we're not coming from windowed)
	 * causes all other windows on the system to be resized to the new resolution. */
	GraphicsWindow::CreateGraphicsWindow( p );

	SetPresentParametersFromVideoModeParams( p, &g_d3dpp );

	// Display the window immediately, so we don't display the desktop ...
	while( 1 )
	{
		// Try the video mode.
		RString sErr = SetD3DParams( bNewDeviceOut );
		if( sErr.empty() )
			break;

		/* It failed. We're probably selecting a video mode that isn't supported.
		 * If we're fullscreen, search the mode list and find the nearest lower mode. */
		if( p.windowed || !D3DReduceParams( &g_d3dpp ) )
			return sErr;

		// Store the new settings we're about to try.
		p.height = g_d3dpp.BackBufferHeight;
		p.width = g_d3dpp.BackBufferWidth;
		if( g_d3dpp.FullScreen_RefreshRateInHz == D3DPRESENT_RATE_DEFAULT )
			p.rate = REFRESH_DEFAULT;
		else
			p.rate = g_d3dpp.FullScreen_RefreshRateInHz;
	}

	/* Call this again after changing the display mode. If we're going to a window
	 * from fullscreen, the first call can't set a larger window than the old
	 * fullscreen resolution or set the window position. */
	GraphicsWindow::CreateGraphicsWindow( p );

	ResolutionChanged();

	return RString(); // mode change successful
}

void RageDisplay_D3D::ResolutionChanged()
{
	//LOG->Warn( "RageDisplay_D3D::ResolutionChanged" );

	RageDisplay::ResolutionChanged();
}

int RageDisplay_D3D::GetMaxTextureSize() const
{
	return g_DeviceCaps.MaxTextureWidth;
}

bool RageDisplay_D3D::BeginFrame()
{
	GraphicsWindow::Update();

	switch( g_pd3dDevice->TestCooperativeLevel() )
	{
	case D3DERR_DEVICELOST:
		return false;
	case D3DERR_DEVICENOTRESET:
		{
			bool bIgnore = false;
			RString sError = SetD3DParams( bIgnore );
			if( sError != "" )
				RageException::Throw( sError );

			break;
		}
	}

	g_pd3dDevice->Clear( 0, nullptr, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
						 D3DCOLOR_XRGB(0,0,0), 1.0f, 0x00000000 );
	g_pd3dDevice->BeginScene();

	return RageDisplay::BeginFrame();
}

static RageTimer g_LastFrameEndedAt( RageZeroTimer );
void RageDisplay_D3D::EndFrame()
{
	g_pd3dDevice->EndScene();

	FrameLimitBeforeVsync( GetActualVideoModeParams().rate );
	g_pd3dDevice->Present( 0, 0, 0, 0 );
	FrameLimitAfterVsync();

	RageDisplay::EndFrame();
}

bool RageDisplay_D3D::SupportsTextureFormat( RagePixelFormat pixfmt, bool realtime )
{
	// Some cards (Savage) don't support alpha in palettes.
	// Don't allow paletted textures if this is the case.
	if( pixfmt == RagePixelFormat_PAL  &&  !(g_DeviceCaps.TextureCaps & D3DPTEXTURECAPS_ALPHAPALETTE) )
		return false;

	if( D3DFORMATS[pixfmt] == D3DFMT_UNKNOWN )
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
	RageSurface * result = nullptr;

	// get the render target
	IDirect3DSurface9* pSurface;
	if (SUCCEEDED(g_pd3dDevice->GetRenderTarget(0, &pSurface)))
	{
		// get the render target surface description
		D3DSURFACE_DESC desc;
		pSurface->GetDesc(&desc);

		// create an offscreen plain surface of the same format in the SYSTEMMEM pool
		IDirect3DSurface9* pCopy;
		if (SUCCEEDED(g_pd3dDevice->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &pCopy, nullptr)))
		{
			// copy the data from the render target into the offscreen plain surface
			if (SUCCEEDED(g_pd3dDevice->GetRenderTargetData(pSurface, pCopy)))
			{
				// Update desc from the copy.
				pCopy->GetDesc(&desc);

				D3DLOCKED_RECT lr;

				{
					RECT rect;
					rect.left = 0;
					rect.top = 0;
					rect.right = desc.Width;
					rect.bottom = desc.Height;

					pCopy->LockRect(&lr, &rect, D3DLOCK_READONLY);
				}

				// since we no longer have an easy function to force a conversion to A8R8G8B8, we need to figure out our pixel format.
				// (yes, we could create a couple of surfaces in the default pool, copy the bits into the one matching our source,
				// then use IDirect3DDevice::StretchRect to convert it without stretching into our desired format. This would mean
				// a copy to device memory and a copy back again, though.)
				// possible formats are found in FindBackBufferType
				RagePixelFormat pf;
				switch (desc.Format)
				{
				default:               pf = RagePixelFormat_Invalid; FAIL_M("Unknown pixel format");  break;
				case D3DFMT_X8R8G8B8:  pf = RagePixelFormat_RGBA8; break;
				case D3DFMT_A8R8G8B8:  pf = RagePixelFormat_RGBA8; break;
					// 16-bit formats are not here. Does anybody actually use them?
				}

				RageSurface *surface = CreateSurfaceFromPixfmt(pf, lr.pBits, desc.Width, desc.Height, lr.Pitch);
				ASSERT(nullptr != surface);

				// We need to make a copy, since lr.pBits will go away when we call UnlockRect().
				result =
					CreateSurface(surface->w, surface->h,
						surface->format->BitsPerPixel,
						surface->format->Rmask, surface->format->Gmask,
						surface->format->Bmask, surface->format->Amask);
				RageSurfaceUtils::CopySurface(surface, result);
				delete surface;

				pCopy->UnlockRect();
			}

			pCopy->Release();
		}

		pSurface->Release();
	}

	return result;
}

ActualVideoModeParams RageDisplay_D3D::GetActualVideoModeParams() const
{
	return GraphicsWindow::GetParams();
}

void RageDisplay_D3D::SendCurrentMatrices()
{
	RageMatrix m;
	RageMatrixMultiply( &m, GetCentering(), GetProjectionTop() );

	// Convert to OpenGL-style "pixel-centered" coords
	RageMatrix m2 = GetCenteringMatrix( -0.5f, -0.5f, 0, 0 );
	RageMatrix projection;
	RageMatrixMultiply( &projection, &m2, &m );
	g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&projection );

	g_pd3dDevice->SetTransform( D3DTS_VIEW, (D3DMATRIX*)GetViewTop() );
	g_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)GetWorldTop() );

	FOREACH_ENUM( TextureUnit, tu )
	{
		// Optimization opportunity: Turn off texture transform if not using texture coords.
		g_pd3dDevice->SetTextureStageState( tu, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );

		// If no texture is set for this texture unit, don't bother setting it up.
		IDirect3DBaseTexture9* pTexture = nullptr;
		g_pd3dDevice->GetTexture( tu, &pTexture );
		if( pTexture == nullptr )
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
			g_pd3dDevice->SetTransform( (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+Enum::to_integral(tu)), (D3DMATRIX*)&tex );

			// Tell D3D to use transformed reflection vectors as texture co-ordinate 0
			// and then transform this coordinate by the specified texture matrix.
			g_pd3dDevice->SetTextureStageState( tu, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR );
		}
		else
		{
			/* Direct3D is expecting a 3x3 matrix loaded into the 4x4 in order
			 * to transform the 2-component texture coordinates. We currently
			 * only use translate and scale, and ignore the z component entirely,
			 * so convert the texture matrix from 4x4 to 3x3 by dropping z. */

			const RageMatrix &tex1 = *GetTextureTop();
			const RageMatrix tex2 = RageMatrix
			(
				tex1.m[0][0], tex1.m[0][1],  tex1.m[0][3],	0,
				tex1.m[1][0], tex1.m[1][1],  tex1.m[1][3],	0,
				tex1.m[3][0], tex1.m[3][1],  tex1.m[3][3],	0,
				0,				0,			0,		0
			);
			g_pd3dDevice->SetTransform( D3DTRANSFORMSTATETYPE(D3DTS_TEXTURE0+Enum::to_integral(tu)), (D3DMATRIX*)&tex2 );

			g_pd3dDevice->SetTextureStageState( tu, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU );
		}
	}
}

class RageCompiledGeometrySWD3D : public RageCompiledGeometry
{
public:
	void Allocate( const std::vector<msMesh> &vMeshes )
	{
		m_vVertex.resize( std::max<unsigned int>(1u, GetTotalVertices()) );
		m_vTriangles.resize( std::max<unsigned int>(1u, GetTotalTriangles()) );
	}
	void Change( const std::vector<msMesh> &vMeshes )
	{
		for( std::size_t i=0; i<vMeshes.size(); i++ )
		{
			const MeshInfo& meshInfo = m_vMeshInfo[i];
			const msMesh& mesh = vMeshes[i];
			const std::vector<RageModelVertex> &Vertices = mesh.Vertices;
			const std::vector<msTriangle> &Triangles = mesh.Triangles;

			for( std::size_t j=0; j<Vertices.size(); j++ )
				m_vVertex[meshInfo.iVertexStart+j] = Vertices[j];

			for( std::size_t j=0; j<Triangles.size(); j++ )
				for( std::size_t k=0; k<3; k++ )
					m_vTriangles[meshInfo.iTriangleStart+j].nVertexIndices[k] = (std::uint16_t) meshInfo.iVertexStart + Triangles[j].nVertexIndices[k];
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

		g_pd3dDevice->SetFVF( D3DFVF_RageModelVertex );
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
	std::vector<RageModelVertex> m_vVertex;
	std::vector<msTriangle>		m_vTriangles;
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
	static std::vector<std::uint16_t> vIndices;
	std::size_t uOldSize = vIndices.size();
	std::size_t uNewSize = std::max(uOldSize, static_cast<std::size_t>(iNumIndices));
	vIndices.resize( uNewSize );
	for( std::uint16_t i=(std::uint16_t)uOldSize/6; i<(std::uint16_t)iNumQuads; i++ )
	{
		vIndices[i*6+0] = i*4+0;
		vIndices[i*6+1] = i*4+1;
		vIndices[i*6+2] = i*4+2;
		vIndices[i*6+3] = i*4+2;
		vIndices[i*6+4] = i*4+3;
		vIndices[i*6+5] = i*4+0;
	}

	g_pd3dDevice->SetFVF( D3DFVF_RageSpriteVertex );
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
	static std::vector<std::uint16_t> vIndices;
	std::size_t uOldSize = vIndices.size();
	std::size_t uNewSize = std::max(uOldSize, static_cast<std::size_t>(iNumIndices));
	vIndices.resize( uNewSize );
	for( std::uint16_t i=(std::uint16_t)uOldSize/6; i<(std::uint16_t)iNumQuads; i++ )
	{
		vIndices[i*6+0] = i*2+0;
		vIndices[i*6+1] = i*2+1;
		vIndices[i*6+2] = i*2+2;
		vIndices[i*6+3] = i*2+1;
		vIndices[i*6+4] = i*2+2;
		vIndices[i*6+5] = i*2+3;
	}

	g_pd3dDevice->SetFVF( D3DFVF_RageSpriteVertex );
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

void RageDisplay_D3D::DrawSymmetricQuadStripInternal( const RageSpriteVertex v[], int iNumVerts )
{
	int iNumPieces = (iNumVerts-3)/3;
	int iNumTriangles = iNumPieces*4;
	int iNumIndices = iNumTriangles*3;

	// make a temporary index buffer
	static std::vector<std::uint16_t> vIndices;
	std::size_t uOldSize = vIndices.size();
	std::size_t uNewSize = std::max(uOldSize, static_cast<std::size_t>(iNumIndices));
	vIndices.resize( uNewSize );
	for( std::uint16_t i=(std::uint16_t)uOldSize/12; i<(std::uint16_t)iNumPieces; i++ )
	{
		// { 1, 3, 0 } { 1, 4, 3 } { 1, 5, 4 } { 1, 2, 5 }
		vIndices[i*12+0] = i*3+1;
		vIndices[i*12+1] = i*3+3;
		vIndices[i*12+2] = i*3+0;
		vIndices[i*12+3] = i*3+1;
		vIndices[i*12+4] = i*3+4;
		vIndices[i*12+5] = i*3+3;
		vIndices[i*12+6] = i*3+1;
		vIndices[i*12+7] = i*3+5;
		vIndices[i*12+8] = i*3+4;
		vIndices[i*12+9] = i*3+1;
		vIndices[i*12+10] = i*3+2;
		vIndices[i*12+11] = i*3+5;
	}

	g_pd3dDevice->SetFVF( D3DFVF_RageSpriteVertex );
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
	g_pd3dDevice->SetFVF( D3DFVF_RageSpriteVertex );
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
	g_pd3dDevice->SetFVF( D3DFVF_RageSpriteVertex );
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
	g_pd3dDevice->SetFVF( D3DFVF_RageSpriteVertex );
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

	/* If lighting is off, then the current material will have no effect. We
	 * want to still be able to color models with lighting off, so shove the
	 * material color in texture factor and modify the texture stage to use it
	 * instead of the vertex color (our models don't have vertex coloring anyway). */
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
	FOREACH_ENUM( TextureUnit, i )
		SetTexture( i, 0 );
}

int RageDisplay_D3D::GetNumTextureUnits()
{
	return g_DeviceCaps.MaxSimultaneousTextures;
}

void RageDisplay_D3D::SetTexture( TextureUnit tu, std::uintptr_t iTexture )
{
//	g_DeviceCaps.MaxSimultaneousTextures = 1;
	if( tu >= (int) g_DeviceCaps.MaxSimultaneousTextures )	// not supported
		return;

	if( iTexture == 0 )
	{
		g_pd3dDevice->SetTexture( tu, nullptr );

		/* Intentionally commented out. Don't mess with texture stage state
		 * when just setting the texture. Model sets its texture modes before
		 * setting the final texture. */
		//g_pd3dDevice->SetTextureStageState( tu, D3DTSS_COLOROP, D3DTOP_DISABLE );
	}
	else
	{
		IDirect3DTexture9* pTex = reinterpret_cast<IDirect3DTexture9*>(iTexture);
		g_pd3dDevice->SetTexture( tu, pTex );

		/* Intentionally commented out. Don't mess with texture stage state
		 * when just setting the texture. Model sets its texture modes before
		 * setting the final texture. */
		//g_pd3dDevice->SetTextureStageState( tu, D3DTSS_COLOROP, D3DTOP_MODULATE );

		// Set palette (if any)
		SetPalette( iTexture );
	}
}

void RageDisplay_D3D::SetTextureMode( TextureUnit tu, TextureMode tm )
{
	if( tu >= (int) g_DeviceCaps.MaxSimultaneousTextures )	// not supported
		return;

	switch( tm )
	{
	case TextureMode_Modulate:
		// Use D3DTA_CURRENT instead of diffuse so that multitexturing works
		// properly.  For stage 0, D3DTA_CURRENT is the diffuse color.

		g_pd3dDevice->SetTextureStageState( tu, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		g_pd3dDevice->SetTextureStageState( tu, D3DTSS_COLORARG2, D3DTA_CURRENT );
		g_pd3dDevice->SetTextureStageState( tu, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		g_pd3dDevice->SetTextureStageState( tu, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		g_pd3dDevice->SetTextureStageState( tu, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
		g_pd3dDevice->SetTextureStageState( tu, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
		break;
	case TextureMode_Add:
		g_pd3dDevice->SetTextureStageState( tu, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		g_pd3dDevice->SetTextureStageState( tu, D3DTSS_COLORARG2, D3DTA_CURRENT );
		g_pd3dDevice->SetTextureStageState( tu, D3DTSS_COLOROP,   D3DTOP_ADD );
		g_pd3dDevice->SetTextureStageState( tu, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		g_pd3dDevice->SetTextureStageState( tu, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
		g_pd3dDevice->SetTextureStageState( tu, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
		break;
	case TextureMode_Glow:
		g_pd3dDevice->SetTextureStageState( tu, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		g_pd3dDevice->SetTextureStageState( tu, D3DTSS_COLORARG2, D3DTA_CURRENT );
		g_pd3dDevice->SetTextureStageState( tu, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
		g_pd3dDevice->SetTextureStageState( tu, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		g_pd3dDevice->SetTextureStageState( tu, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
		g_pd3dDevice->SetTextureStageState( tu, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
		break;
	}
}

void RageDisplay_D3D::SetTextureFiltering( TextureUnit tu, bool b )
{
	if( tu >= (int) g_DeviceCaps.MaxSimultaneousTextures ) // not supported
		return;

	g_pd3dDevice->SetSamplerState( tu, D3DSAMP_MINFILTER, b ? D3DTEXF_LINEAR : D3DTEXF_POINT );
	g_pd3dDevice->SetSamplerState( tu, D3DSAMP_MAGFILTER, b ? D3DTEXF_LINEAR : D3DTEXF_POINT );
}

void RageDisplay_D3D::SetBlendMode( BlendMode mode )
{
	g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );

	if( mode == BLEND_INVERT_DEST )
		g_pd3dDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_SUBTRACT );
	else
		g_pd3dDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );

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
	// This is not the right way to do BLEND_SUBTRACT.  This code is only here
	// to prevent crashing when someone tries to use it. -Kyz
	case BLEND_SUBTRACT:
		g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
		g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
		break;
	case BLEND_MODULATE:
		g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ZERO );
		g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR );
		break;
	case BLEND_COPY_SRC:
		g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ONE );
		g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
		break;
	/* Effects currently missing in D3D: BLEND_ALPHA_MASK, BLEND_ALPHA_KNOCK_OUT
	 * These two may require DirectX9 since D3DRS_SRCALPHA and D3DRS_DESTALPHA
	 * don't seem to exist in DX8. -aj */
	case BLEND_ALPHA_MASK:
		// RGB: iSourceRGB = GL_ZERO; iDestRGB = GL_ONE;
		g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ZERO );
		g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		// Alpha: iSourceAlpha = GL_ZERO; iDestAlpha = GL_SRC_ALPHA;
		/*
		g_pd3dDevice->SetRenderState( D3DRS_SRCALPHA,  D3DBLEND_ZERO );
		g_pd3dDevice->SetRenderState( D3DRS_DESTALPHA, D3DBLEND_SRCALPHA );
		*/
		break;
	case BLEND_ALPHA_KNOCK_OUT:
		// RGB: iSourceRGB = GL_ZERO; iDestRGB = GL_ONE;
		g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ZERO );
		g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		// Alpha: iSourceAlpha = GL_ZERO; iDestAlpha = GL_ONE_MINUS_SRC_ALPHA;
		/*
		g_pd3dDevice->SetRenderState( D3DRS_SRCBLENDALPHA,  D3DBLEND_ZERO );
		g_pd3dDevice->SetRenderState( D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA );
		*/
		break;
	case BLEND_ALPHA_MULTIPLY:
		g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
		g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
		break;
	case BLEND_WEIGHTED_MULTIPLY:
		g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_DESTCOLOR );
		g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR );
		break;
	case BLEND_INVERT_DEST:
		g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ONE );
		g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		break;
	case BLEND_NO_EFFECT:
		g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ZERO );
		g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		break;
	default:
		FAIL_M(ssprintf("Invalid BlendMode: %i", mode));
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
	D3DVIEWPORT9 viewData;
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
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
	DWORD dw;
	switch( mode )
	{
	case ZTEST_OFF:			dw = D3DCMP_ALWAYS;		break;
	case ZTEST_WRITE_ON_PASS:	dw = D3DCMP_LESSEQUAL;	break;
	case ZTEST_WRITE_ON_FAIL:	dw = D3DCMP_GREATER;	break;
	default:
		dw = D3DCMP_NEVER;
		FAIL_M(ssprintf("Invalid ZTestMode: %i", mode));
	}
	g_pd3dDevice->SetRenderState( D3DRS_ZFUNC, dw );
}

void RageDisplay_D3D::ClearZBuffer()
{
	g_pd3dDevice->Clear( 0, nullptr, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,0), 1.0f, 0x00000000 );
}

void RageDisplay_D3D::SetTextureWrapping( TextureUnit tu, bool b )
{
	if( tu >= (int) g_DeviceCaps.MaxSimultaneousTextures )	// not supported
		return;

	int mode = b ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP;
	g_pd3dDevice->SetSamplerState( tu, D3DSAMP_ADDRESSU, mode );
	g_pd3dDevice->SetSamplerState( tu, D3DSAMP_ADDRESSV, mode );
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
	 * We want to still be able to color models with lighting off, so shove the
	 * material color in texture factor and modify the texture stage to use it
	 * instead of the vertex color (our models don't have vertex coloring anyway). */
	DWORD bLighting;
	g_pd3dDevice->GetRenderState( D3DRS_LIGHTING, &bLighting );

	if( bLighting )
	{
		D3DMATERIAL9 mat;
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

	D3DLIGHT9 light;
	ZERO( light );
	light.Type = D3DLIGHT_DIRECTIONAL;

	/* Z for lighting is flipped for D3D compared to OpenGL.
	 * XXX: figure out exactly why this is needed. Our transforms are probably
	 * goofed up, but the Z test is the same for both API's, so I'm not sure
	 * why we don't see other weirdness. -Chris */
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
		FAIL_M(ssprintf("Invalid CullMode: %i", mode));
	}
}

void RageDisplay_D3D::DeleteTexture( std::uintptr_t iTexHandle )
{
	if( iTexHandle == 0 )
		return;

	IDirect3DTexture9* pTex = reinterpret_cast<IDirect3DTexture9*>(iTexHandle);
	pTex->Release();

	// Delete palette (if any)
	if( g_TexResourceToPaletteIndex.find(iTexHandle) != g_TexResourceToPaletteIndex.end() )
		g_TexResourceToPaletteIndex.erase( g_TexResourceToPaletteIndex.find(iTexHandle) );
	if( g_TexResourceToTexturePalette.find(iTexHandle) != g_TexResourceToTexturePalette.end() )
		g_TexResourceToTexturePalette.erase( g_TexResourceToTexturePalette.find(iTexHandle) );
}


std::uintptr_t RageDisplay_D3D::CreateTexture(
	RagePixelFormat pixfmt,
	RageSurface* img,
	bool bGenerateMipMaps )
{
	HRESULT hr;
	IDirect3DTexture9* pTex;
	hr = g_pd3dDevice->CreateTexture( power_of_two(img->w), power_of_two(img->h), 1, 0, D3DFORMATS[pixfmt], D3DPOOL_MANAGED, &pTex, nullptr );

	if( FAILED(hr) )
		RageException::Throw( "CreateTexture(%i,%i,%s) failed: %s",
		img->w, img->h, RagePixelFormatToString(pixfmt).c_str(), GetErrorString(hr).c_str() );

	std::uintptr_t uTexHandle = reinterpret_cast<std::uintptr_t>(pTex);

	if( pixfmt == RagePixelFormat_PAL )
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
	std::uintptr_t uTexHandle,
	RageSurface* img,
	int xoffset, int yoffset, int width, int height )
{
	IDirect3DTexture9* pTex = reinterpret_cast<IDirect3DTexture9*>(uTexHandle);
	ASSERT( pTex != nullptr );

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

	// Copy bits
	int texpixfmt;
	for(texpixfmt = 0; texpixfmt < NUM_RagePixelFormat; ++texpixfmt)
		if(D3DFORMATS[texpixfmt] == desc.Format) break;
	ASSERT( texpixfmt != NUM_RagePixelFormat );

	RageSurface *Texture = CreateSurfaceFromPixfmt(RagePixelFormat(texpixfmt), lr.pBits, width, height, lr.Pitch);
	ASSERT( Texture != nullptr );
	RageSurfaceUtils::Blit( img, Texture, width, height );

	delete Texture;

	pTex->UnlockRect( 0 );
}

void RageDisplay_D3D::SetAlphaTest( bool b )
{
	g_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, b );
	g_pd3dDevice->SetRenderState( D3DRS_ALPHAREF, 0 );
	g_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );
}

RageMatrix RageDisplay_D3D::GetOrthoMatrix( float l, float r, float b, float t, float zn, float zf )
{
	RageMatrix m = RageDisplay::GetOrthoMatrix( l, r, b, t, zn, zf );

	// Convert from OpenGL's [-1,+1] Z values to D3D's [0,+1].
	RageMatrix tmp;
	RageMatrixScaling( &tmp, 1, 1, 0.5f );
	RageMatrixMultiply( &m, &tmp, &m );

	RageMatrixTranslation( &tmp, 0, 0, 0.5f );
	RageMatrixMultiply( &m, &tmp, &m );

	return m;
}

void RageDisplay_D3D::SetSphereEnvironmentMapping( TextureUnit tu, bool b )
{
	g_bSphereMapping[tu] = b;
}

void RageDisplay_D3D::SetCelShaded( int stage )
{
	// todo: implement me!
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
