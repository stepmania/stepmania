#include "global.h"

#include "RageDisplay.h"
#include "RageDisplay_GLES2.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageTimer.h"
#include "RageMath.h"
#include "RageTypes.h"
#include "RageUtil.h"
#include "RageSurface.h"
#include "RageTextureManager.h"
#include "DisplaySpec.h"

#include "arch/LowLevelWindow/LowLevelWindow.h"

#include <GL/glew.h>

#include <cstddef>
#include <cstdint>
#include <vector>

#ifdef NO_GL_FLUSH
#define glFlush()
#endif

namespace
{
	RageDisplay::RagePixelFormatDesc
	PIXEL_FORMAT_DESC[NUM_RagePixelFormat] = {
		{
			/* R8G8B8A8 */
			32,
			{ 0xFF000000,
			  0x00FF0000,
			  0x0000FF00,
			  0x000000FF }
		}, {
			/* B8G8R8A8 */
			32,
			{ 0x0000FF00,
			  0x00FF0000,
			  0xFF000000,
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
			/* R5G5B5X1 */
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
			/* B8G8R8 */
			24,
			{ 0x0000FF,
			  0x00FF00,
			  0xFF0000,
			  0x000000 }
		}, {
			/* A1R5G5B5 */
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
		}
	};

	/* g_GLPixFmtInfo is used for both texture formats and surface formats.
	 * For example, it's fine to ask for a RagePixelFormat_RGB5 texture, but to
	 * supply a surface matching RagePixelFormat_RGB8.  OpenGL will simply
	 * discard the extra bits.
	 *
	 * It's possible for a format to be supported as a texture format but
	 * not as a surface format.  For example, if packed pixels aren't
	 * supported, we can still use GL_RGB5_A1, but we'll have to convert to
	 * a supported surface pixel format first.  It's not ideal, since we'll
	 * convert to RGBA8 and OGL will convert back, but it works fine.
	 */
	struct GLPixFmtInfo_t {
		GLenum internalfmt; /* target format */
		GLenum format; /* target format */
		GLenum type; /* data format */
	} const g_GLPixFmtInfo[NUM_RagePixelFormat] = {
		{
			/* R8G8B8A8 */
			GL_RGBA8,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
		}, {
			/* R8G8B8A8 */
			GL_RGBA8,
			GL_BGRA,
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
			// TODO: These don't work on ES2. Work out what needs to happen.
			/* A1R5G5B5 (matches D3DFMT_A1R5G5B5) */
			GL_RGB5_A1,
			GL_BGRA,
			GL_UNSIGNED_SHORT_1_5_5_5_REV,
		}, {
			/* X1R5G5B5 */
			GL_RGB5,
			GL_BGRA,
			GL_UNSIGNED_SHORT_1_5_5_5_REV,
		}
	};

	LowLevelWindow *g_pWind;

	void FixLittleEndian()
	{
		if constexpr (!Endian::little) {
			return;
		}

		static bool bInitialized = false;
		if (bInitialized)
			return;
		bInitialized = true;

		for( int i = 0; i < NUM_RagePixelFormat; ++i )
		{
			RageDisplay::RagePixelFormatDesc &pf = PIXEL_FORMAT_DESC[i];

			/* OpenGL and RageSurface handle byte formats differently; we need
			 * to flip non-paletted masks to make them line up. */
			if (g_GLPixFmtInfo[i].type != GL_UNSIGNED_BYTE || pf.bpp == 8)
				continue;

			for( int mask = 0; mask < 4; ++mask)
			{
				int m = pf.masks[mask];
				switch( pf.bpp )
				{
				case 24: m = Swap24(m); break;
				case 32: m = Swap32(m); break;
				default:
					 FAIL_M(ssprintf("Unsupported BPP value: %i", pf.bpp));
				}
				pf.masks[mask] = m;
			}
		}
	}
	namespace Caps
	{
		int iMaxTextureUnits = 1;
		int iMaxTextureSize = 256;
	}
	namespace State
	{
		bool bZTestEnabled = false;
		bool bZWriteEnabled = false;
		bool bAlphaTestEnabled = false;
	}
}

RageDisplay_GLES2::RageDisplay_GLES2()
{
	LOG->Trace( "RageDisplay_GLES2::RageDisplay_GLES2()" );
	LOG->MapLog("renderer", "Current renderer: OpenGL ES 2.0");

	FixLittleEndian();
//	RageDisplay_GLES2_Helpers::Init();

	g_pWind = nullptr;
}

RString
RageDisplay_GLES2::Init( const VideoModeParams &p, bool bAllowUnacceleratedRenderer )
{
	g_pWind = LowLevelWindow::Create();

	bool bIgnore = false;
	RString sError = SetVideoMode( p, bIgnore );
	if (sError != "")
		return sError;

	// Get GPU capabilities up front so we don't have to query later.
	glGetIntegerv( GL_MAX_TEXTURE_SIZE, &Caps::iMaxTextureSize );
	glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS, &Caps::iMaxTextureUnits );

