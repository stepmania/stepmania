#include "global.h"
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
#include "StepMania.h"

#include "arch/arch.h"
#include "arch/LowLevelWindow/LowLevelWindow.h"

#include <math.h>

RageDisplay*		DISPLAY	= NULL;

////////////
// Globals
////////////
RageTimer			g_LastCheckTimer;
int					g_iNumVerts;
int					g_iFPS, g_iVPF, g_iCFPS;

int					g_PerspectiveMode = 0;

int					g_ModelMatrixCnt=0;
int RageDisplay::GetFPS() const { return g_iFPS; }
int RageDisplay::GetVPF() const { return g_iVPF; }
int RageDisplay::GetCumFPS() const { return g_iCFPS; }

static int			g_iFramesRenderedSinceLastCheck,
					g_iFramesRenderedSinceLastReset,
					g_iVertsRenderedSinceLastCheck,
					g_iNumChecksSinceLastReset;

PWSWAPINTERVALEXTPROC GLExt::wglSwapIntervalEXT;
PFNGLCOLORTABLEPROC GLExt::glColorTableEXT;
PFNGLCOLORTABLEPARAMETERIVPROC GLExt::glGetColorTableParameterivEXT;

/* We don't actually use normals (we don't tunr on lighting), there's just
 * no GL_T2F_C4F_V3F. */
const GLenum RageVertexFormat = GL_T2F_C4F_N3F_V3F;


LowLevelWindow *wind;

void GetGLExtensions(set<string> &ext)
{
    const char *buf = (const char *)glGetString(GL_EXTENSIONS);

	vector<CString> lst;
	split(buf, " ", lst);

	for(unsigned i = 0; i < lst.size(); ++i)
		ext.insert(lst[i]);
}

RageDisplay::RageDisplay( bool windowed, int width, int height, int bpp, int rate, bool vsync )
{
	LOG->Trace( "RageDisplay::RageDisplay()" );

	m_oglspecs = new oglspecs_t;

	wind = MakeLowLevelWindow();

	SetVideoMode( windowed, width, height, bpp, rate, vsync );

	// Log driver details
	LOG->Info("OGL Vendor: %s", glGetString(GL_VENDOR));
	LOG->Info("OGL Renderer: %s", glGetString(GL_RENDERER));
	LOG->Info("OGL Version: %s", glGetString(GL_VERSION));
	LOG->Info("OGL Extensions: %s", glGetString(GL_EXTENSIONS));
	if( IsSoftwareRenderer() )
		LOG->Warn("This is a software renderer!");


	/* Log this, so if people complain that the radar looks bad on their
	 * system we can compare them: */
	glGetFloatv(GL_LINE_WIDTH_RANGE, m_oglspecs->line_range);
	glGetFloatv(GL_LINE_WIDTH_GRANULARITY, &m_oglspecs->line_granularity);
	LOG->Info("Line width range: %f-%f +%f", m_oglspecs->line_range[0], m_oglspecs->line_range[1], m_oglspecs->line_granularity);

	glGetFloatv(GL_POINT_SIZE_RANGE, m_oglspecs->point_range);
	glGetFloatv(GL_POINT_SIZE_GRANULARITY, &m_oglspecs->point_granularity);
	LOG->Info("Point size range: %f-%f +%f", m_oglspecs->point_range[0], m_oglspecs->point_range[1], m_oglspecs->point_granularity);

	/* sizeof("string constant") is the size of the pointer (~4), not the length of the string. */
	m_oglspecs->bAALinesCauseProblems = strncmp((const char*)glGetString(GL_RENDERER),"3Dfx/Voodoo3 (tm)/2 TMUs/16 MB SDRAM/ICD (Nov  2 2000)",strlen("3Dfx/Voodoo3"))==0;
	if( m_oglspecs->bAALinesCauseProblems )
		LOG->Info("Anti-aliased lines are known to cause problems with this driver.");
	m_oglspecs->bPackedPixelsCauseProblems = 
	    strncmp((const char*)glGetString(GL_RENDERER),"GLDirect",strlen("GLDirect"))==0;
	if( m_oglspecs->bPackedPixelsCauseProblems )
		LOG->Info("Packed pixel formats are known to cause problems with this driver.");
}

void RageDisplay::Update(float fDeltaTime)
{
	wind->Update(fDeltaTime);
}

