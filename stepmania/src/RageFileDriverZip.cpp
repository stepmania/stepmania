/*
 * Derived from Info-ZIP 5.50.  Heavily stripped and rewritten.  Only retains
 * STORE and DEFLATE decompression types; that's all that's in use these days.
 * Could probably readd SHRINK easily.
 *  - Glenn
 */

#include "global.h"
#include "RageFileDriverZip.h"
#include "RageFileDriverSlice.h"
#include "RageFileDriverDeflate.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"
#include <cerrno>

#define INBUFSIZE 1024*4

static struct FileDriverEntry_ZIP: public FileDriverEntry
{
	FileDriverEntry_ZIP(): FileDriverEntry( "ZIP" ) { }
	RageFileDriver *Create( CString Root ) const { return new RageFileDriverZip( Root ); }
} const g_RegisterDriver;

struct FileInfo
{
	CString fn;
	long offset;
	long data_offset;

	long extra_field_length;
	long filename_length;
	unsigned short compression_method;
	unsigned long crc;                 /* crc (needed if extended header) */
	unsigned long compr_size;          /* compressed size (needed if extended header) */
	unsigned long uncompr_size;        /* uncompressed size (needed if extended header) */
	unsigned short diskstart;           /* no of volume where this entry starts */
};

struct end_central_dir_record
{
	unsigned short number_this_disk;
	unsigned short num_disk_start_cdir;
	unsigned short num_entries_centrl_dir_ths_disk;
	unsigned short total_entries_central_dir;
	unsigned long size_central_directory;
	unsigned long offset_start_central_directory;
	unsigned short zipfile_comment_length;
};

#define STORED		0
#define DEFLATED	8

RageFileDriverZip::RageFileDriverZip( CString path ):
	RageFileDriver( new NullFilenameDB ),
	m_Mutex( ssprintf("RageFileDriverZip(%s)", path.c_str()) )
{
	m_bFileOwned = true;
	m_sPath = path;

	RageFile *pFile = new RageFile;
	m_pZip = pFile;

	if( !pFile->Open(path) )
		RageException::Throw( "Couldn't open %s: %s", path.c_str(), pFile->GetError().c_str() );

	ParseZipfile();
}

RageFileDriverZip::RageFileDriverZip( RageFileBasic *pFile ):
	RageFileDriver( new NullFilenameDB ),
	m_Mutex( ssprintf("RageFileDriverZip(%p)", pFile) )
{
	m_sPath = ssprintf("%p", pFile);

	m_bFileOwned = false;
	m_pZip = pFile;
	
	ParseZipfile();
}

#define ECREC_SIZE  18

static CString central_hdr_sig = "\x50\x4B\x01\x02";
static CString local_hdr_sig   = "\x50\x4B\x03\x04";
static CString end_central_sig = "\x50\x4B\x05\x06";


/* XXX */
static unsigned short makeword(const unsigned char *b)
{
	return (unsigned short)((b[1] << 8) | b[0]);
}


static unsigned long makelong(const unsigned char *sig)
{
	return (((unsigned long)sig[3]) << 24)
		+ (((unsigned long)sig[2]) << 16)
		+ (((unsigned long)sig[1]) << 8)
		+  ((unsigned long)sig[0]);
}

