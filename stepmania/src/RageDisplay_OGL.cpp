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

/*
 * This header pulls in GL headers and defines things that require them.
 * This only needs to be included if you actually use these; most of the
 * time, RageDisplay.h is sufficient. 
 */

#include "SDL_utils.h"
/* ours is more up-to-date */
#define NO_SDL_GLEXT
#define __glext_h_ /* try harder to stop glext.h from being forced on us by someone else */
#include "SDL_opengl.h"
#undef __glext_h_

#include "glext.h"

/* Windows's broken gl.h defines GL_EXT_paletted_texture incompletely: */
#ifndef GL_TEXTURE_INDEX_SIZE_EXT
#define GL_TEXTURE_INDEX_SIZE_EXT         0x80ED
#endif
#include <set>

/* Not in glext.h: */
typedef bool (APIENTRY * PWSWAPINTERVALEXTPROC) (int interval);


/* Extension functions we use.  Put these in a namespace instead of in oglspecs_t,
 * so they can be called like regular functions. */
namespace GLExt {
	extern PWSWAPINTERVALEXTPROC wglSwapIntervalEXT;
	extern PFNGLCOLORTABLEPROC glColorTableEXT;
	extern PFNGLCOLORTABLEPARAMETERIVPROC glGetColorTableParameterivEXT;
};

#include "RageDisplay.h"
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

#include "arch/arch.h"
#include "arch/LowLevelWindow/LowLevelWindow.h"

#include <math.h>

RageDisplay*		DISPLAY	= NULL;


//
// Globals
//

PWSWAPINTERVALEXTPROC GLExt::wglSwapIntervalEXT = NULL;
PFNGLCOLORTABLEPROC GLExt::glColorTableEXT = NULL;
PFNGLCOLORTABLEPARAMETERIVPROC GLExt::glGetColorTableParameterivEXT = NULL;
bool	g_bAALinesBroken = false;
bool	g_bEXT_texture_env_combine = true;
/* OpenGL system information that generally doesn't change at runtime. */

/* Range and granularity of points and lines: */
float g_line_range[2];
float g_line_granularity;
float g_point_range[2];
float g_point_granularity;

/* OpenGL version * 10: */
int g_glVersion;

/* Available extensions: */
set<string> g_glExts;

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
	glGetFloatv(GL_LINE_WIDTH_RANGE, g_line_range);
	glGetFloatv(GL_LINE_WIDTH_GRANULARITY, &g_line_granularity);
	LOG->Info("Line width range: %.3f-%.3f +%.3f", g_line_range[0], g_line_range[1], g_line_granularity);

	glGetFloatv(GL_POINT_SIZE_RANGE, g_point_range);
	glGetFloatv(GL_POINT_SIZE_GRANULARITY, &g_point_granularity);
	LOG->Info("Point size range: %.3f-%.3f +%.3f", g_point_range[0], g_point_range[1], g_point_granularity);
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

RageDisplay::~RageDisplay()
{
	delete wind;
}

bool HasExtension(CString ext)
{
	return g_glExts.find(ext) != g_glExts.end();
}

void SetupExtensions()
{
	double fGLVersion = atof( (const char *) glGetString(GL_VERSION) );
	g_glVersion = int(roundf(fGLVersion * 10));
	GetGLExtensions(g_glExts);

	/* Find extension functions and reset broken flags */
	GLExt::wglSwapIntervalEXT = (PWSWAPINTERVALEXTPROC) wind->GetProcAddress("wglSwapIntervalEXT");
	GLExt::glColorTableEXT = (PFNGLCOLORTABLEPROC) wind->GetProcAddress("glColorTableEXT");
	GLExt::glGetColorTableParameterivEXT = (PFNGLCOLORTABLEPARAMETERIVPROC) wind->GetProcAddress("glGetColorTableParameterivEXT");
	g_bAALinesBroken = false;
	g_bEXT_texture_env_combine = HasExtension("GL_EXT_texture_env_combine");

	// Checks for known bad drivers
	CString sRenderer = (const char*)glGetString(GL_RENDERER);
	
	if( sRenderer.Left(12) == "3Dfx/Voodoo3" )
	{
		LOG->Info("Anti-aliased lines are known to cause problems with this driver.");
		g_bAALinesBroken = true;

		LOG->Info("Paletted textures are broken with this driver.");
		GLExt::glColorTableEXT = NULL;
		GLExt::glGetColorTableParameterivEXT = NULL;
	}
}

