#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: MsdFile

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "MsdFile.h"


MsdFile::MsdFile()
{
	m_iNumValues = 0;
}

MsdFile::~MsdFile()
{
}

//returns true if successful, false otherwise
bool MsdFile::ReadFile( CString sNewPath )
{
	// Get file size in bytes
	HANDLE hFile = CreateFile(
	  sNewPath,          // pointer to name of the file
	  GENERIC_READ,       // access (read-write) mode
	  FILE_SHARE_READ|FILE_SHARE_WRITE,	// share mode
	  NULL,				   // pointer to security attributes
	  OPEN_EXISTING,  // how to create
	  FILE_ATTRIBUTE_NORMAL,  // file attributes
	  NULL         // handle to file with attributes to 
	);

	int iBufferSize = GetFileSize( hFile, NULL ) + 1000; // +1000 just in case
	CloseHandle( hFile );

	FILE* fp = fopen(sNewPath, "r");
	if( fp == NULL )
		return false;

	// allocate a string to hold the file
	char* szFileString = new char[iBufferSize];
	char* szParams[MAX_VALUES][MAX_PARAMS_PER_VALUE];

	int iBytesRead = fread( szFileString, 1, iBufferSize, fp );

	ASSERT( iBufferSize > iBytesRead );
	/* why are we using Windows API calls for simple file access, anyway? XXX -glenn */
	/* What is the C way to get the file size?  I don't see 'fstat' in C library... -Chris */

	m_iNumValues = 0;

	bool ReadingValue=false;
	int i;
	for( i=0; i<iBytesRead; i++ )
	{
		switch( szFileString[i] )
		{
		case '#':	// begins a new value
			{
				/* Unfortunately, many of these files are missing ;'s.
				 * For now, if we get a # when we thought we were inside
				 * a value/parameter, assume we missed the ;.  Back up,
				 * delete whitespace and end the value. */
				if(ReadingValue) {
					/* Make sure this # is the first non-whitespace character on the line. */
					bool FirstChar = true;
					int j;
					for(j = i-1; j >= 0 && !strchr("\r\n", szFileString[j]); --j)
					{
						if(szFileString[j] == ' ' || szFileString[j] == '\t')
							continue;

						FirstChar = false;
						break;
					}
					if(!FirstChar) {
						/* Oops, we're not; handle this like a regular character. */
						break;
					}

					for(j = i-1; j >= 0 && isspace(szFileString[j]); --j)
						szFileString[j] = 0;
				}

				ReadingValue=true;
				m_iNumValues++;
				int iCurValueIndex = m_iNumValues-1;
				m_iNumParams[iCurValueIndex] = 1;
				szParams[iCurValueIndex][0] = &szFileString[i+1];
			}
			break;
		case ':':	// begins a new parameter
			{
				szFileString[i] = '\0';
				int iCurValueIndex = m_iNumValues-1;
				m_iNumParams[iCurValueIndex] ++;
				int iCurParamIndex = m_iNumParams[iCurValueIndex]-1;
				szParams[iCurValueIndex][iCurParamIndex] = &szFileString[i+1];
			}
			break;
		case ';':	// ends a value
			{
				szFileString[i] = '\0';
				// fast forward until just before the next '#'
				while( szFileString[i+1] != '#'  &&  i<iBytesRead )
					i++;
				ReadingValue=false;
			}
			break;
		case '/':
			if( szFileString[i+1] == '/' )
			{
				// advance until new line
				for( ; i<iBytesRead; i++ )
				{
					if( szFileString[i] == '\n' )
						break;
					else
						szFileString[i] = ' ';
				}
			}
			// fall through.  If we didn't take the if above, then this '/' is part of a parameter!
		default:
			;	// do nothing
		}
	}

	szFileString[i] = '\0';

	for( i=0; i<m_iNumValues; i++ )
	{
		for( int j=0; j<m_iNumParams[i]; j++ )
		{
			m_sParams[i][j] = szParams[i][j];
		}
	}

	fclose( fp );
	delete [] szFileString;

	return true;
}