void RageFileDriverZip::ReadEndCentralRecord( end_central_dir_record &ec )
{
	const int OrigPos = m_pZip->Tell();
	typedef unsigned char ec_byte_rec[ ECREC_SIZE+4 ];
#define NUMBER_THIS_DISK                  4
#define NUM_DISK_WITH_START_CENTRAL_DIR   6
#define NUM_ENTRIES_CENTRL_DIR_THS_DISK   8
#define TOTAL_ENTRIES_CENTRAL_DIR         10
#define SIZE_CENTRAL_DIRECTORY            12
#define OFFSET_START_CENTRAL_DIRECTORY    16
#define ZIPFILE_COMMENT_LENGTH            20

	ec_byte_rec byterec;
	const int got = m_pZip->Read( byterec, ECREC_SIZE+4 );
	if( got == -1 )
		RageException::Throw( "Couldn't open %s: %s", m_sPath.c_str(), m_pZip->GetError().c_str() );
	if ( got != ECREC_SIZE+4 )
		RageException::Throw( "%s: unexpected EOF", m_sPath.c_str() );

	ec.number_this_disk = makeword(&byterec[NUMBER_THIS_DISK]);
	ec.num_disk_start_cdir = makeword(&byterec[NUM_DISK_WITH_START_CENTRAL_DIR]);
	ec.num_entries_centrl_dir_ths_disk = makeword(&byterec[NUM_ENTRIES_CENTRL_DIR_THS_DISK]);
	ec.total_entries_central_dir = makeword(&byterec[TOTAL_ENTRIES_CENTRAL_DIR]);
	ec.size_central_directory = makelong(&byterec[SIZE_CENTRAL_DIRECTORY]);
	ec.offset_start_central_directory = makelong(&byterec[OFFSET_START_CENTRAL_DIRECTORY]);
	ec.zipfile_comment_length = makeword(&byterec[ZIPFILE_COMMENT_LENGTH]);

	const int expect_ecrec_offset = ec.offset_start_central_directory + ec.size_central_directory;
	if( expect_ecrec_offset > OrigPos  )
		RageException::Throw( "Couldn't open %s: missing %ld bytes in zipfile", m_sPath.c_str(),
				expect_ecrec_offset - OrigPos  );
}

void RageFileDriverZip::ParseZipfile()
{
	/* Look for the end-central record. */
	const int searchlen = min( m_pZip->GetFileSize(), 66000 );
	const int Size = m_pZip->GetFileSize();
	int realpos = Size;

	/* Loop through blocks of data, starting at the end.  In general, need not
	 * check whole zim_pZip for signature, but may want to do so if testing. */

	int real_ecrec_offset = -1;

	char tmp[INBUFSIZE];
	int tmp_used = 0;
	while( real_ecrec_offset == -1 && realpos > 0 && realpos >= Size-searchlen )
	{
		realpos -= INBUFSIZE;
		realpos = max( 0, realpos );
		m_pZip->Seek( realpos );
		int got = m_pZip->Read( tmp+tmp_used, sizeof(tmp)-tmp_used );
		if( got == -1 )
			RageException::Throw( "Couldn't open %s: %s", m_sPath.c_str(), m_pZip->GetError().c_str() );
		if( got == 0 )
			break;          /* fall through and fail */
		got += tmp_used;

		/* 'P' must be at least (ECREC_SIZE+4) bytes from end of zim_pZip */
		for( int pos = got-(ECREC_SIZE+4); real_ecrec_offset == -1 && pos >= 0; --pos )
		{
			const char *p = tmp+pos;
			if ( *p != (unsigned char)0x50 ) /* ASCII 'P' */
				continue;

			if( strncmp((char *)p, end_central_sig, 4))
				continue;
			real_ecrec_offset = realpos + pos;
		}

		/* sig may span block boundary: */
		memcpy((char *)tmp+INBUFSIZE-3, (char *)tmp, 3);
		tmp_used = 3;
	}

	if( real_ecrec_offset == -1 )
		RageException::Throw( "Couldn't open %s: End-of-central-directory signature not found", m_sPath.c_str() );

	m_pZip->Seek( real_ecrec_offset );

	/* Get the end-central data. */
	end_central_dir_record ec;
	ReadEndCentralRecord( ec );

	bool something = ec.number_this_disk == 1 && ec.num_disk_start_cdir == 1;
	if (!something && ec.number_this_disk != 0)
		RageException::Throw( "Couldn't open %s: zipfile is part of multi-disk archive", m_sPath.c_str() );

	/* Seek to where the start of central directory should be, and make
	 * sure it's there. */
	m_pZip->Seek( ec.offset_start_central_directory );

	CString sig;
	int got = m_pZip->Read( sig, 4 );
	if( got != 4 || sig != central_hdr_sig )
		RageException::Throw( "Couldn't open %s: start of central directory not found; zipfile corrupt", m_sPath.c_str() );

	m_pZip->Seek( ec.offset_start_central_directory );

	/* Loop through files in central directory. */
	while(1)
	{
		CString sig;
		if ( m_pZip->Read( sig, 4 ) != 4 )
			break;

		/* Is it a new entry? */
		if( sig != central_hdr_sig )
		{
			/* Not a new central directory entry.  Is the number of processed entries
			 * compatible with the number of entries as stored in the end_central record? */
			if( Files.size() == (unsigned)ec.total_entries_central_dir )
			{
				/* Are we at the end_central record? */
				if( sig != end_central_sig )
					LOG->Warn( "%s: expected end central file header signature not found", m_sPath.c_str() );
			} else {
				LOG->Warn( "%s: expected central file header signature not found", m_sPath.c_str() );
			}
			break;
		}

		FileInfo info;
		info.data_offset = -1;
		int got = ProcessCdirFileHdr( info );
		if( got == -1 ) /* error */
			break;
		if( got == 0 ) /* skip */
			continue;

		m_pZip->Seek( m_pZip->Tell() + info.extra_field_length );

		FileInfo *pInfo = new FileInfo(info);
		Files.push_back( pInfo );
		FDB->AddFile( pInfo->fn, pInfo->uncompr_size, pInfo->crc, pInfo );
	}
}

