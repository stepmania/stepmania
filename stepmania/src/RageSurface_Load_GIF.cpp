#include "global.h"
#include "RageSurface_Load_GIF.h"
#include "RageFile.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageSurface.h"

#define	MAXCOLORMAPSIZE		256

#define	MAX_LWZ_BITS		12

#define INTERLACE		0x40
#define LOCALCOLORMAP	0x80
#define BitSet(byte, bit)	(((byte) & (bit)) == (bit))

#define	ReadOK(file,buffer,len)	(file.Read( buffer, len, 1) != 0)

#define LM_to_uint(a,b)			(((b)<<8)|(a))


static RageSurface *ReadImage( RageFile &f, int len, int height,
			const RageSurfaceColor localColorMap[MAXCOLORMAPSIZE],
			int interlace, int ignore );

static bool ReadPalette( RageFile &f, int number, RageSurfaceColor buffer[MAXCOLORMAPSIZE] )
{
	for( int i = 0; i < number; ++i )
	{
		if( !ReadOK(f, &buffer[i].r, sizeof(buffer[i].r)) )
			return false;
		if( !ReadOK(f, &buffer[i].g, sizeof(buffer[i].g)) )
			return false;
		if( !ReadOK(f, &buffer[i].b, sizeof(buffer[i].b)) )
			return false;
		buffer[i].a = 0xFF;
	}

    return true;
}


static int GetDataBlock( RageFile &f, unsigned char *buf )
{
	unsigned char count;

	if( !ReadOK(f, &count, 1) )
	{
		/* pm_message("error in getting DataBlock size" ); */
		return -1;
	}

	if( count != 0 && !ReadOK(f, buf, count) )
	{
		/* pm_message("error in reading DataBlock" ); */
		return -1;
	}
	return count;
}


RageSurfaceUtils::OpenResult RageSurface_Load_GIF( const CString &sPath, RageSurface *&ret, bool bHeaderOnly, CString &error )
{
	unsigned char buf[256];
	int imageCount = 0;
	int imageNumber = 1;
	RageFile f;

	if( !f.Open( sPath ) )
	{
		error = f.GetError();
		return RageSurfaceUtils::OPEN_FATAL_ERROR;
	}
	
	if( !ReadOK(f, buf, 6) )
	{
		error = "error reading magic number";
		return RageSurfaceUtils::OPEN_FATAL_ERROR;
	}
	if( strncmp((char *) buf, "GIF", 3) != 0 )
	{
		error = "not a GIF file";
		return RageSurfaceUtils::OPEN_UNKNOWN_FILE_FORMAT;
	}

	{
		char version[4];
		strncpy(version, (char *) buf + 3, 3);
		version[3] = '\0';

		if( (strcmp(version, "87a") != 0) && (strcmp(version, "89a") != 0) )
		{
			error = "bad version number, not '87a' or '89a'";
			return RageSurfaceUtils::OPEN_FATAL_ERROR;
		}
	}

	if( !ReadOK(f, buf, 7) )
	{
		error = "failed to read screen descriptor";
		return RageSurfaceUtils::OPEN_FATAL_ERROR;
	}

	RageSurfaceColor GlobalColorMap[MAXCOLORMAPSIZE];
	unsigned int GlobalBitPixel = 0;

	GlobalBitPixel = 2 << (buf[4] & 0x07);

    if( BitSet(buf[4], LOCALCOLORMAP) )
	{
		/* Global Colormap */
		if( !ReadPalette(f, GlobalBitPixel, GlobalColorMap ) )
		{
			error = "error reading global colormap";
			return RageSurfaceUtils::OPEN_FATAL_ERROR;
		}
    }

    int transparency = -1;

    while(1)
	{
		unsigned char type;
		if( !ReadOK(f, &type, 1) )
		{
			error = "EOF / read error on image data";
			return RageSurfaceUtils::OPEN_FATAL_ERROR;
		}
		switch( type )
		{
		case ';':
		{
			/* GIF terminator */
			if( imageCount < imageNumber )
			{
				error = ssprintf( "only %d image%s found in file",
					imageCount, imageCount > 1 ? "s" : "");
				return RageSurfaceUtils::OPEN_FATAL_ERROR;
			}
		}

		case '!':
		{
			/* Extension */
			unsigned char label;
			if( !ReadOK(f, &label, 1) )
			{
				error = "EOF / read error on extention function code";
				return RageSurfaceUtils::OPEN_FATAL_ERROR;
			}

			switch( label )
			{
			case 0xf9:
				GetDataBlock( f, (unsigned char *) buf );
				if( (buf[0] & 0x1) != 0 )
					transparency  = buf[3];
			}

			while( GetDataBlock(f, (unsigned char *) buf) != 0 )
				;

			continue;
		}
		case ',':
		{
			++imageCount;

			if( !ReadOK(f, buf, 9) )
			{
				error = "couldn't read left/top/width/height";
				return RageSurfaceUtils::OPEN_FATAL_ERROR;
			}

			int bitPixel = 1 << ((buf[8] & 0x07) + 1);
			RageSurfaceColor LocalColorMap[MAXCOLORMAPSIZE];

			if( BitSet(buf[8], LOCALCOLORMAP) )
			{
				if( !ReadPalette(f, bitPixel, LocalColorMap) )
				{
					error = "error reading local colormap";
					return RageSurfaceUtils::OPEN_FATAL_ERROR;
				}
			} else {
				bitPixel = GlobalBitPixel;
				memcpy( LocalColorMap, GlobalColorMap, sizeof(LocalColorMap) );
			}

			ret = ReadImage( f, LM_to_uint(buf[4], buf[5]), LM_to_uint(buf[6], buf[7]),
					LocalColorMap, BitSet(buf[8], INTERLACE),
					imageCount != imageNumber );

			if( !ret )
				continue;

			if( transparency != -1 )
				ret->format->palette->colors[ transparency ].a = 0;

			return RageSurfaceUtils::OPEN_OK;
		}
		default: continue; /* Not a valid start character */
		}
	}

    return RageSurfaceUtils::OPEN_FATAL_ERROR;
}

