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
#include "RageDisplayInternal.h"

#include "RageUtil.h"
#include "RageLog.h"
#include "RageTimer.h"
#include "RageException.h"
#include "RageTexture.h"
#include "RageTextureManager.h"
#include "RageMath.h"
#include "RageTypes.h"
#include "GameConstantsAndTypes.h"

RageDisplay*		DISPLAY	= NULL;

////////////
// Globals
////////////
SDL_Surface			*g_screen = NULL;		// this class is a singleton, so there can be only one
int					g_flags = 0;		/* SDL video flags */
GLenum				g_vertMode = GL_TRIANGLES;
RageTimer			g_LastCheckTimer;
int					g_iNumVerts;
int					g_iFPS, g_iVPF;

int					g_PerspectiveMode = 0;

int					g_CurrentHeight, g_CurrentWidth, g_CurrentBPP;
int					g_ModelMatrixCnt=0;
int RageDisplay::GetFPS() const { return g_iFPS; }
int RageDisplay::GetVPF() const { return g_iVPF; }

static int			g_iFramesRenderedSinceLastCheck,
					g_iVertsRenderedSinceLastCheck;

PWSWAPINTERVALEXTPROC GLExt::wglSwapIntervalEXT;

const GLenum RageVertexFormat = GL_T2F_C4F_N3F_V3F;

void GetGLExtensions(set<string> &ext)
{
    const char *buf = (const char *)glGetString(GL_EXTENSIONS);

	vector<CString> lst;
	split(buf, " ", lst);

	for(unsigned i = 0; i < lst.size(); ++i)
		ext.insert(lst[i]);
    LOG->Trace("OpenGL extensions: %s", buf);
}

RageDisplay::RageDisplay( bool windowed, int width, int height, int bpp, int rate, bool vsync )
{
//	LOG->Trace( "RageDisplay::RageDisplay()" );
	m_oglspecs = new oglspecs_t;
	
	SDL_InitSubSystem(SDL_INIT_VIDEO);

	SetVideoMode( windowed, width, height, bpp, rate, vsync );

	SetupOpenGL();

	/* Log this, so if people complain that the radar looks bad on their
	 * system we can compare them: */
	glGetFloatv(GL_LINE_WIDTH_RANGE, m_oglspecs->line_range);
	LOG->Trace("Line width range: %f, %f", m_oglspecs->line_range[0], m_oglspecs->line_range[1]);
	glGetFloatv(GL_LINE_WIDTH_GRANULARITY, &m_oglspecs->line_granularity);
	LOG->Trace("Line width granularity: %f", m_oglspecs->line_granularity);
	glGetFloatv(GL_POINT_SIZE_RANGE, m_oglspecs->point_range);
	LOG->Trace("Point size range: %f-%f", m_oglspecs->point_range[0], m_oglspecs->point_range[1]);
	glGetFloatv(GL_POINT_SIZE_GRANULARITY, &m_oglspecs->point_granularity);
	LOG->Trace("Point size granularity: %f", m_oglspecs->point_granularity);
}

void RageDisplay::SetupOpenGL()
{
	/*
	 * Set up OpenGL for 2D rendering.
	 */
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	/*
	 * Set state variables
	 */
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	/* Use <= for depth testing.  This lets us set all components of an actor to the
	 * same depth. */
	glDepthFunc(GL_LEQUAL);

	/* Line antialiasing is fast on most hardware, and saying "don't care"
	 * should turn it off if it isn't. */
	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	glHint(GL_POINT_SMOOTH_HINT, GL_DONT_CARE);

	/* Initialize the default ortho projection. */
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, SCREEN_NEAR, SCREEN_FAR );
	glMatrixMode( GL_MODELVIEW );
}

RageDisplay::~RageDisplay()
{
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	delete m_oglspecs;
}

bool RageDisplay::HasExtension(CString ext) const
{
	return m_oglspecs->glExts.find(ext) != m_oglspecs->glExts.end();
}

