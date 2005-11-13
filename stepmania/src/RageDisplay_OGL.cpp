#include "global.h"

/* ours may be more up-to-date */
#define __glext_h_

#if defined(WIN32)
#include <windows.h>
#endif

#if !defined(MACOSX)
# include <GL/gl.h>
# include <GL/glu.h>
#else
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
#endif

#undef __glext_h_
#include "glext.h"

#include "RageSurface.h"
#include "RageSurfaceUtils.h"
#include "ScreenDimensions.h" // XXX

/* Windows's broken gl.h defines GL_EXT_paletted_texture incompletely: */
#ifndef GL_TEXTURE_INDEX_SIZE_EXT
#define GL_TEXTURE_INDEX_SIZE_EXT         0x80ED
#endif
#include <set>
#include <sstream>

#if defined(MACOSX)
#include "archutils/Darwin/Vsync.h"
#endif

#include "RageDisplay.h"
#include "RageDisplay_OGL.h"
#include "RageDisplay_OGL_Extensions.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageTimer.h"
#include "RageException.h"
#include "RageTexture.h"
#include "RageTextureManager.h"
#include "RageMath.h"
#include "RageTypes.h"
#include "RageUtil.h"
#include "EnumHelper.h"
#include "ProductInfo.h"

#include "arch/LowLevelWindow/LowLevelWindow.h"

#include "arch/arch.h"

#if defined(_MSC_VER)
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#endif

//
// Globals
//

static bool g_bReversePackedPixelsWorks = true;
static bool g_bColorIndexTableWorks = true;

/* OpenGL system information that generally doesn't change at runtime. */

/* Range and granularity of points and lines: */
float g_line_range[2];
float g_point_range[2];

/* OpenGL version * 10: */
int g_glVersion;

/* GLU version * 10: */
int g_gluVersion;

static int g_iMaxTextureUnits = 0;

/* We don't actually use normals (we don't turn on lighting), there's just
 * no GL_T2F_C4F_V3F. */
const GLenum RageSpriteVertexFormat = GL_T2F_C4F_N3F_V3F;

/* If we support texture matrix scaling, a handle to the vertex program: */
static GLhandleARB g_bTextureMatrixShader = 0;

static LowLevelWindow *g_pWind;

static void InvalidateAllGeometry();

static RageDisplay::PixelFormatDesc PIXEL_FORMAT_DESC[NUM_PixelFormat] = {
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

/* g_GLPixFmtInfo is used for both texture formats and surface formats.  For example,
 * it's fine to ask for a PixelFormat_RGB5 texture, but to supply a surface matching
 * PixelFormat_RGB8.  OpenGL will simply discard the extra bits.
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
} const g_GLPixFmtInfo[NUM_PixelFormat] = {
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


static void FixLittleEndian()
{
#if defined(ENDIAN_LITTLE)
	static bool bInitialized = false;
	if( bInitialized )
		return;
	bInitialized = true;

	for( int i = 0; i < NUM_PixelFormat; ++i )
	{
		RageDisplay::PixelFormatDesc &pf = PIXEL_FORMAT_DESC[i];

		/* OpenGL and RageSurface handle byte formats differently; we need
		 * to flip non-paletted masks to make them line up. */
		if( g_GLPixFmtInfo[i].type != GL_UNSIGNED_BYTE || pf.bpp == 8 )
			continue;

		for( int mask = 0; mask < 4; ++mask)
		{
			int m = pf.masks[mask];
			switch( pf.bpp )
			{
			case 24: m = Swap24(m); break;
			case 32: m = Swap32(m); break;
			default: ASSERT(0);
			}
			pf.masks[mask] = m;
		}
	}
#endif
}


static void FlushGLErrors()
{
	/* Making an OpenGL call doesn't also flush the error state; if we happen
	 * to have an error from a previous call, then the assert below will fail. 
	 * Flush it. */
	while( glGetError() != GL_NO_ERROR )
		;
}

#define AssertNoGLError() \
{ \
	GLenum error = glGetError(); \
	ASSERT_M( error == GL_NO_ERROR, GLToString(error) ) \
}

static void TurnOffHardwareVBO()
{
	if( GLExt.glBindBufferARB )
	{
		GLExt.glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
		GLExt.glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
	}
}

RageDisplay_OGL::RageDisplay_OGL()
{
	LOG->Trace( "RageDisplay_OGL::RageDisplay_OGL()" );
	LOG->MapLog("renderer", "Current renderer: OpenGL");

	FixLittleEndian();
	InitStringMap();

	g_pWind = NULL;
	g_bTextureMatrixShader = 0;
}

CString GetInfoLog( GLhandleARB h )
{
	GLint iLength;
	GLExt.glGetObjectParameterivARB( h, GL_OBJECT_INFO_LOG_LENGTH_ARB, &iLength );
	if( !iLength )
		return CString();

	GLcharARB *pInfoLog = new GLcharARB[iLength];
	GLExt.glGetInfoLogARB( h, iLength, &iLength, pInfoLog );
	CString sRet = pInfoLog;
	delete [] pInfoLog;
	return sRet;
}

GLhandleARB CompileShader( GLenum ShaderType, CString sBuffer )
{
	GLhandleARB VertexShader = GLExt.glCreateShaderObjectARB( GL_VERTEX_SHADER_ARB );
	const GLcharARB *pData = sBuffer.data();
	int iLength = sBuffer.size();
	GLExt.glShaderSourceARB( VertexShader, 1, &pData, (GLint*)&iLength );

	GLExt.glCompileShaderARB( VertexShader );

	GLint bCompileStatus  = GL_FALSE;
	GLExt.glGetObjectParameterivARB( VertexShader, GL_OBJECT_COMPILE_STATUS_ARB, &bCompileStatus );

	if( !bCompileStatus )
	{
		LOG->Trace( "Compile failure: %s", GetInfoLog( VertexShader ).c_str() );
		return 0;
	}

	return VertexShader;
}

enum
{
	ATTRIB_TEXTURE_MATRIX_SCALE = 1
};

/* XXX: How should we include these?  Doing them like this is ugly.  Linking them in
 * from another file as a text symbol would be ideal, but that's completely different
 * on each platform, so it'd be a maintenance nightmare.  Reading them from a file would
 * be annoying, too. */
const GLcharARB *g_TextureMatrixScaleShader =
" \
attribute vec4 TextureMatrixScale; \
void main( void ) \
{ \
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; \
	vec4 multiplied_tex_coord = gl_TextureMatrix[0] * gl_MultiTexCoord0; \
	gl_TexCoord[0] = (multiplied_tex_coord * TextureMatrixScale) + \
					(gl_MultiTexCoord0 * (vec4(1)-TextureMatrixScale)); \
	gl_FrontColor = gl_Color; \
} \
";

