/*
 * Ref: http://www.info-zip.org/pub/infozip/doc/appnote-981119-iz.zip
 */

#include "global.h"
#include "RageFileDriverZip.h"
#include "RageFileDriverSlice.h"
#include "RageFileDriverDeflate.h"
#include "RageFile.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"
#include <cerrno>

static struct FileDriverEntry_ZIP: public FileDriverEntry
{
	FileDriverEntry_ZIP(): FileDriverEntry( "ZIP" ) { }
	RageFileDriver *Create( RString Root ) const { return new RageFileDriverZip( Root ); }
} const g_RegisterDriver;


RageFileDriverZip::RageFileDriverZip():
	RageFileDriver( new NullFilenameDB ),
	m_Mutex( "RageFileDriverZip" )
{
	m_bFileOwned = false;
	m_pZip = NULL;
}

RageFileDriverZip::RageFileDriverZip( RString sPath ):
	RageFileDriver( new NullFilenameDB ),
	m_Mutex( "RageFileDriverZip" )
{
	m_bFileOwned = false;
	m_pZip = NULL;
	Load( sPath );
}

/* deprecated */
RageFileDriverZip::RageFileDriverZip( RageFileBasic *pFile ):
	RageFileDriver( new NullFilenameDB ),
	m_Mutex( "RageFileDriverZip" )
{
	m_bFileOwned = false;
	m_pZip = NULL;
	Load( pFile );
}

bool RageFileDriverZip::Load( const RString &sPath )
{
	ASSERT( m_pZip == NULL ); /* don't load twice */

	m_bFileOwned = true;
	m_sPath = sPath;
	m_Mutex.SetName( ssprintf("RageFileDriverZip(%s)", sPath.c_str()) );

	RageFile *pFile = new RageFile;

	if( !pFile->Open(sPath) )
	{
		LOG->Warn( "Couldn't open %s: %s", sPath.c_str(), pFile->GetError().c_str() );
		delete pFile;
		return false;
	}

	m_pZip = pFile;

	return ParseZipfile();
}

bool RageFileDriverZip::Load( RageFileBasic *pFile )
{
	ASSERT( m_pZip == NULL ); /* don't load twice */
	m_sPath = ssprintf("%p", pFile);
	m_Mutex.SetName( ssprintf("RageFileDriverZip(%p)", pFile) );

	m_pZip = pFile;

	return ParseZipfile();
}


bool RageFileDriverZip::ReadEndCentralRecord( int &iTotalEntries, int &iCentralDirectoryOffset )
{
	RString sError;
	RString sSig = FileReading::ReadString( *m_pZip, 4, sError );
	FileReading::read_16_le( *m_pZip, sError ); /* skip number of this disk */
	FileReading::read_16_le( *m_pZip, sError ); /* skip disk with central directory */
	FileReading::read_16_le( *m_pZip, sError ); /* skip number of entries on this disk */
	iTotalEntries = FileReading::read_16_le( *m_pZip, sError );
	FileReading::read_32_le( *m_pZip, sError ); /* skip size of the central directory */
	iCentralDirectoryOffset = FileReading::read_32_le( *m_pZip, sError );
	FileReading::read_16_le( *m_pZip, sError ); /* skip zipfile comment length */

	if( sError != "" )
	{
		LOG->Warn( "%s: %s", m_sPath.c_str(), sError.c_str() );
		return false;
	}

	return true;
}

/* Find the end of central directory record, and seek to it. */
bool RageFileDriverZip::SeekToEndCentralRecord()
{
	const int iSearchTo = max( m_pZip->GetFileSize() - 1024*32, 0 );
	int iRealPos = m_pZip->GetFileSize();

	while( iRealPos > 0 && iRealPos >= iSearchTo )
	{
		/* Move back in the file; leave some overlap between checks, to handle
		 * the case where the signature crosses the block boundary. */
		char buf[1024*4];
		iRealPos -= sizeof(buf) - 4;
		iRealPos = max( 0, iRealPos );
		m_pZip->Seek( iRealPos );

		int iGot = m_pZip->Read( buf, sizeof(buf) );
		if( iGot == -1 )
		{
			LOG->Warn( "%s: %s", m_sPath.c_str(), m_pZip->GetError().c_str() );
			return false;
		}

		for( int iPos = iGot; iPos >= 0; --iPos )
		{
			if( memcmp(buf + iPos, "\x50\x4B\x05\x06", 4) )
				continue;

			m_pZip->Seek( iRealPos + iPos );
			return true;
		}
	}

	return false;
}

