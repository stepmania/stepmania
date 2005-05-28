/* RageDisplay_OGL_Extensions - OpenGL extension helpers. */

#ifndef RAGE_DISPLAY_OGL_EXTENSIONS_H
#define RAGE_DISPLAY_OGL_EXTENSIONS_H

/* Not in glext.h: */
typedef bool (APIENTRY * PWSWAPINTERVALEXTPROC) (int interval);

class LowLevelWindow;
struct GLExt_t
{
	bool m_bEXT_texture_env_combine;
	bool m_bGL_EXT_bgra;

	PWSWAPINTERVALEXTPROC wglSwapIntervalEXT;
	PFNGLCOLORTABLEPROC glColorTableEXT;
	PFNGLCOLORTABLEPARAMETERIVPROC glGetColorTableParameterivEXT;
	PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
	PFNGLGENBUFFERSARBPROC glGenBuffersARB;
	PFNGLBINDBUFFERARBPROC glBindBufferARB;
	PFNGLBUFFERDATAARBPROC glBufferDataARB;
	PFNGLBUFFERSUBDATAARBPROC glBufferSubDataARB;
	PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;
	PFNGLDRAWRANGEELEMENTSPROC glDrawRangeElements;

	// GL_ARB_shader_objects:
	bool m_bGL_ARB_shader_objects;
	PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB;
	PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB;
	PFNGLSHADERSOURCEARBPROC glShaderSourceARB;
	PFNGLCOMPILESHADERARBPROC glCompileShaderARB;
	PFNGLGETOBJECTPARAMETERFVARBPROC glGetObjectParameterfvARB;
	PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
	PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
	PFNGLATTACHOBJECTARBPROC glAttachObjectARB;
	PFNGLDELETEOBJECTARBPROC glDeleteObjectARB;
	PFNGLLINKPROGRAMARBPROC glLinkProgramARB;
	PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB;
	PFNGLVERTEXATTRIB2FARBPROC glVertexAttrib2fARB;
	PFNGLVERTEXATTRIB3FARBPROC glVertexAttrib3fARB;
	PFNGLVERTEXATTRIB4FARBPROC glVertexAttrib4fARB;
	PFNGLENABLEVERTEXATTRIBARRAYARBPROC glEnableVertexAttribArrayARB;
	PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArrayARB;
	PFNGLVERTEXATTRIBPOINTERARBPROC glVertexAttribPointerARB;

	// GL_ARB_vertex_shader:
	bool m_bGL_ARB_vertex_shader;
	PFNGLBINDATTRIBLOCATIONARBPROC glBindAttribLocationARB;

	bool m_bGL_ARB_shading_language_100;
	int m_iShadingLanguageVersion; /* * 100 */

	void Load( LowLevelWindow *pWind );

	bool HasExtension( const CString &sExt ) const;
};

extern GLExt_t GLExt;

#endif

/*
 * Copyright (c) 2003-2005 Glenn Maynard
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
