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

#include "arch/arch.h"

#pragma comment(lib, "D3d8.lib")
#pragma comment(lib, "D3dx8.lib")

#include <math.h>

RageDisplay*		DISPLAY	= NULL;

//
// Globals
//
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
int GetUnusedPaletteIndex()
{
	for( int i=0; i<256; i++ )
	{
		bool bFreeToUse = true;
		for( map<unsigned,int>::const_iterator iter=g_TexResourceToPaletteIndex.begin();
			iter!=g_TexResourceToPaletteIndex.end();
			iter++ )
		{
			if( iter->second == i )
			{
				bFreeToUse = false;
				break;
			}
		}
		if( bFreeToUse )
			return i;
	}
	ASSERT( 0 );	// couldn't find a free palette index.  Resource leak?
	return 0;
}

#define D3DFVF_RAGEVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1)	// D3D FVF flags which describe our vertex structure


const PixelFormatDesc PIXEL_FORMAT_DESC[NUM_PIX_FORMATS] = {
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


RageDisplay::RageDisplay( bool windowed, int width, int height, int bpp, int rate, bool vsync )
{
	LOG->Trace( "RageDisplay::RageDisplay()" );

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


	g_pd3d = Direct3DCreate8( D3D_SDK_VERSION );

	if( FAILED( g_pd3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &g_DeviceCaps) ) )
		throw RageException( 
			"There was an error while initializing your video card.\n\n"
			"Your system is reporting that Direct3D8 hardware acceleration\n"
			"is not available.  In most cases, you can download an updated\n"
			"driver from your card's manufacturer."
		);

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

	// Save the original desktop format.
	g_pd3d->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &g_DesktopMode );

	SetVideoMode( windowed, width, height, bpp, rate, vsync );
}

void RageDisplay::Update(float fDeltaTime)
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

bool RageDisplay::IsSoftwareRenderer()
{
	return false;
}

RageDisplay::~RageDisplay()
{
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_EventState(SDL_VIDEORESIZE, SDL_IGNORE);

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
		throw RageException( ssprintf("Invalid BPP '%u' specified", iBPP) );

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

	RageException::Throw( "Couldn't find an appropriate back buffer format." );
}

#ifndef _XBOX
HWND GetHwnd()
{
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if( SDL_GetWMInfo(&info) < 0 ) 
		RageException::Throw( "SDL_GetWMInfo failed" );
	return info.window;
}
#endif

/* Set the video mode. */
bool RageDisplay::SetVideoMode( bool windowed, int width, int height, int bpp, int rate, bool vsync )
{
	int flags = SDL_RESIZABLE;
	if( !windowed )
		flags |= SDL_FULLSCREEN;

	g_Windowed = windowed;
	SDL_ShowCursor( g_Windowed );

	SDL_Surface *screen = SDL_SetVideoMode(width, height, bpp, flags);
	if(!screen)
		RageException::Throw("SDL_SetVideoMode failed: %s", SDL_GetError());

	g_CurrentWidth = screen->w;
	g_CurrentHeight = screen->h;
	g_CurrentBPP = bpp;


    ZeroMemory( &g_d3dpp, sizeof(g_d3dpp) );
	g_d3dpp.BackBufferWidth			=	width;
    g_d3dpp.BackBufferHeight		=	height;
    g_d3dpp.BackBufferFormat		=	FindBackBufferType( windowed, bpp );
    g_d3dpp.BackBufferCount			=	1;
    g_d3dpp.MultiSampleType			=	D3DMULTISAMPLE_NONE;
	g_d3dpp.SwapEffect				=	D3DSWAPEFFECT_DISCARD;
#ifndef _XBOX
	g_d3dpp.hDeviceWindow			=	GetHwnd();
#endif
    g_d3dpp.Windowed				=	windowed;
    g_d3dpp.EnableAutoDepthStencil	=	TRUE;
    g_d3dpp.AutoDepthStencilFormat	=	D3DFMT_D16;
    g_d3dpp.Flags					=	0;
	g_d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	/* Windowed must always use D3DPRESENT_INTERVAL_DEFAULT. */
	g_d3dpp.FullScreen_PresentationInterval = 
		(windowed || vsync) ? D3DPRESENT_INTERVAL_DEFAULT : D3DPRESENT_INTERVAL_IMMEDIATE;

	if( g_pd3dDevice == NULL )		// device is not yet created.  We need to create it
	{
		g_pd3d->CreateDevice(
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
		ASSERT( g_pd3dDevice );
	}
	else
	{
		g_pd3dDevice->Reset( &g_d3dpp );
	}
	
	ResolutionChanged();

	this->SetDefaultRenderStates();
	
	g_pd3dDevice->BeginScene();

	return false;	// we can always reuse the D3D device
}

void RageDisplay::ResolutionChanged()
{
	// no need to clear because D3D uses an overlay
//	SetViewport(0,0);
//
//	/* Clear any junk that's in the framebuffer. */
//	Clear();
//	Flip();
}

void RageDisplay::SetViewport(int shift_left, int shift_down)
{
	/* left and down are on a 0..SCREEN_WIDTH, 0..SCREEN_HEIGHT scale.
	 * Scale them to the actual viewport range. */
	shift_left = int( shift_left * float(g_CurrentWidth) / SCREEN_WIDTH );
	shift_down = int( shift_down * float(g_CurrentHeight) / SCREEN_HEIGHT );

	D3DVIEWPORT8 viewData = { shift_left, -shift_down, g_CurrentWidth, g_CurrentHeight, 0.f, 1.f };
	g_pd3dDevice->SetViewport( &viewData );
}

int RageDisplay::GetMaxTextureSize() const
{
	return g_DeviceCaps.MaxTextureWidth;
}

void RageDisplay::Clear()
{
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
						 D3DCOLOR_XRGB(0,0,0), 1.0f, 0x00000000 );
}

