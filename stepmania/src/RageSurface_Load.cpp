#include "global.h"
#include "RageSurface_Load.h"
#include "RageSurface_Load_PNG.h"
#include "RageSurface_Load_JPEG.h"
#include "RageSurface_Load_GIF.h"
#include "RageUtil.h"

SDL_Surface *RageSurface::LoadFile( const CString &sPath )
{
	const CString ext = GetExtension( sPath );

	CString error;
	SDL_Surface *ret = NULL;
	if( !ext.CompareNoCase("png") )
		ret = RageSurface_Load_PNG( sPath, error );
	else if( !ext.CompareNoCase("gif") )
		ret = RageSurface_Load_GIF( sPath, error );
	else if( !ext.CompareNoCase("jpg") )
		ret = RageSurface_Load_JPEG( sPath, error );
	else if( !ext.CompareNoCase("bmp") )
	{
		SDL_RWops *rw = OpenRWops( sPath, false );
		if( rw == NULL )
		{
			/* XXX */
			SDL_SetError( "fail" );
			return NULL;
		}

		ret = SDL_LoadBMP_RW( rw, false );
		SDL_RWclose( rw );
		SDL_FreeRW( rw );

		mySDL_FixupPalettedAlpha( ret );
	}
	else
	{
		error = ssprintf( "Unsupported file type \"%s\"", ext.c_str() );
	}

	if( ret == NULL )
		SDL_SetError( "%s", error.c_str() );

	return ret;
}

/*
 * (c) 2004 Glenn Maynard
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