void DumpOpenGLDebugInfo()
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
			LOG->Warn("%s", str.c_str());
		} else
			LOG->Info("%s", str.c_str());
	}
#endif
}

void RageDisplay::ResolutionChanged()
{
	SetViewport(0,0);

	/* Clear any junk that's in the framebuffer. */
	BeginFrame();
	EndFrame();
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

	this->SetDefaultRenderStates();
	DumpOpenGLDebugInfo();

	/* Now that we've initialized, we can search for extensions (some of which
	 * we may need to set up the video mode). */
	SetupExtensions();

	/* Set vsync the Windows way, if we can.  (What other extensions are there
	 * to do this, for other archs?) */
	if( GLExt::wglSwapIntervalEXT )
	    GLExt::wglSwapIntervalEXT(vsync);
	
	ResolutionChanged();

	return NewOpenGLContext;
}

void RageDisplay::SetViewport(int shift_left, int shift_down)
{
	/* left and down are on a 0..SCREEN_WIDTH, 0..SCREEN_HEIGHT scale.
	 * Scale them to the actual viewport range. */
	shift_left = int( shift_left * float(wind->GetWidth()) / SCREEN_WIDTH );
	shift_down = int( shift_down * float(wind->GetHeight()) / SCREEN_HEIGHT );

	glViewport(shift_left, -shift_down, wind->GetWidth(), wind->GetHeight());
}

int RageDisplay::GetMaxTextureSize() const
{
	GLint size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
	return size;
}

void RageDisplay::BeginFrame()
{
	glClearColor( 0,0,0,1 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void RageDisplay::EndFrame()
{
	wind->SwapBuffers();
	ProcessStatsOnFlip();
}

bool RageDisplay::SupportsTextureFormat( PixelFormat pixfmt )
{
	switch( pixfmt )
	{
	case FMT_PAL:
		return GLExt::glColorTableEXT && GLExt::glGetColorTableParameterivEXT;
	default:
		return true;	// No way to query this in OGL.  You pass it a format and hope it doesn't have to convert.
	}
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
	int pitch = image->pitch;
	for( int y=0; y<wind->GetHeight(); y++ )
		memcpy( 
			(char *)temp->pixels + pitch * y,
			(char *)image->pixels + pitch * (wind->GetHeight()-1-y),
			3*wind->GetWidth() );

	SDL_SaveBMP( temp, sPath );

	SDL_FreeSurface( image );
	SDL_FreeSurface( temp );
}


bool RageDisplay::IsWindowed() const { return wind->IsWindowed(); }
int RageDisplay::GetWidth() const { return wind->GetWidth(); }
int RageDisplay::GetHeight() const { return wind->GetHeight(); }
int RageDisplay::GetBPP() const { return wind->GetBPP(); }


void RageDisplay::DrawQuads( const RageVertex v[], int iNumVerts )
{
	ASSERT( (iNumVerts%4) == 0 );

	if(iNumVerts == 0)
		return;

	glMatrixMode( GL_PROJECTION );
	glLoadMatrixf( (const float*)GetProjection() );
	glMatrixMode( GL_MODELVIEW );
	glLoadMatrixf( (const float*)GetModelViewTop() );

#if 1
	static float *Vertex, *Color, *Texture;	
	static int Size = 0;
	if(iNumVerts > Size)
	{
		Size = iNumVerts;
		delete [] Vertex;
		delete [] Color;
		delete [] Texture;
		Vertex = new float[Size*3];
		Color = new float[Size*4];
		Texture = new float[Size*2];
	}

	for(unsigned i = 0; i < unsigned(iNumVerts); ++i)
	{
		Vertex[i*3+0]  = v[i].p[0];
		Vertex[i*3+1]  = v[i].p[1];
		Vertex[i*3+2]  = v[i].p[2];
		Color[i*4+0]   = v[i].c[0];
		Color[i*4+1]   = v[i].c[1];
		Color[i*4+2]   = v[i].c[2];
		Color[i*4+3]   = v[i].c[3];
		Texture[i*2+0] = v[i].t[0];
		Texture[i*2+1] = v[i].t[1];
	}
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertex);

	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_FLOAT, 0, Color);

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, Texture);

	glDisableClientState(GL_NORMAL_ARRAY);