	// Log driver details
	g_pWind->LogDebugInformation();
	LOG->Info( "OGL Vendor: %s", glGetString(GL_VENDOR) );
	LOG->Info( "OGL Renderer: %s", glGetString(GL_RENDERER) );
	LOG->Info( "OGL Version: %s", glGetString(GL_VERSION) );
	LOG->Info( "OGL Max texture size: %i", Caps::iMaxTextureSize );
	LOG->Info( "OGL Texture units: %i", Caps::iMaxTextureUnits );

	/* Pretty-print the extension string: */
	LOG->Info( "OGL Extensions:" );
	{
		// glGetString(GL_EXTENSIONS) doesn't work for GL3 core profiles.
		// this will be useful in the future.
#if 0
		std::vector<string> extensions;
		const char *ext = 0;
		for (int i = 0; (ext = (const char*)glGetStringi(GL_EXTENSIONS, i)); i++)
		{
			extensions.push_back(string(ext));
		}

		sort( extensions.begin(), extensions.end() );
		std::size_t next = 0;
		while( next < extensions.size() )
		{
			std::size_t last = next;
			string type;
			for( std::size_t i = next; i<extensions.size(); ++i )
			{
				std::vector<string> segments;
				split(extensions[i], '_', segments);
				string this_type;
				if (segments.size() > 2)
					this_type = join("_", segments.begin(), segments.begin()+2);
				if (i > next && this_type != type)
					break;
				type = this_type;
				last = i;
			}

			if (next == last)
			{
				printf( "  %s\n", extensions[next].c_str() );
				++next;
				continue;
			}

			string sList = ssprintf( "  %s: ", type.c_str() );
			while( next <= last )
			{
				std::vector<string> segments;
				split( extensions[next], '_', segments );
				string ext_short = join( "_", segments.begin()+2, segments.end() );
				sList += ext_short;
				if (next < last)
					sList += ", ";
				if (next == last || sList.size() + extensions[next+1].size() > 78)
				{
					printf( "%s\n", sList.c_str() );
					sList = "    ";
				}
				++next;
			}
		}
#else
		const char *szExtensionString = (const char *) glGetString(GL_EXTENSIONS);
		std::vector<RString> asExtensions;
		split( szExtensionString, " ", asExtensions );
		sort( asExtensions.begin(), asExtensions.end() );
		std::size_t iNextToPrint = 0;
		while( iNextToPrint < asExtensions.size() )
		{
			std::size_t iLastToPrint = iNextToPrint;
			RString sType;
			for( std::size_t i = iNextToPrint; i<asExtensions.size(); ++i )
			{
				std::vector<RString> asBits;
				split( asExtensions[i], "_", asBits );
				RString sThisType;
				if (asBits.size() > 2)
					sThisType = join( "_", asBits.begin(), asBits.begin()+2 );
				if (i > iNextToPrint && sThisType != sType)
					break;
				sType = sThisType;
				iLastToPrint = i;
			}

			if (iNextToPrint == iLastToPrint)
			{
				LOG->Info( "  %s", asExtensions[iNextToPrint].c_str() );
				++iNextToPrint;
				continue;
			}

			RString sList = ssprintf( "  %s: ", sType.c_str() );
			while( iNextToPrint <= iLastToPrint )
			{
				std::vector<RString> asBits;
				split( asExtensions[iNextToPrint], "_", asBits );
				RString sShortExt = join( "_", asBits.begin()+2, asBits.end() );
				sList += sShortExt;
				if (iNextToPrint < iLastToPrint)
					sList += ", ";
				if (iNextToPrint == iLastToPrint || sList.size() + asExtensions[iNextToPrint+1].size() > 120)
				{
					LOG->Info( "%s", sList.c_str() );
					sList = "    ";
				}
				++iNextToPrint;
			}
#endif
		}
	}