#define CREC_SIZE   42
typedef unsigned char cdir_byte_hdr[ CREC_SIZE ];
#define C_VERSION_MADE_BY_0               0
#define C_VERSION_MADE_BY_1               1
#define C_VERSION_NEEDED_TO_EXTRACT_0     2
#define C_VERSION_NEEDED_TO_EXTRACT_1     3
#define C_GENERAL_PURPOSE_BIT_FLAG        4
#define C_COMPRESSION_METHOD              6
#define C_LAST_MOD_DOS_DATETIME           8
#define C_CRC32                           12
#define C_COMPRESSED_SIZE                 16
#define C_UNCOMPRESSED_SIZE               20
#define C_FILENAME_LENGTH                 24
#define C_EXTRA_FIELD_LENGTH              26
#define C_FILE_COMMENT_LENGTH             28
#define C_DISK_NUMBER_START               30
#define C_INTERNAL_FILE_ATTRIBUTES        32
#define C_EXTERNAL_FILE_ATTRIBUTES        34
#define C_RELATIVE_OFFSET_LOCAL_HEADER    38
int RageFileDriverZip::ProcessCdirFileHdr( FileInfo &info )
{
	/* Read the next central directory entry and do any necessary machine-type
	 * conversions (byte ordering, structure padding compensation--do so by
	 * copying the data from the array into which it was read (byterec) to the
	 * usable struct. */
	cdir_byte_hdr byterec;
	int got = m_pZip->Read( (char *)byterec, CREC_SIZE );
	if ( got == -1 )
		RageException::Throw( "Couldn't open %s: %s", m_sPath.c_str(), m_pZip->GetError().c_str() );
	if ( got != CREC_SIZE )
	{
		LOG->Warn( "%s: unexpected EOF", m_sPath.c_str() );
		return -1;
	}

//	crec.version_made_by[0] = byterec[C_VERSION_MADE_BY_0];
//	crec.version_made_by[1] = byterec[C_VERSION_MADE_BY_1];
	const int version_needed_to_extract = byterec[C_VERSION_NEEDED_TO_EXTRACT_0];
	if( version_needed_to_extract > 20 ) /* compatible with PKUNZIP 2.0 */
	{
		LOG->Warn( "File \"%s\" in \"%s\" uses unsupported ZIP version %i.%i",
			info.fn.c_str(), m_sPath.c_str(), 
			version_needed_to_extract / 10, version_needed_to_extract % 10 );
		return 0;
	}

	const int general_purpose_bit_flag = makeword(&byterec[C_GENERAL_PURPOSE_BIT_FLAG]);
	if( general_purpose_bit_flag & 1 )
	{
		LOG->Warn( "Skipped encrypted \"%s\" in \"%s\"", info.fn.c_str(), m_sPath.c_str() );
		return 0;
	}

	info.compression_method = makeword(&byterec[C_COMPRESSION_METHOD]);
//	int last_mod_dos_datetime = makelong(&byterec[C_LAST_MOD_DOS_DATETIME]);
	info.crc = makelong(&byterec[C_CRC32]);
	info.compr_size = makelong(&byterec[C_COMPRESSED_SIZE]);
	info.uncompr_size = makelong(&byterec[C_UNCOMPRESSED_SIZE]);
	info.filename_length = makeword(&byterec[C_FILENAME_LENGTH]);
	info.extra_field_length = makeword(&byterec[C_EXTRA_FIELD_LENGTH]);
//	int file_comment_length = makeword(&byterec[C_FILE_COMMENT_LENGTH]);
	info.diskstart = makeword(&byterec[C_DISK_NUMBER_START]);
//	int internal_file_attributes = makeword(&byterec[C_INTERNAL_FILE_ATTRIBUTES]);
	int external_file_attributes = makelong(&byterec[C_EXTERNAL_FILE_ATTRIBUTES]);  /* LONG, not word! */
	info.offset = makelong(&byterec[C_RELATIVE_OFFSET_LOCAL_HEADER]);

	/* Skip directories. */
	if( external_file_attributes & 0x08 )
		return 0;

	got = m_pZip->Read( info.fn, info.filename_length );
	if( got == -1 )
		RageException::Throw( "Couldn't open %s: %s", m_sPath.c_str(), m_pZip->GetError().c_str() );
	if( got != info.filename_length )
	{
		LOG->Warn( "%s: bad filename length %li", m_sPath.c_str(), info.filename_length );
		return 0;
	}

	if( info.compression_method != STORED && info.compression_method != DEFLATED )
	{
		LOG->Warn( "File \"%s\" in \"%s\" uses unsupported compression method %i",
			info.fn.c_str(), m_sPath.c_str(), info.compression_method );

		return 0;
	}

	return 1;
}