#else
	glInterleavedArrays( RageVertexFormat, sizeof(RageVertex), v );
#endif

	glDrawArrays( GL_QUADS, 0, iNumVerts );

	StatsAddVerts( iNumVerts );
}
void RageDisplay::DrawFan( const RageVertex v[], int iNumVerts )
{
	ASSERT( iNumVerts >= 3 );
	glMatrixMode( GL_PROJECTION );
	glLoadMatrixf( (const float*)GetProjection() );
	glMatrixMode( GL_MODELVIEW );
	glLoadMatrixf( (const float*)GetModelViewTop() );
	glInterleavedArrays( RageVertexFormat, sizeof(RageVertex), v );
	glDrawArrays( GL_TRIANGLE_FAN, 0, iNumVerts );
	StatsAddVerts( iNumVerts );
}

void RageDisplay::DrawStrip( const RageVertex v[], int iNumVerts )
{
	ASSERT( iNumVerts >= 3 );
	glMatrixMode( GL_PROJECTION );
	glLoadMatrixf( (const float*)GetProjection() );
	glMatrixMode( GL_MODELVIEW );
	glLoadMatrixf( (const float*)GetModelViewTop() );
	glInterleavedArrays( RageVertexFormat, sizeof(RageVertex), v );
	glDrawArrays( GL_TRIANGLE_STRIP, 0, iNumVerts );
	StatsAddVerts( iNumVerts );
}

void RageDisplay::DrawTriangles( const RageVertex v[], int iNumVerts )
{
	if( iNumVerts == 0 )
		return;
	ASSERT( iNumVerts >= 3 );
	ASSERT( (iNumVerts%3) == 0 );
	glMatrixMode( GL_PROJECTION );
	glLoadMatrixf( (const float*)GetProjection() );
	glMatrixMode( GL_MODELVIEW );
	glLoadMatrixf( (const float*)GetModelViewTop() );
	glInterleavedArrays( RageVertexFormat, sizeof(RageVertex), v );
	glDrawArrays( GL_TRIANGLES, 0, iNumVerts );
	StatsAddVerts( iNumVerts );
}

void RageDisplay::DrawIndexedTriangles( const RageVertex v[], const Uint16 pIndices[], int iNumIndices )
{
	if( iNumIndices == 0 )
		return;
	ASSERT( iNumIndices >= 3 );
	ASSERT( (iNumIndices%3) == 0 );
	glMatrixMode( GL_PROJECTION );
	glLoadMatrixf( (const float*)GetProjection() );
	glMatrixMode( GL_MODELVIEW );
	glLoadMatrixf( (const float*)GetModelViewTop() );
	glInterleavedArrays( RageVertexFormat, sizeof(RageVertex), v );
	glDrawElements( GL_TRIANGLES, iNumIndices, GL_UNSIGNED_SHORT, pIndices );
	StatsAddVerts( iNumIndices );
}

/* Draw a line as a quad.  GL_LINES with antialiasing off can draw line
 * ends at odd angles--they're forced to axis-alignment regardless of the
 * angle of the line. */
