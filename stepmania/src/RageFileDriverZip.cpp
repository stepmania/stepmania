/*
 * Derived from Info-ZIP 5.50.  Heavily stripped and rewritten.  Only retains
 * STORE and DEFLATE decompression types; that's all that's in use these days.
 * Could probably readd SHRINK easily.
 *  - Glenn
 */

#include "global.h"
#include "RageFileDriverZip.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"

#include <cerrno>

#if defined(_WINDOWS) || defined(_XBOX)
	#include "zlib/zlib.h"
	#pragma comment(lib, "zlib/zdll.lib")
#elif defined(DARWIN)
    /* Since crypto++ was added to the repository, <zlib.h> includes the zlib.h
     * in there rather than the correct system one. I don't know why it would do
     * this since crypto51 is not being listed as one of the include directories.
     * I've never run into this problem before and looking at the command line
     * used to compile RageFileDriverZip.o, I have no idea how it's happening.
     * --Steve
     */
    #include "/usr/include/zlib.h"
#else
	#include <zlib.h>
#endif

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

class RageFileObjZipDeflated: public RageFileObj
{
private:
	const FileInfo &info;
	RageFile zip;
	int CFilePos, UFilePos;

	z_stream dstrm;
	char decomp_buf[INBUFSIZE], *decomp_buf_ptr;
	int decomp_buf_avail;

public:
	RageFileObjZipDeflated( const RageFile &f, const FileInfo &info, RageFile &p );
	RageFileObjZipDeflated( const RageFileObjZipDeflated &cpy, RageFile &p );
	~RageFileObjZipDeflated();
	int Read(void *buffer, size_t bytes);
	int Write(const void *buffer, size_t bytes) { SetError( "Not implemented" ); return -1; }
	void Rewind();
	int Seek( int offset );
	int GetFileSize() { return info.uncompr_size; }
	RageFileObj *Copy( RageFile &p ) const
	{
		RageException::Throw( "Loading ZIPs from deflated ZIPs is currently disabled; see RageFileObjZipDeflated" );

		// return new RageFileObjZipDeflated( *this, p );
	}
};

class RageFileObjZipStored: public RageFileObj
{
private:
	const FileInfo &info;
	RageFile zip;
	int FilePos;

public:
	RageFileObjZipStored( const RageFile &f, const FileInfo &info, RageFile &p );
	int Read(void *buffer, size_t bytes);
	int Write(const void *buffer, size_t bytes) { SetError( "Not implemented" ); return -1; }

	void Rewind()
	{
		zip.Seek( info.data_offset );
		FilePos = 0;
	}

	int Seek( int offset );
	int GetFileSize() { return info.uncompr_size; }

	RageFileObj *Copy( RageFile &p ) const
	{
		RageFileObjZipStored *pRet = new RageFileObjZipStored( zip, info, p );
		pRet->FilePos = FilePos;
		return pRet;
	}
};

