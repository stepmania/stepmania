#include "global.h"
#include "RageFileDriver.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"

RageFileDriver::~RageFileDriver()
{
	delete FDB;
}

int RageFileDriver::GetPathValue( const RString &sPath )
{
	vector<RString> asParts;
	split( sPath, "/", asParts, true );

	RString sPartialPath;

	for( unsigned i = 0; i < asParts.size(); ++i )
	{
		sPartialPath += asParts[i];
		if( i+1 < asParts.size() )
			sPartialPath += "/";

		const RageFileManager::FileType Type = GetFileType( sPartialPath );
		switch( Type )
		{
		case RageFileManager::TYPE_NONE:
			return asParts.size()-i;

		/* If this is the last part (the whole path), it needs to be a file; otherwise a directory. */
		case RageFileManager::TYPE_FILE:
			if( i != asParts.size()-1 )
				return -1;
			break;
		case RageFileManager::TYPE_DIR:
			if( i == asParts.size()-1 )
				return -1;
			break;
		}
	}

	return 0;
}

void RageFileDriver::GetDirListing( const RString &sPath, vector<RString> &asAddTo, bool bOnlyDirs, bool bReturnPathToo )
{
	FDB->GetDirListing( sPath, asAddTo, bOnlyDirs, bReturnPathToo );
}

RageFileManager::FileType RageFileDriver::GetFileType( const RString &sPath )
{
	return FDB->GetFileType( sPath );
}

int RageFileDriver::GetFileSizeInBytes( const RString &sPath )
{
	return FDB->GetFileSize( sPath );
}

int RageFileDriver::GetFileHash( const RString &sPath )
{
	return FDB->GetFileHash( sPath );
}

void RageFileDriver::FlushDirCache( const RString &sPath )
{
	FDB->FlushDirCache();
}


const struct FileDriverEntry *g_pFileDriverList = NULL;

FileDriverEntry::FileDriverEntry( const RString &sType )
{
	m_pLink = g_pFileDriverList;
	g_pFileDriverList = this;
	m_sType = sType;
}

FileDriverEntry::~FileDriverEntry()
{
	g_pFileDriverList = NULL; /* invalidate */
}

RageFileDriver *MakeFileDriver( const RString &sType, const RString &sRoot )
{
	for( const FileDriverEntry *p = g_pFileDriverList; p; p = p->m_pLink )
		if( !p->m_sType.CompareNoCase(sType) )
			return p->Create( sRoot );
	return NULL;
}

/*
 * Copyright (c) 2003-2004 Glenn Maynard
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