void DrawPolyLine(const RageVertex &p1, const RageVertex &p2, float LineWidth )
{
	/* soh cah toa strikes strikes again! */
	float opp = p2.p.x - p1.p.x;
	float adj = p2.p.y - p1.p.y;
	float hyp = powf(opp*opp + adj*adj, 0.5f);

	float lsin = opp/hyp;
	float lcos = adj/hyp;

	RageVertex v[4];

	v[0] = v[1] = p1;
	v[2] = v[3] = p2;

	float ydist = lsin * LineWidth/2;
	float xdist = lcos * LineWidth/2;
	
	v[0].p.x += xdist;
	v[0].p.y -= ydist;
	v[1].p.x -= xdist;
	v[1].p.y += ydist;
	v[2].p.x -= xdist;
	v[2].p.y += ydist;
	v[3].p.x += xdist;
	v[3].p.y -= ydist;

	DISPLAY->DrawQuad(v);
}

void RageDisplay::DrawLineStrip( const RageVertex v[], int iNumVerts, float LineWidth )
{
	ASSERT( iNumVerts >= 2 );
	
	glMatrixMode( GL_PROJECTION );
	glLoadMatrixf( (const float*)GetProjection() );
	glMatrixMode( GL_MODELVIEW );
	glLoadMatrixf( (const float*)GetModelViewTop() );

	if( g_bAALinesBroken )
	{
		/* Draw a line strip with rounded corners using polys.  This is used on
		 * cards that have strange allergic reactions to antialiased points and
		 * lines. */
		int i;
		for(i = 0; i < iNumVerts-1; ++i)
			DrawPolyLine(v[i], v[i+1], LineWidth);

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
	else
	{
		/* Draw a nice AA'd line loop.  One problem with this is that point and line
		 * sizes don't always precisely match, which doesn't look quite right.
		 * It's worth it for the AA, though. */
		glEnable(GL_LINE_SMOOTH);

		/* Our line width is wrt the regular internal SCREEN_WIDTHxSCREEN_HEIGHT screen,
		 * but these width functions actually want raster sizes (that is, actual pixels).
		 * Scale the line width and point size by the average ratio of the scale. */
		float WidthVal = float(wind->GetWidth()) / SCREEN_WIDTH;
		float HeightVal = float(wind->GetHeight()) / SCREEN_HEIGHT;
		LineWidth *= (WidthVal + HeightVal) / 2;

		/* Clamp the width to the hardware max for both lines and points (whichever
		 * is more restrictive). */
		LineWidth = clamp(LineWidth, g_line_range[0], g_line_range[1]);
		LineWidth = clamp(LineWidth, g_point_range[0], g_point_range[1]);

		/* Hmm.  The granularity of lines and points might be different; for example,
		 * if lines are .5 and points are .25, we might want to snap the width to the
		 * nearest .5, so the hardware doesn't snap them to different sizes.  Does it
		 * matter? */
		glLineWidth(LineWidth);

		/* Draw the line loop: */
		glInterleavedArrays( RageVertexFormat, sizeof(RageVertex), v );
		glDrawArrays( GL_LINE_STRIP, 0, iNumVerts );

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
}

void RageDisplay::SetTexture( RageTexture* pTexture )
{
	glEnable( GL_TEXTURE_2D );
	unsigned id = pTexture ? pTexture->GetTexHandle() : 0; 
	glBindTexture( GL_TEXTURE_2D, id );
}
void RageDisplay::SetTextureModeModulate()
{
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void RageDisplay::SetTextureModeGlow(GlowMode m)
{
	if(m == GLOW_WHITEN && !g_bEXT_texture_env_combine)
		m = GLOW_BRIGHTEN; /* we can't do GLOW_WHITEN */

	switch(m)
	{
	case GLOW_BRIGHTEN:
		glBlendFunc( GL_SRC_ALPHA, GL_ONE );
		return;

	case GLOW_WHITEN:
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
		return;
	}
}
void RageDisplay::SetTextureFiltering( bool b )
{

}

void RageDisplay::SetBlendMode( BlendMode mode )
{
	glEnable(GL_BLEND);

	switch( mode )
	{
	case BLEND_NORMAL:
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		break;
	case BLEND_ADD:
		glBlendFunc( GL_ONE, GL_ONE );
		break;
	case BLEND_NO_EFFECT:
		glBlendFunc( GL_ZERO, GL_ONE );
		break;
	default:
		ASSERT(0);
	}
}

bool RageDisplay::IsZBufferEnabled() const
{
	bool a;
	glGetBooleanv( GL_DEPTH_TEST, (unsigned char*)&a );
	return a;
}

void RageDisplay::SetZBuffer( bool b )
{
	glDepthFunc(GL_LEQUAL);
	if( b )
		glEnable( GL_DEPTH_TEST );
	else
		glDisable( GL_DEPTH_TEST );
}
void RageDisplay::ClearZBuffer()
{
    glClear( GL_DEPTH_BUFFER_BIT );
}

void RageDisplay::SetTextureWrapping( bool b )
{
	GLenum mode = b ? GL_REPEAT : GL_CLAMP;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode );
}

void RageDisplay::SetMaterial( 
	float emissive[4],
	float ambient[4],
	float diffuse[4],
	float specular[4],
	float shininess
	)
{
	glMaterialfv( GL_FRONT, GL_EMISSION, emissive );
	glMaterialfv( GL_FRONT, GL_AMBIENT, ambient );
	glMaterialfv( GL_FRONT, GL_DIFFUSE, diffuse );
	glMaterialfv( GL_FRONT, GL_SPECULAR, specular );
	glMaterialf( GL_FRONT, GL_SHININESS, shininess );
}

void RageDisplay::SetLighting( bool b )
{
	if( b )	glEnable( GL_LIGHTING );
	else	glDisable( GL_LIGHTING );
}

void RageDisplay::SetLightOff( int index )
{
	glDisable( GL_LIGHT0+index );
}
void RageDisplay::SetLightDirectional( 
	int index, 
	RageColor ambient, 
	RageColor diffuse, 
	RageColor specular, 
	RageVector3 dir )
{
	// Light coordinates are transformed by the modelview matrix, but
	// we are being passed in world-space coords.
	glPushMatrix();
	glLoadIdentity();

	glEnable( GL_LIGHT0+index );
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	float position[4] = {dir.x, dir.y, dir.z, 0};
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glPopMatrix();
}

void RageDisplay::SetBackfaceCull( bool b )
{
	if( b )
		glEnable( GL_CULL_FACE );
	else
        glDisable( GL_CULL_FACE );
}

const PixelFormatDesc PIXEL_FORMAT_DESC[NUM_PIX_FORMATS] = {
	{
		/* B8G8R8A8 */
		32,
		{ 0x000000FF,
		  0x0000FF00,
		  0x00FF0000,
		  0xFF000000 }
	}, {
		/* B4G4R4A4 */
		16,
		{ 0xF000,
		  0x0F00,
		  0x00F0,
		  0x000F },
	}, {
		/* B5G5R5A1 */
		16,
		{ 0xF800,
		  0x07C0,
		  0x003E,
		  0x0001 },
	}, {
		/* B5G5R5 */
		16,
		{ 0xF800,
		  0x07C0,
		  0x003E,
		  0x0000 },
	}, {
		/* B8G8R8 */
		24,
		{ 0x0000FF,
		  0x00FF00,
		  0xFF0000,
		  0x000000 }
	}, {
		/* Paletted */
		8,
		{ 0,0,0,0 } /* N/A */
	}
};


struct GLPixFmtInfo_t {
	GLenum internalfmt; /* target format */
	GLenum format; /* target format */
	GLenum type; /* data format */
} GL_PIXFMT_INFO[NUM_PIX_FORMATS] = {
	/* XXX: GL_UNSIGNED_SHORT_4_4_4_4 is affected by endianness; GL_UNSIGNED_BYTE
	 * is not, but all SDL masks are affected by endianness, so GL_UNSIGNED_BYTE
	 * is reversed.  This isn't endian-safe. */
	{
		/* B8G8R8A8 */
		GL_RGBA8,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
	}, {
		/* B4G4R4A4 */
		GL_RGBA4,
		GL_RGBA,
		GL_UNSIGNED_SHORT_4_4_4_4,
	}, {
		/* B5G5R5A1 */
		GL_RGB5_A1,
		GL_RGBA,
		GL_UNSIGNED_SHORT_5_5_5_1,
	}, {
		/* B5G5R5 */
		GL_RGB5,
		GL_RGBA,
		GL_UNSIGNED_SHORT_5_5_5_1,
	}, {
		/* B8G8R8 */
		GL_RGB8,
		GL_RGB,
		GL_UNSIGNED_BYTE,
	}, {
		/* Paletted */
		GL_COLOR_INDEX8_EXT,
		GL_COLOR_INDEX,
		GL_UNSIGNED_BYTE,
	}
};


void RageDisplay::DeleteTexture( unsigned uTexHandle )
{
	unsigned int uTexID = uTexHandle;
	glDeleteTextures(1,reinterpret_cast<GLuint*>(&uTexID));

	GLenum error = glGetError();
	ASSERT( error == GL_NO_ERROR );
}


unsigned RageDisplay::CreateTexture( 
	PixelFormat pixfmt,
	SDL_Surface*& img )
{
	unsigned int uTexHandle;
	glGenTextures(1, reinterpret_cast<GLuint*>(&uTexHandle));
	ASSERT(uTexHandle);
	
	glBindTexture( GL_TEXTURE_2D, uTexHandle );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	/* This is an experiment: some people are seeing skips when the danger
	 * background comes in.  One hypothesis: we're loading the danger background,
	 * keeping it in memory without displaying it for several games, it gets
	 * swapped out of texture memory, and then we skip swapping it in.
	 *
	 * It's rather small, so I'm not entirely convinced this is what's happening.
	 * Let's try boosting the texture priority for the danger background and
	 * see if it goes away.  If it does, I'll make texture pris a texture hint
	 * (or find another way to do this; perhaps we should do a dummy render of
	 * a texture when it's reused from our texture cache if it hasn't been used
	 * in a long time). */
	// TODO: Find a way to add this back in. -Chris
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 0.5f);
//	if(ID.filename.Find("danger") != -1)
//		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 1.0f);

	// texture must be power of two
	ASSERT( img->w == power_of_two(img->w) );
	ASSERT( img->h == power_of_two(img->h) );

	/* TODO:  Add this back in later.  -Chris */
//	if( IsPackedPixelFormat(pixfmt) && g_bPackedPixelsCauseProblems )
//	{
		/* HACK:
		 * GLDirect crashes if we give it packed pixel format.  We need
		 * to convert to a non-packed format (FMT_RGBA8).  However, we
		 * can still get 16-bit textures by passing it the appropriate
		 * internalformat; it'll simply truncate the extra bits.  This
		 * means that it'll still use the dithering we gave it; we just
		 * have to do a bit of redundant conversion work. */
//		pixfmt = FMT_RGBA8;
//		ConvertSDLSurface(img, img->w, img->h, PixFmtMasks[pixfmt].bpp,
//			PixFmtMasks[pixfmt].masks[0], PixFmtMasks[pixfmt].masks[1],
//			PixFmtMasks[pixfmt].masks[2], PixFmtMasks[pixfmt].masks[3]);
//	}
	

//retry:

	if(pixfmt == FMT_PAL)
	{
		/* The image is paletted.  Let's try to set up a paletted texture. */
		GLubyte palette[256*4];
		memset(palette, 0, sizeof(palette));
		int p = 0;
		/* Copy the palette to the simple, unpacked data OGL expects. If
		 * we're color keyed, change it over as we go. */
		for(int i = 0; i < img->format->palette->ncolors; ++i)
		{
			palette[p++] = img->format->palette->colors[i].r;
			palette[p++] = img->format->palette->colors[i].g;
			palette[p++] = img->format->palette->colors[i].b;

			if(img->flags & SDL_SRCCOLORKEY && i == int(img->format->colorkey))
				palette[p++] = 0;
			else
				palette[p++] = 0xFF; /* opaque */
		}

		/* Set the palette. */
		GLExt::glColorTableEXT(GL_TEXTURE_2D, GL_RGBA8, 256, GL_RGBA, GL_UNSIGNED_BYTE, palette);

		GLint RealFormat = 0;
		GLExt::glGetColorTableParameterivEXT(GL_TEXTURE_2D, GL_COLOR_TABLE_FORMAT, &RealFormat);
		ASSERT( RealFormat == GL_RGBA8);	/* This is a case I don't expect to happen. */
	}

	glPixelStorei(GL_UNPACK_ROW_LENGTH, img->pitch / img->format->BytesPerPixel);

	GLenum glTexFormat = GL_PIXFMT_INFO[pixfmt].internalfmt;
	GLenum glImageFormat = GL_PIXFMT_INFO[pixfmt].format;
	GLenum glImageType = GL_PIXFMT_INFO[pixfmt].type;

	glTexImage2D(GL_TEXTURE_2D, 0, glTexFormat, 
			img->w, img->h, 0,
			glImageFormat, glImageType, img->pixels);

	GLenum error = glGetError();
	ASSERT( error == GL_NO_ERROR );

	/* If we're paletted, and didn't get the 8-bit palette we asked for ...*/
	if( pixfmt == FMT_PAL )
	{
		GLint size = 0;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GLenum(GL_TEXTURE_INDEX_SIZE_EXT), &size);
		if(size != 8)
		{
			/* I don't know any reason this should actually fail (paletted textures
			 * but no support for 8-bit palettes?), so let's just disable paletted
			 * textures the first time this happens. */
			LOG->Info("Expected an 8-bit palette, got a %i-bit one instead; disabling paletted textures", size);
//			DISPLAY->DisablePalettedTexture();
			
			/* TODO:  Fix this */
//			ASSERT(0);
//			pixfmt = FMT_RGBA8;
//			ConvertSDLSurface(img, img->w, img->h, PixFmtMasks[pixfmt].bpp,
//				PixFmtMasks[pixfmt].masks[0], PixFmtMasks[pixfmt].masks[1],
//				PixFmtMasks[pixfmt].masks[2], PixFmtMasks[pixfmt].masks[3]);
//			goto retry;
		}
	}

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glFlush();

	return uTexHandle;
}


