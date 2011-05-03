#include "global.h"
#include "RageDisplay_OGL_Helpers.h"
#include "RageUtil.h"

#include "RageLog.h"
#include "RageUtil.h"
#include "arch/LowLevelWindow/LowLevelWindow.h"

#include <map>
#include <set>

namespace
{
	map<GLenum, RString> g_Strings;
	void InitStringMap()
	{
		static bool bInitialized = false;
		if( bInitialized )
			return;
		bInitialized = true;

#define X(a) g_Strings[a] = #a;
		X(GL_RGBA8);	X(GL_RGBA4);	X(GL_RGB5_A1);	X(GL_RGB5);	X(GL_RGBA);	X(GL_RGB);
		X(GL_BGR);	X(GL_BGRA);
		X(GL_COLOR_INDEX8_EXT);	X(GL_COLOR_INDEX4_EXT);	X(GL_COLOR_INDEX);
		X(GL_UNSIGNED_BYTE);	X(GL_UNSIGNED_SHORT_4_4_4_4); X(GL_UNSIGNED_SHORT_5_5_5_1);
		X(GL_UNSIGNED_SHORT_1_5_5_5_REV);
		X(GL_INVALID_ENUM); X(GL_INVALID_VALUE); X(GL_INVALID_OPERATION);
		X(GL_STACK_OVERFLOW); X(GL_STACK_UNDERFLOW); X(GL_OUT_OF_MEMORY);
#undef X
	}
};

void RageDisplay_Legacy_Helpers::Init()
{
	InitStringMap();
}

RString RageDisplay_Legacy_Helpers::GLToString( GLenum e )
{
	if( g_Strings.find(e) != g_Strings.end() )
		return g_Strings[e];

	return ssprintf( "%i", int(e) );
}
/*
static void GetGLExtensions( set<string> &ext )
{
	const char *szBuf = (const char *) glGetString( GL_EXTENSIONS );

	vector<RString> asList;
	split( szBuf, " ", asList );

	for( unsigned i = 0; i < asList.size(); ++i )
		ext.insert( asList[i] );
}
*/

/*
 * Copyright (c) 2001-2011 Chris Danford, Glenn Maynard, Colby Klein
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
