#include "global.h"
/*
 -----------------------------------------------------------------------------
 Class: RageFile
 
 Desc: See header.
 
 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
        Steve Checkoway
 -----------------------------------------------------------------------------
 */

#include "global.h"
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

bool RageFile::Open(const CString& path, const char *mode)
{
    Close();
    mPath = path;
    FixSlashesInPlace(mPath);
    mFP = fopen(mPath, mode);
    
    return mFP == NULL;
}

void RageFile::Close()
{
    if (IsOpen())
	fclose(mFP);
    
    mFP = NULL;
}

CString RageFile::GetLine()
{
    if (!IsOpen())
        RageException::Throw("\"%s\" is not open.", mPath.c_str());
    
    CString buf("");
    char buffer[256]; // long line!
    
    do
    {
	char *ret = fgets(buffer, 256, mFP);
	
	buf += buffer;
	
	if (ret == NULL)
	    break; // EOF or error
    } while (strlen(buffer) == 255);
    
    return buf;
}

size_t RageFile::Read(void *buffer, size_t bytes)
{
    if (!IsOpen())
	RageException::Throw("\"%s\" is not open.", mPath.c_str());
    
    return fread(buffer, 1L, bytes, mFP);
}

size_t RageFile::Write(const void *buffer, size_t bytes)
{
    if (!IsOpen())
	RageException::Throw("\"%s\" is not open.", mPath.c_str());
    
    return fwrite(buffer, 1L, bytes, mFP);
}
