#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: RageFile

 Desc: See header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageFile.h"
#include "RageUtil.h"


void FixSlashesInPlace( CString &sPath )
{
	sPath.Replace( "/", SLASH );
	sPath.Replace( "\\", SLASH );
}

CString FixSlashes( CString sPath )
{
	sPath.Replace( "/", SLASH );
	sPath.Replace( "\\", SLASH );
	return sPath;
}

void CollapsePath( CString &sPath )
{
	CStringArray as;
	split( sPath, SLASH, as );

	for( unsigned i=0; i<as.size(); i++ )
	{
		if( as[i] == ".." )
		{
			as.erase( as.begin()+i-1 );
			as.erase( as.begin()+i-1 );
			i -= 2;
		}
	}
	sPath = join( SLASH, as );
}

// replacement for fopen
FILE* Ragefopen( const char *szPath, const char *szMode )
{
	return fopen( FixSlashes(szPath), szMode ); 
}
