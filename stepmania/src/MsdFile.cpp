#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: MsdFile

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

/* The original MSD format is simply:
 * 
 * #PARAM0:PARAM1:PARAM2:PARAM3;
 * #NEXTPARAM0:PARAM1:PARAM2:PARAM3;
 * 
 * (The first field is typically an identifier, but doesn't have to be.)
 *
 * The semicolon is not optional, though if we hit a # on a new line, eg:
 * #VALUE:PARAM1
 * #VALUE2:PARAM2
 * we'll recover.
 *
 * TODO: Normal text fields need some way of escaping.  We need to be able to escape
 * colons and "//".  Also, we should escape #s, so if we really want to put a # at the
 * beginning of a line, we can. 
 */
#include "MsdFile.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageFile.h"

void MsdFile::AddParam( char *buf, int len )
{
	values.back().params.push_back(CString(buf, len));
}

void MsdFile::AddValue() /* (no extra charge) */
{
	values.push_back(value_t());
	values.back().params.reserve( 32 );
}

void MsdFile::ReadBuf( char *buf, int len )
{
	values.reserve( 64 );

	int value_start = -1;

	bool ReadingValue=false;
	int i = 0;
	while(i < len)
	{
		if( i+1 < len && buf[i] == '/' && buf[i+1] == '/' )
		{
			/* //; erase with spaces until newline */
			do {
				buf[i] = ' ';
				i++;
			} while(i < len && buf[i] != '\n');

			continue;
		}

		if(ReadingValue && buf[i] == '#') {
			/* Unfortunately, many of these files are missing ;'s.
			 * If we get a # when we thought we were inside a value, assume we
			 * missed the ;.  Back up and end the value. */
			/* Make sure this # is the first non-whitespace character on the line. */
			bool FirstChar = true;
			int j;
			for(j = i-1; j >= 0 && !strchr("\r\n", buf[j]); --j)
			{
				if(buf[j] == ' ' || buf[j] == '\t')
					continue;

				FirstChar = false;
				break;
			}

			if(!FirstChar) {
				/* Oops, we're not; handle this like a regular character. */
				i++;
				continue;
			}

			/* Skip newlines and whitespace before adding the value. */
			while( j >= 1 && strchr("\r\n", buf[j-1]) )
				--j;

			AddParam(buf+value_start, j - value_start);
			ReadingValue=false;
		}

		/* # starts a new value. */
		if(!ReadingValue && buf[i] == '#') {
			AddValue();
			ReadingValue=true;
		}

		if(!ReadingValue)
		{
			i++;
			continue; /* nothing else is meaningful outside of a value */
		}

		/* : and ; end the current param, if any. */
		if(value_start != -1 && (buf[i] == ':' || buf[i] == ';'))
			AddParam(buf+value_start, i - value_start);

		/* # and : begin new params. */
		if(buf[i] == '#' || buf[i] == ':')
		{
			i++; /* skip */
			value_start = i;
			continue;
		}

		/* ; ends the current value. */
		if(buf[i] == ';')
			ReadingValue=false;

		i++;
	}
	
	/* Add any unterminated value at the very end. */
	if(ReadingValue)
		AddParam(buf+value_start, i - value_start);
}

// returns true if successful, false otherwise
bool MsdFile::ReadFile( CString sNewPath )
{
	error = "";

	RageFile f;
	/* Open a file. */
	if( !f.Open( sNewPath ) )
	{
		error = f.GetError();
		return false;
	}

	const int iBufferSize = f.GetFileSize();

	// allocate a string to hold the file
	char* szFileString = new char[iBufferSize];

	int iBytesRead = f.Read( szFileString, iBufferSize );

	ASSERT( iBufferSize >= iBytesRead );

	ReadBuf(szFileString, iBytesRead);

	delete [] szFileString;

	return true;
}

CString MsdFile::GetParam(unsigned val, unsigned par) const
{
	if(val >= GetNumValues()) return "";
	if(par >= GetNumParams(val)) return "";

	return values[val].params[par];
}
