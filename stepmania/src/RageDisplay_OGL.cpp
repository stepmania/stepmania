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

#include "RageFile.h"
#include "PrefsManager.h" // XXX

/* Windows's broken gl.h defines GL_EXT_paletted_texture incompletely: */
#ifndef GL_TEXTURE_INDEX_SIZE_EXT
#define GL_TEXTURE_INDEX_SIZE_EXT         0x80ED
#endif
#include <set>
#include <sstream>

/* Not in glext.h: */
typedef bool (APIENTRY * PWSWAPINTERVALEXTPROC) (int interval);


/* Extension functions we use.  Put these in a namespace instead of in oglspecs_t,
 * so they can be called like regular functions. */
namespace GLExt {
	extern PWSWAPINTERVALEXTPROC wglSwapIntervalEXT;
	extern PFNGLCOLORTABLEPROC glColorTableEXT;
	extern PFNGLCOLORTABLEPARAMETERIVPROC glGetColorTableParameterivEXT;
	extern PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
	extern PFNGLGENBUFFERSARBPROC glGenBuffersARB;
	extern PFNGLBINDBUFFERARBPROC glBindBufferARB;
	extern PFNGLBUFFERDATAARBPROC glBufferDataARB;
	extern PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;
};

#if defined(DARWIN)
#include "archutils/Darwin/Vsync.h"
#endif

#include "RageDisplay.h"
#include "RageDisplay_OGL.h"
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
#include "SDL_endian.h"

#include "arch/arch.h"
#include "arch/LowLevelWindow/LowLevelWindow.h"

#ifdef WIN32
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#endif

//
// Globals
//

PWSWAPINTERVALEXTPROC GLExt::wglSwapIntervalEXT = NULL;
PFNGLCOLORTABLEPROC GLExt::glColorTableEXT = NULL;
PFNGLCOLORTABLEPARAMETERIVPROC GLExt::glGetColorTableParameterivEXT = NULL;
PFNGLACTIVETEXTUREARBPROC GLExt::glActiveTextureARB = NULL;
PFNGLGENBUFFERSARBPROC GLExt::glGenBuffersARB = NULL;
PFNGLBINDBUFFERARBPROC GLExt::glBindBufferARB = NULL;
PFNGLBUFFERDATAARBPROC GLExt::glBufferDataARB = NULL;
PFNGLDELETEBUFFERSARBPROC GLExt::glDeleteBuffersARB = NULL;
static bool g_bEXT_texture_env_combine = true;
static bool g_bGL_EXT_bgra = true;

static bool g_bReversePackedPixelsWorks = true;
static bool g_b4BitPalettesWork = true;

/* OpenGL system information that generally doesn't change at runtime. */

/* Range and granularity of points and lines: */
float g_line_range[2];
float g_line_granularity;
float g_point_range[2];
float g_point_granularity;

/* OpenGL version * 10: */
int g_glVersion;

/* GLU version * 10: */
int g_gluVersion;

/* Available extensions: */
set<string> g_glExts;

/* We don't actually use normals (we don't tunr on lighting), there's just
 * no GL_T2F_C4F_V3F. */
const GLenum RageSpriteVertexFormat = GL_T2F_C4F_N3F_V3F;


LowLevelWindow *wind;


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

static map<GLenum, CString> g_Strings;
static void InitStringMap()
{
	static bool Initialized = false;
	if(Initialized) return;
	Initialized = true;
	#define X(a) g_Strings[a] = #a;
	X(GL_RGBA8);	X(GL_RGBA4);	X(GL_RGB5_A1);	X(GL_RGB5);	X(GL_RGBA);	X(GL_RGB);
	X(GL_BGR);	X(GL_BGRA);
	X(GL_COLOR_INDEX8_EXT);	X(GL_COLOR_INDEX4_EXT);	X(GL_COLOR_INDEX);
	X(GL_UNSIGNED_BYTE);	X(GL_UNSIGNED_SHORT_4_4_4_4); X(GL_UNSIGNED_SHORT_5_5_5_1);
	X(GL_UNSIGNED_SHORT_1_5_5_5_REV);
	X(GL_INVALID_ENUM); X(GL_INVALID_VALUE); X(GL_INVALID_OPERATION);
	X(GL_STACK_OVERFLOW); X(GL_STACK_UNDERFLOW); X(GL_OUT_OF_MEMORY);
}

static CString GLToString( GLenum e )
{
	if( g_Strings.find(e) != g_Strings.end() )
		return g_Strings[e];

	return ssprintf( "%i", int(e) );
}

/* GL_PIXFMT_INFO is used for both texture formats and surface formats.  For example,
 * it's fine to ask for a FMT_RGB5 texture, but to supply a surface matching
 * FMT_RGB8.  OpenGL will simply discard the extra bits.
 *
 * It's possible for a format to be supported as a texture format but not as a
 * surface format.  For example, if packed pixels aren't supported, we can still
 * use GL_RGB5_A1, but we'll have to convert to a supported surface pixel format
 * first.  It's not ideal, since we'll convert to RGBA8 and OGL will convert back,
 * but it works fine.
 */
struct GLPixFmtInfo_t {
	GLenum internalfmt; /* target format */
	GLenum format; /* target format */
	GLenum type; /* data format */
} GL_PIXFMT_INFO[RageDisplay::NUM_PIX_FORMATS] = {
	{
		/* R8G8B8A8 */
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
	}, {
		/* B8G8R8 */
		GL_RGB8,
		GL_BGR,
		GL_UNSIGNED_BYTE,
	}, {
		/* A1R5G5B5 (matches D3DFMT_A1R5G5B5) */
		GL_RGB5_A1,
		GL_BGRA,
		GL_UNSIGNED_SHORT_1_5_5_5_REV,
	}
};


static void FixLilEndian()
{
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	static bool Initialized = false;
	if( Initialized )
		return;
	Initialized = true;

	for( int i = 0; i < RageDisplay::NUM_PIX_FORMATS; ++i )
	{
		RageDisplay::PixelFormatDesc &pf = PIXEL_FORMAT_DESC[i];

		/* OpenGL and SDL handle byte formats differently; we need
		 * to flip non-paletted masks to make them line up. */
		if( GL_PIXFMT_INFO[i].type != GL_UNSIGNED_BYTE || pf.bpp == 8 )
			continue;

		for( int mask = 0; mask < 4; ++mask)
		{
			int m = pf.masks[mask];
			switch( pf.bpp )
			{
			case 24: m = mySDL_Swap24(m); break;
			case 32: m = SDL_Swap32(m); break;
			default: ASSERT(0);
			}
			pf.masks[mask] = m;
		}
	}
#endif
}