void RageDisplay::SetupExtensions()
{
	double fGLVersion = atof( (const char *) glGetString(GL_VERSION) );
	m_oglspecs->glVersion = int(roundf(fGLVersion * 10));
	LOG->Trace( "OpenGL version %.1f", m_oglspecs->glVersion / 10.);
	GetGLExtensions(m_oglspecs->glExts);

	/* Check for extensions: */
	m_oglspecs->EXT_texture_env_combine = HasExtension("GL_EXT_texture_env_combine");
	m_oglspecs->ARB_texture_compression = HasExtension("GL_ARB_texture_compression");
	m_oglspecs->EXT_texture_compression_s3tc = HasExtension("GL_EXT_texture_compression_s3tc");
	m_oglspecs->WGL_EXT_swap_control = HasExtension("WGL_EXT_swap_control");

	/* Find extension functions. */
	wglSwapIntervalEXT = (PWSWAPINTERVALEXTPROC) SDL_GL_GetProcAddress("wglSwapIntervalEXT");
	
	/* Make sure we have all components for detected extensions. */
	if(m_oglspecs->WGL_EXT_swap_control)
		ASSERT(wglSwapIntervalEXT);
}

/* Set the video mode.  In some cases, changing the video mode will reset
 * the rendering context; returns true if we need to reload textures. */
bool RageDisplay::SetVideoMode( bool windowed, int width, int height, int bpp, int rate, bool vsync )
{
//	LOG->Trace( "RageDisplay::SetVideoMode( %d, %d, %d, %d, %d, %d )", windowed, width, height, bpp, rate, vsync );

	g_flags = 0;
	if( !windowed )
		g_flags |= SDL_FULLSCREEN;
	g_flags |= SDL_DOUBLEBUF;
	g_flags |= SDL_RESIZABLE;
	g_flags |= SDL_OPENGL;

	SDL_ShowCursor( ~g_flags & SDL_FULLSCREEN );

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

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, TRUE);

#ifdef SDL_HAS_REFRESH_RATE
	if(rate == REFRESH_DEFAULT)
		SDL_SM_SetRefreshRate(0);
	else
		SDL_SM_SetRefreshRate(rate);
#endif

	bool need_reload = false;

#ifndef SDL_HAS_CHANGEVIDEOMODE
	/* We can't change the video mode without nuking the GL context. */
	need_reload = true;
#endif

	if( bpp != g_CurrentBPP )
		need_reload = true; /* can't do this with SDL_SM_ChangeVideoMode_OpenGL */

	if(need_reload) {
		if(TEXTUREMAN)
			TEXTUREMAN->InvalidateTextures();
	}

	if(!g_screen || need_reload) {
		g_screen = SDL_SetVideoMode(width, height, bpp, g_flags);
		if(!g_screen)
			RageException::Throw("SDL_SetVideoMode failed: %s", SDL_GetError());

		SDL_WM_SetCaption("StepMania", "StepMania");
	}
#ifdef SDL_HAS_CHANGEVIDEOMODE
	else
	{
		SDL_SM_ChangeVideoMode_OpenGL(g_flags, g_screen, width, height);
	}
#endif

	/* Now that we've initialized, we can search for extensions (some of which
	 * we may need to set up the video mode). */
	SetupExtensions();

	/* Set vsync the Windows way, if we can.  (What other extensions are there
	 * to do this, for other archs?) */
	if(m_oglspecs->WGL_EXT_swap_control) {
	    wglSwapIntervalEXT(vsync);
	}

	if(need_reload) {
		/* OpenGL state was lost; set up again. */
		SetupOpenGL();
	}
	
	g_CurrentWidth = g_screen->w;
	g_CurrentHeight = g_screen->h;
	g_CurrentBPP = bpp;

	SetViewport(0,0);

	/* Clear any junk that's in the framebuffer. */
	Clear();
	Flip();

	return need_reload;
}

void RageDisplay::ResolutionChanged(int width, int height)
{
	g_CurrentWidth = width;
	g_CurrentHeight = height;

	SetViewport(0,0);

	/* Clear any junk that's in the framebuffer. */
	Clear();
	Flip();
}