void InitScalingScript()
{
	g_bTextureMatrixShader = 0;

	if( !GLExt.m_bGL_ARB_shader_objects ||
		!GLExt.m_bGL_ARB_vertex_shader ||
		!GLExt.m_bGL_ARB_shading_language_100 )
		return;

	GLhandleARB VertexShader = CompileShader( GL_VERTEX_SHADER_ARB, g_TextureMatrixScaleShader );
	if( VertexShader == 0 )
		return;

	g_bTextureMatrixShader = GLExt.glCreateProgramObjectARB();
	GLExt.glAttachObjectARB( g_bTextureMatrixShader, VertexShader );
	GLExt.glDeleteObjectARB( VertexShader );

	// Bind attributes.
	GLExt.glBindAttribLocationARB( g_bTextureMatrixShader, ATTRIB_TEXTURE_MATRIX_SCALE, "TextureMatrixScale" );

	// Link the program.
	GLExt.glLinkProgramARB( g_bTextureMatrixShader );
	GLint bLinkStatus = false;
	GLExt.glGetObjectParameterivARB( g_bTextureMatrixShader, GL_OBJECT_LINK_STATUS_ARB, &bLinkStatus );

	if( !bLinkStatus )
	{
		LOG->Trace( "Scaling shader link failed: %s", GetInfoLog(g_bTextureMatrixShader).c_str() );
		GLExt.glDeleteObjectARB( g_bTextureMatrixShader );
		return;
	}

	GLExt.glVertexAttrib2fARB( ATTRIB_TEXTURE_MATRIX_SCALE, 1, 1 );
}

CString RageDisplay_OGL::Init( VideoModeParams p, bool bAllowUnacceleratedRenderer )
{
	g_pWind = MakeLowLevelWindow();

	bool bIgnore = false;
	CString sError = SetVideoMode( p, bIgnore );
	if( sError != "" )
		return sError;

	// Log driver details
	LOG->Info( "OGL Vendor: %s", glGetString(GL_VENDOR) );
	LOG->Info( "OGL Renderer: %s", glGetString(GL_RENDERER) );
	LOG->Info( "OGL Version: %s", glGetString(GL_VERSION) );
	LOG->Info( "OGL Max texture size: %i", GetMaxTextureSize() );
	LOG->Info( "OGL Texture units: %i", g_iMaxTextureUnits );
	LOG->Info( "OGL Extensions: %s", glGetString(GL_EXTENSIONS) );
	LOG->Info( "GLU Version: %s", gluGetString(GLU_VERSION) );

	if( g_pWind->IsSoftwareRenderer(sError) )
	{
		if( !bAllowUnacceleratedRenderer )
			return sError + "  Please obtain an updated driver from your video card manufacturer.\n\n";
		LOG->Warn( "Low-performance OpenGL renderer: " + sError );
	}

#if defined(_WINDOWS)
	/* GLDirect is a Direct3D wrapper for OpenGL.  It's rather buggy; and if in
	 * any case GLDirect can successfully render us, we should be able to do so
	 * too using Direct3D directly.  (If we can't, it's a bug that we can work
	 * around--if GLDirect can do it, so can we!) */
	if( !strncmp( (const char *) glGetString(GL_RENDERER), "GLDirect", 8 ) )
		return "GLDirect was detected.  GLDirect is not compatible with " PRODUCT_NAME ", and should be disabled.\n";
#endif

	/* Log this, so if people complain that the radar looks bad on their
	 * system we can compare them: */
	glGetFloatv( GL_LINE_WIDTH_RANGE, g_line_range );
	glGetFloatv( GL_POINT_SIZE_RANGE, g_point_range );

	InitScalingScript();

	return CString();
}

RageDisplay_OGL::~RageDisplay_OGL()
{
	delete g_pWind;
}

static void CheckPalettedTextures()
{
	CString sError;
	do
	{
		if( !GLExt.HasExtension("GL_EXT_paletted_texture") )
		{
			sError = "GL_EXT_paletted_texture missing";
			break;
		}

		if( GLExt.glColorTableEXT == NULL )
		{
			sError = "glColorTableEXT missing";
			break;
		}

		if( GLExt.glGetColorTableParameterivEXT == NULL )
		{
			sError = "glGetColorTableParameterivEXT missing";
			break;
		}

		/* Check to see if paletted textures really work. */
		GLenum glTexFormat		= g_GLPixFmtInfo[PixelFormat_PAL].internalfmt;
		GLenum glImageFormat	= g_GLPixFmtInfo[PixelFormat_PAL].format;
		GLenum glImageType		= g_GLPixFmtInfo[PixelFormat_PAL].type;

		int iBits = 8;

		FlushGLErrors();
#define GL_CHECK_ERROR(f) \
{ \
	GLenum glError = glGetError(); \
	if( glError != GL_NO_ERROR ) { \
		sError = ssprintf(f " failed (%s)", GLToString(glError).c_str() ); \
		break; \
	} \
}

		glTexImage2D( GL_PROXY_TEXTURE_2D,
				0, glTexFormat, 
				16, 16, 0,
				glImageFormat, glImageType, NULL );
		GL_CHECK_ERROR( "glTexImage2D" );

		GLuint iFormat = 0;
		glGetTexLevelParameteriv( GL_PROXY_TEXTURE_2D, 0, GLenum(GL_TEXTURE_INTERNAL_FORMAT), (GLint *) &iFormat );
		GL_CHECK_ERROR( "glGetTexLevelParameteriv(GL_TEXTURE_INTERNAL_FORMAT)" );
		if( iFormat != glTexFormat )
		{
			sError = ssprintf( "Expected format %s, got %s instead",
					GLToString(glTexFormat).c_str(), GLToString(iFormat).c_str() );
			break;
		}

		GLubyte palette[256*4];
		memset(palette, 0, sizeof(palette));
		GLExt.glColorTableEXT(GL_PROXY_TEXTURE_2D, GL_RGBA8, 256, GL_RGBA, GL_UNSIGNED_BYTE, palette);
		GL_CHECK_ERROR( "glColorTableEXT" );

		GLint iSize = 0;
		glGetTexLevelParameteriv( GL_PROXY_TEXTURE_2D, 0, GLenum(GL_TEXTURE_INDEX_SIZE_EXT), &iSize );
		GL_CHECK_ERROR( "glGetTexLevelParameteriv(GL_TEXTURE_INDEX_SIZE_EXT)" );
		if( iBits > iSize || iSize > 8 )
		{
			sError = ssprintf( "Expected %i-bit palette, got a %i-bit one instead", iBits, int(iSize) );
			break;
		}

		GLint iRealWidth = 0;
		GLExt.glGetColorTableParameterivEXT( GL_PROXY_TEXTURE_2D, GL_COLOR_TABLE_WIDTH, &iRealWidth );
		GL_CHECK_ERROR( "glGetColorTableParameterivEXT(GL_COLOR_TABLE_WIDTH)" );
		if( iRealWidth != 1 << iBits )
		{
			sError = ssprintf( "GL_COLOR_TABLE_WIDTH returned %i instead of %i", int(iRealWidth), 1 << iBits );
			break;
		}

		GLint iRealFormat = 0;
		GLExt.glGetColorTableParameterivEXT( GL_PROXY_TEXTURE_2D, GL_COLOR_TABLE_FORMAT, &iRealFormat );
		GL_CHECK_ERROR( "glGetColorTableParameterivEXT(GL_COLOR_TABLE_FORMAT)" );
		if( iRealFormat != GL_RGBA8 )
		{
			sError = ssprintf( "GL_COLOR_TABLE_FORMAT returned %s instead of GL_RGBA8", GLToString(iRealFormat).c_str() );
			break;
		}
	} while(0);
#undef GL_CHECK_ERROR

	if( sError == "" )
		return;

	/* If 8-bit palettes don't work, disable them entirely--don't trust 4-bit
	 * palettes if it can't even get 8-bit ones right. */
	GLExt.glColorTableEXT = NULL;
	GLExt.glGetColorTableParameterivEXT = NULL;
	LOG->Info( "Paletted textures disabled: %s.", sError.c_str() );
}