void GetGLExtensions(set<string> &ext)
{
    const char *buf = (const char *)glGetString(GL_EXTENSIONS);

	vector<CString> lst;
	split(buf, " ", lst);

	for(unsigned i = 0; i < lst.size(); ++i)
		ext.insert(lst[i]);
}

static void FlushGLErrors()
{
	/* Making an OpenGL call doesn't also flush the error state; if we happen
	 * to have an error from a previous call, then the assert below will fail. 
	 * Flush it. */
	while( glGetError() != GL_NO_ERROR )
		;
}

static AssertNoGLError()
{
	GLenum error = glGetError();
	if( error != GL_NO_ERROR )
	{
		LOG->Trace( GLToString(error) );
		ASSERT(0);
	}
}


#if defined(__unix__) && !defined(unix)
#define unix
#endif

#if defined(unix)
#define Font X11___Font
#define Screen X11___Screen
#include "GL/glx.h"
#undef Font
#undef Screen
#endif

#if defined(unix)
static Display *g_X11Display = NULL;
#endif

static void LogGLXDebugInformation()
{
#if defined(unix)
	ASSERT( g_X11Display );

	const int scr = DefaultScreen( g_X11Display );

	LOG->Info( "Display: %s", DisplayString(g_X11Display) );
	LOG->Info( "Screen: %i", scr );
	LOG->Info( "Direct rendering: %s", glXIsDirect( g_X11Display, glXGetCurrentContext() )? "yes":"no" );

	int XServerVersion = XVendorRelease( g_X11Display ); /* eg. 40201001 */
	int major = XServerVersion / 10000000; XServerVersion %= 10000000;
	int minor = XServerVersion / 100000;   XServerVersion %= 100000;
	int revision = XServerVersion / 1000;  XServerVersion %= 1000;
	int patch = XServerVersion;
	LOG->Info( "X server vendor: %s, %i.%i.%i.%i", XServerVendor( g_X11Display ), major, minor, revision, patch );

	LOG->Info( "Server GLX vendor: %s", glXQueryServerString( g_X11Display, scr, GLX_VENDOR ) );
	LOG->Info( "Server GLX version: %s", glXQueryServerString( g_X11Display, scr, GLX_VERSION ) );
	LOG->Info( "Client GLX vendor: %s", glXGetClientString( g_X11Display, GLX_VENDOR ) );
	LOG->Info( "Client GLX version: %s", glXGetClientString( g_X11Display, GLX_VERSION ) );
#endif
}

RageDisplay_OGL::RageDisplay_OGL( VideoModeParams p, bool bAllowUnacceleratedRenderer )
{
	LOG->Trace( "RageDisplay_OGL::RageDisplay_OGL()" );
	LOG->MapLog("renderer", "Current renderer: OpenGL");

	FixLilEndian();
	InitStringMap();

	wind = MakeLowLevelWindow();

	try {
		SetVideoMode( p );
	} catch(...) {
		/* SetVideoMode can throw. */
		delete wind;
		throw;
	}

	// Log driver details
	LOG->Info("OGL Vendor: %s", glGetString(GL_VENDOR));
	LOG->Info("OGL Renderer: %s", glGetString(GL_RENDERER));
	LOG->Info("OGL Version: %s", glGetString(GL_VERSION));
	LOG->Info("OGL Extensions: %s", glGetString(GL_EXTENSIONS));
	LOG->Info("OGL Max texture size: %i", GetMaxTextureSize() );
	LOG->Info("GLU Version: %s", gluGetString(GLU_VERSION));

	LogGLXDebugInformation();

	if( IsSoftwareRenderer() )
	{
		if( !bAllowUnacceleratedRenderer )
		{
			delete wind;
			RageException::ThrowNonfatal(
				"Your system is reporting that OpenGL hardware acceleration is not available.  "
				"Please obtain an updated driver from your video card manufacturer.\n\n" );
		}
		LOG->Warn("This is a software renderer!");
	}

#if defined(_WINDOWS)
	/* GLDirect is a Direct3D wrapper for OpenGL.  It's rather buggy; and if in
	 * any case GLDirect can successfully render us, we should be able to do so
	 * too using Direct3D directly.  (If we can't, it's a bug that we can work
	 * around--if GLDirect can do it, so can we!) */
	if( !strncmp( (const char *) glGetString(GL_RENDERER), "GLDirect", 8 ) )
	{
		delete wind;
		RageException::ThrowNonfatal( "GLDirect was detected.  GLDirect is not compatible with StepMania, and should be disabled.\n" );
	}
#endif

#if defined(unix)
	if( !glXIsDirect( g_X11Display, glXGetCurrentContext() ) )
	{
		if( !bAllowUnacceleratedRenderer )
		{
			delete wind;
			RageException::ThrowNonfatal(
				"Your system is reporting that direct rendering is not available.  "
				"Please obtain an updated driver from your video card manufacturer." );
		}
		LOG->Warn("Direct rendering is not enabled!");
	}
#endif

	/* Log this, so if people complain that the radar looks bad on their
	 * system we can compare them: */
	glGetFloatv(GL_LINE_WIDTH_RANGE, g_line_range);
	glGetFloatv(GL_LINE_WIDTH_GRANULARITY, &g_line_granularity);
	LOG->Info("Line width range: %.3f-%.3f +%.3f", g_line_range[0], g_line_range[1], g_line_granularity);

	glGetFloatv(GL_POINT_SIZE_RANGE, g_point_range);
	glGetFloatv(GL_POINT_SIZE_GRANULARITY, &g_point_granularity);
	LOG->Info("Point size range: %.3f-%.3f +%.3f", g_point_range[0], g_point_range[1], g_point_granularity);
}

#if defined(unix) && defined(HAVE_LIBXTST)
#include <X11/extensions/XTest.h>
#endif

void RageDisplay_OGL::Update(float fDeltaTime)
{
	wind->Update(fDeltaTime);

	if( PREFSMAN->m_bDisableScreenSaver )
	{
		/* Disable the screensaver. */
#if defined(unix) && defined(HAVE_LIBXTST)
		ASSERT( g_X11Display );

		/* This causes flicker. */
		// XForceScreenSaver( g_X11Display, ScreenSaverReset );
		
		/*
		 * Instead, send a null relative mouse motion, to trick X into thinking there has been
		 * user activity. 
		 *
		 * This also handles XScreenSaver; XForceScreenSaver only handles the internal X11
		 * screen blanker.
		 *
		 * This will delay the X blanker, DPMS and XScreenSaver from activating, and will
		 * disable the blanker and XScreenSaver if they're already active (unless XSS is
		 * locked).  For some reason, it doesn't un-blank DPMS if it's already active.
		 */

		XLockDisplay( g_X11Display );

		int event_base, error_base, major, minor;
		if( XTestQueryExtension( g_X11Display, &event_base, &error_base, &major, &minor ) )
		{
			XTestFakeRelativeMotionEvent( g_X11Display, 0, 0, 0 );
			XSync( g_X11Display, False );
		}

		XUnlockDisplay( g_X11Display );
#endif
	}
}