void RageDisplay::Flip()
{
	g_pd3dDevice->EndScene();
	g_pd3dDevice->Present( 0, 0, 0, 0 );
	g_pd3dDevice->BeginScene();
	ProcessStatsOnFlip();
}

D3DFORMAT D3DFORMATS[NUM_PIX_FORMATS] = 
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

bool RageDisplay::SupportsTextureFormat( PixelFormat pixfmt )
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

void RageDisplay::SaveScreenshot( CString sPath )
{
#ifndef _XBOX
	IDirect3DSurface8* pSurface;
	g_pd3dDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pSurface );
	D3DXSaveSurfaceToFile( sPath, D3DXIFF_BMP, pSurface, 0, NULL );
	pSurface->Release();
#endif
}


bool RageDisplay::IsWindowed() const { return g_Windowed; }
int RageDisplay::GetWidth() const { return g_CurrentWidth; }
int RageDisplay::GetHeight() const { return g_CurrentHeight; }
int RageDisplay::GetBPP() const { return g_CurrentBPP; }

#define SEND_CURRENT_MATRICES \
	g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)GetProjection() ); \
	RageMatrix m; \
	/* Convert to OpenGL-style "pixel-centered" coords */ \
	RageMatrixTranslation( &m, -0.5f, -0.5f, 0 ); \
	RageMatrixMultiply( &m, &m, GetModelViewTop() ); \
	g_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&m );


void RageDisplay::DrawQuads( const RageVertex v[], int iNumVerts )
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
void RageDisplay::DrawFan( const RageVertex v[], int iNumVerts )
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

void RageDisplay::DrawStrip( const RageVertex v[], int iNumVerts )
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

void RageDisplay::DrawTriangles( const RageVertex v[], int iNumVerts )
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

void RageDisplay::DrawIndexedTriangles( const RageVertex v[], const Uint16 pIndices[], int iNumIndices )
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

void RageDisplay::DrawLineStrip( const RageVertex v[], int iNumVerts, float LineWidth )
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

void RageDisplay::SetTexture( RageTexture* pTexture )
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
	if( g_TexResourceToPaletteIndex.find(uTexHandle) != g_TexResourceToPaletteIndex.end() )
	{
		int iPaletteIndex = g_TexResourceToPaletteIndex.find(uTexHandle)->second;
#ifndef _XBOX
		g_pd3dDevice->SetCurrentTexturePalette( iPaletteIndex );
#else
		ASSERT(0);
#endif
	}
}
void RageDisplay::SetTextureModeModulate()
{
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
}