struct LWZState
{
	bool fresh;
    int code_size, set_code_size;
    int max_code, max_code_size;
    int firstcode, oldcode;
    int clear_code, end_code;
    int table[2][(1 << MAX_LWZ_BITS)];
    int stack[(1 << (MAX_LWZ_BITS)) * 2], *sp;

	struct Code
	{
		/* code state */
		unsigned char buf[280];
		int curbit, lastbit, last_byte;
		bool done;
		void Init();
		int Get( RageFile &f, int code_size );
	};
	Code m_Code;

	LWZState() { fresh = false; }
	bool Init( RageFile &f );
	int ReadByte( RageFile &f );
};

void LWZState::Code::Init()
{
	curbit = lastbit = 0;
	last_byte = 2;
	done = false;
	memset( buf, 0, sizeof(buf) );
}

int LWZState::Code::Get( RageFile &f, int code_size )
{
	if( (curbit + code_size) >= lastbit )
	{
		if (done)
		{
//			if( curbit >= lastbit )
//				RWSetMsg("ran off the end of my bits");
			return -1;
		}
		buf[0] = buf[last_byte - 2];
		buf[1] = buf[last_byte - 1];

		unsigned char count = (unsigned char) GetDataBlock( f, &buf[2] );
		if( count == 0 )
			done = true;

		last_byte = 2 + count;
		curbit = (curbit - lastbit) + 16;
		lastbit = (2 + count) * 8;
	}
	int ret = 0;
	int i, j;
	for (i = curbit, j = 0; j < code_size; ++i, ++j)
		ret |= ((buf[i / 8] & (1 << (i % 8))) != 0) << j;

	curbit += code_size;

	return ret;
}


bool LWZState::Init( RageFile &f )
{
    unsigned char input_code_size;

	/* code size: */
    if( !ReadOK(f, &input_code_size, 1) )
	{
//		RWSetMsg("EOF / read error on image data");
		return false;
    }

	set_code_size = input_code_size;
	code_size = set_code_size + 1;
	clear_code = 1 << set_code_size;
	end_code = clear_code + 1;
	max_code_size = 2 * clear_code;
	max_code = clear_code + 2;

	m_Code.Init();

	fresh = true;

	memset( table, 0, sizeof(table) );

	for( int i = 0; i < clear_code; ++i )
		table[1][i] = i;

	sp = stack;

	return true;
}