bool RageDisplay_OGL::IsSoftwareRenderer()
{
#if defined(WIN32)
	return 
		( strcmp((const char*)glGetString(GL_VENDOR),"Microsoft Corporation")==0 ) &&
		( strcmp((const char*)glGetString(GL_RENDERER),"GDI Generic")==0 );
#else
	return false;
#endif
}

RageDisplay_OGL::~RageDisplay_OGL()
{
	delete wind;
}

bool HasExtension(CString ext)
{
	return g_glExts.find(ext) != g_glExts.end();
}

static void CheckPalettedTextures( bool LowColor )
{
	CString error;
	do
	{
		if( GLExt::glColorTableEXT == NULL )
		{
			error = "glColorTableEXT missing";
			break;
		}

		if( GLExt::glGetColorTableParameterivEXT == NULL )
		{
			error = "glGetColorTableParameterivEXT missing";
			break;
		}

		/* Check to see if paletted textures really work. */
		GLenum glTexFormat = GL_PIXFMT_INFO[RageDisplay::FMT_PAL].internalfmt;
		GLenum glImageFormat = GL_PIXFMT_INFO[RageDisplay::FMT_PAL].format;
		GLenum glImageType = GL_PIXFMT_INFO[RageDisplay::FMT_PAL].type;

		int bits = 8;

		if( LowColor )
		{
			glTexFormat = GL_COLOR_INDEX4_EXT;
			bits = 4;
		}

		FlushGLErrors();
#define GL_CHECK_ERROR(f) \
{ \
	GLenum glError = glGetError(); \
	if( glError != GL_NO_ERROR ) { \
		error = ssprintf(f " failed (%s)", GLToString(glError).c_str() ); \
		break; \
	} \
}

		glTexImage2D(GL_PROXY_TEXTURE_2D,
				0, glTexFormat, 
				16, 16, 0,
				glImageFormat, glImageType, NULL);
		GL_CHECK_ERROR( "glTexImage2D" );

		GLuint ifmt = 0;
		glGetTexLevelParameteriv( GL_PROXY_TEXTURE_2D, 0, GLenum(GL_TEXTURE_INTERNAL_FORMAT), (GLint *)&ifmt );
		GL_CHECK_ERROR( "glGetTexLevelParameteriv(GL_TEXTURE_INTERNAL_FORMAT)" );
		if( ifmt != glTexFormat )
		{
			error = ssprintf( "Expected format %s, got %s instead",
					GLToString(glTexFormat).c_str(),
					GLToString(ifmt).c_str() );
			break;
		}

		GLubyte palette[256*4];
		memset(palette, 0, sizeof(palette));
		GLExt::glColorTableEXT(GL_PROXY_TEXTURE_2D, GL_RGBA8, 1 << bits, GL_RGBA, GL_UNSIGNED_BYTE, palette);
		GL_CHECK_ERROR( "glColorTableEXT" );

		GLint size = 0;
		glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GLenum(GL_TEXTURE_INDEX_SIZE_EXT), &size);
		GL_CHECK_ERROR( "glGetTexLevelParameteriv(GL_TEXTURE_INDEX_SIZE_EXT)" );
		if( bits > size || size > 8 )
		{
			error = ssprintf("Expected %i-bit palette, got a %i-bit one instead", bits, int(size));
			break;
		}

		GLint RealWidth = 0;
		GLExt::glGetColorTableParameterivEXT(GL_PROXY_TEXTURE_2D, GL_COLOR_TABLE_WIDTH, &RealWidth);
		GL_CHECK_ERROR( "glGetColorTableParameterivEXT(GL_COLOR_TABLE_WIDTH)" );
		if( RealWidth != 1 << bits )
		{
			error = ssprintf("GL_COLOR_TABLE_WIDTH returned %i instead of %i", int(RealWidth), 1 << bits );
			break;
		}

		GLint RealFormat = 0;
		GLExt::glGetColorTableParameterivEXT(GL_PROXY_TEXTURE_2D, GL_COLOR_TABLE_FORMAT, &RealFormat);
		GL_CHECK_ERROR( "glGetColorTableParameterivEXT(GL_COLOR_TABLE_FORMAT)" );
		if( RealFormat != GL_RGBA8 )
		{
			error = ssprintf("GL_COLOR_TABLE_FORMAT returned %s instead of GL_RGBA8", GLToString(RealFormat).c_str() );
			break;
		}
	} while(0);
#undef GL_CHECK_ERROR

	if( error == "" )
		return;

	if( LowColor )
	{
		/* Disable 4-bit palettes, but allow 8-bit ones. */
		g_b4BitPalettesWork = false;
		LOG->Info("4-bit paletted textures disabled: %s.", error.c_str());
	} else {
		/* If 8-bit palettes don't work, disable them entirely--don't trust 4-bit
		 * palettes if it can't even get 8-bit ones right. */
		GLExt::glColorTableEXT = NULL;
		GLExt::glGetColorTableParameterivEXT = NULL;
		LOG->Info("Paletted textures disabled: %s.", error.c_str());
	}
}

static void CheckReversePackedPixels()
{
	/* Try to create a texture. */
	FlushGLErrors();
	glTexImage2D(GL_PROXY_TEXTURE_2D,
				0, GL_RGBA, 
				16, 16, 0,
				GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, NULL);

	const GLenum glError = glGetError();
	if( glError == GL_NO_ERROR )
		g_bReversePackedPixelsWorks = true;
	else
	{
		g_bReversePackedPixelsWorks = false;
		LOG->Info("GL_UNSIGNED_SHORT_1_5_5_5_REV failed (%s), disabled",
			GLToString(glError).c_str() );
	}
}

