/* This is a partial XPM reader; we only use it for reading compiled-in icons
 * and loading splashes. */
#include "global.h"
#include "RageSurface_Load_XPM.h"
#include "RageFile.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageSurface.h"

#define CheckLine() \
	if( xpm[line] == NULL ) { \
		error = "short file"; \
		return NULL; \
	}

RageSurface *RageSurface_Load_XPM( char * const *xpm, CString &error )
{
	int line = 0;

	int width, height, num_colors, color_length;

	CheckLine();
	if( sscanf( xpm[line++], "%i %i %i %i", &width, &height, &num_colors, &color_length ) != 4 )
	{
		error = "parse error reading specs";
		return NULL;
	}

	if( width > 2048 || height > 2048 || num_colors > 1024*16 || color_length > 4 )
	{
		error = "spec error";
		return NULL;
	}

	vector<RageSurfaceColor> colors;

	map<CString,int> name_to_color;
	for( int i = 0; i < num_colors; ++i )
	{
		CheckLine();

		/* "id c #AABBCC"; id is color_length long.  id may contain spaces. */
		CString color = xpm[line++];

		if( color_length+4 > (int) color.size() )
			continue;

		CString name;
		name = color.substr( 0, color_length );

		if( color.substr( color_length, 4 ) != " c #")
			continue;

		CString clr = color.substr( color_length+4 );
		int r, g, b;
		if( sscanf( clr, "%2x%2x%2x", &r, &g, &b ) != 3 )
			continue;
		RageSurfaceColor colorval;
		colorval.r = (uint8_t) r;
		colorval.g = (uint8_t) g;
		colorval.b = (uint8_t) b;
		colorval.a = 0xFF;

		colors.push_back( colorval );

		name_to_color[name] = colors.size()-1;
	}

	RageSurface *img;
	if( colors.size() <= 256 )
	{
		img = CreateSurface( width, height, 8, 0, 0, 0, 0 );
		memcpy( img->fmt.palette->colors, &colors[0], colors.size()*sizeof(RageSurfaceColor) );
	} else {
		img = CreateSurface( width, height, 32,
			0xFF000000, 0x00FF0000, 0x0000FF00, 0 );
	}

	for( int y = 0; y < height; ++y )
	{
		CheckLine();
		const CString row = xpm[line++];
		if( (int) row.size() != width*color_length )
		{
			error = ssprintf( "row %i is not expected length (%i != %i)", y, int(row.size()), width*color_length );
			delete img;
		    return NULL;
		}

		int8_t *p = (int8_t *) img->pixels;
		p += y * img->pitch;
		int32_t *p32 = (int32_t *) p;
		for( int x = 0; x < width; ++x )
		{
			CString color_name = row.substr( x*color_length, color_length );
			map<CString,int>::const_iterator it;
			it = name_to_color.find( color_name );
			if( it == name_to_color.end() )
			{
				error = ssprintf( "%ix%i is unknown color \"%s\"", x, y, color_name.c_str() );
				delete img;
				return NULL;
			}

			if( colors.size() <= 256 )
			{
				p[x] = (int8_t) it->second;
			} else {
				const RageSurfaceColor &color = colors[it->second];
				p32[x] = (color.r << 24) + (color.g << 16) + (color.b << 8);
			}
		}
	}

    return img;
}

/*
 * Copyright (c) 2004 Glenn Maynard
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