bool RageDisplay::IsSoftwareRenderer()
{
	return 
		( strcmp((const char*)glGetString(GL_VENDOR),"Microsoft Corporation")==0 ) &&
		( strcmp((const char*)glGetString(GL_RENDERER),"GDI Generic")==0 );
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

	float left = 0, right = SCREEN_WIDTH, bottom = SCREEN_HEIGHT, top = 0;
	if(strncmp((const char*)glGetString(GL_RENDERER),"GLDirect",strlen("GLDirect"))==0)
	{
	    /* GLDirect incorrectly uses Direct3D's device coordinate system
	     * instead of OpenGL's, so we need to compensate. */
	    left += 0.5f;
	    right += 0.5f;
	    bottom += 0.5f;
	    top += 0.5f;
	}

	glOrtho(left, right, bottom, top, SCREEN_NEAR, SCREEN_FAR );
	glMatrixMode( GL_MODELVIEW );
}

RageDisplay::~RageDisplay()
{
	delete wind;
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
	GetGLExtensions(m_oglspecs->glExts);

	/* Check for extensions: */
	m_oglspecs->EXT_texture_env_combine = HasExtension("GL_EXT_texture_env_combine");
	m_oglspecs->WGL_EXT_swap_control = HasExtension("WGL_EXT_swap_control");
	m_oglspecs->EXT_paletted_texture = HasExtension("GL_EXT_paletted_texture");

	/* Find extension functions. */
	GLExt::wglSwapIntervalEXT = (PWSWAPINTERVALEXTPROC) wind->GetProcAddress("wglSwapIntervalEXT");
	GLExt::glColorTableEXT = (PFNGLCOLORTABLEPROC) wind->GetProcAddress("glColorTableEXT");
	GLExt::glGetColorTableParameterivEXT = (PFNGLCOLORTABLEPARAMETERIVPROC) wind->GetProcAddress("glGetColorTableParameterivEXT");

	/* Make sure we have all components for detected extensions. */
	if(m_oglspecs->WGL_EXT_swap_control)
	{
		if(!GLExt::wglSwapIntervalEXT)
		{
			LOG->Warn("wglSwapIntervalEXT but wglSwapIntervalEXT() not found");
			m_oglspecs->WGL_EXT_swap_control=false;
		}
	}
	if(m_oglspecs->EXT_paletted_texture)
	{
		if(!GLExt::glColorTableEXT || !GLExt::glGetColorTableParameterivEXT)
		{
			LOG->Warn("GL_EXT_paletted_texture but glColorTableEXT or glGetColorTableParameterivEXT not found");
			m_oglspecs->EXT_paletted_texture = false;
		}
	}
}

/* Set the video mode.  In some cases, changing the video mode will reset
 * the rendering context; returns true if we need to reload textures. */
bool RageDisplay::SetVideoMode( bool windowed, int width, int height, int bpp, int rate, bool vsync )
{
//	LOG->Trace( "RageDisplay::SetVideoMode( %d, %d, %d, %d, %d, %d )", windowed, width, height, bpp, rate, vsync );
	bool NewOpenGLContext = wind->SetVideoMode( windowed, width, height, bpp, rate, vsync );

	if(NewOpenGLContext)
	{
		/* We have a new OpenGL context, so we have to tell our textures that
		 * their OpenGL texture number is invalid. */
		if(TEXTUREMAN)
			TEXTUREMAN->InvalidateTextures();
	}

	SetupOpenGL();
	DumpOpenGLDebugInfo();

	/* Now that we've initialized, we can search for extensions (some of which
	 * we may need to set up the video mode). */
	SetupExtensions();

	/* Set vsync the Windows way, if we can.  (What other extensions are there
	 * to do this, for other archs?) */
	if(m_oglspecs->WGL_EXT_swap_control)
	    GLExt::wglSwapIntervalEXT(vsync);
	
	ResolutionChanged();

	return NewOpenGLContext;
}

