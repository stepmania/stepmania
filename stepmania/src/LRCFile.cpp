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
#include <sstream>
#include "RageUtil.h"
#include "Regex.h"

void LRCFile::AddParam( const CString &buf )
{
	values.back().params.push_back(buf);
}

void LRCFile::AddValue() /* (no extra charge) */
{
	values.push_back(value_t());
}

void LRCFile::ReadBuf( char *buf, int len )
{
	stringstream input(CString(buf, len));
	string line;

	while(getline(input, line))
	{
		if(!line.compare(0, 2, "//"))
			continue;

		/* "[data1] data2".  Ignore whitespace at the beginning of the line. */
		static Regex x("^ *\\[([^]]+)\\] *(.*)$");
		
		vector<CString> matches;
		if(!x.Compare(line, matches))
			continue;

		StripCrnl(matches[1]);

		AddValue();
		AddParam(matches[0]);
		AddParam(matches[1]);
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