void RageDisplay::SetViewport(int shift_left, int shift_down)
{
	/* left and down are on a 0..SCREEN_WIDTH, 0..SCREEN_HEIGHT scale.
	 * Scale them to the actual viewport range. */
	shift_left = int( shift_left * float(g_CurrentWidth) / SCREEN_WIDTH );
	shift_down = int( shift_down * float(g_CurrentWidth) / SCREEN_WIDTH );

	glViewport(shift_left, -shift_down, g_CurrentWidth, g_CurrentHeight);
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
	SDL_GL_SwapBuffers();
	g_iFramesRenderedSinceLastCheck++;

	if( g_LastCheckTimer.PeekDeltaTime() >= 1.0f )	// update stats every 1 sec.
	{
		g_LastCheckTimer.GetDeltaTime();
		g_iFPS = g_iFramesRenderedSinceLastCheck;
		g_iVPF = g_iVertsRenderedSinceLastCheck / g_iFPS;
		g_iFramesRenderedSinceLastCheck = g_iVertsRenderedSinceLastCheck = 0;
		LOG->Trace( "FPS: %d, VPF: %d", g_iFPS, g_iVPF );
	}

}

void RageDisplay::ResetStats()
{
	g_iFPS = g_iVPF = 0;
	g_iFramesRenderedSinceLastCheck = g_iVertsRenderedSinceLastCheck = 0;
	g_LastCheckTimer.GetDeltaTime();
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
	ASSERT( (iNumVerts%4) == 0 );

	glInterleavedArrays( RageVertexFormat, sizeof(RageVertex), v );
	glDrawArrays( GL_QUADS, 0, iNumVerts );

	g_iVertsRenderedSinceLastCheck += iNumVerts;
}
void RageDisplay::DrawFan( const RageVertex v[], int iNumVerts )
{
	ASSERT( iNumVerts >= 3 );
	glInterleavedArrays( RageVertexFormat, sizeof(RageVertex), v );
	glDrawArrays( GL_TRIANGLE_FAN, 0, iNumVerts );
	g_iVertsRenderedSinceLastCheck += iNumVerts;
}

void RageDisplay::DrawStrip( const RageVertex v[], int iNumVerts )
{
	ASSERT( iNumVerts >= 3 );
	glInterleavedArrays( RageVertexFormat, sizeof(RageVertex), v );
	glDrawArrays( GL_TRIANGLE_STRIP, 0, iNumVerts );
	g_iVertsRenderedSinceLastCheck += iNumVerts;
}

void RageDisplay::DrawLoop( const RageVertex v[], int iNumVerts, float LineWidth )
{
	ASSERT( iNumVerts >= 3 );

	glEnable(GL_LINE_SMOOTH);

	/* Our line width is wrt the regular internal SCREEN_WIDTHxSCREEN_HEIGHT screen,
	 * but these width functions actually want raster sizes (that is, actual pixels).
	 * Scale the line width and point size by the average ratio of the scale. */
	float WidthVal = float(g_CurrentWidth) / SCREEN_WIDTH;
	float HeightVal = float(g_CurrentHeight) / SCREEN_HEIGHT;
	LineWidth *= (WidthVal + HeightVal) / 2;

	/* Clamp the width to the hardware max for both lines and points (whichever
	 * is more restrictive). */
	LineWidth = clamp(LineWidth, m_oglspecs->line_range[0], m_oglspecs->line_range[1]);
	LineWidth = clamp(LineWidth, m_oglspecs->point_range[0], m_oglspecs->point_range[1]);

	/* Hmm.  The granularity of lines and points might be different; for example,
	 * if lines are .5 and points are .25, we might want to snap the width to the
	 * nearest .5, so the hardware doesn't snap them to different sizes.  Does it
	 * matter? */

	glLineWidth(LineWidth);

	/* Draw the line loop: */
	glInterleavedArrays( RageVertexFormat, sizeof(RageVertex), v );
	glDrawArrays( GL_LINE_LOOP, 0, iNumVerts );

	glDisable(GL_LINE_SMOOTH);

	/* Round off the corners.  This isn't perfect; the point is sometimes a little
	 * larger than the line, causing a small bump on the edge.  Not sure how to fix
	 * that. */
	glPointSize(LineWidth);

	/* Hack: if the points will all be the same, we don't want to draw
	 * any points at all, since there's nothing to connect.  That'll happen
	 * if both scale factors in the matrix are ~0.  (Actually, I think
	 * it's true if two of the three scale factors are ~0, but we don't
	 * use this for anything 3d at the moment anyway ...) */
	RageMatrix mat;
	glGetFloatv( GL_MODELVIEW_MATRIX, (float*)mat );

	if(mat.m[0][0] < 1e-5 && mat.m[1][1] < 1e-5) 
	    return;

	glEnable(GL_POINT_SMOOTH);

	glInterleavedArrays( RageVertexFormat, sizeof(RageVertex), v );
	glDrawArrays( GL_POINTS, 0, iNumVerts );

	glDisable(GL_POINT_SMOOTH);
}

