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
#include "RageMath.h"
#include "SDL.h"
#include "SDL_opengl.h"
#include "RageTypes.h"
#include "GameConstantsAndTypes.h"


RageDisplay*		DISPLAY	= NULL;


////////////
// Globals
////////////
const int MAX_NUM_VERTICIES = 1000;
SDL_Surface			*g_screen = NULL;		// this class is a singleton, so there can be only one
vector<RageMatrix>	g_matModelStack;	// model matrix stack
RageMatrix			g_matView;			// view matrix
RageMatrix&			GetTopModelMatrix() { return g_matModelStack.back(); }
int					g_flags = 0;		/* SDL video flags */
GLenum				g_vertMode = GL_TRIANGLES;
RageVertex			g_vertQueue[MAX_NUM_VERTICIES];
RageTimer			g_LastCheckTimer;
int					g_iNumVerts;
float				g_fLastCheckTime;
int					g_iFramesRenderedSinceLastCheck;
int					g_iVertsRenderedSinceLastCheck;
int					g_iDrawsSinceLastCheck;
int					g_iFPS, g_iTPF, g_iDPF;


RageDisplay::RageDisplay( bool windowed, int width, int height, int bpp, RefreshRateMode rate, bool vsync )
{
//	LOG->Trace( "RageDisplay::RageDisplay()" );

	SDL_InitSubSystem(SDL_INIT_VIDEO);
	
	SetVideoMode( windowed, width, height, bpp, rate, vsync );
}

RageDisplay::~RageDisplay()
{
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


//-----------------------------------------------------------------------------
// Name: SwitchDisplayMode()
// Desc:
//-----------------------------------------------------------------------------
void RageDisplay::SetVideoMode( bool windowed, int width, int height, int bpp, RefreshRateMode rate, bool vsync )
{
//	LOG->Trace( "RageDisplay::SetVideoMode( %d, %d, %d, %d, %d, %d )", windowed, width, height, bpp, rate, vsync );


//	if( windowed )
//		flags |= SDL_FULLSCREEN;
	g_flags |= SDL_DOUBLEBUF;
	g_flags |= SDL_RESIZABLE;
	g_flags |= SDL_OPENGL;

	SDL_ShowCursor( ~g_flags & SDL_FULLSCREEN );

	SDL_InitSubSystem(SDL_INIT_VIDEO);

	switch( bpp )
	{
	case 15:
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		break;
	case 16:
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		break;
	case 24:
	case 32:
	default:
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	}

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, bpp);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, TRUE);

	g_screen = SDL_SetVideoMode(width, height, bpp, g_flags);
	if(!g_screen)
		throw RageException("Failed to open screen!");

	/*
	 * Set up OpenGL for 2D rendering.
	 */
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glViewport(0, 0, g_screen->w, g_screen->h);

	/*
	 * Set state variables
	 */
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

//	glBegin( GL_TRIANGLES );
//	glColor4f( 1,1,1,1 );
//	glVertex3f( 0, 0, 0 ); 
//	glVertex3f( 320, 0, 0 ); 
//	glVertex3f( 320, 240, 0 );
//	glEnd();

//	SDL_GL_SwapBuffers();
}

int RageDisplay::GetMaxTextureSize() const
{
	GLint size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
	return size;
}