	glewExperimental = true;
	glewInit();

	/* Log this, so if people complain that the radar looks bad on their
	 * system we can compare them: */
	//glGetFloatv( GL_LINE_WIDTH_RANGE, g_line_range );
	//glGetFloatv( GL_POINT_SIZE_RANGE, g_point_range );

	return RString();
}

// Return true if mode change was successful.
// bNewDeviceOut is set true if a new device was created and textures
// need to be reloaded.
RString RageDisplay_GLES2::TryVideoMode( const VideoModeParams &p, bool &bNewDeviceOut )
{
	VideoModeParams vm = p;
	vm.windowed = 1; // force windowed until I trust this thing.
	LOG->Warn( "RageDisplay_GLES2::TryVideoMode( %d, %d, %d, %d, %d, %d )",
		vm.windowed, vm.width, vm.height, vm.bpp, vm.rate, vm.vsync );

	RString err = g_pWind->TryVideoMode( vm, bNewDeviceOut );
	if (err != "")
		return err;	// failed to set video mode

	if (bNewDeviceOut)
	{
		// NOTE: This isn't needed in an actual GLES2 context...
		glewInit();

		/* We have a new OpenGL context, so we have to tell our textures that
		 * their OpenGL texture number is invalid. */
		if (TEXTUREMAN)
			TEXTUREMAN->InvalidateTextures();

		/* Delete all render targets.  They may have associated resources other than
		 * the texture itself. */
		//FOREACHM( unsigned, RenderTarget *, g_mapRenderTargets, rt )
		//	delete rt->second;
		//g_mapRenderTargets.clear();

		/* Recreate all vertex buffers. */
		//InvalidateObjects();

		//InitShaders();
	}

	ResolutionChanged();

	return RString();
}

int RageDisplay_GLES2::GetMaxTextureSize() const
{
	return Caps::iMaxTextureSize;
}

bool RageDisplay_GLES2::BeginFrame()
{
	/* We do this in here, rather than ResolutionChanged, or we won't update the
	 * viewport for the concurrent rendering context. */
	int fWidth = g_pWind->GetActualVideoModeParams().width;
	int fHeight = g_pWind->GetActualVideoModeParams().height;

	glViewport( 0, 0, fWidth, fHeight );

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	SetZWrite( true );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	return RageDisplay::BeginFrame();
}

void RageDisplay_GLES2::EndFrame()
{
	glFlush();

	// XXX: This is broken on NVidia, as their xrandr sucks.
	FrameLimitBeforeVsync( g_pWind->GetActualVideoModeParams().rate );
	g_pWind->SwapBuffers();
	FrameLimitAfterVsync();

	g_pWind->Update();

	RageDisplay::EndFrame();
}

RageDisplay_GLES2::~RageDisplay_GLES2()
{
	delete g_pWind;
}

void
RageDisplay_GLES2::GetDisplaySpecs(DisplaySpecs &out) const
{
	out.clear();
	g_pWind->GetDisplaySpecs(out);
}

RageSurface*
RageDisplay_GLES2::CreateScreenshot()
{
	const RagePixelFormatDesc &desc = PIXEL_FORMAT_DESC[RagePixelFormat_RGB8];
	RageSurface *image = CreateSurface(
		640, 480, desc.bpp,
		desc.masks[0], desc.masks[1], desc.masks[2], desc.masks[3] );

	memset( image->pixels, 0, 480*image->pitch );

	return image;
}

const RageDisplay::RagePixelFormatDesc*
RageDisplay_GLES2::GetPixelFormatDesc(RagePixelFormat pf) const
{
	ASSERT( pf >= 0 && pf < NUM_RagePixelFormat );
	return &PIXEL_FORMAT_DESC[pf];
}

RageMatrix
RageDisplay_GLES2::GetOrthoMatrix( float l, float r, float b, float t, float zn, float zf )
{
	RageMatrix m(
		2/(r-l),      0,            0,           0,
		0,            2/(t-b),      0,           0,
		0,            0,            -2/(zf-zn),   0,
		-(r+l)/(r-l), -(t+b)/(t-b), -(zf+zn)/(zf-zn),  1 );
	return m;
}

class RageCompiledGeometryGLES2 : public RageCompiledGeometry
{
public:

	void Allocate( const std::vector<msMesh> &vMeshes )
	{
		// TODO
	}
	void Change( const std::vector<msMesh> &vMeshes )
	{
		// TODO
	}
	void Draw( int iMeshIndex ) const
	{
		// TOO
	}
};