void RageDisplay::UpdateTexture( 
	unsigned uTexHandle, 
	PixelFormat pixfmt,
	SDL_Surface*& img,
	int xoffset, int yoffset, int width, int height )
{
	glBindTexture( GL_TEXTURE_2D, uTexHandle );

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	/* XXX: We should use m_lVidPitch; we might be padded.  However, I can't
	 * find any codec that don't force the width to a multiple of at least
	 * 4 anyway, so I can't test it, so I'll leave it like this for now. */
	glPixelStorei(GL_UNPACK_ROW_LENGTH, img->w);

//	GLenum glTexFormat = GL_PIXFMT_INFO[pixfmt].internalfmt;
	GLenum glImageFormat = GL_PIXFMT_INFO[pixfmt].format;
	GLenum glImageType = GL_PIXFMT_INFO[pixfmt].type;

	glTexSubImage2D(GL_TEXTURE_2D, 0,
		xoffset, yoffset,
		width, height,
		glImageFormat, glImageType, img->pixels);

	/* Must unset PixelStore when we're done! */
	glPixelStorei(GL_UNPACK_SWAP_BYTES, 0);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glFlush();
}

void RageDisplay::SetAlphaTest( bool b )
{
	glAlphaFunc( GL_GREATER, 0.01f );
	if( b )
		glEnable( GL_ALPHA_TEST );
	else
		glDisable( GL_ALPHA_TEST );
}