void RageDisplay::SetTextureModeGlow(GlowMode m)
{
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );

}

void RageDisplay::SetTextureFiltering( bool b )
{
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, b ? D3DTEXF_LINEAR : D3DTEXF_POINT );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, b ? D3DTEXF_LINEAR : D3DTEXF_POINT );
}

void RageDisplay::SetBlendMode( BlendMode mode )
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

bool RageDisplay::IsZBufferEnabled() const
{
	DWORD b;
	g_pd3dDevice->GetRenderState( D3DRS_ZENABLE, &b );
	return b!=0;
}

void RageDisplay::SetZBuffer( bool b )
{
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE,      b );
	g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, b );
}
void RageDisplay::ClearZBuffer()
{
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,0), 1.0f, 0x00000000 );
}

void RageDisplay::SetTextureWrapping( bool b )
{
	int mode = b ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP;
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU, mode );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV, mode );
}

void RageDisplay::SetMaterial( 
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

void RageDisplay::SetLighting( bool b )
{
	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, b );
}

void RageDisplay::SetLightOff( int index )
{
	g_pd3dDevice->LightEnable( index, false );
}
void RageDisplay::SetLightDirectional( 
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

void RageDisplay::SetBackfaceCull( bool b )
{
	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, b ? D3DCULL_CW : D3DCULL_NONE );
}

void RageDisplay::DeleteTexture( unsigned uTexHandle )
{
	IDirect3DTexture8* pTex = (IDirect3DTexture8*) uTexHandle;
	pTex->Release();

	// Delete palette (if any)
	if( g_TexResourceToPaletteIndex.find(uTexHandle) != g_TexResourceToPaletteIndex.end() )
		g_TexResourceToPaletteIndex.erase( g_TexResourceToPaletteIndex.find(uTexHandle) );
}


unsigned RageDisplay::CreateTexture( 
	PixelFormat pixfmt,
	SDL_Surface*& img )
{
	// texture must be power of two
	ASSERT( img->w == power_of_two(img->w) );
	ASSERT( img->h == power_of_two(img->h) );


	HRESULT hr;
	IDirect3DTexture8* pTex;
	hr = g_pd3dDevice->CreateTexture( img->w, img->h, 1, 0, D3DFORMATS[pixfmt], D3DPOOL_MANAGED, &pTex );

	unsigned uTexHandle = (unsigned)pTex;

	if( pixfmt == FMT_PAL )
	{
		int iPalIndex = GetUnusedPaletteIndex();

		// Save palette
		PALETTEENTRY pal[256];
		memset( pal, 0, sizeof(pal) );
		for( int i=0; i<img->format->palette->ncolors; i++ )
		{
			SDL_Color c = img->format->palette->colors[i];
			pal[i].peRed = c.r;
			pal[i].peGreen = c.g;
			pal[i].peBlue = c.b;
			bool bIsColorKey = img->flags & SDL_SRCCOLORKEY && (unsigned)i == img->format->colorkey;
			pal[i].peFlags = bIsColorKey ? 0x00 : 0xFF;
		}
#ifndef _XBOX
		g_pd3dDevice->SetPaletteEntries( iPalIndex, pal );
#else
		ASSERT(0);
#endif

		ASSERT( g_TexResourceToPaletteIndex.find(uTexHandle) == g_TexResourceToPaletteIndex.end() );
		g_TexResourceToPaletteIndex[uTexHandle] = iPalIndex;
	}

	UpdateTexture( uTexHandle, pixfmt, img, 0, 0, img->w, img->h );

	return uTexHandle;
}

void RageDisplay::UpdateTexture( 
	unsigned uTexHandle, 
	PixelFormat pixfmt,
	SDL_Surface*& img,
	int xoffset, int yoffset, int width, int height )
{
	IDirect3DTexture8* pTex = (IDirect3DTexture8*)uTexHandle;

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

void RageDisplay::SetAlphaTest( bool b )
{
	g_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  b );
	g_pd3dDevice->SetRenderState( D3DRS_ALPHAREF,         0 );
	g_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );
}