void SetupExtensions()
{
	const float fGLVersion = (float) atof( (const char *) glGetString(GL_VERSION) );
	g_glVersion = int(roundf(fGLVersion * 10));
	GetGLExtensions(g_glExts);

	const float fGLUVersion = (float) atof( (const char *) gluGetString(GLU_VERSION) );
	g_gluVersion = int(roundf(fGLUVersion * 10));

	/* Find extension functions and reset broken flags */
#if !defined(DARWIN)
	GLExt::wglSwapIntervalEXT = (PWSWAPINTERVALEXTPROC) wind->GetProcAddress("wglSwapIntervalEXT");
#else
    GLExt::wglSwapIntervalEXT = wglSwapIntervalEXT;
#endif
	GLExt::glColorTableEXT = (PFNGLCOLORTABLEPROC) wind->GetProcAddress("glColorTableEXT");
	GLExt::glGetColorTableParameterivEXT = (PFNGLCOLORTABLEPARAMETERIVPROC) wind->GetProcAddress("glGetColorTableParameterivEXT");
	GLExt::glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC) wind->GetProcAddress("glActiveTextureARB");
	GLExt::glGenBuffersARB = (PFNGLGENBUFFERSARBPROC) wind->GetProcAddress("glGenBuffersARB");
	GLExt::glBindBufferARB = (PFNGLBINDBUFFERARBPROC) wind->GetProcAddress("glBindBufferARB");
	GLExt::glBufferDataARB = (PFNGLBUFFERDATAARBPROC) wind->GetProcAddress("glBufferDataARB");
	GLExt::glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) wind->GetProcAddress("glDeleteBuffersARB");
	g_bEXT_texture_env_combine = HasExtension("GL_EXT_texture_env_combine");
	g_bGL_EXT_bgra = HasExtension("GL_EXT_bgra");
	CheckPalettedTextures( false );
	if( g_b4BitPalettesWork ) // don't bother if the last one failed
		CheckPalettedTextures( true );
	CheckReversePackedPixels();

	// Checks for known bad drivers
	CString sRenderer = (const char*)glGetString(GL_RENDERER);
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
		if( i != Actual )
			continue;
//		if(i == Actual) str += "*** ";
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

void RageDisplay_OGL::ResolutionChanged()
{
 	SetViewport(0,0);

	/* Clear any junk that's in the framebuffer. */
	BeginFrame();
	EndFrame();
}

// Return true if mode change was successful.
// bNewDeviceOut is set true if a new device was created and textures
// need to be reloaded.
CString RageDisplay_OGL::TryVideoMode( VideoModeParams p, bool &bNewDeviceOut )
{
//	LOG->Trace( "RageDisplay_OGL::SetVideoMode( %d, %d, %d, %d, %d, %d )", windowed, width, height, bpp, rate, vsync );
	CString err;
	err = wind->TryVideoMode( p, bNewDeviceOut );
	if( err != "" )
		return err;	// failed to set video mode

	if( bNewDeviceOut )
	{
		/* We have a new OpenGL context, so we have to tell our textures that
		 * their OpenGL texture number is invalid. */
		if(TEXTUREMAN)
			TEXTUREMAN->InvalidateTextures();
	}

#if defined(unix)
	{
		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);

		g_X11Display = NULL;
		if ( SDL_GetWMInfo(&info) < 0 )
			LOG->Warn("SDL_GetWMInfo failed: %s", SDL_GetError());
		else
			g_X11Display = info.info.x11.display;
	}
#endif

	this->SetDefaultRenderStates();
	DumpOpenGLDebugInfo();

	/* Now that we've initialized, we can search for extensions (some of which
	 * we may need to set up the video mode). */
	SetupExtensions();

	/* Set vsync the Windows way, if we can.  (What other extensions are there
	 * to do this, for other archs?) */
	if( GLExt::wglSwapIntervalEXT )
	    GLExt::wglSwapIntervalEXT(p.vsync);
	
	ResolutionChanged();

	return "";	// successfully set mode
}

void RageDisplay_OGL::SetViewport(int shift_left, int shift_down)
{
	/* left and down are on a 0..SCREEN_WIDTH, 0..SCREEN_HEIGHT scale.
	 * Scale them to the actual viewport range. */
	shift_left = int( shift_left * float(wind->GetVideoModeParams().width) / SCREEN_WIDTH );
	shift_down = int( shift_down * float(wind->GetVideoModeParams().height) / SCREEN_HEIGHT );

	glViewport(shift_left, -shift_down, wind->GetVideoModeParams().width, wind->GetVideoModeParams().height);
}

int RageDisplay_OGL::GetMaxTextureSize() const
{
	GLint size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
	return size;
}

void RageDisplay_OGL::BeginFrame()
{
	glClearColor( 0,0,0,1 );
	SetZWrite( true );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void RageDisplay_OGL::EndFrame()
{
	wind->SwapBuffers();
	ProcessStatsOnFlip();
}

bool RageDisplay_OGL::Supports4BitPalettes()
{
	return g_b4BitPalettesWork;
}

SDL_Surface* RageDisplay_OGL::CreateScreenshot()
{
	int width = wind->GetVideoModeParams().width;
	int height = wind->GetVideoModeParams().height;

	const PixelFormatDesc &desc = PIXEL_FORMAT_DESC[FMT_RGB8];
	SDL_Surface *image = SDL_CreateRGBSurfaceSane(
		SDL_SWSURFACE, width, height,
        desc.bpp, desc.masks[0], desc.masks[1], desc.masks[2], desc.masks[3] );

	SDL_Surface *temp = SDL_CreateRGBSurfaceSane(
		SDL_SWSURFACE, width, height,
        desc.bpp, desc.masks[0], desc.masks[1], desc.masks[2], desc.masks[3] );

	glReadPixels(0, 0, wind->GetVideoModeParams().width, wind->GetVideoModeParams().height, GL_RGB,
	             GL_UNSIGNED_BYTE, image->pixels);

	// flip vertically
	int pitch = image->pitch;
	for( int y=0; y<wind->GetVideoModeParams().height; y++ )
		memcpy( 
			(char *)temp->pixels + pitch * y,
			(char *)image->pixels + pitch * (height-1-y),
			3*width );
	SDL_FreeSurface( image );

	return temp;
}

RageDisplay::VideoModeParams RageDisplay_OGL::GetVideoModeParams() const { return wind->GetVideoModeParams(); }

static void SetupVertices( const RageSpriteVertex v[], int iNumVerts )
{
	static float *Vertex, *Texture, *Normal;	
	static GLubyte *Color;
	static int Size = 0;
	if(iNumVerts > Size)
	{
		Size = iNumVerts;
		delete [] Vertex;
		delete [] Color;
		delete [] Texture;
		delete [] Normal;
		Vertex = new float[Size*3];
		Color = new GLubyte[Size*4];
		Texture = new float[Size*2];
		Normal = new float[Size*3];
	}

	for(unsigned i = 0; i < unsigned(iNumVerts); ++i)
	{
		Vertex[i*3+0]  = v[i].p[0];
		Vertex[i*3+1]  = v[i].p[1];
		Vertex[i*3+2]  = v[i].p[2];
		Color[i*4+0]   = v[i].c.r;
		Color[i*4+1]   = v[i].c.g;
		Color[i*4+2]   = v[i].c.b;
		Color[i*4+3]   = v[i].c.a;
		Texture[i*2+0] = v[i].t[0];
		Texture[i*2+1] = v[i].t[1];
		Normal[i*3+0] = v[i].n[0];
		Normal[i*3+1] = v[i].n[1];
		Normal[i*3+2] = v[i].n[2];
	}
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, Vertex);

	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, Color);

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, Texture);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normal);
}

