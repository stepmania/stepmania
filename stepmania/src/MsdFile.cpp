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
	m_szFileString = NULL;
	int m_iNumValues = 0;
}

MsdFile::~MsdFile()
{
	if( m_szFileString )
		delete m_szFileString;
}

//returns true if successful, false otherwise
bool MsdFile::ReadFile( CString sNewPath )
{
	if( m_szFileString )
		delete m_szFileString;

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

	int iBufferSize = GetFileSize( hFile, NULL ) + 1;
	CloseHandle( hFile );

	// allocate a string to hold the file
	m_szFileString = new char[iBufferSize];

	FILE* fp = fopen(sNewPath, "r");
	if( fp == NULL )
		return false;

	int iBytesRead = fread( m_szFileString, 1, iBufferSize, fp );

	ASSERT( iBufferSize > iBytesRead );

	m_iNumValues = 0;
	
	for( int i=0; i<iBytesRead; i++ )
	{
		switch( m_szFileString[i] )
		{
		case '#':	// begins a new value
			{
				m_iNumValues++;
				int iCurValueIndex = m_iNumValues-1;
				m_iNumParams[iCurValueIndex] = 1;
				m_szValuesAndParams[iCurValueIndex][0] = &m_szFileString[i+1];
			}
			break;
		case ':':	// begins a new parameter
			{
				m_szFileString[i] = '\0';
				int iCurValueIndex = m_iNumValues-1;
				m_iNumParams[iCurValueIndex] ++;
				int iCurParamIndex = m_iNumParams[iCurValueIndex]-1;
				m_szValuesAndParams[iCurValueIndex][iCurParamIndex] = &m_szFileString[i+1];
			}
			break;
		case ';':	// ends a value
			{
				m_szFileString[i] = '\0';
				// fast forward until just before the next '#'
				while( m_szFileString[i+1] != '#'  &&  i<iBytesRead )
					i++;
			}
			break;
		case '/':
			if( m_szFileString[i+1] == '/' )
			{
				// advance until new line
				for( ; i<iBytesRead; i++ )
				{
					if( m_szFileString[i] == '\n' )
						break;
					else
						m_szFileString[i] = ' ';
				}
			}
			// fall through.  If we didn't take the if above, then this '/' is part of a parameter!
		default:
			;	// do nothing
		}
	}

	m_szFileString[i] = '\0';

	int iCurValueIndex = m_iNumValues-1;
	int iCurParamIndex = m_iNumParams[iCurValueIndex]-1;

	for( i=0; i<m_iNumValues; i++ )
	{
		for( int j=0; j<m_iNumParams[i]; j++ )
		{
			m_sValuesAndParams[i][j] = m_szValuesAndParams[i][j];
		}
	}

	fclose( fp );

	return true;
}