void RageDisplay::Clear()
{
	glClearColor( 0,0,0,1 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void RageDisplay::Flip()
{
	FlushQueue();
	SDL_GL_SwapBuffers();
	g_iFramesRenderedSinceLastCheck++;

	if( g_LastCheckTimer.PeekDeltaTime() >= 1.0f )	// update stats every 1 sec.
	{
		g_LastCheckTimer.GetDeltaTime();
		g_iFPS = g_iFramesRenderedSinceLastCheck;
		g_iFramesRenderedSinceLastCheck = 0;
		g_iTPF = g_iVertsRenderedSinceLastCheck / g_iFPS;
		g_iVertsRenderedSinceLastCheck = 0;
		g_iDPF = g_iDrawsSinceLastCheck / g_iFPS;
		g_iDrawsSinceLastCheck = 0;
		//LOG->Trace( "FPS: %d, TPF: %d, DPF: %d", m_iFPS, m_iTPF, m_iDPF );
	}

}


void RageDisplay::GetHzAtResolution(int width, int height, int bpp, CArray<int,int> &add) const 
{

}

bool RageDisplay::IsWindowed() const 
{
	return true; // FIXME
}
void RageDisplay::DrawQuad( const RageVertex v[4] )	// upper-left, upper-right, lower-left, lower-right
{
	DrawQuads( v, 4 );
}
void RageDisplay::DrawQuads( const RageVertex v[], int iNumVerts )
{
	if( g_vertMode != GL_QUADS )
		FlushQueue();

	ASSERT( (iNumVerts%4) == 0 );
	g_vertMode = GL_QUADS;
	AddVerts( v, iNumVerts );
}
void RageDisplay::DrawFan( const RageVertex v[], int iNumVerts )
{
	if( g_vertMode != GL_TRIANGLE_FAN )
		FlushQueue();

	ASSERT( iNumVerts >= 3 );
	g_vertMode = GL_TRIANGLE_FAN;
	AddVerts( v, iNumVerts );
	FlushQueue();
}
void RageDisplay::DrawStrip( const RageVertex v[], int iNumVerts )
{
	if( g_vertMode != GL_TRIANGLE_STRIP )
		FlushQueue();

	ASSERT( iNumVerts >= 3 );
	g_vertMode = GL_TRIANGLE_STRIP;
	AddVerts( v, iNumVerts );
	FlushQueue();
}
void RageDisplay::AddVerts( const RageVertex v[], int iNumVerts )
{
	for( int i=0; i<iNumVerts; i++ )
	{
		// Don't overflow the queue
		if( g_iNumVerts+1 > MAX_NUM_VERTICIES )	
			FlushQueue();

		// transform the verticies as we copy
		RageVec3TransformCoord( &g_vertQueue[g_iNumVerts].p, &v[i].p, &GetTopModelMatrix() ); 
		g_vertQueue[g_iNumVerts].c = v[i].c;
		g_vertQueue[g_iNumVerts].t = v[i].t;
		g_iNumVerts++; 
	}
}

void RageDisplay::FlushQueue()
{
	if( g_iNumVerts == 0 )
		return;

	glInterleavedArrays( GL_T2F_C4UB_V3F, sizeof(RageVertex), g_vertQueue );
	glDrawArrays( g_vertMode, 0, g_iNumVerts );

	g_iVertsRenderedSinceLastCheck += g_iNumVerts;
	g_iNumVerts = 0;

	g_iDrawsSinceLastCheck++;
}

void RageDisplay::SetViewTransform( const RageMatrix* pMatrix )
{
	FlushQueue();
	// OpenGL doesn't have a separate view matrix.  We need to save it and muliply in later
	g_matView = *pMatrix;
}
void RageDisplay::SetProjectionTransform( const RageMatrix* pMatrix )
{
	FlushQueue();
	glMatrixMode( GL_MODELVIEW );
	glLoadMatrixf( (float*)pMatrix );
}
void RageDisplay::GetViewTransform( RageMatrix* pMatrixOut )
{
	*pMatrixOut = g_matView;
}
void RageDisplay::GetProjectionTransform( RageMatrix* pMatrixOut )
{
	glGetFloatv( GL_PROJECTION_MATRIX, (float*)pMatrixOut );
}

void RageDisplay::ResetMatrixStack() 
{ 
	RageMatrix ident;
	RageMatrixIdentity( &ident );
	g_matModelStack.clear();
	g_matModelStack.push_back(ident);
}

void RageDisplay::PushMatrix() 
{ 
	g_matModelStack.push_back( GetTopModelMatrix() );	
	ASSERT(g_matModelStack.size()<20);		// check for infinite loop
}

void RageDisplay::PopMatrix() 
{ 
	g_matModelStack.erase( g_matModelStack.end()-1, g_matModelStack.end() ); 
	ASSERT(g_matModelStack.size()>=1);	// popped a matrix we didn't push
}

void RageDisplay::Translate( float x, float y, float z )
{
	RageMatrix matTemp;
	RageMatrixTranslation( &matTemp, x, y, z );
	RageMatrix& matTop = GetTopModelMatrix();
	RageMatrixMultiply( &matTop, &matTemp, &matTop );
}

void RageDisplay::TranslateLocal( float x, float y, float z )
{
	RageMatrix matTemp;
	RageMatrixTranslation( &matTemp, x, y, z );
	RageMatrix& matTop = GetTopModelMatrix();
	RageMatrixMultiply( &matTop, &matTop, &matTemp );
}

void RageDisplay::Scale( float x, float y, float z )
{
	RageMatrix matTemp;
	RageMatrixScaling( &matTemp, x, y, z );
	RageMatrix& matTop = GetTopModelMatrix();
	RageMatrixMultiply( &matTop, &matTemp, &matTop );
}

void RageDisplay::RotateX( float r )
{
	RageMatrix matTemp;
	RageMatrixRotationX( &matTemp, r );
	RageMatrix& matTop = GetTopModelMatrix();
	RageMatrixMultiply( &matTop, &matTemp, &matTop );
}

void RageDisplay::RotateY( float r )
{
	RageMatrix matTemp;
	RageMatrixRotationY( &matTemp, r );
	RageMatrix& matTop = GetTopModelMatrix();
	RageMatrixMultiply( &matTop, &matTemp, &matTop );
}

void RageDisplay::RotateZ( float r )
{
	RageMatrix matTemp;
	RageMatrixRotationZ( &matTemp, r );
	RageMatrix& matTop = GetTopModelMatrix();
	RageMatrixMultiply( &matTop, &matTemp, &matTop );
}

void RageDisplay::SetTexture( RageTexture* pTexture )
{
	static int iLastTexID = 0;
	int iNewTexID = pTexture ? pTexture->GetGLTextureID() : 0;

	if( iLastTexID != iNewTexID )
		FlushQueue();
	iLastTexID = iNewTexID;

	glBindTexture( GL_TEXTURE_2D, iNewTexID );
}
void RageDisplay::SetTextureModeModulate()
{
	int a;
	glGetTexEnviv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &a );

	if( a != GL_MODULATE )
		FlushQueue();

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void RageDisplay::SetTextureModeGlow()
{
	int a, b;
	glGetIntegerv( GL_BLEND_SRC, &a );
	glGetIntegerv( GL_BLEND_DST, &b );

	if( a!=GL_SRC_ALPHA || b!=GL_ONE )
		FlushQueue();

	glBlendFunc( GL_SRC_ALPHA, GL_ONE );
}
void RageDisplay::SetBlendModeNormal()
{
	int a, b;
	glGetIntegerv( GL_BLEND_SRC, &a );
	glGetIntegerv( GL_BLEND_DST, &b );

	if( a!=GL_SRC_ALPHA || b!=GL_ONE_MINUS_SRC_ALPHA )
		FlushQueue();

	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
}
void RageDisplay::SetBlendModeAdd()
{
	int a, b;
	glGetIntegerv( GL_BLEND_SRC, &a );
	glGetIntegerv( GL_BLEND_DST, &b );

	if( a!=GL_ONE || b!=GL_ONE )
		FlushQueue();

	glBlendFunc( GL_ONE, GL_ONE );
}
void RageDisplay::EnableZBuffer()
{
	bool a;
	glGetBooleanv( GL_DEPTH_TEST, (unsigned char*)&a );

	if( !a )
		FlushQueue();

	glEnable( GL_DEPTH_TEST );
}
void RageDisplay::DisableZBuffer()
{
	bool a;
	glGetBooleanv( GL_DEPTH_TEST, (unsigned char*)&a );

	if( a )
		FlushQueue();

	glDisable( GL_DEPTH_TEST );
}
void RageDisplay::EnableTextureWrapping()
{
	int a, b;
	glGetTexParameteriv( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &a );
	glGetTexParameteriv( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &b );

	if( a!=GL_REPEAT || b!=GL_REPEAT )
		FlushQueue();

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
}
void RageDisplay::DisableTextureWrapping()
{
	int a, b;
	glGetTexParameteriv( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &a );
	glGetTexParameteriv( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &b );

	if( a!=GL_CLAMP || b!=GL_CLAMP )
		FlushQueue();

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
}
