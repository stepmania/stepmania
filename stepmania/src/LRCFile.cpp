#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: LRCFile

 Desc: See header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
	Kevin Slaughter
-----------------------------------------------------------------------------
*/

#include "LRCFile.h"
#include "RageLog.h"
#include <fcntl.h>
#if defined(WIN32)
#include <io.h>
#endif
#include "RageUtil.h"

void LRCFile::AddParam( char *buf, int len )
{
	values.back().params.push_back(CString(buf, len));
}

void LRCFile::AddValue() /* (no extra charge) */
{
	values.push_back(value_t());
}

void LRCFile::ReadBuf( char *buf, int len )
{
	int value_start = -1;

	bool ReadingValue=false;
	int i = 0;
	while(i < len)
	{
		if(!strncmp(buf+i, "//", 2))
		{
			/* //; erase with spaces until newline */
			do {
				buf[i] = ' ';
				i++;
			} while(i < len && buf[i] != '\n');

			continue;
		}

		if(ReadingValue && buf[i] == '[') {
			/* If we get a [ when we thought we were inside a value, start a new
			    one. Back up and end the value. */
			/* Make sure this [ is the first non-whitespace character on the line. */
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

			AddParam(buf+value_start, j - value_start);
			ReadingValue=false;
		}

		/* # starts a new value. */
		if(!ReadingValue && buf[i] == '[') {
			AddValue();
			ReadingValue=true;
		}

		if(!ReadingValue)
		{
			i++;
			continue; /* nothing else is meaningful outside of a value */
		}

		/* ']' ends the current value bracket */
		if(buf[i] == ']')
		{
			AddParam( buf+value_start, i - value_start );
			i++;
			// START TO READ IN LYRICS HERE
			value_start = i;
			// The next value cannot be a '['.. This WILL cause problems if it is.
			// ** NOTE: I will add error-checking here soon. -- Miryokuteki
			continue;
		}


		/* [ begins a new param. */
		if(buf[i] == '[' )
		{
			i++; /* Start AFTER the bracket */
			value_start = i;
			continue;
		}


		/* ; ends the current value/lyric. */
		if(buf[i] == '\n' || buf[i] == '\r\n')
		{
			// Lyric line ends at a line break. Add the param
			AddParam(buf+value_start, i - value_start);
			ReadingValue=false;
		}

		i++; 
	}
	
	/* Add any unterminated value at the very end. */
	if(ReadingValue)
	{
		AddParam(buf+value_start, i - value_start);
	}
}

// returns true if successful, false otherwise
bool LRCFile::ReadFile( CString sNewPath )
{
   int fd;

   /* Open a file */
   if( (fd = open(sNewPath, O_RDONLY, 0)) == -1 )
	   return false;

	int iBufferSize = GetFileSizeInBytes(sNewPath) + 1000; // +1000 because sometimes the bytes read is > filelength.  Why?

	// allocate a string to hold the file
	char* szFileString = new char[iBufferSize];

	int iBytesRead = read( fd, szFileString, iBufferSize );
	close( fd );

	ASSERT( iBufferSize > iBytesRead );
	szFileString[iBytesRead] = '\0';

	ReadBuf(szFileString, iBytesRead);

	delete [] szFileString;

	return true;
}

CString LRCFile::GetParam(unsigned val, unsigned par) const
{
	if(val >= GetNumValues()) return "";
	if(par >= GetNumParams(val)) return "";

	return values[val].params[par];
}
