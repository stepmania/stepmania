#include "global.h"
#include "MovieTexture.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "MovieTexture_Null.h"
#include "PrefsManager.h"
#include "RageFile.h"

void ForceToAscii( RString &str )
{
	for( unsigned i=0; i<str.size(); ++i )
		if( str[i] < 0x20 || str[i] > 0x7E )
			str[i] = '?';
}

bool RageMovieTexture::GetFourCC( RString fn, RString &handler, RString &type )
{
	RString ignore, ext;
	splitpath( fn, ignore, ignore, ext);
	if( !ext.CompareNoCase(".mpg") ||
		!ext.CompareNoCase(".mpeg") ||
		!ext.CompareNoCase(".mpv") ||
		!ext.CompareNoCase(".mpe") )
	{
		handler = type = "MPEG";
		return true;
	}

	//Not very pretty but should do all the same error checking without iostream
#define HANDLE_ERROR(x) { \
		LOG->Warn( "Error reading %s: %s", fn.c_str(), x ); \
		handler = type = ""; \
		return false; \
	}

	RageFile file;
	if( !file.Open(fn) )
		HANDLE_ERROR("Could not open file.");
	if( !file.Seek(0x70) )
		HANDLE_ERROR("Could not seek.");
	type = "    ";
	if( file.Read((char *)type.c_str(), 4) != 4 )
		HANDLE_ERROR("Could not read.");
	ForceToAscii( type );
	
	if( file.Seek(0xBC) != 0xBC )
		HANDLE_ERROR("Could not seek.");
	handler = "    ";
	if( file.Read((char *)handler.c_str(), 4) != 4 )
		HANDLE_ERROR("Could not read.");
	ForceToAscii( handler );

	return true;
#undef HANDLE_ERROR
}

/*
 * (c) 2003-2004 Glenn Maynard
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
