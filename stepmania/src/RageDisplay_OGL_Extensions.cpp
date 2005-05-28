#include "global.h"

#define __glext_h_

#if defined(WIN32)
#include <windows.h>
#endif

#include <set>

#if !defined(DARWIN)
# include <GL/gl.h>
# include <GL/glu.h>
#else
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
#endif

#undef __glext_h_
#include "glext.h"

#include "RageDisplay_OGL_Extensions.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "arch/LowLevelWindow/LowLevelWindow.h"

GLExt_t GLExt;

/* Available extensions: */
static set<string> g_glExts;

bool GLExt_t::HasExtension( const CString &sExt ) const
{
	return g_glExts.find(sExt) != g_glExts.end();
}

#define F(n)  { (void **) &GLExt.n , #n }

struct func_t
{
	void **p;
	const char *name;
};

static bool LoadAllOrNothing( struct func_t *funcs, LowLevelWindow *pWind )
{
	bool bGotAll = true;
	for( unsigned i = 0; funcs[i].p != NULL; ++i )
	{
		*funcs[i].p = pWind->GetProcAddress( funcs[i].name );
		if( *funcs[i].p == NULL )
		{
			bGotAll = false;
			break;
		}
	}

	if( bGotAll )
		return true;

	/* If any function in the array wasn't found, clear them all. */
	for( unsigned i = 0; funcs[i].p != NULL; ++i )
		*funcs[i].p = NULL;

	return false;
}

static void GetGLExtensions( set<string> &ext )
{
    const char *szBuf = (const char *) glGetString( GL_EXTENSIONS );

	vector<CString> asList;
	split( szBuf, " ", asList );

	for( unsigned i = 0; i < asList.size(); ++i )
		ext.insert( asList[i] );
}

void GLExt_t::Load( LowLevelWindow *pWind )
{
	memset( this, 0, sizeof(*this) );

	GetGLExtensions( g_glExts );

	m_bEXT_texture_env_combine = HasExtension("GL_EXT_texture_env_combine");
	m_bGL_EXT_bgra = HasExtension("GL_EXT_bgra");

#if defined(WIN32)
	if( HasExtension("WGL_EXT_swap_control") )
		wglSwapIntervalEXT = (PWSWAPINTERVALEXTPROC) pWind->GetProcAddress("wglSwapIntervalEXT");
#elif defined(DARWIN)
	wglSwapIntervalEXT = wglSwapIntervalEXT;
#endif

	if( HasExtension("GL_EXT_paletted_texture") )
	{
		glColorTableEXT = (PFNGLCOLORTABLEPROC) pWind->GetProcAddress("glColorTableEXT");
		glGetColorTableParameterivEXT = (PFNGLCOLORTABLEPARAMETERIVPROC) pWind->GetProcAddress("glGetColorTableParameterivEXT");
	}

	if( HasExtension("GL_ARB_multitexture") )
		glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC) pWind->GetProcAddress("glActiveTextureARB");

	/*
	 * Find extension functions.
	 *
	 * X11R6.7.0 (or possibly ATI's drivers) seem to be returning bogus values for glBindBufferARB
	 * if we don't actually check for GL_ARB_vertex_buffer_object. 
	 * https://sf.net/tracker/download.php?group_id=37892&atid=421366&file_id=88086&aid=958820
	 * https://sf.net/tracker/download.php?group_id=37892&atid=421366&file_id=85542&aid=944836
	 *
	 * Let's check them all, to be safe.
	 */
	if( HasExtension("GL_ARB_vertex_buffer_object") )
	{
		func_t funcs[] = {
			F( glGenBuffersARB ),
			F( glBindBufferARB ),
			F( glBufferDataARB ),
			F( glBufferSubDataARB ),
			F( glDeleteBuffersARB ),
			{ NULL, NULL },
		};

		LoadAllOrNothing( funcs, pWind );
	}

	if( HasExtension("GL_EXT_draw_range_elements") )
		GLExt.glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC) pWind->GetProcAddress("glDrawRangeElements");

	m_bGL_ARB_shader_objects = HasExtension("GL_ARB_shader_objects");
	if( m_bGL_ARB_shader_objects )
	{
		func_t funcs[] = {
			F( glCreateShaderObjectARB ),
			F( glCreateShaderObjectARB ),
			F( glCreateProgramObjectARB ),
			F( glShaderSourceARB ),
			F( glCompileShaderARB ),
			F( glGetObjectParameterfvARB ),
			F( glGetObjectParameterivARB ),
			F( glGetInfoLogARB ),
			F( glAttachObjectARB ),
			F( glDeleteObjectARB ),
			F( glLinkProgramARB ),
			F( glUseProgramObjectARB ),
			F( glVertexAttrib2fARB ),
			F( glVertexAttrib3fARB ),
			F( glVertexAttrib4fARB ),
			F( glEnableVertexAttribArrayARB ),
			F( glDisableVertexAttribArrayARB ),
			F( glVertexAttribPointerARB ),
			{ NULL, NULL }
		};

		if( !LoadAllOrNothing(funcs, pWind) )
			m_bGL_ARB_shader_objects = false;
	}

	m_bGL_ARB_vertex_shader = m_bGL_ARB_shader_objects && HasExtension("GL_ARB_vertex_shader");
	if( m_bGL_ARB_vertex_shader )
	{
		func_t funcs[] =
		{
			F( glBindAttribLocationARB ),
			{ NULL, NULL }
		};
		if( !LoadAllOrNothing(funcs, pWind) )
			m_bGL_ARB_vertex_shader = false;
	}

	m_bGL_ARB_shading_language_100 = HasExtension("GL_ARB_shading_language_100");
	if( m_bGL_ARB_shading_language_100 )
	{
		while( glGetError() != GL_NO_ERROR )
			;
		const char *pzVersion = (const char *) glGetString( GL_SHADING_LANGUAGE_VERSION );

		GLenum glError = glGetError();
		if( glError == GL_INVALID_ENUM )
		{
			LOG->Info( "No GL_SHADING_LANGUAGE_VERSION; assuming 1.0" );
			m_iShadingLanguageVersion = 100;
		}
		else
		{
			const float fVersion = strtof( pzVersion, NULL );
			m_iShadingLanguageVersion = int(roundf(fVersion * 100));
			/* The version string may contain extra information beyond the version number. */
			LOG->Info( "OpenGL shading language: %s", pzVersion );
		}
	}
}

/*
 * Copyright (c) 2002-2005 Glenn Maynard
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