void RageDisplay_OGL::SendCurrentMatrices()
{
	glMatrixMode( GL_PROJECTION );
	glLoadMatrixf( (const float*)GetProjectionTop() );
	RageMatrix modelView;
	RageMatrixMultiply( &modelView, GetCentering(), GetViewTop() );
	RageMatrixMultiply( &modelView, &modelView, GetWorldTop() );
	glMatrixMode( GL_MODELVIEW );
	glLoadMatrixf( (const float*)&modelView );
}

class RageModelVertexArraySWOGL : public RageModelVertexArray
{
public:
	RageModelVertexArraySWOGL()
	{
		m_sizeVerts = 0;
		m_sizeTriangles = 0;
		m_pPosition = NULL;
		m_pTexture = NULL;
		m_pNormal = NULL;
		m_pBone = NULL;
		m_pTriangles = NULL;
	}

	~RageModelVertexArraySWOGL()
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

	void OnChanged() const
	{
		// nothing to do.  We send all of the vertices every time we draw.
	}
	void Draw() const
	{
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, m_pPosition);

		glDisableClientState(GL_COLOR_ARRAY);

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, m_pTexture);

		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, m_pNormal);

		glDrawElements( GL_TRIANGLES, m_sizeTriangles*3, GL_UNSIGNED_SHORT, m_pTriangles );
	}
protected:
	size_t		m_sizeVerts;
	size_t		m_sizeTriangles;
	RageVector3 *m_pPosition;
	RageVector2 *m_pTexture;
	RageVector3 *m_pNormal;
	Sint8		*m_pBone;
	msTriangle	*m_pTriangles;
};

class RageModelVertexArrayHWOGL : public RageModelVertexArraySWOGL
{
protected:
	// vertex buffer object names
	GLuint m_nPositions;
	GLuint m_nTextureCoords;
	GLuint m_nNormals;
	GLuint m_nTriangles;
public:
	RageModelVertexArrayHWOGL()
	{
		m_nPositions = 0;
		m_nTextureCoords = 0;
		m_nNormals = 0;
		m_nTriangles = 0;

		FlushGLErrors();

		GLExt::glGenBuffersARB( 1, &m_nPositions );
		GLExt::glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nPositions );
		AssertNoGLError();

		GLExt::glGenBuffersARB( 1, &m_nTextureCoords );
		GLExt::glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nTextureCoords );
		AssertNoGLError();

		GLExt::glGenBuffersARB( 1, &m_nNormals );
		GLExt::glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nNormals );
		AssertNoGLError();

		GLExt::glGenBuffersARB( 1, &m_nTriangles );
		GLExt::glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, m_nTriangles );
		AssertNoGLError();

		GLExt::glBindBufferARB( GL_ARRAY_BUFFER_ARB, NULL );
		GLExt::glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, NULL );
	}

	~RageModelVertexArrayHWOGL()
	{
		FlushGLErrors();

		GLExt::glDeleteBuffersARB( 1, &m_nPositions );
		AssertNoGLError();
		GLExt::glDeleteBuffersARB( 1, &m_nTextureCoords );
		AssertNoGLError();
		GLExt::glDeleteBuffersARB( 1, &m_nNormals );
		AssertNoGLError();
		GLExt::glDeleteBuffersARB( 1, &m_nTriangles );
		AssertNoGLError();
	}
	
	void OnChanged() const
	{
		// some implementations don't handle buffers of size 0.
		if( m_sizeVerts == 0  ||  m_nTriangles == 0 )
			return;

		GLExt::glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nPositions );
		GLExt::glBufferDataARB( GL_ARRAY_BUFFER_ARB, m_sizeVerts*sizeof(RageVector3), m_pPosition, GL_STATIC_DRAW_ARB );
		AssertNoGLError();

		GLExt::glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nTextureCoords );
		GLExt::glBufferDataARB( GL_ARRAY_BUFFER_ARB, m_sizeVerts*sizeof(RageVector2), m_pTexture, GL_STATIC_DRAW_ARB );
		AssertNoGLError();

		GLExt::glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nNormals );
		GLExt::glBufferDataARB( GL_ARRAY_BUFFER_ARB, m_sizeVerts*sizeof(RageVector3), m_pNormal, GL_STATIC_DRAW_ARB );
		AssertNoGLError();

		GLExt::glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, m_nTriangles );
		GLExt::glBufferDataARB( GL_ELEMENT_ARRAY_BUFFER_ARB, m_sizeTriangles*sizeof(msTriangle), m_pTriangles, GL_STATIC_DRAW_ARB );
		AssertNoGLError();

		GLExt::glBindBufferARB( GL_ARRAY_BUFFER_ARB, NULL );
		GLExt::glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, NULL );
	}
	void Draw() const
	{
		if( m_sizeVerts == 0  ||  m_nTriangles == 0 )
			return;

		FlushGLErrors();

		glEnableClientState(GL_VERTEX_ARRAY);
		GLExt::glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nPositions );
		AssertNoGLError();
		glVertexPointer(3, GL_FLOAT, 0, NULL );
		AssertNoGLError();

		glDisableClientState(GL_COLOR_ARRAY);

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		GLExt::glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nTextureCoords );
		AssertNoGLError();
		glTexCoordPointer(2, GL_FLOAT, 0, NULL);
		AssertNoGLError();

		glEnableClientState(GL_NORMAL_ARRAY);
		GLExt::glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nNormals );
		AssertNoGLError();
		glNormalPointer(GL_FLOAT, 0, NULL);
		AssertNoGLError();

		GLExt::glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, m_nTriangles );
		AssertNoGLError();

		glDrawElements( GL_TRIANGLES, m_sizeTriangles*3, GL_UNSIGNED_SHORT, 0 );  //last 0 is offset in element-array
		AssertNoGLError();

		GLExt::glBindBufferARB( GL_ARRAY_BUFFER_ARB, NULL );
		GLExt::glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, NULL );
	}
};