bool RageFileDriverZip::ParseZipfile()
{
	if( !SeekToEndCentralRecord() )
	{
		LOG->Warn( "Couldn't open %s: couldn't find end of central directory record", m_sPath.c_str() );
		return false;
	}

	/* Read the end of central directory record. */
	int iTotalEntries, iCentralDirectoryOffset;
	if( !ReadEndCentralRecord(iTotalEntries, iCentralDirectoryOffset) )
		return false; /* warned already */

	/* Seek to the start of the central file directory. */
	m_pZip->Seek( iCentralDirectoryOffset );

	/* Loop through files in central directory. */
	for( int i = 0; i < iTotalEntries; ++i )
	{
		FileInfo info;
		info.m_iDataOffset = -1;
		int got = ProcessCdirFileHdr( info );
		if( got == -1 ) /* error */
			break;
		if( got == 0 ) /* skip */
			continue;

		FileInfo *pInfo = new FileInfo( info );
		m_pFiles.push_back( pInfo );
		FDB->AddFile( "/" + pInfo->m_sName, pInfo->m_iUncompressedSize, pInfo->m_iCRC32, pInfo );
	}

	if( m_pFiles.size() == 0 )
		LOG->Warn( "%s: no files found in central file header", m_sPath.c_str() );

	return true;
}

int RageFileDriverZip::ProcessCdirFileHdr( FileInfo &info )
{
	RString sError;
	RString sSig = FileReading::ReadString( *m_pZip, 4, sError );
	if( sSig != "\x50\x4B\x01\x02" )
	{
		LOG->Warn( "%s: central directory record signature not found", m_sPath.c_str() );
		return -1;
	}

	FileReading::read_8( *m_pZip, sError ); /* skip version made by */
	int iOSMadeBy = FileReading::read_8( *m_pZip, sError );
	FileReading::read_16_le( *m_pZip, sError ); /* skip version needed to extract */
	int iGeneralPurpose = FileReading::read_16_le( *m_pZip, sError );
	info.m_iCompressionMethod = (ZipCompressionMethod) FileReading::read_16_le( *m_pZip, sError );
	FileReading::read_16_le( *m_pZip, sError ); /* skip last mod file time */
	FileReading::read_16_le( *m_pZip, sError ); /* skip last mod file date */
	info.m_iCRC32 = FileReading::read_32_le( *m_pZip, sError );
	info.m_iCompressedSize = FileReading::read_32_le( *m_pZip, sError );
	info.m_iUncompressedSize = FileReading::read_32_le( *m_pZip, sError );
	int iFilenameLength = FileReading::read_16_le( *m_pZip, sError );
	int iExtraFieldLength = FileReading::read_16_le( *m_pZip, sError );
	int iFileCommentLength = FileReading::read_16_le( *m_pZip, sError );
	FileReading::read_16_le( *m_pZip, sError ); /* relative offset of local header */
	FileReading::read_16_le( *m_pZip, sError ); /* skip internal file attributes */
	unsigned iExternalFileAttributes = FileReading::read_32_le( *m_pZip, sError );
	info.m_iOffset = FileReading::read_32_le( *m_pZip, sError );

	/* Check for errors before reading variable-length fields. */
	if( sError != "" )
	{
		LOG->Warn( "%s: %s", m_sPath.c_str(), sError.c_str() );
		return -1;
	}

	info.m_sName = FileReading::ReadString( *m_pZip, iFilenameLength, sError );
	FileReading::SkipBytes( *m_pZip, iExtraFieldLength, sError ); /* skip extra field */
	FileReading::SkipBytes( *m_pZip, iFileCommentLength, sError ); /* skip file comment */

	if( sError != "" )
	{
		LOG->Warn( "%s: %s", m_sPath.c_str(), sError.c_str() );
		return -1;
	}

	/* Check usability last, so we always read past the whole entry and don't leave the
	 * file pointer in the middle of a record. */
	if( iGeneralPurpose & 1 )
	{
		LOG->Warn( "Skipped encrypted \"%s\" in \"%s\"", info.m_sName.c_str(), m_sPath.c_str() );
		return 0;
	}

	/* Skip directories. */
	if( iExternalFileAttributes & (1<<4) )
		return 0;

	info.m_iFilePermissions = 0;
	enum { MADE_BY_UNIX = 3 };
	switch( iOSMadeBy )
	{
	case MADE_BY_UNIX:
		info.m_iFilePermissions = (iExternalFileAttributes >> 16) & 0x1FF;
		break;
	}

	if( info.m_iCompressionMethod != STORED && info.m_iCompressionMethod != DEFLATED )
	{
		LOG->Warn( "File \"%s\" in \"%s\" uses unsupported compression method %i",
			info.m_sName.c_str(), m_sPath.c_str(), info.m_iCompressionMethod );

		return 0;
	}

	return 1;
}