RageFileDriverZip::RageFileDriverZip( CString path ):
	RageFileDriver( new NullFilenameDB )
{
	if( !zip.Open(path) )
		RageException::Throw( "Couldn't open %s: %s", path.c_str(), zip.GetError().c_str() );

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

void RageFileDriverZip::ReadEndCentralRecord( RageFile &zip, end_central_dir_record &ec )
{
	const int OrigPos = zip.Tell();
	typedef unsigned char ec_byte_rec[ ECREC_SIZE+4 ];
#define NUMBER_THIS_DISK                  4
#define NUM_DISK_WITH_START_CENTRAL_DIR   6
#define NUM_ENTRIES_CENTRL_DIR_THS_DISK   8
#define TOTAL_ENTRIES_CENTRAL_DIR         10
#define SIZE_CENTRAL_DIRECTORY            12
#define OFFSET_START_CENTRAL_DIRECTORY    16
#define ZIPFILE_COMMENT_LENGTH            20

	ec_byte_rec byterec;
	const int got = zip.Read( byterec, ECREC_SIZE+4 );
	if( got == -1 )
		RageException::Throw( "Couldn't open %s: %s", zip.GetPath().c_str(), zip.GetError().c_str() );
    if ( got != ECREC_SIZE+4 )
		RageException::Throw( "%s: unexpected EOF", zip.GetPath().c_str() );

    ec.number_this_disk = makeword(&byterec[NUMBER_THIS_DISK]);
    ec.num_disk_start_cdir = makeword(&byterec[NUM_DISK_WITH_START_CENTRAL_DIR]);
    ec.num_entries_centrl_dir_ths_disk = makeword(&byterec[NUM_ENTRIES_CENTRL_DIR_THS_DISK]);
    ec.total_entries_central_dir = makeword(&byterec[TOTAL_ENTRIES_CENTRAL_DIR]);
    ec.size_central_directory = makelong(&byterec[SIZE_CENTRAL_DIRECTORY]);
    ec.offset_start_central_directory = makelong(&byterec[OFFSET_START_CENTRAL_DIRECTORY]);
    ec.zipfile_comment_length = makeword(&byterec[ZIPFILE_COMMENT_LENGTH]);

    const int expect_ecrec_offset = ec.offset_start_central_directory + ec.size_central_directory;
    if( expect_ecrec_offset > OrigPos  )
		RageException::Throw( "Couldn't open %s: missing %ld bytes in zipfile", zip.GetPath().c_str(),
			expect_ecrec_offset - OrigPos  );
}

void RageFileDriverZip::ParseZipfile()
{
	/* Look for the end-central record. */
	const int searchlen = min( zip.GetFileSize(), 66000 );
	const int Size = zip.GetFileSize();
	int realpos = Size;

	/* Loop through blocks of data, starting at the end.  In general, need not
	 * check whole zipfile for signature, but may want to do so if testing. */

	int real_ecrec_offset = -1;

	char tmp[INBUFSIZE];
	int tmp_used = 0;
	while( real_ecrec_offset == -1 && realpos > 0 && realpos >= Size-searchlen )
	{
		realpos -= INBUFSIZE;
		realpos = max( 0, realpos );
		zip.Seek( realpos );
		int got = zip.Read( tmp+tmp_used, sizeof(tmp)-tmp_used );
		if( got == -1 )
			RageException::Throw( "Couldn't open %s: %s", zip.GetPath().c_str(), zip.GetError().c_str() );
		if( got == 0 )
			break;          /* fall through and fail */
		got += tmp_used;

		/* 'P' must be at least (ECREC_SIZE+4) bytes from end of zipfile */
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
		RageException::Throw( "Couldn't open %s: End-of-central-directory signature not found", zip.GetPath().c_str() );

	zip.Seek( real_ecrec_offset );

	/* Get the end-central data. */
	end_central_dir_record ec;
	ReadEndCentralRecord( zip, ec );

	bool something = ec.number_this_disk == 1 && ec.num_disk_start_cdir == 1;
	if (!something && ec.number_this_disk != 0)
		RageException::Throw( "Couldn't open %s: zipfile is part of multi-disk archive", zip.GetPath().c_str() );

	/* Seek to where the start of central directory should be, and make
	 * sure it's there. */
	zip.Seek( ec.offset_start_central_directory );

	CString sig;
	int got = zip.Read( sig, 4 );
	if( got != 4 || sig != central_hdr_sig )
		RageException::Throw( "Couldn't open %s: start of central directory not found; zipfile corrupt", zip.GetPath().c_str() );

	zip.Seek( ec.offset_start_central_directory );

	/* Loop through files in central directory. */
    while(1)
    {
		CString sig;
		if ( zip.Read( sig, 4 ) != 4 )
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
					LOG->Warn( "%s: expected end central file header signature not found", zip.GetPath().c_str() );
			} else {
				LOG->Warn( "%s: expected central file header signature not found", zip.GetPath().c_str() );
			}
			break;
		}

		FileInfo info;
		info.data_offset = -1;
		int got = ProcessCdirFileHdr(zip, info);
		if( got == -1 ) /* error */
			break;
		if( got == 0 ) /* skip */
			continue;

		zip.SeekCur( info.extra_field_length );

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
int RageFileDriverZip::ProcessCdirFileHdr( RageFile &zip, FileInfo &info )
{
	/* Read the next central directory entry and do any necessary machine-type
	 * conversions (byte ordering, structure padding compensation--do so by
	 * copying the data from the array into which it was read (byterec) to the
	 * usable struct. */
    cdir_byte_hdr byterec;
	int got = zip.Read( (char *)byterec, CREC_SIZE );
    if ( got == -1 )
		RageException::Throw( "Couldn't open %s: %s", zip.GetPath().c_str(), zip.GetError().c_str() );
	if ( got != CREC_SIZE )
	{
		LOG->Warn( "%s: unexpected EOF", zip.GetPath().c_str() );
        return -1;
	}

//    crec.version_made_by[0] = byterec[C_VERSION_MADE_BY_0];
//    crec.version_made_by[1] = byterec[C_VERSION_MADE_BY_1];
    const int version_needed_to_extract = byterec[C_VERSION_NEEDED_TO_EXTRACT_0];
	if( version_needed_to_extract > 20 ) /* compatible with PKUNZIP 2.0 */
	{
		LOG->Warn( "File \"%s\" in \"%s\" uses unsupported ZIP version %i.%i",
			info.fn.c_str(), zip.GetRealPath().c_str(), 
			version_needed_to_extract / 10, version_needed_to_extract % 10 );
		return 0;
	}

    const int general_purpose_bit_flag = makeword(&byterec[C_GENERAL_PURPOSE_BIT_FLAG]);
	if( general_purpose_bit_flag & 1 )
	{
		LOG->Warn( "Skipped encrypted \"%s\" in \"%s\"",
			info.fn.c_str(), zip.GetRealPath().c_str() );
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

	got = zip.Read( info.fn, info.filename_length );
	if( got == -1 )
		RageException::Throw( "Couldn't open %s: %s", zip.GetPath().c_str(), zip.GetError().c_str() );
	if( got != info.filename_length )
	{
		LOG->Warn( "%s: bad filename length %li", zip.GetPath().c_str(), info.filename_length );
		return 0;
	}

    if( info.compression_method != STORED && info.compression_method != DEFLATED )
	{
		LOG->Warn( "File \"%s\" in \"%s\" uses unsupported compression method %i",
			info.fn.c_str(), zip.GetRealPath().c_str(), info.compression_method );

		return 0;
	}

    return 1;
}

RageFileDriverZip::~RageFileDriverZip()
{
	for( unsigned i = 0; i < Files.size(); ++i )
		delete Files[i];
}

RageFileObj *RageFileDriverZip::Open( const CString &path, int mode, RageFile &p, int &err )
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

	/* If we havn't figured out the offset to the real data yet, do so now. */
	if( info->data_offset == -1 )
	{
		zip.Seek( info->offset );

		/* Should be in proper position now, so check for sig. */
		CString sig;
		int got = zip.Read( sig, 4 );
		if( got == -1 )
			RageException::Throw( "Read error in %s: %s", zip.GetPath().c_str(), zip.GetError().c_str() );
		if( got != 4 )
			RageException::Throw( "%s: unexpected EOF", zip.GetPath().c_str() );

		if( sig != local_hdr_sig )
			RageException::Throw( "%s: bad zipfile offset", zip.GetPath().c_str() );

#define LREC_SIZE   26   /* lengths of local file headers, central */
#define L_FILENAME_LENGTH                 22
#define L_EXTRA_FIELD_LENGTH              24
		unsigned char byterec[LREC_SIZE];
		got = zip.Read( (char *)byterec, LREC_SIZE );
		if( got == -1 )
			RageException::Throw( "Read error in %s: %s", zip.GetPath().c_str(), zip.GetError().c_str() );
		if( got != LREC_SIZE )
			RageException::Throw( "%s: unexpected EOF", zip.GetPath().c_str() );

		const int filename_length = makeword(&byterec[L_FILENAME_LENGTH]);
		const int extra_field_length = makeword(&byterec[L_EXTRA_FIELD_LENGTH]);
		info->data_offset = zip.Tell() + filename_length + extra_field_length;
	}

    zip.Seek( info->data_offset );

	switch( info->compression_method )
	{
	case STORED:
		return new RageFileObjZipStored( zip, *info, p );
	case DEFLATED:
		return new RageFileObjZipDeflated( zip, *info, p );
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

/* We make a copy of the RageFile: multiple files may read from the same ZIP at once;
 * this way, we don't have to keep seeking around. */
RageFileObjZipDeflated::RageFileObjZipDeflated( const RageFile &f, const FileInfo &info_, RageFile &p ):
	RageFileObj( p ),
	info(info_),
	zip( f )
{
	decomp_buf_avail = 0;

    dstrm.zalloc = Z_NULL;
    dstrm.zfree = Z_NULL;

    int err = inflateInit2( &dstrm, -MAX_WBITS );
    if( err == Z_MEM_ERROR )
		RageException::Throw( "inflateInit2( %i ): out of memory", -MAX_WBITS );
    if( err != Z_OK )
		LOG->Trace( "Huh? inflateInit2() err = %i", err );

	decomp_buf_ptr = decomp_buf;
	CFilePos = UFilePos = 0;
}

RageFileObjZipDeflated::RageFileObjZipDeflated( const RageFileObjZipDeflated &cpy, RageFile &p ):
	RageFileObj( p ),
	info( cpy.info ),
	zip( cpy.zip )
{
	/* XXX completely untested */
	/* Copy the entire decode state. */
	/* inflateInit2 isn't widespread yet */
	ASSERT( 0 );
/*
	inflateCopy( &dstrm, const_cast<z_streamp>(&cpy.dstrm) );

	decomp_buf_ptr = decomp_buf + (cpy.decomp_buf_ptr - cpy.decomp_buf);
	decomp_buf_avail = cpy.decomp_buf_avail;
	CFilePos = cpy.CFilePos;
	UFilePos = cpy.UFilePos;
	*/
}


RageFileObjZipDeflated::~RageFileObjZipDeflated()
{
	//We MUST use inflateEnd() here insted of inflateReset().  Use
	//of inflateReset() will cause large quantities of globally 
	//allocated ram to be leaked, making it impervious to most leak 
	//detection. See zlib.h documentation on inflateReset() for 
	//more information.
	int err = inflateEnd( &dstrm );
	if( err != Z_OK )
		LOG->Trace( "Huh? inflateEnd() err = %i", err );
}

int RageFileObjZipDeflated::Read( void *buf, size_t bytes )
{
	bool done=false;
	int ret = 0;
	while( bytes && CFilePos < (int) info.compr_size && !done )
	{
		if ( !decomp_buf_avail )
		{
			decomp_buf_ptr = decomp_buf;
			decomp_buf_avail = 0;
			int got = zip.Read( decomp_buf, sizeof(decomp_buf) );
			if( got == -1 )
			{
				SetError( zip.GetError() );
				return -1;
			}
			if( got == 0 )
				break;

			decomp_buf_avail = got;
		}

		dstrm.next_in = (Bytef *) decomp_buf_ptr;
		dstrm.avail_in = decomp_buf_avail;
		dstrm.next_out = (Bytef *) buf;
		dstrm.avail_out = bytes;


		int err = inflate(&dstrm, Z_PARTIAL_FLUSH);
		switch( err )
		{
		case Z_DATA_ERROR:
			SetError( "Data error" );
			return -1;
		case Z_MEM_ERROR:
			SetError( "out of memory" );
			return -1;
		case Z_STREAM_END:
			done = true;
			break;
		case Z_OK:
			break;
		default:
			LOG->Trace( "Huh? inflate err %i", err );
		}

		const int used = (char *)dstrm.next_in - decomp_buf_ptr;
		CFilePos += used;
		decomp_buf_ptr += used;
		decomp_buf_avail -= used;

		const int got = (char *)dstrm.next_out - (char *)buf;
		UFilePos += got;
		ret += got;
		buf = (char *)buf + got;
		bytes -= got;
	}

	return ret;
}

void RageFileObjZipDeflated::Rewind()
{
	inflateReset( &dstrm );
	decomp_buf_ptr = decomp_buf;
	decomp_buf_avail = 0;

    zip.Seek( info.data_offset );
	CFilePos = 0;
	UFilePos = 0;
}

int RageFileObjZipDeflated::Seek( int offset )
{
	/* Optimization: if offset is the end of the file, it's a lseek(0,SEEK_END).  Don't
	 * decode anything. */
	if( offset == (int) info.uncompr_size )
	{
		UFilePos = info.uncompr_size;
		CFilePos = info.compr_size;
		zip.Seek( info.data_offset + info.compr_size );
		decomp_buf_ptr = decomp_buf;
		decomp_buf_avail = 0;
		inflateReset( &dstrm );
		return offset;
	}

	/* Can this be optimized? */
	return RageFileObj::Seek( offset );
}

RageFileObjZipStored::RageFileObjZipStored( const RageFile &f, const FileInfo &info_, RageFile &p ):
	RageFileObj( p ),
	info(info_),
	zip( f )
{
	FilePos = 0;
}

int RageFileObjZipStored::Read( void *buf, size_t bytes )
{
	const int bytes_left = info.compr_size-this->FilePos;
	const int got = zip.Read( buf, min( (int) bytes, bytes_left ) );
	if( got == -1 )
	{
		SetError( zip.GetError() );
		return -1;
	}

	FilePos += got;

	return got;
}


int RageFileObjZipStored::Seek( int offset )
{
	ASSERT( offset >= 0 );
	offset = min( (unsigned) offset, info.compr_size );

	int ret = zip.Seek( info.data_offset + offset );
	if( ret == -1 )
	{
		SetError( zip.GetError() );
		return -1;
	}
	ret -= info.data_offset;
	ASSERT( ret >= 0 );
	FilePos = ret;

	return ret;
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