RageCompiledGeometry*
RageDisplay_GLES2::CreateCompiledGeometry()
{
	return new RageCompiledGeometryGLES2;
}

void
RageDisplay_GLES2::DeleteCompiledGeometry( RageCompiledGeometry *p )
{
	delete p;
}

RString
RageDisplay_GLES2::GetApiDescription() const
{
	return "OpenGL ES 2.0";
}

ActualVideoModeParams
RageDisplay_GLES2::GetActualVideoModeParams() const
{
	return g_pWind->GetActualVideoModeParams();
}

void
RageDisplay_GLES2::SetBlendMode( BlendMode mode )
{
	// TODO
}

bool
RageDisplay_GLES2::SupportsTextureFormat( RagePixelFormat pixfmt, bool realtime )
{
	/* If we support a pixfmt for texture formats but not for surface formats, then
	 * we'll have to convert the texture to a supported surface format before uploading.
	 * This is too slow for dynamic textures. */
	if (realtime && !SupportsSurfaceFormat(pixfmt))
		return false;

	switch (g_GLPixFmtInfo[pixfmt].format)
	{
	case GL_COLOR_INDEX:
		return false;
	case GL_BGR:
	case GL_BGRA:
		//return !!GLEW_EXT_bgra;
		return false; // no BGRA on ES2 (without exts)
	default:
		return true;
	}

	return true;
}

bool
RageDisplay_GLES2::SupportsPerVertexMatrixScale()
{
	return true;
}

std::uintptr_t
RageDisplay_GLES2::CreateTexture(
	RagePixelFormat pixfmt,
	RageSurface* img,
	bool bGenerateMipMaps
	)
{
	// TODO
	return 1;
}

void
RageDisplay_GLES2::UpdateTexture(
	std::uintptr_t iTexHandle,
	RageSurface* img,
	int xoffset, int yoffset, int width, int height
	)
{
	// TODO
}

void
RageDisplay_GLES2::DeleteTexture( std::uintptr_t iTexHandle )
{
	// TODO
}

void
RageDisplay_GLES2::ClearAllTextures()
{
	FOREACH_ENUM( TextureUnit, i )
		SetTexture( i, 0 );

	// HACK:  Reset the active texture to 0.
	// TODO:  Change all texture functions to take a stage number.
	glActiveTexture(GL_TEXTURE0);
}

int
RageDisplay_GLES2::GetNumTextureUnits()
{
	return Caps::iMaxTextureUnits;
}

static bool
SetTextureUnit( TextureUnit tu )
{
	if ((int) tu > Caps::iMaxTextureUnits)
		return false;
	glActiveTexture( enum_add2(GL_TEXTURE0, tu) );
	return true;
}

void
RageDisplay_GLES2::SetTexture( TextureUnit tu, std::uintptr_t iTexture )
{
	if (!SetTextureUnit( tu ))
		return;

	if (iTexture)
	{
		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, static_cast<GLuint>(iTexture) );
	}
	else
	{
		glDisable( GL_TEXTURE_2D );
	}
}

void
RageDisplay_GLES2::SetTextureMode( TextureUnit tu, TextureMode tm )
{
	// TODO
}

void
RageDisplay_GLES2::SetTextureWrapping( TextureUnit tu, bool b )
{
	// TODO
}

void
RageDisplay_GLES2::SetTextureFiltering( TextureUnit tu, bool b )
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, b ? GL_LINEAR : GL_NEAREST);

	GLint iMinFilter = 0;
	if (b)
	{
		GLint iWidth1 = -1;
		GLint iWidth2 = -1;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &iWidth1);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 1, GL_TEXTURE_WIDTH, &iWidth2);
		if (iWidth1 > 1 && iWidth2 != 0)
		{
			/* Mipmaps are enabled. */
			if (g_pWind->GetActualVideoModeParams().bTrilinearFiltering)
				iMinFilter = GL_LINEAR_MIPMAP_LINEAR;
			else
				iMinFilter = GL_LINEAR_MIPMAP_NEAREST;
		}
		else
		{
			iMinFilter = GL_LINEAR;
		}
	}
	else
	{
		iMinFilter = GL_NEAREST;
	}

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, iMinFilter );
}

bool
RageDisplay_GLES2::IsZWriteEnabled() const
{
	return State::bZWriteEnabled;
}

bool
RageDisplay_GLES2::IsZTestEnabled() const
{
	return State::bZTestEnabled;
}