RageFileDriverZip::~RageFileDriverZip()
{
	for( unsigned i = 0; i < Files.size(); ++i )
		delete Files[i];

	if( m_bFileOwned )
		delete m_pZip;
}

RageFileBasic *RageFileDriverZip::Open( const CString &path, int mode, int &err )
{
	if( mode == RageFile::WRITE )
	{
		err = ERROR_WRITING_NOT_SUPPORTED;
		return NULL;
	}

	FileInfo *info = (FileInfo *) FDB->GetFilePriv( path );
	if( info == NULL )
	{
		err = ENOENT;
		return NULL;
	}

	m_Mutex.Lock();

	/* If we havn't figured out the offset to the real data yet, do so now. */
	if( info->data_offset == -1 )
	{
		m_pZip->Seek( info->offset );

		/* Should be in proper position now, so check for sig. */
		CString sig;
		int got = m_pZip->Read( sig, 4 );
		if( got == -1 )
			RageException::Throw( "Read error in %s: %s", m_sPath.c_str(), m_pZip->GetError().c_str() );
		if( got != 4 )
			RageException::Throw( "%s: unexpected EOF", m_sPath.c_str() );

		if( sig != local_hdr_sig )
			RageException::Throw( "%s: bad zipfile offset", m_sPath.c_str() );

#define LREC_SIZE   26   /* lengths of local file headers, central */
#define L_FILENAME_LENGTH                 22
#define L_EXTRA_FIELD_LENGTH              24
		unsigned char byterec[LREC_SIZE];
		got = m_pZip->Read( (char *)byterec, LREC_SIZE );
		if( got == -1 )
			RageException::Throw( "Read error in %s: %s", m_sPath.c_str(), m_pZip->GetError().c_str() );
		if( got != LREC_SIZE )
			RageException::Throw( "%s: unexpected EOF", m_sPath.c_str() );

		const int filename_length = makeword(&byterec[L_FILENAME_LENGTH]);
		const int extra_field_length = makeword(&byterec[L_EXTRA_FIELD_LENGTH]);
		info->data_offset = m_pZip->Tell() + filename_length + extra_field_length;
	}

	/* We won't do any further access to zip, except to copy it (which is
	 * threadsafe), so we can unlock now. */
	m_Mutex.Unlock();

	RageFileDriverSlice *pSlice = new RageFileDriverSlice( m_pZip->Copy(), info->data_offset, info->compr_size );
	pSlice->DeleteFileWhenFinished();
	
	switch( info->compression_method )
	{
	case STORED:
		return pSlice;
	case DEFLATED:
	{
		RageFileObjInflate *pInflate = new RageFileObjInflate( pSlice, info->uncompr_size );
		pInflate->DeleteFileWhenFinished();
		return pInflate;
	}
	default:
		/* unknown compression method */
		ASSERT( 0 );
		err = EINVAL;
		return NULL;
	}
}