static void CheckReversePackedPixels()
{
	/* Try to create a texture. */
	FlushGLErrors();
	glTexImage2D( GL_PROXY_TEXTURE_2D,
				0, GL_RGBA, 
				16, 16, 0,
				GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, NULL );

	const GLenum glError = glGetError();
	if( glError == GL_NO_ERROR )
	{
		g_bReversePackedPixelsWorks = true;
	}
	else
	{
		g_bReversePackedPixelsWorks = false;
		LOG->Info( "GL_UNSIGNED_SHORT_1_5_5_5_REV failed (%s), disabled",
			GLToString(glError).c_str() );
	}
}

void SetupExtensions()
{
	const float fGLVersion = strtof( (const char *) glGetString(GL_VERSION), NULL );
	g_glVersion = int(roundf(fGLVersion * 10));

	const float fGLUVersion = strtof( (const char *) gluGetString(GLU_VERSION), NULL );
	g_gluVersion = int(roundf(fGLUVersion * 10));

	GLExt.Load( g_pWind );

	g_iMaxTextureUnits = 1;
	if( GLExt.glActiveTextureARB != NULL )
		glGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, (GLint *) &g_iMaxTextureUnits );

	CheckPalettedTextures();
	CheckReversePackedPixels();

	{
		GLint iMaxTableSize = 0;
		glGetIntegerv( GL_MAX_PIXEL_MAP_TABLE, &iMaxTableSize );
		if( iMaxTableSize < 256 )
		{
			/* The minimum GL_MAX_PIXEL_MAP_TABLE is 32; if it's not at least 256,
			 * we can't fit a palette in it, so we can't send paletted data as input
			 * for a non-paletted texture. */
			LOG->Info( "GL_MAX_PIXEL_MAP_TABLE is only %d", int(iMaxTableSize) );
			g_bColorIndexTableWorks = false;
		}
		else
		{
			g_bColorIndexTableWorks = true;
		}
	}
}

void RageDisplay_OGL::ResolutionChanged()
{
 	SetViewport(0,0);

	/* Clear any junk that's in the framebuffer. */
	if( BeginFrame() )
		EndFrame();
}

// Return true if mode change was successful.
// bNewDeviceOut is set true if a new device was created and textures
// need to be reloaded.
CString RageDisplay_OGL::TryVideoMode( VideoModeParams p, bool &bNewDeviceOut )
{
//	LOG->Trace( "RageDisplay_OGL::SetVideoMode( %d, %d, %d, %d, %d, %d )", windowed, width, height, bpp, rate, vsync );
	CString err;
	err = g_pWind->TryVideoMode( p, bNewDeviceOut );
	if( err != "" )
		return err;	// failed to set video mode

	/* Now that we've initialized, we can search for extensions.  Do this before InvalidateAllGeometry,
	 * since AllocateBuffers needs it. */
	SetupExtensions();

	if( bNewDeviceOut )
	{
		/* We have a new OpenGL context, so we have to tell our textures that
		 * their OpenGL texture number is invalid. */
		if( TEXTUREMAN )
			TEXTUREMAN->InvalidateTextures();

		/* Recreate all vertex buffers. */
		InvalidateAllGeometry();
	}

	this->SetDefaultRenderStates();

	/* Set vsync the Windows way, if we can.  (What other extensions are there
	 * to do this, for other archs?) */
	if( GLExt.wglSwapIntervalEXT )
	    GLExt.wglSwapIntervalEXT(p.vsync);
	
	ResolutionChanged();

	return CString();	// successfully set mode
}

void RageDisplay_OGL::SetViewport(int shift_left, int shift_down)
{
	/* left and down are on a 0..SCREEN_WIDTH, 0..SCREEN_HEIGHT scale.
	 * Scale them to the actual viewport range. */
	shift_left = int( shift_left * float(g_pWind->GetActualVideoModeParams().width) / SCREEN_WIDTH );
	shift_down = int( shift_down * float(g_pWind->GetActualVideoModeParams().height) / SCREEN_HEIGHT );

	glViewport( shift_left, -shift_down, g_pWind->GetActualVideoModeParams().width, g_pWind->GetActualVideoModeParams().height );
}

int RageDisplay_OGL::GetMaxTextureSize() const
{
	GLint size;
	glGetIntegerv( GL_MAX_TEXTURE_SIZE, &size );
	return size;
}

bool RageDisplay_OGL::BeginFrame()
{
	glClearColor( 0,0,0,1 );
	SetZWrite( true );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	return true;
}

void RageDisplay_OGL::EndFrame()
{
	// glFlush(), not glFinish(); NVIDIA_GLX's glFinish()'s behavior is
	// nowhere near performance-friendly and uses unholy amounts of CPU for
	// Gog-knows-what.
	glFlush();

	g_pWind->SwapBuffers();

	g_pWind->Update();

	RageDisplay::EndFrame();
}

RageSurface* RageDisplay_OGL::CreateScreenshot()
{
	int width = g_pWind->GetActualVideoModeParams().width;
	int height = g_pWind->GetActualVideoModeParams().height;

	const PixelFormatDesc &desc = PIXEL_FORMAT_DESC[PixelFormat_RGBA8];
	RageSurface *image = CreateSurface( width, height, desc.bpp,
		desc.masks[0], desc.masks[1], desc.masks[2], 0 );

	FlushGLErrors();

	glReadBuffer( GL_FRONT );
	AssertNoGLError();
	
	glReadPixels( 0, 0, g_pWind->GetActualVideoModeParams().width, g_pWind->GetActualVideoModeParams().height, GL_RGBA,
	             GL_UNSIGNED_BYTE, image->pixels );
	AssertNoGLError();

	RageSurfaceUtils::FlipVertically( image );

	return image;
}

VideoModeParams RageDisplay_OGL::GetActualVideoModeParams() const 
{
	return g_pWind->GetActualVideoModeParams();
}

static void SetupVertices( const RageSpriteVertex v[], int iNumVerts )
{
	static float *Vertex, *Texture, *Normal;	
	static GLubyte *Color;
	static int Size = 0;
	if( iNumVerts > Size )
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

	for( unsigned i = 0; i < unsigned(iNumVerts); ++i )
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
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 3, GL_FLOAT, 0, Vertex );

	glEnableClientState( GL_COLOR_ARRAY );
	glColorPointer( 4, GL_UNSIGNED_BYTE, 0, Color );

	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glTexCoordPointer( 2, GL_FLOAT, 0, Texture );

	glEnableClientState( GL_NORMAL_ARRAY );
	glNormalPointer( GL_FLOAT, 0, Normal );
}

void RageDisplay_OGL::SendCurrentMatrices()
{
	RageMatrix projection;
	RageMatrixMultiply( &projection, GetCentering(), GetProjectionTop() );
	glMatrixMode( GL_PROJECTION );
	glLoadMatrixf( (const float*)&projection );

	// OpenGL has just "modelView", whereas D3D has "world" and "view"
	RageMatrix modelView;
	RageMatrixMultiply( &modelView, GetViewTop(), GetWorldTop() );
	glMatrixMode( GL_MODELVIEW );
	glLoadMatrixf( (const float*)&modelView );

	glMatrixMode( GL_TEXTURE );
	glLoadMatrixf( (const float*)GetTextureTop() );
}