int LWZState::ReadByte( RageFile &f )
{
	if( fresh )
	{
		fresh = false;
		do {
			firstcode = oldcode = m_Code.Get( f, code_size );
		} while ( firstcode == clear_code );
		return firstcode;
	}

	if( sp > stack )
		return *--sp;

    int code;
	while( (code = m_Code.Get(f, code_size)) >= 0 )
	{
		if( code == clear_code )
		{
			memset( table, 0, sizeof(table) );
			for( int i = 0; i < clear_code; ++i )
				table[1][i] = i;
			code_size = set_code_size + 1;
			max_code_size = 2 * clear_code;
			max_code = clear_code + 2;
			sp = stack;
			firstcode = oldcode = m_Code.Get( f, code_size );
			return firstcode;
		}
		else if( code == end_code )
		{
			int count;
			unsigned char buf[260];

			if( m_Code.done )
				return -2;

			while( (count = GetDataBlock(f, buf)) > 0 )
				;

			if( count != 0 )
			{
				/*
				 * pm_message("missing EOD in data stream (common occurence)");
				 */
			}
			return -2;
		}
		int incode = code;

		if( code >= max_code )
		{
			*sp++ = firstcode;
			code = oldcode;
		}
		while( code >= clear_code )
		{
			*sp++ = table[1][code];
//			if (code == table[0][code])
//				RWSetMsg("circular table entry BIG ERROR");
			code = table[0][code];
		}

		*sp++ = firstcode = table[1][code];

		if( (code = max_code) < (1 << MAX_LWZ_BITS) )
		{
			table[0][code] = oldcode;
			table[1][code] = firstcode;
			++max_code;
			if (max_code >= max_code_size &&
				max_code_size < (1 << MAX_LWZ_BITS))
			{
				max_code_size *= 2;
				++code_size;
			}
		}
		oldcode = incode;

		if( sp > stack )
			return *--sp;
    }
    return code;
}

static RageSurface *ReadImage( RageFile &f, int len, int height,
		const RageSurfaceColor localColorMap[MAXCOLORMAPSIZE],
		int interlace, int ignore )
{
    int xpos = 0, ypos = 0, pass = 0;

    /* Initialize the compression routines */
	LWZState state;
    if( !state.Init(f) )
	{
//		RWSetMsg("error reading image");
		return NULL;
    }
	/* If this is an "uninteresting picture" ignore it. */
	if( ignore )
	{
		while( state.ReadByte(f) >= 0 )
			;
		return NULL;
	}

    RageSurface *image = CreateSurface( len, height, 8, 0, 0, 0, 0 );
	memcpy( image->fmt.palette->colors, localColorMap, 256*sizeof(RageSurfaceColor) );

	int v;
	while( (v = state.ReadByte(f)) >= 0 )
	{
		char *data = (char *) image->pixels;
		data[xpos + ypos * image->pitch] = (char) v;

		++xpos;
		if( xpos == len )
		{
			xpos = 0;
			if( interlace )
			{
				int step[] = { 8, 8, 4, 2 };
				ypos += step[pass];

				if( ypos >= height )
				{
					++pass;
					if( pass == 4 )
					    return image;
					int start[] = { 0, 4, 2, 1 };
					ypos = start[pass];
				}
			} else {
				++ypos;
			}
		}
		if (ypos >= height)
			break;
	}

    return image;
}

/*
 * Copyright 1990, 1991, 1993 David Koblas.
 * Copyright 1996 Torsten Martinsen.
 *   Permission to use, copy, modify, and distribute this software
 *   and its documentation for any purpose and without fee is hereby
 *   granted, provided that the above copyright notice appear in all
 *   copies and that both that copyright notice and this permission
 *   notice appear in supporting documentation.  This software is
 *   provided "as is" without express or implied warranty.
 */

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