/* NOP for now.  This could check to see if the ZIP's mtime has changed, and reload. */
void RageFileDriverZip::FlushDirCache( const CString &sPath )
{

}

/*
 * Copyright (c) 1990-2002 Info-ZIP.  All rights reserved.
 * Copyright (c) 2003-2004 Glenn Maynard.  All rights reserved.
 * 
 * For the purposes of this copyright and license, "Info-ZIP" is defined as
 * the following set of individuals:
 * 
 *    Mark Adler, John Bush, Karl Davis, Harald Denker, Jean-Michel Dubois,
 *    Jean-loup Gailly, Hunter Goatley, Ian Gorman, Chris Herborth, Dirk Haase,
 *    Greg Hartwig, Robert Heath, Jonathan Hudson, Paul Kienitz, David Kirschbaum,
 *    Johnny Lee, Onno van der Linden, Igor Mandrichenko, Steve P. Miller,
 *    Sergio Monesi, Keith Owens, George Petrov, Greg Roelofs, Kai Uwe Rommel,
 *    Steve Salisbury, Dave Smith, Christian Spieler, Antoine Verheijen,
 *    Paul von Behren, Rich Wales, Mike White
 * 
 * This software is provided "as is," without warranty of any kind, express
 * or implied.  In no event shall Info-ZIP or its contributors be held liable
 * for any direct, indirect, incidental, special or consequential damages
 * arising out of the use of or inability to use this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 *     1. Redistributions of source code must retain the above copyright notice,
 *        definition, disclaimer, and this list of conditions.
 * 
 *     2. Redistributions in binary form (compiled executables) must reproduce
 *        the above copyright notice, definition, disclaimer, and this list of
 *        conditions in documentation and/or other materials provided with the
 *        distribution.  The sole exception to this condition is redistribution
 *        of a standard UnZipSFX binary as part of a self-extracting archive;
 *        that is permitted without inclusion of this license, as long as the
 *        normal UnZipSFX banner has not been removed from the binary or disabled.
 * 
 *     3. Altered versions--including, but not limited to, ports to new operating
 *        systems, existing ports with new graphical interfaces, and dynamic,
 *        shared, or static library versions--must be plainly marked as such
 *        and must not be misrepresented as being the original source.  Such
 *        altered versions also must not be misrepresented as being Info-ZIP
 *        releases--including, but not limited to, labeling of the altered
 *        versions with the names "Info-ZIP" (or any variation thereof, including,
 *        but not limited to, different capitalizations), "Pocket UnZip," "WiZ"
 *        or "MacZip" without the explicit permission of Info-ZIP.  Such altered
 *        versions are further prohibited from misrepresentative use of the
 *        Zip-Bugs or Info-ZIP e-mail addresses or of the Info-ZIP URL(s).
 * 
 *     4. Info-ZIP retains the right to use the names "Info-ZIP," "Zip," "UnZip,"
 *        "UnZipSFX," "WiZ," "Pocket UnZip," "Pocket Zip," and "MacZip" for its
 *        own source and binary releases.
 */