class RageCompiledGeometrySWOGL : public RageCompiledGeometry
{
public:
	
	void Allocate( const vector<msMesh> &vMeshes )
	{
		m_vPosition.resize( GetTotalVertices() );
		m_vTexture.resize( GetTotalVertices() );
		m_vNormal.resize( GetTotalVertices() );
		m_vTexMatrixScale.resize( GetTotalVertices() );
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
			{
				m_vPosition[meshInfo.iVertexStart+j] = Vertices[j].p;
				m_vTexture[meshInfo.iVertexStart+j] = Vertices[j].t;
				m_vNormal[meshInfo.iVertexStart+j] = Vertices[j].n;
				m_vTexMatrixScale[meshInfo.iVertexStart+j] = Vertices[j].TextureMatrixScale;
			}

			for( unsigned j=0; j<Triangles.size(); j++ )
				for( unsigned k=0; k<3; k++ )
				{
					int iVertexIndexInVBO = meshInfo.iVertexStart + Triangles[j].nVertexIndices[k];
					m_vTriangles[meshInfo.iTriangleStart+j].nVertexIndices[k] = (uint16_t) iVertexIndexInVBO;
				}
		}
	}
	void Draw( int iMeshIndex ) const
	{
		TurnOffHardwareVBO();

		const MeshInfo& meshInfo = m_vMeshInfo[iMeshIndex];

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, &m_vPosition[0]);

		glDisableClientState(GL_COLOR_ARRAY);

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, &m_vTexture[0]);

		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, &m_vNormal[0]);

		if( meshInfo.m_bNeedsTextureMatrixScale )
		{
			// Kill the texture translation.
			// XXX: Change me to scale the translation by the TextureTranslationScale of the first vertex.
			RageMatrix mat;
			glGetFloatv( GL_TEXTURE_MATRIX , (float*)mat );

			/*
			for( int i=0; i<4; i++ )
			{
				CString s;
				for( int j=0; j<4; j++ )
					s += ssprintf( "%f ", mat.m[i][j] );
				LOG->Trace( s );
			}
			*/

			mat.m[3][0] = 0;
			mat.m[3][1] = 0;
			mat.m[3][2] = 0;

			glMatrixMode( GL_TEXTURE );
			glLoadMatrixf( (const float*)mat );
		}

		glDrawElements( 
			GL_TRIANGLES, 
			meshInfo.iTriangleCount*3, 
			GL_UNSIGNED_SHORT, 
			&m_vTriangles[0]+meshInfo.iTriangleStart );
	}

protected:
	vector<RageVector3> m_vPosition;
	vector<RageVector2> m_vTexture;
	vector<RageVector3> m_vNormal;
	vector<msTriangle>	m_vTriangles;
	vector<RageVector2>	m_vTexMatrixScale;
};

class RageCompiledGeometryHWOGL : public RageCompiledGeometrySWOGL
{
protected:
	// vertex buffer object names
	GLuint m_nPositions;
	GLuint m_nTextureCoords;
	GLuint m_nNormals;
	GLuint m_nTriangles;
	GLuint m_nTextureMatrixScale;

	void AllocateBuffers();
	void UploadData();

public:
	RageCompiledGeometryHWOGL();
	~RageCompiledGeometryHWOGL();

	/* This is called when our OpenGL context is invalidated. */
	void Invalidate();
	
	void Allocate( const vector<msMesh> &vMeshes );
	void Change( const vector<msMesh> &vMeshes );
	void Draw( int iMeshIndex ) const;
};

static set<RageCompiledGeometryHWOGL *> g_GeometryList;
static void InvalidateAllGeometry()
{
	FOREACHS( RageCompiledGeometryHWOGL*, g_GeometryList, it )
		(*it)->Invalidate();
}

RageCompiledGeometryHWOGL::RageCompiledGeometryHWOGL()
{
	g_GeometryList.insert( this );
	m_nPositions = m_nTextureCoords = m_nNormals = m_nTriangles = m_nTextureMatrixScale = 0;

	AllocateBuffers();
}

RageCompiledGeometryHWOGL::~RageCompiledGeometryHWOGL()
{
	g_GeometryList.erase( this );
	FlushGLErrors();

	GLExt.glDeleteBuffersARB( 1, &m_nPositions );
	AssertNoGLError();
	GLExt.glDeleteBuffersARB( 1, &m_nTextureCoords );
	AssertNoGLError();
	GLExt.glDeleteBuffersARB( 1, &m_nNormals );
	AssertNoGLError();
	GLExt.glDeleteBuffersARB( 1, &m_nTriangles );
	AssertNoGLError();
	GLExt.glDeleteBuffersARB( 1, &m_nTextureMatrixScale );
	AssertNoGLError();
}

void RageCompiledGeometryHWOGL::AllocateBuffers()
{
	FlushGLErrors();

	if( !m_nPositions )
	{
		GLExt.glGenBuffersARB( 1, &m_nPositions );
		AssertNoGLError();
	}

	if( !m_nTextureCoords )
	{
		GLExt.glGenBuffersARB( 1, &m_nTextureCoords );
		AssertNoGLError();
	}

	if( !m_nNormals )
	{
		GLExt.glGenBuffersARB( 1, &m_nNormals );
		AssertNoGLError();
	}

	if( !m_nTriangles )
	{
		GLExt.glGenBuffersARB( 1, &m_nTriangles );
		AssertNoGLError();
	}

	if( !m_nTextureMatrixScale )
	{
		GLExt.glGenBuffersARB( 1, &m_nTextureMatrixScale );
		AssertNoGLError();
	}
}

void RageCompiledGeometryHWOGL::UploadData()
{
	GLExt.glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nPositions );
	GLExt.glBufferDataARB( 
		GL_ARRAY_BUFFER_ARB, 
		GetTotalVertices()*sizeof(RageVector3), 
		&m_vPosition[0], 
		GL_STATIC_DRAW_ARB );
//		AssertNoGLError();

	GLExt.glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nTextureCoords );
	GLExt.glBufferDataARB( 
		GL_ARRAY_BUFFER_ARB, 
		GetTotalVertices()*sizeof(RageVector2), 
		&m_vTexture[0], 
		GL_STATIC_DRAW_ARB );
//		AssertNoGLError();

	GLExt.glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nNormals );
	GLExt.glBufferDataARB( 
		GL_ARRAY_BUFFER_ARB, 
		GetTotalVertices()*sizeof(RageVector3), 
		&m_vNormal[0], 
		GL_STATIC_DRAW_ARB );
//		AssertNoGLError();

	GLExt.glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, m_nTriangles );
	GLExt.glBufferDataARB( 
		GL_ELEMENT_ARRAY_BUFFER_ARB, 
		GetTotalTriangles()*sizeof(msTriangle), 
		&m_vTriangles[0], 
		GL_STATIC_DRAW_ARB );
//		AssertNoGLError();


	if( m_bAnyNeedsTextureMatrixScale )
	{
		GLExt.glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nTextureMatrixScale );
		GLExt.glBufferDataARB( 
			GL_ARRAY_BUFFER_ARB, 
			GetTotalVertices()*sizeof(RageVector2),
			&m_vTexMatrixScale[0],
			GL_STATIC_DRAW_ARB );
	}
}