RageModelVertexArray* RageDisplay_OGL::CreateRageModelVertexArray()
{
	if( GLExt::glGenBuffersARB )
		return new RageModelVertexArraySWOGL;
	else
		return new RageModelVertexArraySWOGL;
}

void RageDisplay_OGL::DeleteRageModelVertexArray( RageModelVertexArray* p )
{
	delete p;
}

void RageDisplay_OGL::DrawQuads( const RageSpriteVertex v[], int iNumVerts )
{
	ASSERT( (iNumVerts%4) == 0 );

	if(iNumVerts == 0)
		return;

	SendCurrentMatrices();

	SetupVertices( v, iNumVerts );
	glDrawArrays( GL_QUADS, 0, iNumVerts );

	StatsAddVerts( iNumVerts );
}

void RageDisplay_OGL::DrawFan( const RageSpriteVertex v[], int iNumVerts )
{
	ASSERT( iNumVerts >= 3 );
	glMatrixMode( GL_PROJECTION );

	SendCurrentMatrices();

	SetupVertices( v, iNumVerts );
	glDrawArrays( GL_TRIANGLE_FAN, 0, iNumVerts );
	StatsAddVerts( iNumVerts );
}

void RageDisplay_OGL::DrawStrip( const RageSpriteVertex v[], int iNumVerts )
{
	ASSERT( iNumVerts >= 3 );

	SendCurrentMatrices();

	SetupVertices( v, iNumVerts );
	glDrawArrays( GL_TRIANGLE_STRIP, 0, iNumVerts );
	StatsAddVerts( iNumVerts );
}

void RageDisplay_OGL::DrawTriangles( const RageSpriteVertex v[], int iNumVerts )
{
	if( iNumVerts == 0 )
		return;
	ASSERT( (iNumVerts%3) == 0 );

	SendCurrentMatrices();

	SetupVertices( v, iNumVerts );
	glDrawArrays( GL_TRIANGLES, 0, iNumVerts );
	StatsAddVerts( iNumVerts );
}

void RageDisplay_OGL::DrawIndexedTriangles( const RageModelVertexArray *p )
{
	SendCurrentMatrices();

	p->Draw();

	StatsAddVerts( p->sizeTriangles()*3 );
}

void RageDisplay_OGL::DrawLineStrip( const RageSpriteVertex v[], int iNumVerts, float LineWidth )
{
	ASSERT( iNumVerts >= 2 );

	if( !GetVideoModeParams().bSmoothLines )
	{
		RageDisplay::DrawLineStrip(v, iNumVerts, LineWidth );
		return;
	}

	SendCurrentMatrices();

	/* Draw a nice AA'd line loop.  One problem with this is that point and line
	 * sizes don't always precisely match, which doesn't look quite right.
	 * It's worth it for the AA, though. */
	glEnable(GL_LINE_SMOOTH);

	/* Our line width is wrt the regular internal SCREEN_WIDTHxSCREEN_HEIGHT screen,
	 * but these width functions actually want raster sizes (that is, actual pixels).
	 * Scale the line width and point size by the average ratio of the scale. */
	float WidthVal = float(wind->GetVideoModeParams().width) / SCREEN_WIDTH;
	float HeightVal = float(wind->GetVideoModeParams().height) / SCREEN_HEIGHT;
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
	SetupVertices( v, iNumVerts );
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

	SetupVertices( v, iNumVerts );
	glDrawArrays( GL_POINTS, 0, iNumVerts );

	glDisable(GL_POINT_SMOOTH);
}

void RageDisplay_OGL::ClearAllTextures()
{
	for( int i=0; i<MAX_TEXTURE_UNITS; i++ )
		SetTexture( i, NULL );

	// HACK:  Reset the active texture to 0.
	// TODO:  Change all texture functions to take a stage number.
	if( GLExt::glActiveTextureARB )
		GLExt::glActiveTextureARB(GL_TEXTURE0_ARB);
}

void RageDisplay_OGL::SetTexture( int iTextureUnitIndex, RageTexture* pTexture )
{
	if( GLExt::glActiveTextureARB == NULL )
	{
		// multitexture isn't supported.  Ignore all textures except for 0.
		if( iTextureUnitIndex != 0 )
			return;
	}
	else
	{
		switch( iTextureUnitIndex )
		{
		case 0:
			GLExt::glActiveTextureARB(GL_TEXTURE0_ARB);
			break;
		case 1:
			GLExt::glActiveTextureARB(GL_TEXTURE1_ARB);
			break;
		default:
			ASSERT(0);
		}
	}

	if( pTexture )
	{
		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, pTexture->GetTexHandle() );
	}
	else
	{
		glDisable( GL_TEXTURE_2D );
	}
}
void RageDisplay_OGL::SetTextureModeModulate()
{
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void RageDisplay_OGL::SetTextureModeGlow(GlowMode m)
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

void RageDisplay_OGL::SetTextureModeAdd()
{
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
}

void RageDisplay_OGL::SetTextureFiltering( bool b )
{

}

void RageDisplay_OGL::SetBlendMode( BlendMode mode )
{
	glEnable(GL_BLEND);

	glDepthRange( 0.05, 1.0 );
	switch( mode )
	{
	case BLEND_NORMAL:
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		break;
	case BLEND_ADD:
		glBlendFunc( GL_SRC_ALPHA, GL_ONE );
		break;
	case BLEND_NO_EFFECT:
		/* XXX: Would it be faster and have the same effect to say glDisable(GL_COLOR_WRITEMASK)? */
		glBlendFunc( GL_ZERO, GL_ONE );

		/* This is almost exclusively used to draw masks to the Z-buffer.  Make sure
		 * masks always win the depth test when drawn at the same position. */
		glDepthRange( 0.0, 0.95 );
		break;
	default:
		ASSERT(0);
	}
}

bool RageDisplay_OGL::IsZWriteEnabled() const
{
	bool a;
	glGetBooleanv( GL_DEPTH_WRITEMASK, (unsigned char*)&a );
	return a;
}

bool RageDisplay_OGL::IsZTestEnabled() const
{
	GLenum a;
	glGetIntegerv( GL_DEPTH_FUNC, (GLint*)&a );
	return a != GL_ALWAYS;
}

void RageDisplay_OGL::ClearZBuffer()
{
	bool write = IsZWriteEnabled();
	SetZWrite( true );
    glClear( GL_DEPTH_BUFFER_BIT );
	SetZWrite( write );
}

void RageDisplay_OGL::SetZWrite( bool b )
{
	glDepthMask( b );
}

void RageDisplay_OGL::SetZTest( bool b )
{
	glEnable( GL_DEPTH_TEST );
	if( b )
		glDepthFunc( GL_LEQUAL );
	else
		glDepthFunc( GL_ALWAYS );
}

void RageDisplay_OGL::SetTextureWrapping( bool b )
{
	GLenum mode = b ? GL_REPEAT : GL_CLAMP_TO_EDGE;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode );
}

