#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: RageDisplay

 Desc: See header.

 Copyright (c) 2004 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "SDL_utils.h"
#include "RageFile.h"

#include "RageDisplay.h"
#include "RageDisplay_Null.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageTimer.h"
#include "RageTexture.h"
#include "RageTextureManager.h"
#include "RageMath.h"
#include "RageTypes.h"
#include "StepMania.h"
#include "RageUtil.h"

static RageDisplay::PixelFormatDesc PIXEL_FORMAT_DESC[RageDisplay::NUM_PIX_FORMATS] = {
	{
		/* R8G8B8A8 */
		32,
		{ 0xFF000000,
		  0x00FF0000,
		  0x0000FF00,
		  0x000000FF }
	}, {
		/* R4G4B4A4 */
		16,
		{ 0xF000,
		  0x0F00,
		  0x00F0,
		  0x000F },
	}, {
		/* R5G5B5A1 */
		16,
		{ 0xF800,
		  0x07C0,
		  0x003E,
		  0x0001 },
	}, {
		/* R5G5B5 */
		16,
		{ 0xF800,
		  0x07C0,
		  0x003E,
		  0x0000 },
	}, {
		/* R8G8B8 */
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
		/* B8G8R8A8 */
		24,
		{ 0x0000FF,
		  0x00FF00,
		  0xFF0000,
		  0x000000 }
	}, {
		/* A1B5G5R5 */
		16,
		{ 0x7C00,
		  0x03E0,
		  0x001F,
		  0x8000 },
	}
};


RageDisplay_Null::RageDisplay_Null( VideoModeParams p )
{
	LOG->MapLog("renderer", "Current renderer: null");
	SetVideoMode( p );
}

SDL_Surface* RageDisplay_Null::CreateScreenshot()
{
	const PixelFormatDesc &desc = PIXEL_FORMAT_DESC[FMT_RGB8];
	SDL_Surface *image = SDL_CreateRGBSurfaceSane(
		SDL_SWSURFACE, 640, 480,
		desc.bpp, desc.masks[0], desc.masks[1], desc.masks[2], desc.masks[3] );

	memset( image->pixels, 0, 480*image->pitch );

	return image;
}

const RageDisplay::PixelFormatDesc *RageDisplay_Null::GetPixelFormatDesc(PixelFormat pf) const
{
	ASSERT( pf < NUM_PIX_FORMATS );
	return &PIXEL_FORMAT_DESC[pf];
}


RageMatrix RageDisplay_Null::GetOrthoMatrix( float l, float r, float b, float t, float zn, float zf )
{
	RageMatrix m(
		2/(r-l),      0,            0,           0,
		0,            2/(t-b),      0,           0,
		0,            0,            -2/(zf-zn),   0,
		-(r+l)/(r-l), -(t+b)/(t-b), -(zf+zn)/(zf-zn),  1 );
	return m;
}


void RageDisplay_Null::EndFrame()
{
	ProcessStatsOnFlip();
}
	

class RageModelVertexArrayNull : public RageModelVertexArray
{
public:
	RageModelVertexArrayNull()
	{
		m_sizeVerts = 0;
		m_sizeTriangles = 0;
		m_pPosition = NULL;
		m_pTexture = NULL;
		m_pNormal = NULL;
		m_pBone = NULL;
		m_pTriangles = NULL;
	}

	~RageModelVertexArrayNull()
	{
		m_sizeVerts = 0;
		m_sizeTriangles = 0;
		SAFE_DELETE( m_pPosition );
		SAFE_DELETE( m_pTexture );
		SAFE_DELETE( m_pNormal );
		SAFE_DELETE( m_pBone );
		SAFE_DELETE( m_pTriangles );
	}
	
	size_t sizeVerts() const
	{
		return m_sizeVerts;
	}
	void resizeVerts( size_t size )
	{
		SAFE_DELETE( m_pPosition );
		SAFE_DELETE( m_pTexture );
		SAFE_DELETE( m_pNormal );
		SAFE_DELETE( m_pBone );

		m_sizeVerts = size;
		m_pPosition = new RageVector3[size];
		m_pTexture = new RageVector2[size];
		m_pNormal = new RageVector3[size];
		m_pBone = new Sint8[size];
	}

	size_t sizeTriangles() const
	{
		return m_sizeTriangles;
	}
	void resizeTriangles( size_t size )
	{
		SAFE_DELETE( m_pTriangles );

		m_sizeTriangles = size;
		m_pTriangles = new msTriangle[size];
	}

	RageVector3&	Position	( int index ) { return m_pPosition[index]; }
	RageVector2&	TexCoord	( int index ) { return m_pTexture[index]; }
	RageVector3&	Normal		( int index ) { return m_pNormal[index]; }
	Sint8&			Bone		( int index ) { return m_pBone[index]; }
	msTriangle&		Triangle	( int index ) { return m_pTriangles[index]; }

	void OnChanged() const { }
	void Draw() const { }

protected:
	size_t		m_sizeVerts;
	size_t		m_sizeTriangles;
	RageVector3 *m_pPosition;
	RageVector2 *m_pTexture;
	RageVector3 *m_pNormal;
	Sint8		*m_pBone;
	msTriangle	*m_pTriangles;
};

RageModelVertexArray* RageDisplay_Null::CreateRageModelVertexArray()
{
	return new RageModelVertexArrayNull;
}

void RageDisplay_Null::DeleteRageModelVertexArray( RageModelVertexArray* p )
{
}