void RageCompiledGeometryHWOGL::Invalidate()
{
	/* Our vertex buffers no longer exist.  Reallocate and reupload. */
	m_nPositions = m_nTextureCoords = m_nNormals = m_nTriangles = m_nTextureMatrixScale = 0;
	AllocateBuffers();
	UploadData();
}

void RageCompiledGeometryHWOGL::Allocate( const vector<msMesh> &vMeshes )
{
	RageCompiledGeometrySWOGL::Allocate( vMeshes );
	GLExt.glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nPositions );
	GLExt.glBufferDataARB( 
		GL_ARRAY_BUFFER_ARB, 
		GetTotalVertices()*sizeof(RageVector3), 
		NULL, 
		GL_STATIC_DRAW_ARB );
	AssertNoGLError();

	GLExt.glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nTextureCoords );
	GLExt.glBufferDataARB( 
		GL_ARRAY_BUFFER_ARB, 
		GetTotalVertices()*sizeof(RageVector2), 
		NULL, 
		GL_STATIC_DRAW_ARB );
	AssertNoGLError();

	GLExt.glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nNormals );
	GLExt.glBufferDataARB( 
		GL_ARRAY_BUFFER_ARB, 
		GetTotalVertices()*sizeof(RageVector3), 
		NULL, 
		GL_STATIC_DRAW_ARB );
	AssertNoGLError();

	GLExt.glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, m_nTriangles );
	GLExt.glBufferDataARB( 
		GL_ELEMENT_ARRAY_BUFFER_ARB, 
		GetTotalTriangles()*sizeof(msTriangle), 
		NULL, 
		GL_STATIC_DRAW_ARB );
	AssertNoGLError();

	GLExt.glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nTextureMatrixScale );
	GLExt.glBufferDataARB( 
		GL_ARRAY_BUFFER_ARB, 
		GetTotalVertices()*sizeof(RageVector2), 
		NULL,
		GL_STATIC_DRAW_ARB );
}

void RageCompiledGeometryHWOGL::Change( const vector<msMesh> &vMeshes )
{
	RageCompiledGeometrySWOGL::Change( vMeshes );

	UploadData();
}

void RageCompiledGeometryHWOGL::Draw( int iMeshIndex ) const
{
	FlushGLErrors();

	const MeshInfo& meshInfo = m_vMeshInfo[iMeshIndex];
	if( !meshInfo.iVertexCount || !meshInfo.iTriangleCount )
		return;

	glEnableClientState(GL_VERTEX_ARRAY);
	GLExt.glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nPositions );
	AssertNoGLError();
	glVertexPointer(3, GL_FLOAT, 0, NULL );
	AssertNoGLError();

	glDisableClientState(GL_COLOR_ARRAY);

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	GLExt.glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nTextureCoords );
	AssertNoGLError();
	glTexCoordPointer(2, GL_FLOAT, 0, NULL);
	AssertNoGLError();

	// TRICKY:  Don't bind and send normals if lighting is disabled.  This 
	// will save some effort transforming these values.
	GLboolean bLighting;
	glGetBooleanv( GL_LIGHTING, &bLighting );
	GLboolean bTextureGenS;
	glGetBooleanv( GL_TEXTURE_GEN_S, &bTextureGenS );
	GLboolean bTextureGenT;
	glGetBooleanv( GL_TEXTURE_GEN_T, &bTextureGenT );
	if( bLighting || bTextureGenS || bTextureGenT )
	{
		glEnableClientState(GL_NORMAL_ARRAY);
		GLExt.glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nNormals );
		AssertNoGLError();
		glNormalPointer(GL_FLOAT, 0, NULL);
		AssertNoGLError();
	}
	else
	{
		glDisableClientState(GL_NORMAL_ARRAY);
	}

	if( meshInfo.m_bNeedsTextureMatrixScale )
	{
		if( g_bTextureMatrixShader != 0 )
		{
			/* If we're using texture matrix scales, set up that buffer, too, and enable the
			 * vertex shader.  This shader doesn't support all OpenGL state, so only enable it
			 * if we're using it. */
			GLExt.glEnableVertexAttribArrayARB( ATTRIB_TEXTURE_MATRIX_SCALE );
			AssertNoGLError();
			GLExt.glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nTextureMatrixScale );
			AssertNoGLError();
			GLExt.glVertexAttribPointerARB( ATTRIB_TEXTURE_MATRIX_SCALE, 2, GL_FLOAT, false, 0, NULL );
			AssertNoGLError();

			GLExt.glUseProgramObjectARB( g_bTextureMatrixShader );
		}
		else
		{
			// Kill the texture translation.
			// XXX: Change me to scale the translation by the TextureTranslationScale of the first vertex.
			RageMatrix mat;
			glGetFloatv( GL_TEXTURE_MATRIX , (float*)mat );

			/*
			for( int i=0; i<4; i++ )
			{
				CString s;
				for( int j=0; j<4; j++ )
					s += ssprintf( "%f ", mat.m[i][j] );
				LOG->Trace( s );
			}
			*/

			mat.m[3][0] = 0;
			mat.m[3][1] = 0;
			mat.m[3][2] = 0;

			glMatrixMode( GL_TEXTURE );
			glLoadMatrixf( (const float*)mat );
		}
	}

	GLExt.glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, m_nTriangles );
	AssertNoGLError();

#define BUFFER_OFFSET(o) ((char*)(o))

	ASSERT( GLExt.glDrawRangeElements);
	GLExt.glDrawRangeElements( 
		GL_TRIANGLES, 
		meshInfo.iVertexStart,	// minimum array index contained in indices
		meshInfo.iVertexStart+meshInfo.iVertexCount-1,
					// maximum array index contained in indices
		meshInfo.iTriangleCount*3,	// number of elements to be rendered
		GL_UNSIGNED_SHORT,
		BUFFER_OFFSET(meshInfo.iTriangleStart*sizeof(msTriangle)) );
	AssertNoGLError();

	if( meshInfo.m_bNeedsTextureMatrixScale && g_bTextureMatrixShader != 0 )
	{
		GLExt.glDisableVertexAttribArrayARB( ATTRIB_TEXTURE_MATRIX_SCALE );
		GLExt.glUseProgramObjectARB( 0 );
	}
}

RageCompiledGeometry* RageDisplay_OGL::CreateCompiledGeometry()
{
	if( GLExt.glGenBuffersARB )
		return new RageCompiledGeometryHWOGL;
	else
		return new RageCompiledGeometrySWOGL;
}

void RageDisplay_OGL::DeleteCompiledGeometry( RageCompiledGeometry* p )
{
	delete p;
}

void RageDisplay_OGL::DrawQuadsInternal( const RageSpriteVertex v[], int iNumVerts )
{
	TurnOffHardwareVBO();
	SendCurrentMatrices();

	SetupVertices( v, iNumVerts );
	glDrawArrays( GL_QUADS, 0, iNumVerts );
}

void RageDisplay_OGL::DrawQuadStripInternal( const RageSpriteVertex v[], int iNumVerts )
{
	TurnOffHardwareVBO();
	SendCurrentMatrices();

	SetupVertices( v, iNumVerts );
	glDrawArrays( GL_QUAD_STRIP, 0, iNumVerts );
}