void
RageDisplay_GLES2::SetZWrite( bool b )
{
	if (State::bZWriteEnabled != b)
	{
		State::bZWriteEnabled = b;
		glDepthMask( b );
	}
}

void
RageDisplay_GLES2::SetZBias( float f )
{
	float fNear = SCALE( f, 0.0f, 1.0f, 0.05f, 0.0f );
	float fFar = SCALE( f, 0.0f, 1.0f, 1.0f, 0.95f );

	glDepthRange( fNear, fFar );
}

void
RageDisplay_GLES2::SetZTestMode( ZTestMode mode )
{
	glEnable( GL_DEPTH_TEST );
	switch( mode )
	{
	case ZTEST_OFF:
		glDisable( GL_DEPTH_TEST );
		glDepthFunc( GL_ALWAYS );
		State::bZTestEnabled = false;
		break;
	case ZTEST_WRITE_ON_PASS: glDepthFunc( GL_LEQUAL ); break;
	case ZTEST_WRITE_ON_FAIL: glDepthFunc( GL_GREATER ); break;
	default:
		FAIL_M(ssprintf("Invalid ZTestMode: %i", mode));
	}
	State::bZTestEnabled = true;
}





/*


void RageDisplay_Legacy::SetBlendMode( BlendMode mode )
{
	glEnable(GL_BLEND);

	if (glBlendEquation != nullptr)
	{
		if (mode == BLEND_INVERT_DEST)
			glBlendEquation( GL_FUNC_SUBTRACT );
		else if (mode == BLEND_SUBTRACT)
			glBlendEquation( GL_FUNC_REVERSE_SUBTRACT );
		else
			glBlendEquation( GL_FUNC_ADD );
	}

	int iSourceRGB, iDestRGB;
	int iSourceAlpha = GL_ONE, iDestAlpha = GL_ONE_MINUS_SRC_ALPHA;
	switch( mode )
	{
	case BLEND_NORMAL:
		iSourceRGB = GL_SRC_ALPHA; iDestRGB = GL_ONE_MINUS_SRC_ALPHA;
		break;
	case BLEND_ADD:
		iSourceRGB = GL_SRC_ALPHA; iDestRGB = GL_ONE;
		break;
	case BLEND_SUBTRACT:
		iSourceRGB = GL_SRC_ALPHA; iDestRGB = GL_ONE_MINUS_SRC_ALPHA;
		break;
	case BLEND_MODULATE:
		iSourceRGB = GL_ZERO; iDestRGB = GL_SRC_COLOR;
		break;
	case BLEND_COPY_SRC:
		iSourceRGB = GL_ONE; iDestRGB = GL_ZERO;
		iSourceAlpha = GL_ONE; iDestAlpha = GL_ZERO;
		break;
	case BLEND_ALPHA_MASK:
		iSourceRGB = GL_ZERO; iDestRGB = GL_ONE;
		iSourceAlpha = GL_ZERO; iDestAlpha = GL_SRC_ALPHA;
		break;
	case BLEND_ALPHA_KNOCK_OUT:
		iSourceRGB = GL_ZERO; iDestRGB = GL_ONE;
		iSourceAlpha = GL_ZERO; iDestAlpha = GL_ONE_MINUS_SRC_ALPHA;
		break;
	case BLEND_ALPHA_MULTIPLY:
		iSourceRGB = GL_SRC_ALPHA; iDestRGB = GL_ZERO;
		break;
	case BLEND_WEIGHTED_MULTIPLY:
		// output = 2*(dst*src).  0.5,0.5,0.5 is identity; darker colors darken the image,
		// and brighter colors lighten the image.
		iSourceRGB = GL_DST_COLOR; iDestRGB = GL_SRC_COLOR;
		break;
	case BLEND_INVERT_DEST:
		// out = src - dst.  The source color should almost always be #FFFFFF, to make it "1 - dst".
		iSourceRGB = GL_ONE; iDestRGB = GL_ONE;
		break;
	case BLEND_NO_EFFECT:
		iSourceRGB = GL_ZERO; iDestRGB = GL_ONE;
		iSourceAlpha = GL_ZERO; iDestAlpha = GL_ONE;
		break;
	DEFAULT_FAIL( mode );
	}

	if (GLEW_EXT_blend_equation_separate)
		glBlendFuncSeparateEXT( iSourceRGB, iDestRGB, iSourceAlpha, iDestAlpha );
	else
		glBlendFunc( iSourceRGB, iDestRGB );
}

bool RageDisplay_Legacy::IsZWriteEnabled() const
{
	bool a;
	glGetBooleanv( GL_DEPTH_WRITEMASK, (unsigned char*)&a );
	return a;
}


*/