void RageDisplay_OGL::SetMaterial( 
	const RageColor &emissive,
	const RageColor &ambient,
	const RageColor &diffuse,
	const RageColor &specular,
	float shininess
	)
{
	glMaterialfv( GL_FRONT, GL_EMISSION, emissive );
	glMaterialfv( GL_FRONT, GL_AMBIENT, ambient );
	glMaterialfv( GL_FRONT, GL_DIFFUSE, diffuse );
	glMaterialfv( GL_FRONT, GL_SPECULAR, specular );
	glMaterialf( GL_FRONT, GL_SHININESS, shininess );
}

void RageDisplay_OGL::SetLighting( bool b )
{
	if( b )	glEnable( GL_LIGHTING );
	else	glDisable( GL_LIGHTING );
}

void RageDisplay_OGL::SetLightOff( int index )
{
	glDisable( GL_LIGHT0+index );
}
void RageDisplay_OGL::SetLightDirectional( 
	int index, 
	const RageColor &ambient, 
	const RageColor &diffuse, 
	const RageColor &specular, 
	const RageVector3 &dir )
{
	// Light coordinates are transformed by the modelview matrix, but
	// we are being passed in world-space coords.
	glPushMatrix();
	glLoadIdentity();

	glEnable( GL_LIGHT0+index );
	glLightfv(GL_LIGHT0+index, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0+index, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0+index, GL_SPECULAR, specular);
	float position[4] = {dir.x, dir.y, dir.z, 0};
	glLightfv(GL_LIGHT0+index, GL_POSITION, position);

	glPopMatrix();
}

void RageDisplay_OGL::SetCullMode( CullMode mode )
{
	switch( mode )
	{
	case CULL_BACK:
		glEnable( GL_CULL_FACE );
		glCullFace( GL_BACK );
		break;
	case CULL_FRONT:
		glEnable( GL_CULL_FACE );
		glCullFace( GL_FRONT );
		break;
	case CULL_NONE:
        glDisable( GL_CULL_FACE );
		break;
	default:
		ASSERT(0);
	}
}

const RageDisplay::PixelFormatDesc *RageDisplay_OGL::GetPixelFormatDesc(PixelFormat pf) const
{
	ASSERT( pf < NUM_PIX_FORMATS );
	return &PIXEL_FORMAT_DESC[pf];
}

void RageDisplay_OGL::DeleteTexture( unsigned uTexHandle )
{
	unsigned int uTexID = uTexHandle;

	FlushGLErrors();
	glDeleteTextures(1,reinterpret_cast<GLuint*>(&uTexID));

	AssertNoGLError();
}


RageDisplay::PixelFormat RageDisplay_OGL::GetImgPixelFormat( SDL_Surface* &img, bool &FreeImg, int width, int height )
{
	PixelFormat pixfmt = FindPixelFormat( img->format->BitsPerPixel, img->format->Rmask, img->format->Gmask, img->format->Bmask, img->format->Amask );
	
	if( pixfmt == NUM_PIX_FORMATS || !SupportsSurfaceFormat(pixfmt) )
	{
		/* The source isn't in a supported, known pixel format.  We need to convert
		 * it ourself.  Just convert it to RGBA8, and let OpenGL convert it back
		 * down to whatever the actual pixel format is.  This is a very slow code
		 * path, which should almost never be used. */
		pixfmt = FMT_RGBA8;
		ASSERT( SupportsSurfaceFormat(pixfmt) );

		const PixelFormatDesc *pfd = DISPLAY->GetPixelFormatDesc(pixfmt);

		SDL_SetAlpha( img, 0, SDL_ALPHA_OPAQUE );

		SDL_Rect area;
		area.x = area.y = 0;
		area.w = short(width);
		area.h = short(height);

		SDL_Surface *imgconv = SDL_CreateRGBSurfaceSane(SDL_SWSURFACE, width, height,
			pfd->bpp, pfd->masks[0], pfd->masks[1], pfd->masks[2], pfd->masks[3]);
		SDL_BlitSurface(img, &area, imgconv, &area);
		img = imgconv;
		FreeImg = true;
	}
	else
		FreeImg = false;

	return pixfmt;
}