void RageDisplay_OGL::DrawFanInternal( const RageSpriteVertex v[], int iNumVerts )
{
	TurnOffHardwareVBO();

	glMatrixMode( GL_PROJECTION );

	SendCurrentMatrices();

	SetupVertices( v, iNumVerts );
	glDrawArrays( GL_TRIANGLE_FAN, 0, iNumVerts );
}

void RageDisplay_OGL::DrawStripInternal( const RageSpriteVertex v[], int iNumVerts )
{
	TurnOffHardwareVBO();
	SendCurrentMatrices();

	SetupVertices( v, iNumVerts );
	glDrawArrays( GL_TRIANGLE_STRIP, 0, iNumVerts );
}

void RageDisplay_OGL::DrawTrianglesInternal( const RageSpriteVertex v[], int iNumVerts )
{
	TurnOffHardwareVBO();
	SendCurrentMatrices();

	SetupVertices( v, iNumVerts );
	glDrawArrays( GL_TRIANGLES, 0, iNumVerts );
}

void RageDisplay_OGL::DrawCompiledGeometryInternal( const RageCompiledGeometry *p, int iMeshIndex )
{
	TurnOffHardwareVBO();
	SendCurrentMatrices();

	p->Draw( iMeshIndex );
}

void RageDisplay_OGL::DrawLineStripInternal( const RageSpriteVertex v[], int iNumVerts, float fLineWidth )
{
	TurnOffHardwareVBO();

	if( !GetActualVideoModeParams().bSmoothLines )
	{
		/* Fall back on the generic polygon-based line strip. */
		RageDisplay::DrawLineStripInternal(v, iNumVerts, fLineWidth );
		return;
	}

	SendCurrentMatrices();

	/* Draw a nice AA'd line loop.  One problem with this is that point and line
	 * sizes don't always precisely match, which doesn't look quite right.
	 * It's worth it for the AA, though. */
	glEnable( GL_LINE_SMOOTH );

	/* Our line width is wrt the regular internal SCREEN_WIDTHxSCREEN_HEIGHT screen,
	 * but these width functions actually want raster sizes (that is, actual pixels).
	 * Scale the line width and point size by the average ratio of the scale. */
	float fWidthVal = float(g_pWind->GetActualVideoModeParams().width) / SCREEN_WIDTH;
	float fHeightVal = float(g_pWind->GetActualVideoModeParams().height) / SCREEN_HEIGHT;
	fLineWidth *= (fWidthVal + fHeightVal) / 2;

	/* Clamp the width to the hardware max for both lines and points (whichever
	 * is more restrictive). */
	fLineWidth = clamp( fLineWidth, g_line_range[0], g_line_range[1] );
	fLineWidth = clamp( fLineWidth, g_point_range[0], g_point_range[1] );

	/* Hmm.  The granularity of lines and points might be different; for example,
	 * if lines are .5 and points are .25, we might want to snap the width to the
	 * nearest .5, so the hardware doesn't snap them to different sizes.  Does it
	 * matter? */
	glLineWidth( fLineWidth );

	/* Draw the line loop: */
	SetupVertices( v, iNumVerts );
	glDrawArrays( GL_LINE_STRIP, 0, iNumVerts );

	glDisable( GL_LINE_SMOOTH );

	/* Round off the corners.  This isn't perfect; the point is sometimes a little
	 * larger than the line, causing a small bump on the edge.  Not sure how to fix
	 * that. */
	glPointSize( fLineWidth );

	/* Hack: if the points will all be the same, we don't want to draw
	 * any points at all, since there's nothing to connect.  That'll happen
	 * if both scale factors in the matrix are ~0.  (Actually, I think
	 * it's true if two of the three scale factors are ~0, but we don't
	 * use this for anything 3d at the moment anyway ...)  This is needed
	 * because points aren't scaled like regular polys--a zero-size point
	 * will still be drawn. */
	RageMatrix mat;
	glGetFloatv( GL_MODELVIEW_MATRIX, (float*)mat );

	if( mat.m[0][0] < 1e-5 && mat.m[1][1] < 1e-5 )
		return;

	glEnable( GL_POINT_SMOOTH );

	SetupVertices( v, iNumVerts );
	glDrawArrays( GL_POINTS, 0, iNumVerts );

	glDisable( GL_POINT_SMOOTH );
}

void RageDisplay_OGL::ClearAllTextures()
{
	FOREACH_ENUM2( TextureUnit, i )
		SetTexture( i, NULL );

	// HACK:  Reset the active texture to 0.
	// TODO:  Change all texture functions to take a stage number.
	if( GLExt.glActiveTextureARB )
		GLExt.glActiveTextureARB(GL_TEXTURE0_ARB);
}

int RageDisplay_OGL::GetNumTextureUnits()
{
	if( GLExt.glActiveTextureARB == NULL )
		return 1;
	else
		return g_iMaxTextureUnits;
}