bool RageFileDriverZip::ReadLocalFileHeader( FileInfo &info )
{
	/* Seek to and read the local file header. */
	m_pZip->Seek( info.m_iOffset );

	RString sError;
	RString sSig = FileReading::ReadString( *m_pZip, 4, sError );

	if( sError != "" )
	{
		LOG->Warn( "%s: error opening \"%s\": %s", m_sPath.c_str(), info.m_sName.c_str(), sError.c_str() );
		return false;
	}

	if( sSig != "\x50\x4B\x03\x04" )
	{
		LOG->Warn( "%s: local file header not found for \"%s\"", m_sPath.c_str(), info.m_sName.c_str() );
		return false;
	}

	FileReading::SkipBytes( *m_pZip, 22, sError ); /* skip most of the local file header */

	const int iFilenameLength = FileReading::read_16_le( *m_pZip, sError );
	const int iExtraFieldLength = FileReading::read_16_le( *m_pZip, sError );
	info.m_iDataOffset = m_pZip->Tell() + iFilenameLength + iExtraFieldLength;

	if( sError != "" )
	{
		LOG->Warn( "%s: %s", m_sPath.c_str(), sError.c_str() );
		return false;
	}
	
	return true;
}

RageFileDriverZip::~RageFileDriverZip()
{
	for( unsigned i = 0; i < m_pFiles.size(); ++i )
		delete m_pFiles[i];

	if( m_bFileOwned )
		delete m_pZip;
}

const RageFileDriverZip::FileInfo *RageFileDriverZip::GetFileInfo( const RString &sPath ) const
{
	return (const FileInfo *) FDB->GetFilePriv( sPath );
}

RageFileBasic *RageFileDriverZip::Open( const RString &sPath, int iMode, int &iErr )
{
	if( iMode & RageFile::WRITE )
	{
		iErr = ERROR_WRITING_NOT_SUPPORTED;
		return NULL;
	}

	FileInfo *info = (FileInfo *) FDB->GetFilePriv( sPath );
	if( info == NULL )
	{
		iErr = ENOENT;
		return NULL;
	}

	m_Mutex.Lock();

	/* If we havn't figured out the offset to the real data yet, do so now. */
	if( info->m_iDataOffset == -1 )
	{
		if( !ReadLocalFileHeader(*info) )
		{
			m_Mutex.Unlock();
			return NULL;
		}
	}

	/* We won't do any further access to zip, except to copy it (which is
	 * threadsafe), so we can unlock now. */
	m_Mutex.Unlock();

	RageFileDriverSlice *pSlice = new RageFileDriverSlice( m_pZip->Copy(), info->m_iDataOffset, info->m_iCompressedSize );
	pSlice->DeleteFileWhenFinished();
	
	switch( info->m_iCompressionMethod )
	{
	case STORED:
		return pSlice;
	case DEFLATED:
	{
		RageFileObjInflate *pInflate = new RageFileObjInflate( pSlice, info->m_iUncompressedSize );
		pInflate->DeleteFileWhenFinished();
		return pInflate;
	}
	default:
		/* unknown compression method */
		iErr = ENOSYS;
		return NULL;
	}
}

/* NOP for now.  This could check to see if the ZIP's mtime has changed, and reload. */
void RageFileDriverZip::FlushDirCache( const RString &sPath )
{

}

RString RageFileDriverZip::GetGlobalComment() const
{
	ASSERT(0);
}

/*
 * Copyright (c) 2003-2005 Glenn Maynard.  All rights reserved.
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