unsigned RageDisplay_OGL::CreateTexture( 
	PixelFormat pixfmt,
	SDL_Surface* img,
	bool bGenerateMipMaps )
{
	ASSERT( pixfmt < NUM_PIX_FORMATS );
	ASSERT( img->w == power_of_two(img->w) && img->h == power_of_two(img->h) );


	/* Find the pixel format of the image we've been given. */
	bool FreeImg;
	PixelFormat imgpixfmt = GetImgPixelFormat( img, FreeImg, img->w, img->h );

	GLenum glTexFormat = GL_PIXFMT_INFO[pixfmt].internalfmt;
	GLenum glImageFormat = GL_PIXFMT_INFO[imgpixfmt].format;
	GLenum glImageType = GL_PIXFMT_INFO[imgpixfmt].type;

	/* If we support 4-bit palettes, and this image fits in 4 bits, then change
	 * internalformat to GL_COLOR_INDEX4_EXT.  We'll still upload it as 256 colors,
	 * but OpenGL can discard the extra bits. */
	if( (img->unused1 & FOUR_BIT_PALETTE) && Supports4BitPalettes() )
	{
		LOG->Trace("did 4 bit");
		glTexFormat = GL_COLOR_INDEX4_EXT;
	}

	
	// HACK:  OpenGL 1.2 types aren't available in GLU 1.3.  Don't call GLU for mip
	// mapping if we're using an OGL 1.2 type and don't have >= GLU 1.3.
	// http://pyopengl.sourceforge.net/documentation/manual/gluBuild2DMipmaps.3G.html
	if( bGenerateMipMaps && g_gluVersion < 13 )
	{
		switch( pixfmt )
		{
		// OpenGL 1.1 types
		case FMT_RGBA8:
		case FMT_RGB8:
		case FMT_PAL:
		case FMT_BGR8:
			break;
		// OpenGL 1.2 types
		default:
			LOG->Trace( "Can't generate mipmaps for type %s because GLU version %.1f is too old.", GLToString(glImageType).c_str(), g_gluVersion/10.f );
			bGenerateMipMaps = false;
			break;
		}
	}

	// allocate OpenGL texture resource
	unsigned int uTexHandle;
	glGenTextures(1, reinterpret_cast<GLuint*>(&uTexHandle));
	ASSERT(uTexHandle);
	
	glBindTexture( GL_TEXTURE_2D, uTexHandle );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bGenerateMipMaps ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR);
	SetTextureWrapping( false );

	glPixelStorei(GL_UNPACK_ROW_LENGTH, img->pitch / img->format->BytesPerPixel);


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
				palette[p++] = img->format->palette->colors[i].unused;
		}

		/* Set the palette. */
		GLExt::glColorTableEXT(GL_TEXTURE_2D, GL_RGBA8, 256, GL_RGBA, GL_UNSIGNED_BYTE, palette);

		GLint RealFormat = 0;
		GLExt::glGetColorTableParameterivEXT(GL_TEXTURE_2D, GL_COLOR_TABLE_FORMAT, &RealFormat);
		ASSERT( RealFormat == GL_RGBA8);	/* This is a case I don't expect to happen. */
	}


	
	FlushGLErrors();

	if( bGenerateMipMaps )
	{
		GLenum error = gluBuild2DMipmaps(
			GL_TEXTURE_2D, glTexFormat, 
			img->w, img->h,
			glImageFormat, glImageType, img->pixels );

		if( error != 0 )
		{
			ostringstream s;
			s << "gluBuild2DMipmaps(format " << GLToString(glTexFormat) <<
				 ", w " << img->w << ", h " <<  img->h <<
				 ", format " << GLToString(glImageFormat) <<
				 ", type " << GLToString(glImageType) <<
				 "): " << gluErrorString(error);
			LOG->Trace( s.str().c_str() );

			ASSERT(0);
		}
	}
	else
	{
		glTexImage2D(
			GL_TEXTURE_2D, 0, glTexFormat, 
			img->w, img->h, 0,
			glImageFormat, glImageType, img->pixels);
		
		GLenum error = glGetError();
		if( error != GL_NO_ERROR )
		{
			ostringstream s;
			s << "glTexImage2D(format " << GLToString(glTexFormat) <<
				 ", w " << img->w << ", h " <<  img->h <<
				 ", format " << GLToString(glImageFormat) <<
				 ", type " << GLToString(glImageType) <<
				 "): " << GLToString(error);
			LOG->Trace( s.str().c_str() );

			ASSERT(0);
		}
	}



	/* Sanity check: */
	if( pixfmt == FMT_PAL )
	{
		GLint size = 0;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GLenum(GL_TEXTURE_INDEX_SIZE_EXT), &size);
		if(size != 8)
			RageException::Throw("Thought paletted textures worked, but they don't.");
	}

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glFlush();

	if( FreeImg )
		SDL_FreeSurface( img );
	return uTexHandle;
}


void RageDisplay_OGL::UpdateTexture( 
	unsigned uTexHandle, 
	SDL_Surface* img,
	int xoffset, int yoffset, int width, int height )
{
	glBindTexture( GL_TEXTURE_2D, uTexHandle );

	bool FreeImg;
	PixelFormat pixfmt = GetImgPixelFormat( img, FreeImg, width, height );

	glPixelStorei(GL_UNPACK_ROW_LENGTH, img->pitch / img->format->BytesPerPixel);

//	GLenum glTexFormat = GL_PIXFMT_INFO[pixfmt].internalfmt;
	GLenum glImageFormat = GL_PIXFMT_INFO[pixfmt].format;
	GLenum glImageType = GL_PIXFMT_INFO[pixfmt].type;

	glTexSubImage2D(GL_TEXTURE_2D, 0,
		xoffset, yoffset,
		width, height,
		glImageFormat, glImageType, img->pixels);

	/* Must unset PixelStore when we're done! */
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glFlush();

	if( FreeImg )
		SDL_FreeSurface( img );
}

CString RageDisplay_OGL::GetTextureDiagnostics( unsigned id ) const
{
	return "";
}

void RageDisplay_OGL::SetAlphaTest( bool b )
{
	glAlphaFunc( GL_GREATER, 0.01f );
	if( b )
		glEnable( GL_ALPHA_TEST );
	else
		glDisable( GL_ALPHA_TEST );
}

RageMatrix RageDisplay_OGL::GetOrthoMatrix( float l, float r, float b, float t, float zn, float zf )
{
	RageMatrix m(
		2/(r-l),      0,            0,           0,
		0,            2/(t-b),      0,           0,
		0,            0,            -2/(zf-zn),   0,
		-(r+l)/(r-l), -(t+b)/(t-b), -(zf+zn)/(zf-zn),  1 );
	return m;
}


/*
 * Although we pair texture formats (eg. GL_RGB8) and surface formats
 * (pairs of eg. GL_RGB8,GL_UNSIGNED_SHORT_5_5_5_1), it's possible for
 * a format to be supported for a texture format but not a surface
 * format.  This is abstracted, so you don't need to know about this
 * as a user calling CreateTexture.
 *
 * One case of this is if packed pixels aren't supported.  We can still
 * use 16-bit color modes, but we have to send it in 32-bit.  Almost
 * everything supports packed pixels.
 *
 * Another case of this is incomplete packed pixels support.  Some implementations
 * neglect GL_UNSIGNED_SHORT_*_REV. 
 */
bool RageDisplay_OGL::SupportsSurfaceFormat( PixelFormat pixfmt )
{
	switch( GL_PIXFMT_INFO[pixfmt].type )
	{
	case GL_UNSIGNED_SHORT_1_5_5_5_REV:
		return g_bGL_EXT_bgra && g_bReversePackedPixelsWorks;
	default:
		return true;
	}
}


bool RageDisplay_OGL::SupportsTextureFormat( PixelFormat pixfmt, bool realtime )
{
	/* If we support a pixfmt for texture formats but not for surface formats, then
	 * we'll have to convert the texture to a supported surface format before uploading.
	 * This is too slow for dynamic textures. */
	if( realtime && !SupportsSurfaceFormat( pixfmt ) )
		return false;

	switch( GL_PIXFMT_INFO[pixfmt].format )
	{
	case GL_COLOR_INDEX:
		return GLExt::glColorTableEXT && GLExt::glGetColorTableParameterivEXT;
	case GL_BGR:
	case GL_BGRA:
		return g_bGL_EXT_bgra;
	default:
		return true;
	}
}

void RageDisplay_OGL::SetSphereEnironmentMapping( bool b )
{
	if( b )
	{
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
	}
	else
	{
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
	}
}