void RageDisplay_OGL::SetTexture( TextureUnit tu, RageTexture* pTexture )
{
	if( GLExt.glActiveTextureARB == NULL )
	{
		// multitexture isn't supported.  Ignore all textures except for 0.
		if( tu != 0 )
			return;
	}
	else
	{
		switch( tu )
		{
		case 0:
			GLExt.glActiveTextureARB(GL_TEXTURE0_ARB);
			break;
		case 1:
			GLExt.glActiveTextureARB(GL_TEXTURE1_ARB);
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
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
}

void RageDisplay_OGL::SetTextureModeGlow()
{
	if( !GLExt.m_bEXT_texture_env_combine )
	{
		/* This is changing blend state, instead of texture state, which isn't
		 * great, but it's better than doing nothing. */
		glBlendFunc( GL_SRC_ALPHA, GL_ONE );
		return;
	}

	/* Source color is the diffuse color only: */
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT );
	glTexEnvi( GL_TEXTURE_ENV, GLenum(GL_COMBINE_RGB_EXT), GL_REPLACE );
	glTexEnvi( GL_TEXTURE_ENV, GLenum(GL_SOURCE0_RGB_EXT), GL_PRIMARY_COLOR_EXT );

	/* Source alpha is texture alpha * diffuse alpha: */
	glTexEnvi( GL_TEXTURE_ENV, GLenum(GL_COMBINE_ALPHA_EXT), GL_MODULATE );
	glTexEnvi( GL_TEXTURE_ENV, GLenum(GL_OPERAND0_ALPHA_EXT), GL_SRC_ALPHA );
	glTexEnvi( GL_TEXTURE_ENV, GLenum(GL_SOURCE0_ALPHA_EXT), GL_PRIMARY_COLOR_EXT );
	glTexEnvi( GL_TEXTURE_ENV, GLenum(GL_OPERAND1_ALPHA_EXT), GL_SRC_ALPHA );
	glTexEnvi( GL_TEXTURE_ENV, GLenum(GL_SOURCE1_ALPHA_EXT), GL_TEXTURE );
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

void RageDisplay_OGL::SetZBias( float f )
{
	float fNear = SCALE( f, 0.0f, 1.0f, 0.05f, 0.0f );
	float fFar = SCALE( f, 0.0f, 1.0f, 1.0f, 0.95f );

	glDepthRange( fNear, fFar );
}

void RageDisplay_OGL::SetZTestMode( ZTestMode mode )
{
	glEnable( GL_DEPTH_TEST );
	switch( mode )
	{
	case ZTEST_OFF:				glDepthFunc( GL_ALWAYS );	break;
	case ZTEST_WRITE_ON_PASS:	glDepthFunc( GL_LEQUAL );	break;
	case ZTEST_WRITE_ON_FAIL:	glDepthFunc( GL_GREATER );	break;
	default:	ASSERT( 0 );
	}
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
	// TRICKY:  If lighting is off, then setting the material 
	// will have no effect.  Even if lighting is off, we still
	// want Models to have basic color and transparency.
	// We can do this fake lighting by setting the vertex color.
	// XXX: unintended: SetLighting must be called before SetMaterial
	GLboolean bLighting;
	glGetBooleanv( GL_LIGHTING, &bLighting );

	if( bLighting )
	{
		glMaterialfv( GL_FRONT, GL_EMISSION, emissive );
		glMaterialfv( GL_FRONT, GL_AMBIENT, ambient );
		glMaterialfv( GL_FRONT, GL_DIFFUSE, diffuse );
		glMaterialfv( GL_FRONT, GL_SPECULAR, specular );
		glMaterialf( GL_FRONT, GL_SHININESS, shininess );
	}
	else
	{
		RageColor c = diffuse;
		c.r += emissive.r + ambient.r;
		c.g += emissive.g + ambient.g;
		c.b += emissive.b + ambient.b;
		glColor4fv( c );
	}
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
	glLightfv( GL_LIGHT0+index, GL_AMBIENT, ambient );
	glLightfv( GL_LIGHT0+index, GL_DIFFUSE, diffuse );
	glLightfv( GL_LIGHT0+index, GL_SPECULAR, specular );
	float position[4] = {dir.x, dir.y, dir.z, 0};
	glLightfv( GL_LIGHT0+index, GL_POSITION, position );

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
	ASSERT( pf < NUM_PixelFormat );
	return &PIXEL_FORMAT_DESC[pf];
}

void RageDisplay_OGL::DeleteTexture( unsigned uTexHandle )
{
	unsigned int uTexID = uTexHandle;

	FlushGLErrors();
	glDeleteTextures(1,reinterpret_cast<GLuint*>(&uTexID));

	AssertNoGLError();
}


PixelFormat RageDisplay_OGL::GetImgPixelFormat( RageSurface* &img, bool &bFreeImg, int width, int height, bool bPalettedTexture )
{
	PixelFormat pixfmt = FindPixelFormat( img->format->BitsPerPixel, img->format->Rmask, img->format->Gmask, img->format->Bmask, img->format->Amask );
	
	/* If img is paletted, we're setting up a non-paletted texture, and color indexes
	 * are too small, depalettize. */
	bool bSupported = true;
	if( !bPalettedTexture && img->fmt.BytesPerPixel == 1 && !g_bColorIndexTableWorks )
		bSupported = false;

	if( pixfmt == PixelFormat_INVALID || !SupportsSurfaceFormat(pixfmt) )
		bSupported = false;

	if( !bSupported )
	{
		/* The source isn't in a supported, known pixel format.  We need to convert
		 * it ourself.  Just convert it to RGBA8, and let OpenGL convert it back
		 * down to whatever the actual pixel format is.  This is a very slow code
		 * path, which should almost never be used. */
		pixfmt = PixelFormat_RGBA8;
		ASSERT( SupportsSurfaceFormat(pixfmt) );

		const PixelFormatDesc *pfd = DISPLAY->GetPixelFormatDesc(pixfmt);

		RageSurface *imgconv = CreateSurface( width, height,
			pfd->bpp, pfd->masks[0], pfd->masks[1], pfd->masks[2], pfd->masks[3] );
		RageSurfaceUtils::Blit( img, imgconv, width, height );
		img = imgconv;
		bFreeImg = true;
	}
	else
	{
		bFreeImg = false;
	}

	return pixfmt;
}

/* If we're sending a paletted surface to a non-paletted texture, set the palette. */
void SetPixelMapForSurface( int glImageFormat, int glTexFormat, const RageSurfacePalette *palette )
{
	if( glImageFormat != GL_COLOR_INDEX || glTexFormat == GL_COLOR_INDEX8_EXT )
	{
		glPixelTransferi( GL_MAP_COLOR, false );
		return;
	}

	GLushort buf[4][256];
	memset( buf, 0, sizeof(buf) );

	for( int i = 0; i < palette->ncolors; ++i )
	{
		buf[0][i] = SCALE( palette->colors[i].r, 0, 255, 0, 65535 );
		buf[1][i] = SCALE( palette->colors[i].g, 0, 255, 0, 65535 );
		buf[2][i] = SCALE( palette->colors[i].b, 0, 255, 0, 65535 );
		buf[3][i] = SCALE( palette->colors[i].a, 0, 255, 0, 65535 );
	}

	FlushGLErrors();
	glPixelMapusv( GL_PIXEL_MAP_I_TO_R, 256, buf[0] );
	glPixelMapusv( GL_PIXEL_MAP_I_TO_G, 256, buf[1] );
	glPixelMapusv( GL_PIXEL_MAP_I_TO_B, 256, buf[2] );
	glPixelMapusv( GL_PIXEL_MAP_I_TO_A, 256, buf[3] );
	glPixelTransferi( GL_MAP_COLOR, true );
	GLenum error = glGetError();
	ASSERT_M( error == GL_NO_ERROR, GLToString(error) );
}

unsigned RageDisplay_OGL::CreateTexture( 
	PixelFormat pixfmt,
	RageSurface* pImg,
	bool bGenerateMipMaps )
{
	ASSERT( pixfmt < NUM_PixelFormat );
	ASSERT( pImg->w == power_of_two(pImg->w) && pImg->h == power_of_two(pImg->h) );


	/* Find the pixel format of the image we've been given. */
	bool bFreeImg;
	PixelFormat imgpixfmt = GetImgPixelFormat( pImg, bFreeImg, pImg->w, pImg->h, pixfmt == PixelFormat_PAL );
	ASSERT( imgpixfmt != PixelFormat_INVALID );

	GLenum glTexFormat = g_GLPixFmtInfo[pixfmt].internalfmt;
	GLenum glImageFormat = g_GLPixFmtInfo[imgpixfmt].format;
	GLenum glImageType = g_GLPixFmtInfo[imgpixfmt].type;

	/* If the image is paletted, but we're not sending it to a paletted image,
	 * set up glPixelMap. */
	SetPixelMapForSurface( glImageFormat, glTexFormat, pImg->format->palette );

	// HACK:  OpenGL 1.2 types aren't available in GLU 1.3.  Don't call GLU for mip
	// mapping if we're using an OGL 1.2 type and don't have >= GLU 1.3.
	// http://pyopengl.sourceforge.net/documentation/manual/gluBuild2DMipmaps.3G.html
	if( bGenerateMipMaps && g_gluVersion < 13 )
	{
		switch( pixfmt )
		{
		// OpenGL 1.1 types
		case PixelFormat_RGBA8:
		case PixelFormat_RGB8:
		case PixelFormat_PAL:
		case PixelFormat_BGR8:
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
	glGenTextures( 1, reinterpret_cast<GLuint*>(&uTexHandle) );
	ASSERT( uTexHandle );
	
	glBindTexture( GL_TEXTURE_2D, uTexHandle );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	GLint iMinFilter;
	if( bGenerateMipMaps )
	{
		if( g_pWind->GetActualVideoModeParams().bTrilinearFiltering )
			iMinFilter = GL_LINEAR_MIPMAP_LINEAR;
		else
			iMinFilter = GL_LINEAR_MIPMAP_NEAREST;
	}
	else
	{
		iMinFilter = GL_LINEAR;
	}
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, iMinFilter );

	if( g_pWind->GetActualVideoModeParams().bAnisotropicFiltering &&
		GLExt.HasExtension("GL_EXT_texture_filter_anisotropic") )
	{
		GLfloat fLargestSupportedAnisotropy;
		glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargestSupportedAnisotropy );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargestSupportedAnisotropy );
	}

	SetTextureWrapping( false );

	glPixelStorei( GL_UNPACK_ROW_LENGTH, pImg->pitch / pImg->format->BytesPerPixel );


	if( pixfmt == PixelFormat_PAL )
	{
		/* The texture is paletted; set the texture palette. */
		GLubyte palette[256*4];
		memset( palette, 0, sizeof(palette) );
		int p = 0;
		/* Copy the palette to the format OpenGL expects. */
		for( int i = 0; i < pImg->format->palette->ncolors; ++i )
		{
			palette[p++] = pImg->format->palette->colors[i].r;
			palette[p++] = pImg->format->palette->colors[i].g;
			palette[p++] = pImg->format->palette->colors[i].b;
			palette[p++] = pImg->format->palette->colors[i].a;
		}

		/* Set the palette. */
		GLExt.glColorTableEXT( GL_TEXTURE_2D, GL_RGBA8, 256, GL_RGBA, GL_UNSIGNED_BYTE, palette );

		GLint iRealFormat = 0;
		GLExt.glGetColorTableParameterivEXT( GL_TEXTURE_2D, GL_COLOR_TABLE_FORMAT, &iRealFormat );
		ASSERT( iRealFormat == GL_RGBA8 );
	}


	
	{
		ostringstream s;
		
		s << (bGenerateMipMaps? "gluBuild2DMipmaps":"glTexImage2D");
		s << "(format " << GLToString(glTexFormat) <<
				", " << pImg->w << "x" <<  pImg->h <<
				", format " << GLToString(glImageFormat) <<
				", type " << GLToString(glImageType) <<
				", pixfmt " << pixfmt <<
				", imgpixfmt " << imgpixfmt <<
				")";
		LOG->Trace( "%s", s.str().c_str() );
	}

	FlushGLErrors();

	if( bGenerateMipMaps )
	{
		GLenum error = gluBuild2DMipmaps(
			GL_TEXTURE_2D, glTexFormat, 
			pImg->w, pImg->h,
			glImageFormat, glImageType, pImg->pixels );
		ASSERT_M( error == 0, (char *) gluErrorString(error) );
	}
	else
	{
		glTexImage2D(
			GL_TEXTURE_2D, 0, glTexFormat, 
			pImg->w, pImg->h, 0,
			glImageFormat, glImageType, pImg->pixels);
		
		GLenum error = glGetError();
		ASSERT_M( error == GL_NO_ERROR, GLToString(error) );
	}


	/* Sanity check: */
	if( pixfmt == PixelFormat_PAL )
	{
		GLint iSize = 0;
		glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GLenum(GL_TEXTURE_INDEX_SIZE_EXT), &iSize );
		if( iSize != 8 )
			RageException::Throw( "Thought paletted textures worked, but they don't." );
	}

	glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
	glFlush();

	if( bFreeImg )
		delete pImg;
	return uTexHandle;
}

