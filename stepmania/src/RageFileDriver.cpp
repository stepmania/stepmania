#include "global.h"
#include "RageFileDriver.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"

RageFileDriver::~RageFileDriver()
{
	delete FDB;
}

int RageFileDriver::GetPathValue( const RString &path )
{
	vector<RString> parts;
	split( path, "/", parts, true );

	RString PartialPath;

	for( unsigned i = 0; i < parts.size(); ++i )
	{
		PartialPath += parts[i];
		if( i+1 < parts.size() )
			PartialPath += "/";

		const RageFileManager::FileType Type = GetFileType( PartialPath );
		switch( Type )
		{
		case RageFileManager::TYPE_NONE:
			return parts.size()-i;

		/* If this is the last part (the whole path), it needs to be a file; otherwise a directory. */
		case RageFileManager::TYPE_FILE:
			if( i != parts.size()-1 )
				return -1;
			break;
		case RageFileManager::TYPE_DIR:
			if( i == parts.size()-1 )
				return -1;
			break;
		}
	}

	return 0;
}

void RageFileDriver::GetDirListing( const RString &sPath, vector<RString> &AddTo, bool bOnlyDirs, bool bReturnPathToo )
{
	FDB->GetDirListing( sPath, AddTo, bOnlyDirs, bReturnPathToo );
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


const struct FileDriverEntry *g_FileDriverList = NULL;

FileDriverEntry::FileDriverEntry( RString Type )
{
	m_Link = g_FileDriverList;
	g_FileDriverList = this;
	m_Type = Type;
}

FileDriverEntry::~FileDriverEntry()
{
	g_FileDriverList = NULL; /* invalidate */
}

RageFileDriver *MakeFileDriver( RString Type, RString Root )
{
	for( const FileDriverEntry *p = g_FileDriverList; p; p = p->m_Link )
		if( !p->m_Type.CompareNoCase(Type) )
			return p->Create( Root );
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