void RageDisplay::DumpOpenGLDebugInfo()
{
#if defined(WIN32)
	/* Dump Windows pixel format data. */
	int Actual = GetPixelFormat(wglGetCurrentDC());

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize=sizeof(pfd);
	pfd.nVersion=1;
  
	int pfcnt = DescribePixelFormat(GetDC(g_hWndMain),1,sizeof(pfd),&pfd);
	for (int i=1; i <= pfcnt; i++)
	{
		memset(&pfd, 0, sizeof(pfd));
		pfd.nSize=sizeof(pfd);
		pfd.nVersion=1;
		DescribePixelFormat(GetDC(g_hWndMain),i,sizeof(pfd),&pfd);

		bool skip = false;

		bool rgba = (pfd.iPixelType==PFD_TYPE_RGBA);

		bool mcd = ((pfd.dwFlags & PFD_GENERIC_FORMAT) && (pfd.dwFlags & PFD_GENERIC_ACCELERATED));
		bool soft = ((pfd.dwFlags & PFD_GENERIC_FORMAT) && !(pfd.dwFlags & PFD_GENERIC_ACCELERATED));
		bool icd = !(pfd.dwFlags & PFD_GENERIC_FORMAT) && !(pfd.dwFlags & PFD_GENERIC_ACCELERATED);
		bool opengl = !!(pfd.dwFlags & PFD_SUPPORT_OPENGL);
		bool window = !!(pfd.dwFlags & PFD_DRAW_TO_WINDOW);
		bool dbuff = !!(pfd.dwFlags & PFD_DOUBLEBUFFER);

		if(!rgba || soft || !opengl || !window || !dbuff)
			skip = true;

		/* Skip the above, unless it happens to be the one we chose. */
		if(skip && i != Actual)
			continue;

		CString str = ssprintf("Mode %i: ", i);
		if(i == Actual) str += "*** ";
		if(skip) str += "(BOGUS) ";
		if(soft) str += "software ";
		if(icd) str += "ICD ";
		if(mcd) str += "MCD ";
		if(!rgba) str += "indexed ";
		if(!opengl) str += "!OPENGL ";
		if(!window) str += "!window ";
		if(!dbuff) str += "!dbuff ";

		str += ssprintf("%i (%i%i%i) ", pfd.cColorBits, pfd.cRedBits, pfd.cGreenBits, pfd.cBlueBits);
		if(pfd.cAlphaBits) str += ssprintf("%i alpha ", pfd.cAlphaBits);
		if(pfd.cDepthBits) str += ssprintf("%i depth ", pfd.cDepthBits);
		if(pfd.cStencilBits) str += ssprintf("%i stencil ", pfd.cStencilBits);
		if(pfd.cAccumBits) str += ssprintf("%i accum ", pfd.cAccumBits);

		if(i == Actual && skip)
		{
			/* We chose a bogus format. */
			LOG->Warn("%s", str.GetString());
		} else
			LOG->Info("%s", str.GetString());
	}
#endif
}

void RageDisplay::ResolutionChanged()
{
	SetViewport(0,0);

	/* Clear any junk that's in the framebuffer. */
	Clear();
	Flip();
}

void RageDisplay::SetViewport(int shift_left, int shift_down)
{
	/* left and down are on a 0..SCREEN_WIDTH, 0..SCREEN_HEIGHT scale.
	 * Scale them to the actual viewport range. */
	shift_left = int( shift_left * float(wind->GetWidth()) / SCREEN_WIDTH );
	shift_down = int( shift_down * float(wind->GetWidth()) / SCREEN_WIDTH );

	glViewport(shift_left, -shift_down, wind->GetWidth(), wind->GetHeight());
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
	wind->SwapBuffers();
	g_iFramesRenderedSinceLastCheck++;
	g_iFramesRenderedSinceLastReset++;

	if( g_LastCheckTimer.PeekDeltaTime() >= 1.0f )	// update stats every 1 sec.
	{
		g_LastCheckTimer.GetDeltaTime();
		g_iNumChecksSinceLastReset++;
		g_iFPS = g_iFramesRenderedSinceLastCheck;
		g_iCFPS = g_iFramesRenderedSinceLastReset / g_iNumChecksSinceLastReset;
		g_iVPF = g_iVertsRenderedSinceLastCheck / g_iFPS;
		g_iFramesRenderedSinceLastCheck = g_iVertsRenderedSinceLastCheck = 0;
		LOG->Trace( "FPS: %d, CFPS %d, VPF: %d", g_iFPS, g_iCFPS, g_iVPF );
	}
}

float RageDisplay::PredictedSecondsUntilNextFlip() const
{
	float fps = max( 30.f, GetFPS() );	
	return 1.f/fps;
}


