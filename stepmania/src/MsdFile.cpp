#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: MsdFile

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
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
 * Extension: If : is followed by ASCII 1 (^A), we read a binary value, in this
 * form:
 *
 * #VALUE:^A4,DATA:more tags;
 * "4" is the number of bytes of data to expect, not including itself, and not
 * including the comma separating it from the data.  The data is completely unparsed
 * and may contain colons, #, or whitespace without fear of corruption.  The ^A
 * character is used to avoid it showing up in real data.  We only use this internally
 * for caching; this is ugly, so don't use it in distributed data!
 *
 * TODO: Normal text fields need some way of escaping.  We need to be able to escape
 * colons and "//".  Also, we should escape #s, so if we really want to put a # at the
 * beginning of a line, we can. 
 */
#include "MsdFile.h"
#include "RageLog.h"
#include "io.h"
#include "fcntl.h"


MsdFile::MsdFile()
{
	m_iNumValues = 0;
}

MsdFile::~MsdFile()
{
}

void MsdFile::AddParam( char *buf, int len )
{
	int valueno = m_iNumValues-1;
	int paramno = m_iNumParams[valueno];
	ASSERT( paramno < MAX_PARAMS_PER_VALUE );
	m_iNumParams[valueno]++;
	m_sParams[valueno][paramno].assign(buf, len);
}

void MsdFile::AddValue() /* (no extra charge) */
{
	m_iNumValues++;
	ASSERT( m_iNumValues < MAX_VALUES );
}

void MsdFile::ReadBuf( char *buf, int len )
{
	int value_start = -1;
	memset(m_iNumParams, 0, sizeof(m_iNumParams));
	m_iNumValues = 0;

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
			} while(buf[i] != '\n');

			continue;
		}

		if(!ReadingValue) {
			// fast forward until the next '#' (optimization)
			while(i < len && buf[i] != '#') i++;
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
			/* If there's a \001 (^A) after the : (possibly separated by whitespace),
			 * it's binary. */
			bool Found001 = false;
			int pos = value_start;
			do {
				if(buf[pos] == '\001') { Found001 = true; break; }
				if(!strchr("\r\n\t ", buf[pos])) break;
				pos++;
			} while(pos < len);


			if(Found001)
			{
				value_start = pos;
				value_start++;
				
				/* Binary param.  Expect digits followed by a comma. */
				if(!isdigit(buf[value_start]))
				{
					LOG->Trace("Expected digit at position %i", value_start);
					ReadingValue = false; /* so we skip to the next value */
					continue;
				}
				int bytes = atoi(buf+value_start);
				if(bytes < 0) bytes = 0; /* sanity */
				while(isdigit(buf[value_start])) 
					value_start++;
				if(buf[value_start] != ',')
				{
					LOG->Trace("Expected comma at position %i", value_start);
					ReadingValue = false; /* so we skip to the next value */
					continue;
				}
				value_start++;

				/* Make sure we have enough data left. */
				if(len - value_start < bytes)
				{
					LOG->Trace("%i bytes not available at position %i", value_start);
					ReadingValue = false; /* so we skip to the next value */
					continue;
				}

				AddParam(buf+value_start, bytes);
				i = value_start + bytes;
				value_start = -1;
			}
			continue;
		}

		/* ; ends the current value. */
		if(buf[i] == ';')
			ReadingValue=false;

		i++;
	}
}

// returns true if successful, false otherwise
bool MsdFile::ReadFile( CString sNewPath )
{
   int fd;

   /* Open a file */
   if( (fd = open(sNewPath, _O_RDONLY, 0)) == -1 )
	   return false;

	int iBufferSize = _filelength( fd ) + 1000; // +1000 because sometimes the bytes read is > filelength.  Why?

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