void RageDisplay::PushMatrix() 
{ 
	glPushMatrix();
	ASSERT(++g_ModelMatrixCnt<20);
}

void RageDisplay::PopMatrix() 
{ 
	glPopMatrix();
	ASSERT(g_ModelMatrixCnt-->0);
}

/* Switch from orthogonal to perspective view.
 *
 * Tricky: we want to maintain all of the zooms, rotations and translations
 * that have been applied already.  They're in our internal screen space (that
 * is, 640x480 ortho).  We can't simply leave them where they are, since they'll
 * be applied before the perspective transform, which means they'll be in the
 * wrong coordinate space.
 *
 * Handle translations (the right column of the transform matrix) to the viewport.
 * Move rotations and scaling (0,0 through 1,1) to after the perspective transform.
 * Actually, those might be able to stay where they are, I'm not sure; it's translations
 * that are annoying.  So, XXX: see if rots and scales can be left on the modelview
 * matrix instead of messing with the projection matrix.
 *
 * When finished, the current position will be the "viewpoint" (at 0,0).  negative
 * Z goes into the screen, positive X and Y is right and down.
 */
void RageDisplay::EnterPerspective(float fov, bool preserve_loc)
{
	g_PerspectiveMode++;
	if(g_PerspectiveMode > 1) {
		/* havn't yet worked out the details of this */
		LOG->Trace("EnterPerspective called when already in perspective mode");
		g_PerspectiveMode++;
		return;
	}

	/* Save the old matrices. */
	DISPLAY->PushMatrix();
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	float aspect = SCREEN_WIDTH/(float)SCREEN_HEIGHT;
	gluPerspective(fov, aspect, 1.000f, 1000.0f);

	/* Flip the Y coordinate, so positive numbers go down. */
	glScalef(1, -1, 1);

	if(!preserve_loc)
	{
		glMatrixMode( GL_MODELVIEW );
		return;
	}

	RageMatrix matTop;
	glGetFloatv( GL_MODELVIEW_MATRIX, (float*)matTop );

	{
		/* Pull out the 2d translation. */
		float x = matTop.m[3][0], y = matTop.m[3][1];

		/* These values are where the viewpoint should be.  By default, it's in the
		* center of the screen, and these values are from the top-left, so subtract
		* the difference. */
		x -= SCREEN_WIDTH/2;
		y -= SCREEN_HEIGHT/2;
		DISPLAY->SetViewport(int(x), int(y));
	}

	/* Pull out the 2d rotations and scales. */
	{
		RageMatrix mat;
		RageMatrixIdentity(&mat);
		mat.m[0][0] = matTop.m[0][0];
		mat.m[0][1] = matTop.m[0][1];
		mat.m[1][0] = matTop.m[1][0];
		mat.m[1][1] = matTop.m[1][1];
		glMultMatrixf((float *) mat);
	}

	/* We can't cope with perspective matrices or things that touch Z.  (We shouldn't
	* have touched those while in 2d, anyway.) */
	ASSERT(matTop.m[0][2] == 0.f &&	matTop.m[0][3] == 0.f && matTop.m[1][2] == 0.f &&
		matTop.m[1][3] == 0.f && matTop.m[2][0] == 0.f && matTop.m[2][1] == 0.f &&
		matTop.m[2][2] == 1.f && matTop.m[3][2] == 0.f && matTop.m[3][3] == 1.f);

	/* Reset the matrix back to identity. */
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
}