/* This doesn't support img being paletted if the surface itself isn't paletted.
 * This is only used for movies anyway, which are never paletted. */
void RageDisplay_OGL::UpdateTexture( 
	unsigned uTexHandle, 
	RageSurface* pImg,
	int iXOffset, int iYOffset, int iWidth, int iHeight )
{
	glBindTexture( GL_TEXTURE_2D, uTexHandle );

	bool bFreeImg;
	PixelFormat pixfmt = GetImgPixelFormat( pImg, bFreeImg, iWidth, iHeight, false );

	glPixelStorei( GL_UNPACK_ROW_LENGTH, pImg->pitch / pImg->format->BytesPerPixel );

//	GLenum glTexFormat = g_GLPixFmtInfo[pixfmt].internalfmt;
	GLenum glImageFormat = g_GLPixFmtInfo[pixfmt].format;
	GLenum glImageType = g_GLPixFmtInfo[pixfmt].type;

	glTexSubImage2D( GL_TEXTURE_2D, 0,
		iXOffset, iYOffset,
		iWidth, iHeight,
		glImageFormat, glImageType, pImg->pixels );

	/* Must unset PixelStore when we're done! */
	glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
	glFlush();

	if( bFreeImg )
		delete pImg;
}

void RageDisplay_OGL::SetPolygonMode( PolygonMode pm )
{
	GLenum m;
	switch( pm )
	{
	case POLYGON_FILL:	m = GL_FILL; break;
	case POLYGON_LINE:	m = GL_LINE; break;
	default:	ASSERT(0);	return;
	}
	glPolygonMode( GL_FRONT_AND_BACK, m );
}

void RageDisplay_OGL::SetLineWidth( float fWidth )
{
	glLineWidth( fWidth );
}

CString RageDisplay_OGL::GetTextureDiagnostics( unsigned id ) const
{
	return CString();
}

void RageDisplay_OGL::SetAlphaTest( bool b )
{
	glAlphaFunc( GL_GREATER, 0.01f );
	if( b )
		glEnable( GL_ALPHA_TEST );
	else
		glDisable( GL_ALPHA_TEST );
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
	switch( g_GLPixFmtInfo[pixfmt].type )
	{
	case GL_UNSIGNED_SHORT_1_5_5_5_REV:
		return GLExt.m_bGL_EXT_bgra && g_bReversePackedPixelsWorks;
	default:
		return true;
	}
}


bool RageDisplay_OGL::SupportsTextureFormat( PixelFormat pixfmt, bool bRealtime )
{
	/* If we support a pixfmt for texture formats but not for surface formats, then
	 * we'll have to convert the texture to a supported surface format before uploading.
	 * This is too slow for dynamic textures. */
	if( bRealtime && !SupportsSurfaceFormat(pixfmt) )
		return false;

	switch( g_GLPixFmtInfo[pixfmt].format )
	{
	case GL_COLOR_INDEX:
		return GLExt.glColorTableEXT && GLExt.glGetColorTableParameterivEXT;
	case GL_BGR:
	case GL_BGRA:
		return GLExt.m_bGL_EXT_bgra;
	default:
		return true;
	}
}

bool RageDisplay_OGL::SupportsPerVertexMatrixScale()
{
	return g_bTextureMatrixShader != 0;
}

void RageDisplay_OGL::SetSphereEnvironmentMapping( bool b )
{
	if( b )
	{
		glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
		glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
		glEnable( GL_TEXTURE_GEN_S );
		glEnable( GL_TEXTURE_GEN_T );
	}
	else
	{
		glDisable( GL_TEXTURE_GEN_S );
		glDisable( GL_TEXTURE_GEN_T );
	}
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

