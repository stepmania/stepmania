#ifndef RAGE_DISPLAY_INTERNAL_H
#define RAGE_DISPLAY_INTERNAL_H

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

struct oglspecs_t {
	/* OpenGL system information that generally doesn't change at runtime. */
	
	/* Range and granularity of points and lines: */
	float line_range[2];
	float line_granularity;
	float point_range[2];
	float point_granularity;

	/* OpenGL version * 10: */
	int glVersion;

	/* Available extensions: */
	set<string> glExts;

	/* Which extensions we actually use are supported: */
	bool EXT_texture_env_combine,
		 WGL_EXT_swap_control,
		 EXT_paletted_texture;
};

/* Extension functions we use.  Put these in a namespace instead of in oglspecs_t,
 * so they can be called like regular functions. */
namespace GLExt {
	extern PWSWAPINTERVALEXTPROC wglSwapIntervalEXT;
	extern PFNGLCOLORTABLEPROC glColorTableEXT;
	extern PFNGLCOLORTABLEPARAMETERIVPROC glGetColorTableParameterivEXT;
};

#endif

/*
-----------------------------------------------------------------------------
 Class: RageDisplay

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
    Glenn Maynard
	Chris Danford
-----------------------------------------------------------------------------
*/