void RageDisplay::ResetStats()
{
	g_iFPS = g_iVPF = 0;
	g_iFramesRenderedSinceLastCheck = g_iFramesRenderedSinceLastReset = 0;
	g_iNumChecksSinceLastReset = 0;
	g_iVertsRenderedSinceLastCheck = 0;
	g_LastCheckTimer.GetDeltaTime();
}

void RageDisplay::DisablePalettedTexture()
{
	m_oglspecs->EXT_paletted_texture = false;
}

void RageDisplay::SaveScreenshot( CString sPath )
{
	ASSERT( sPath.Right(3).CompareNoCase("bmp") == 0 );	// we can only save bitmaps

	SDL_Surface *image = SDL_CreateRGBSurface(
		SDL_SWSURFACE, wind->GetWidth(), wind->GetHeight(),
        24, 0x0000FF, 0x00FF00, 0xFF0000, 0x000000);
	SDL_Surface *temp = SDL_CreateRGBSurface(
		SDL_SWSURFACE, wind->GetWidth(), wind->GetHeight(),
		24, 0x0000FF, 0x00FF00, 0xFF0000, 0x000000);

	glReadPixels(0, 0, wind->GetWidth(), wind->GetHeight(), GL_RGB,
	             GL_UNSIGNED_BYTE, image->pixels);

	// flip vertically
	for( int y=0; y<wind->GetHeight(); y++ )
		memcpy( 
			(char *)temp->pixels + 3 * wind->GetWidth() * y,
			(char *)image->pixels + 3 * wind->GetWidth() * (wind->GetHeight()-1-y),
			3*wind->GetWidth() );

	SDL_SaveBMP( temp, sPath );

	SDL_FreeSurface( image );
	SDL_FreeSurface( temp );
}


bool RageDisplay::IsWindowed() const 
{
	return wind->IsWindowed();
}

void RageDisplay::DrawQuad( const RageVertex v[4] )	// upper-left, upper-right, lower-right, lower-left
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

/* Draw a line as a quad.  GL_LINES with antialiasing off can draw odd
 * ends at odd angles--they're forced to axis-alignment regardless of the
 * angle of the line. */
void RageDisplay::DrawPolyLine(const RageVertex &p1, const RageVertex &p2, float LineWidth )
{
	/* soh cah toa strikes strikes again! */
	float opp = p2.p.x - p1.p.x;
	float adj = p2.p.y - p1.p.y;
	float hyp = powf(opp*opp + adj*adj, 0.5f);

	float lsin = opp/hyp;
	float lcos = adj/hyp;

	RageVertex p[4];
	p[0] = p[1] = p1;
	p[2] = p[3] = p2;

	float ydist = lsin * LineWidth/2;
	float xdist = lcos * LineWidth/2;
	
	p[0].p.x += xdist;
	p[0].p.y -= ydist;
	p[1].p.x -= xdist;
	p[1].p.y += ydist;
	p[2].p.x -= xdist;
	p[2].p.y += ydist;
	p[3].p.x += xdist;
	p[3].p.y -= ydist;

	DrawQuad(p);
}

/* Draw a line loop with rounded corners using polys.  This is used on
 * cards that have strange allergic reactions to antialiased points and
 * lines. */
void RageDisplay::DrawLoop_Polys( const RageVertex v[], int iNumVerts, float LineWidth )
{
	ASSERT( iNumVerts >= 3 );
	
	int i;
	for(i = 0; i < iNumVerts; ++i)
		DrawPolyLine(v[i], v[(i+1)%iNumVerts], LineWidth);

	/* Join the lines with circles so we get rounded corners. */
	GLUquadricObj *q =  gluNewQuadric();
	for(i = 0; i < iNumVerts; ++i)
	{
		glPushMatrix();
		glColor4fv(v[i].c);
		glTexCoord3fv(v[i].t);
		glTranslatef(v[i].p.x, v[i].p.y, v[i].p.z);

		gluDisk(q, 0, LineWidth/2, 32, 32);
		glPopMatrix();
	}
	gluDeleteQuadric(q);
}

/* Draw a nice AA'd line loop.  One problem with this is that point and line
 * sizes don't always precisely match, which doesn't look quite right.
 * It's worth it for the AA, though. */