void
RageDisplay_GLES2::ClearZBuffer()
{
	bool write = IsZWriteEnabled();
	SetZWrite( true );
	glClear( GL_DEPTH_BUFFER_BIT );
	SetZWrite( write );
}

void
RageDisplay_GLES2::SetCullMode( CullMode mode )
{
	if (mode != CULL_NONE)
		glEnable(GL_CULL_FACE);
	switch( mode )
	{
	case CULL_BACK:
		glCullFace( GL_BACK );
		break;
	case CULL_FRONT:
		glCullFace( GL_FRONT );
		break;
	case CULL_NONE:
		glDisable( GL_CULL_FACE );
		break;
	default:
		FAIL_M(ssprintf("Invalid CullMode: %i", mode));
	}
}

void
RageDisplay_GLES2::SetAlphaTest( bool b )
{
	if (State::bAlphaTestEnabled != b)
	{
		State::bAlphaTestEnabled = b;
		b ? glEnable(GL_ALPHA_TEST) : glDisable(GL_ALPHA_TEST);
	}
}

void
RageDisplay_GLES2::SetMaterial(
	const RageColor &emissive,
	const RageColor &ambient,
	const RageColor &diffuse,
	const RageColor &specular,
	float shininess
	)
{
	// TODO
}

void
RageDisplay_GLES2::SetLineWidth(float fWidth)
{
	glLineWidth(fWidth);
}

void
RageDisplay_GLES2::SetPolygonMode(PolygonMode pm)
{
	GLenum m;
	switch (pm)
	{
	case POLYGON_FILL:	m = GL_FILL; break;
	case POLYGON_LINE:	m = GL_LINE; break;
	default:
		FAIL_M(ssprintf("Invalid PolygonMode: %i", pm));
	}
	glPolygonMode(GL_FRONT_AND_BACK, m);
}

void
RageDisplay_GLES2::SetLighting( bool b )
{
	// TODO
}

void
RageDisplay_GLES2::SetLightOff( int index )
{
	// TODO
}

void
RageDisplay_GLES2::SetLightDirectional(
	int index,
	const RageColor &ambient,
	const RageColor &diffuse,
	const RageColor &specular,
	const RageVector3 &dir )
{
	// TODO
}

void
RageDisplay_GLES2::SetSphereEnvironmentMapping( TextureUnit tu, bool b )
{
	// TODO
}

void
RageDisplay_GLES2::SetCelShaded( int stage )
{
	// TODO
}

void
RageDisplay_GLES2::DrawQuadsInternal( const RageSpriteVertex v[], int iNumVerts )
{
	// TODO
}

void
RageDisplay_GLES2::DrawQuadStripInternal( const RageSpriteVertex v[], int iNumVerts )
{
	// TODO
}

void
RageDisplay_GLES2::DrawFanInternal( const RageSpriteVertex v[], int iNumVerts )
{
	// TODO
}

void
RageDisplay_GLES2::DrawStripInternal( const RageSpriteVertex v[], int iNumVerts )
{
	// TODO
}

void
RageDisplay_GLES2::DrawTrianglesInternal( const RageSpriteVertex v[], int iNumVerts )
{
	// TODO
}

void
RageDisplay_GLES2::DrawCompiledGeometryInternal( const RageCompiledGeometry *p, int
	iMeshIndex )
{
	// TODO
}

void
RageDisplay_GLES2::DrawLineStripInternal( const RageSpriteVertex v[], int iNumVerts, float LineWidth )
{
	// TODO
}

// Is this even used?
void
RageDisplay_GLES2::DrawSymmetricQuadStripInternal( const RageSpriteVertex v[], int iNumVerts )
{
	// TODO
}

bool
RageDisplay_GLES2::SupportsSurfaceFormat( RagePixelFormat pixfmt )
{
	switch (g_GLPixFmtInfo[pixfmt].type)
	{
	case GL_UNSIGNED_SHORT_1_5_5_5_REV:
		return false;
		//return GLEW_EXT_bgra && g_bReversePackedPixelsWorks;
	default:
		return true;
	}
}

/*
 * Copyright (c) 2012 Colby Klein
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