void RageDisplay::ExitPerspective()
{
	g_PerspectiveMode--;
	if(g_PerspectiveMode) return;

	/* Restore the old matrices. */
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	DISPLAY->PopMatrix();

	/* Restore the viewport. */
	DISPLAY->SetViewport(0, 0);
}

/* gluLookAt.  The result is post-multiplied to the matrix (M = L * M) instead of
 * pre-multiplied. */
void RageDisplay::LookAt(const RageVector3 &Eye, const RageVector3 &At, const RageVector3 &Up)
{
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	RageMatrix view;
	glGetFloatv( GL_MODELVIEW_MATRIX, (float*)view ); /* cheesy :) */
	glPopMatrix();

	smPostMultMatrixf(view);
}

void RageDisplay::Translate( float x, float y, float z )
{
	glTranslatef(x, y, z);
}


void RageDisplay::TranslateLocal( float x, float y, float z )
{
	RageMatrix matTemp;
	RageMatrixTranslation( &matTemp, x, y, z );

	smPostMultMatrixf(matTemp);
}

void RageDisplay::Scale( float x, float y, float z )
{
	glScalef(x, y, z);
}

void RageDisplay::RotateX( float r )
{
	glRotatef(r* 180/PI, 1, 0, 0);
}

void RageDisplay::RotateY( float r )
{
	glRotatef(r* 180/PI, 0, 1, 0);
}

void RageDisplay::RotateZ( float r )
{
	glRotatef(r* 180/PI, 0, 0, 1);
}

void RageDisplay::smPostMultMatrixf( const RageMatrix &f )
{
	RageMatrix m;
	glGetFloatv( GL_MODELVIEW_MATRIX, (float*)m );
	RageMatrixMultiply( &m, &f, &m );
	glLoadMatrixf((const float*) m);
}

void RageDisplay::SetTexture( RageTexture* pTexture )
{
	glBindTexture( GL_TEXTURE_2D, pTexture? pTexture->GetGLTextureID() : 0 );
}
void RageDisplay::SetTextureModeModulate()
{
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

/* Set the blend mode for both texture and alpha.  This is all that's
 * available pre-OpenGL 1.4. */
void RageDisplay::SetBlendMode(int src, int dst)
{
	glBlendFunc( src, dst );
}

void RageDisplay::SetTextureModeGlow()
{
	if(!m_oglspecs->EXT_texture_env_combine) {
		SetBlendMode( GL_SRC_ALPHA, GL_ONE );
		return;
	}

	/* Source color is the diffuse color only: */
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PRIMARY_COLOR_EXT);

	/* Source alpha is texture alpha * diffuse alpha: */
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PRIMARY_COLOR_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_TEXTURE);
}

void RageDisplay::SetBlendModeNormal()
{
	SetBlendMode( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
}
void RageDisplay::SetBlendModeAdd()
{
	SetBlendMode( GL_ONE, GL_ONE );
}

bool RageDisplay::ZBufferEnabled() const
{
	bool a;
	glGetBooleanv( GL_DEPTH_TEST, (unsigned char*)&a );
	return a;
}

void RageDisplay::EnableZBuffer()
{
	glEnable( GL_DEPTH_TEST );
}
void RageDisplay::DisableZBuffer()
{
	glDisable( GL_DEPTH_TEST );
}
void RageDisplay::EnableTextureWrapping()
{
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
}
void RageDisplay::DisableTextureWrapping()
{
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
}