void RageDisplay::DrawLoop_LinesAndPoints( const RageVertex v[], int iNumVerts, float LineWidth )
{
	ASSERT( iNumVerts >= 3 );

	glEnable(GL_LINE_SMOOTH);

	/* Our line width is wrt the regular internal SCREEN_WIDTHxSCREEN_HEIGHT screen,
	 * but these width functions actually want raster sizes (that is, actual pixels).
	 * Scale the line width and point size by the average ratio of the scale. */
	float WidthVal = float(wind->GetWidth()) / SCREEN_WIDTH;
	float HeightVal = float(wind->GetHeight()) / SCREEN_HEIGHT;
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
	 * use this for anything 3d at the moment anyway ...)  This is needed
	 * because points aren't scaled like regular polys--a zero-size point
	 * will still be drawn. */
	RageMatrix mat;
	glGetFloatv( GL_MODELVIEW_MATRIX, (float*)mat );

	if(mat.m[0][0] < 1e-5 && mat.m[1][1] < 1e-5) 
	    return;

	glEnable(GL_POINT_SMOOTH);

	glInterleavedArrays( RageVertexFormat, sizeof(RageVertex), v );
	glDrawArrays( GL_POINTS, 0, iNumVerts );

	glDisable(GL_POINT_SMOOTH);
}

void RageDisplay::DrawLoop( const RageVertex v[], int iNumVerts, float LineWidth )
{
	if( m_oglspecs->bAALinesCauseProblems )
		DrawLoop_Polys(v, iNumVerts, LineWidth);
	else
		DrawLoop_LinesAndPoints(v, iNumVerts, LineWidth);
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
	gluPerspective(fov, aspect, 1.000f, 1000.0f);	// This far clipping this might cause Z-fighting if we ever turn the z-buffer on

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
		this->MultMatrix(mat);
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

/* gluLookAt.  The result is pre-multiplied to the matrix (M = L * M) instead of
 * post-multiplied. */
void RageDisplay::LookAt(const RageVector3 &Eye, const RageVector3 &At, const RageVector3 &Up)
{
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	RageMatrix view;
	glGetFloatv( GL_MODELVIEW_MATRIX, (float*)view ); /* cheesy :) */
	glPopMatrix();

	PreMultMatrix(view);
}

void RageDisplay::Translate( float x, float y, float z )
{
	glTranslatef(x, y, z);
}


void RageDisplay::TranslateLocal( float x, float y, float z )
{
	RageMatrix matTemp;
	RageMatrixTranslation( &matTemp, x, y, z );

	PreMultMatrix(matTemp);
}

void RageDisplay::Scale( float x, float y, float z )
{
	glScalef(x, y, z);
}

void RageDisplay::RotateX( float r )
{
	glRotatef(r, 1, 0, 0);
}

void RageDisplay::RotateY( float r )
{
	glRotatef(r, 0, 1, 0);
}

void RageDisplay::RotateZ( float r )
{
	// HACK:  Rotation about (0,0,1) seems to be broken in many Voodoo3 drivers,
	// but only while using orthographic projections.  Strange...
	// Adding a tiny X component fixes the problem.   -Chris
	glRotatef(r, 0.00001f, 0, 1);
}

void RageDisplay::PostMultMatrix( const RageMatrix &f )
{
	glMultMatrixf((const float *) f);
}

void RageDisplay::PreMultMatrix( const RageMatrix &f )
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
	glBlendFunc( GLenum(src), GLenum(dst) );
}

void RageDisplay::SetTextureModeGlow()
{
	if(!m_oglspecs->EXT_texture_env_combine) {
		SetBlendMode( GL_SRC_ALPHA, GL_ONE );
		return;
	}

	/* Source color is the diffuse color only: */
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GLenum(GL_COMBINE_RGB_EXT), GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GLenum(GL_SOURCE0_RGB_EXT), GL_PRIMARY_COLOR_EXT);

	/* Source alpha is texture alpha * diffuse alpha: */
	glTexEnvi(GL_TEXTURE_ENV, GLenum(GL_COMBINE_ALPHA_EXT), GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GLenum(GL_OPERAND0_ALPHA_EXT), GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GLenum(GL_SOURCE0_ALPHA_EXT), GL_PRIMARY_COLOR_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GLenum(GL_OPERAND1_ALPHA_EXT), GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GLenum(GL_SOURCE1_ALPHA_EXT), GL_TEXTURE);
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
void RageDisplay::EnableTextureWrapping(bool yes)
{
	GLenum mode = yes? GL_REPEAT:GL_CLAMP;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode );
}
