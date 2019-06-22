#include "global.h"

#include <stdio.h>

#if defined(_WINDOWS)
#include <tchar.h>
#else
#define _tcslen strlen
#define _tcscpy strcpy
typedef char TCHAR;
#endif

#include <time.h>
#include "CreateZip.h"
#include "RageFile.h"
#include "RageUtil.h"

#define MAX_PATH 1024

// TODO: remove header "extra fields" (the 3 dates)
// Adapted for StepMania from http://www.codeproject.com/KB/files/zip_utils.aspx
//
// THIS FILE is almost entirely based upon code by info-zip.
// It has been modified by Lucian Wischik. The modifications
// were a complete rewrite of the bit of code that generates the
// layout of the zipfile, and support for zipping to/from memory
// or handles or pipes or pagefile or diskfiles, encryption, unicode.
// The original code may be found at http://www.info-zip.org
// The original copyright text follows.
//
//
//
// This is version 1999-Oct-05 of the Info-ZIP copyright and license.
// The definitive version of this document should be available at
// ftp://ftp.cdrom.com/pub/infozip/license.html indefinitely.
//
// Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.
//
// For the purposes of this copyright and license, "Info-ZIP" is defined as
// the following set of individuals:
//
//   Mark Adler, John Bush, Karl Davis, Harald Denker, Jean-Michel Dubois,
//   Jean-loup Gailly, Hunter Goatley, Ian Gorman, Chris Herborth, Dirk Haase,
//   Greg Hartwig, Robert Heath, Jonathan Hudson, Paul Kienitz, David Kirschbaum,
//   Johnny Lee, Onno van der Linden, Igor Mandrichenko, Steve P. Miller,
//   Sergio Monesi, Keith Owens, George Petrov, Greg Roelofs, Kai Uwe Rommel,
//   Steve Salisbury, Dave Smith, Christian Spieler, Antoine Verheijen,
//   Paul von Behren, Rich Wales, Mike White
//
// This software is provided "as is," without warranty of any kind, express
// or implied.  In no event shall Info-ZIP or its contributors be held liable
// for any direct, indirect, incidental, special or consequential damages
// arising out of the use of or inability to use this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//    1. Redistributions of source code must retain the above copyright notice,
//       definition, disclaimer, and this list of conditions.
//
//    2. Redistributions in binary form must reproduce the above copyright
//       notice, definition, disclaimer, and this list of conditions in
//       documentation and/or other materials provided with the distribution.
//
//    3. Altered versions--including, but not limited to, ports to new operating
//       systems, existing ports with new graphical interfaces, and dynamic,
//       shared, or static library versions--must be plainly marked as such
//       and must not be misrepresented as being the original source.  Such
//       altered versions also must not be misrepresented as being Info-ZIP
//       releases--including, but not limited to, labeling of the altered
//       versions with the names "Info-ZIP" (or any variation thereof, including,
//       but not limited to, different capitalizations), "Pocket UnZip," "WiZ"
//       or "MacZip" without the explicit permission of Info-ZIP.  Such altered
//       versions are further prohibited from misrepresentative use of the
//       Zip-Bugs or Info-ZIP e-mail addresses or of the Info-ZIP URL(s).
//
//    4. Info-ZIP retains the right to use the names "Info-ZIP," "Zip," "UnZip,"
//       "WiZ," "Pocket UnZip," "Pocket Zip," and "MacZip" for its own source and
//       binary releases.
//



// These are the result codes:
#define ZR_OK         0x00000000     // nb. the pseudo-code zr-recent is never returned,
#define ZR_RECENT     0x00000001     // but can be passed to FormatZipMessage.
// The following come from general system stuff (e.g. files not openable)
#define ZR_GENMASK    0x0000FF00
#define ZR_NODUPH     0x00000100     // couldn't duplicate the handle
#define ZR_NOFILE     0x00000200     // couldn't create/open the file
#define ZR_NOALLOC    0x00000300     // failed to allocate some resource
#define ZR_WRITE      0x00000400     // a general error writing to the file
#define ZR_NOTFOUND   0x00000500     // couldn't find that file in the zip
#define ZR_MORE       0x00000600     // there's still more data to be unzipped
#define ZR_CORRUPT    0x00000700     // the zipfile is corrupt or not a zipfile
#define ZR_READ       0x00000800     // a general error reading the file
// The following come from mistakes on the part of the caller
#define ZR_CALLERMASK 0x00FF0000
#define ZR_ARGS       0x00010000     // general mistake with the arguments
#define ZR_MEMSIZE    0x00030000     // the memory size is too small
#define ZR_FAILED     0x00040000     // the thing was already failed when you called this function
#define ZR_ENDED      0x00050000     // the zip creation has already been closed
#define ZR_MISSIZE    0x00060000     // the indicated input file size turned out mistaken
#define ZR_PARTIALUNZ 0x00070000     // the file had already been partially unzipped
#define ZR_ZMODE      0x00080000     // tried to mix creating/opening a zip
// The following come from bugs within the zip library itself
#define ZR_BUGMASK    0xFF000000
#define ZR_NOTINITED  0x01000000     // initialisation didn't work
#define ZR_SEEK       0x02000000     // trying to seek in an unseekable file
#define ZR_NOCHANGE   0x04000000     // changed its mind on storage, but not allowed
#define ZR_FLATE      0x05000000     // an internal error in the de/inflation code

	RString FormatZipMessageZ(ZRESULT code)
	{
		const char *msg="unknown zip result code";
		switch (code)
		{
		case ZR_OK: msg="Success"; break;
		case ZR_NODUPH: msg="Couldn't duplicate handle"; break;
		case ZR_NOFILE: msg="Couldn't create/open file"; break;
		case ZR_NOALLOC: msg="Failed to allocate memory"; break;
		case ZR_WRITE: msg="Error writing to file"; break;
		case ZR_NOTFOUND: msg="File not found in the zipfile"; break;
		case ZR_MORE: msg="Still more data to unzip"; break;
		case ZR_CORRUPT: msg="Zipfile is corrupt or not a zipfile"; break;
		case ZR_READ: msg="Error reading file"; break;
		case ZR_ARGS: msg="Caller: faulty arguments"; break;
		case ZR_PARTIALUNZ: msg="Caller: the file had already been partially unzipped"; break;
		case ZR_MEMSIZE: msg="Caller: not enough space allocated for memory zipfile"; break;
		case ZR_FAILED: msg="Caller: there was a previous error"; break;
		case ZR_ENDED: msg="Caller: additions to the zip have already been ended"; break;
		case ZR_ZMODE: msg="Caller: mixing creation and opening of zip"; break;
		case ZR_NOTINITED: msg="Zip-bug: internal initialisation not completed"; break;
		case ZR_SEEK: msg="Zip-bug: trying to seek the unseekable"; break;
		case ZR_MISSIZE: msg="Zip-bug: the anticipated size turned out wrong"; break;
		case ZR_NOCHANGE: msg="Zip-bug: tried to change mind, but not allowed"; break;
		case ZR_FLATE: msg="Zip-bug: an internal error during flation"; break;
		}
		return msg;
	}


typedef unsigned char uch;      // unsigned 8-bit value
typedef unsigned short ush;     // unsigned 16-bit value
typedef unsigned long ulg;      // unsigned 32-bit value
typedef size_t extent;          // file size
typedef unsigned Pos;   // must be at least 32 bits
typedef unsigned IPos; // A Pos is an index in the character window. Pos is used only for parameter passing

#ifndef EOF
#define EOF (-1)
#endif


// Error return values.  The values 0..4 and 12..18 follow the conventions
// of PKZIP.   The values 4..10 are all assigned to "insufficient memory"
// by PKZIP, so the codes 5..10 are used here for other purposes.
#define ZE_MISS         -1      // used by procname(), zipbare()
#define ZE_OK           0       // success
#define ZE_EOF          2       // unexpected end of zip file
#define ZE_FORM         3       // zip file structure error
#define ZE_MEM          4       // out of memory
#define ZE_LOGIC        5       // internal logic error
#define ZE_BIG          6       // entry too large to split
#define ZE_NOTE         7       // invalid comment format
#define ZE_TEST         8       // zip test (-T) failed or out of memory
#define ZE_ABORT        9       // user interrupt or termination
#define ZE_TEMP         10      // error using a temp file
#define ZE_READ         11      // read or seek error
#define ZE_NONE         12      // nothing to do
#define ZE_NAME         13      // missing or empty zip file
#define ZE_WRITE        14      // error writing to a file
#define ZE_CREAT        15      // couldn't open to write
#define ZE_PARMS        16      // bad command line
#define ZE_OPEN         18      // could not open a specified file to read
#define ZE_MAXERR       18      // the highest error number


// internal file attribute
#define UNKNOWN (-1)
#define BINARY  0
#define ASCII   1

#define STORE 0                 // Store method

#define CRCVAL_INITIAL  0L

// MSDOS file or directory attributes
#define MSDOS_HIDDEN_ATTR 0x02
#define MSDOS_DIR_ATTR 0x10

// Lengths of headers after signatures in bytes
#define LOCHEAD 26
#define CENHEAD 42
#define ENDHEAD 18

// Definitions for extra field handling:
#define EB_HEADSIZE       4     /* length of a extra field block header */
#define EB_LEN            2     /* offset of data length field in header */
#define EB_UT_MINLEN      1     /* minimal UT field contains Flags byte */
#define EB_UT_FLAGS       0     /* byte offset of Flags field */
#define EB_UT_TIME1       1     /* byte offset of 1st time value */
#define EB_UT_FL_MTIME    (1 << 0)      /* mtime present */
#define EB_UT_FL_ATIME    (1 << 1)      /* atime present */
#define EB_UT_FL_CTIME    (1 << 2)      /* ctime present */
#define EB_UT_LEN(n)      (EB_UT_MINLEN + 4 * (n))
#define EB_L_UT_SIZE    (EB_HEADSIZE + EB_UT_LEN(3))
#define EB_C_UT_SIZE    (EB_HEADSIZE + EB_UT_LEN(1))


// Macros for writing machine integers to little-endian format
#define PUTSH(a,f) {char _putsh_c=(char)((a)&0xff); wfunc(param,&_putsh_c,1); _putsh_c=(char)((a)>>8); wfunc(param,&_putsh_c,1);}
#define PUTLG(a,f) {PUTSH((a) & 0xffff,(f)) PUTSH((a) >> 16,(f))}


// -- Structure of a ZIP file --
// Signatures for zip file information headers
#define LOCSIG     0x04034b50L
#define CENSIG     0x02014b50L
#define ENDSIG     0x06054b50L
#define EXTLOCSIG  0x08074b50L


#define MIN_MATCH  3
#define MAX_MATCH  258
// The minimum and maximum match lengths


#define WSIZE  (0x8000)
// Maximum window size = 32K. If you are really short of memory, compile
// with a smaller WSIZE but this reduces the compression ratio for files
// of size > WSIZE. WSIZE must be a power of two in the current implementation.
//

#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)
// Minimum amount of lookahead, except at the end of the input file.
// See deflate.c for comments about the MIN_MATCH+1.
//

#define MAX_DIST  (WSIZE-MIN_LOOKAHEAD)
// In order to simplify the code, particularly on 16 bit machines, match
// distances are limited to MAX_DIST instead of WSIZE.
//


#define ZIP_FILENAME 2
#define ZIP_FOLDER   4



// ===========================================================================
// Constants
//

#define MAX_BITS 15
// All codes must not exceed MAX_BITS bits

#define MAX_BL_BITS 7
// Bit length codes must not exceed MAX_BL_BITS bits

#define LENGTH_CODES 29
// number of length codes, not counting the special END_BLOCK code

#define LITERALS  256
// number of literal bytes 0..255

#define END_BLOCK 256
// end of block literal code

#define L_CODES (LITERALS+1+LENGTH_CODES)
// number of Literal or Length codes, including the END_BLOCK code

#define D_CODES   30
// number of distance codes

#define BL_CODES  19
// number of codes used to transfer the bit lengths


#define LIT_BUFSIZE  0x8000
#define DIST_BUFSIZE  LIT_BUFSIZE
// Sizes of match buffers for literals/lengths and distances.  There are
// 4 reasons for limiting LIT_BUFSIZE to 64K:
//   - frequencies can be kept in 16 bit counters
//   - if compression is not successful for the first block, all input data is
//     still in the window so we can still emit a stored block even when input
//     comes from standard input.  (This can also be done for all blocks if
//     LIT_BUFSIZE is not greater than 32K.)
//   - if compression is not successful for a file smaller than 64K, we can
//     even emit a stored file instead of a stored block (saving 5 bytes).
//   - creating new Huffman trees less frequently may not provide fast
//     adaptation to changes in the input data statistics. (Take for
//     example a binary file with poorly compressible code followed by
//     a highly compressible string table.) Smaller buffer sizes give
//     fast adaptation but have of course the overhead of transmitting trees
//     more frequently.
//   - I can't count above 4
// The current code is general and allows DIST_BUFSIZE < LIT_BUFSIZE (to save
// memory at the expense of compression). Some optimizations would be possible
// if we rely on DIST_BUFSIZE == LIT_BUFSIZE.
//

#define REP_3_6      16
// repeat previous bit length 3-6 times (2 bits of repeat count)

#define REPZ_3_10    17
// repeat a zero length 3-10 times  (3 bits of repeat count)

#define REPZ_11_138  18
// repeat a zero length 11-138 times  (7 bits of repeat count)

#define HEAP_SIZE (2*L_CODES+1)
// maximum heap size


// ===========================================================================
// Local data used by the "bit string" routines.
//

#define Buf_size (8 * 2*sizeof(char))
// Number of bits used within bi_buf. (bi_buf may be implemented on
// more than 16 bits on some systems.)

// Output a 16 bit value to the bit stream, lower (oldest) byte first
#define PUTSHORT(state,w) \
{ if (state.bs.out_offset >= state.bs.out_size-1) \
	state.flush_outbuf(state.param,state.bs.out_buf, &state.bs.out_offset); \
	state.bs.out_buf[state.bs.out_offset++] = (char) ((w) & 0xff); \
	state.bs.out_buf[state.bs.out_offset++] = (char) ((ush)(w) >> 8); \
}

#define PUTBYTE(state,b) \
{ if (state.bs.out_offset >= state.bs.out_size) \
	state.flush_outbuf(state.param,state.bs.out_buf, &state.bs.out_offset); \
	state.bs.out_buf[state.bs.out_offset++] = (char) (b); \
}



typedef int64_t lutime_t;       // define it ourselves since we don't include time.h

typedef struct iztimes {
	lutime_t atime,mtime,ctime;
} iztimes; // access, modify, create times

typedef struct zlist {
	ush vem, ver, flg, how;       // See central header in zipfile.c for what vem..off are
	ulg tim, crc, siz, len;
	size_t nam, ext, cext, com;   // offset of ext must be >= LOCHEAD
	ush dsk, att, lflg;           // offset of lflg must be >= LOCHEAD
	ulg atx, off;
	char name[MAX_PATH];          // File name in zip file
	char *extra;                  // Extra field (set only if ext != 0)
	char *cextra;                 // Extra in central (set only if cext != 0)
	char *comment;                // Comment (set only if com != 0)
	char iname[MAX_PATH];         // Internal file name after cleanup
	char zname[MAX_PATH];         // External version of internal name
	int mark;                     // Marker for files to operate on
	int trash;                    // Marker for files to delete
	int dosflag;                  // Set to force MSDOS file attributes
	struct zlist *nxt;        // Pointer to next header in list
} TZipFileInfo;





/*
void __cdecl Trace(const char *x, ...) {va_list paramList; va_start(paramList, x); paramList; va_end(paramList);}
void __cdecl Tracec(bool ,const char *x, ...) {va_list paramList; va_start(paramList, x); paramList; va_end(paramList);}
*/



struct TState;
typedef unsigned (*READFUNC)(TState &state, char *buf,unsigned size);
typedef unsigned (*FLUSHFUNC)(void *param, const char *buf, unsigned *size);
typedef unsigned (*WRITEFUNC)(void *param, const char *buf, unsigned size);
struct TState
{
	void *param;
	int level; bool seekable;
	READFUNC readfunc; FLUSHFUNC flush_outbuf;
	const char *err;
};



int putlocal(struct zlist *z, WRITEFUNC wfunc,void *param)
{
	// Write a local header described by *z to file *f.  Return a ZE_ error code.
	PUTLG(LOCSIG, f);
	PUTSH(z->ver, f);
	PUTSH(z->lflg, f);
	PUTSH(z->how, f);
	PUTLG(z->tim, f);
	PUTLG(z->crc, f);
	PUTLG(z->siz, f);
	PUTLG(z->len, f);
	PUTSH(z->nam, f);
	PUTSH(z->ext, f);
	size_t res = (size_t)wfunc(param, z->iname, (unsigned int)z->nam);
	if (res!=z->nam)
		return ZE_TEMP;
	if (z->ext)
	{
		res = (size_t)wfunc(param, z->extra, (unsigned int)z->ext);
		if (res!=z->ext)
			return ZE_TEMP;
	}
	return ZE_OK;
}

int putextended(struct zlist *z, WRITEFUNC wfunc, void *param)
{
	// Write an extended local header described by *z to file *f. Returns a ZE_ code
	PUTLG(EXTLOCSIG, f);
	PUTLG(z->crc, f);
	PUTLG(z->siz, f);
	PUTLG(z->len, f);
	return ZE_OK;
}

int putcentral(struct zlist *z, WRITEFUNC wfunc, void *param)
{
	// Write a central header entry of *z to file *f. Returns a ZE_ code.
	PUTLG(CENSIG, f);
	PUTSH(z->vem, f);
	PUTSH(z->ver, f);
	PUTSH(z->flg, f);
	PUTSH(z->how, f);
	PUTLG(z->tim, f);
	PUTLG(z->crc, f);
	PUTLG(z->siz, f);
	PUTLG(z->len, f);
	PUTSH(z->nam, f);
	PUTSH(z->cext, f);
	PUTSH(z->com, f);
	PUTSH(z->dsk, f);
	PUTSH(z->att, f);
	PUTLG(z->atx, f);
	PUTLG(z->off, f);
	if ((size_t)wfunc(param, z->iname, (unsigned int)z->nam) != z->nam ||
		(z->cext && (size_t)wfunc(param, z->cextra, (unsigned int)z->cext) != z->cext) ||
		(z->com && (size_t)wfunc(param, z->comment, (unsigned int)z->com) != z->com))
		return ZE_TEMP;
	return ZE_OK;
}


int putend(int n, ulg s, ulg c, size_t m, char *z, WRITEFUNC wfunc, void *param)
{
	// write the end of the central-directory-data to file *f.
	PUTLG(ENDSIG, f);
	PUTSH(0, f);
	PUTSH(0, f);
	PUTSH(n, f);
	PUTSH(n, f);
	PUTLG(s, f);
	PUTLG(c, f);
	PUTSH(m, f);
	// Write the comment, if any
	if (m && wfunc(param, z, (unsigned int)m) != m) return ZE_TEMP;
	return ZE_OK;
}





const ulg crc_table[256] = {
	0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
		0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
		0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
		0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
		0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
		0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
		0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
		0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
		0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
		0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
		0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
		0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
		0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
		0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
		0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
		0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
		0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
		0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
		0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
		0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
		0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
		0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
		0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
		0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
		0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
		0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
		0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
		0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
		0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
		0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
		0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
		0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
		0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
		0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
		0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
		0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
		0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
		0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
		0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
		0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
		0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
		0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
		0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
		0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
		0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
		0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
		0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
		0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
		0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
		0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
		0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
		0x2d02ef8dL
};

#define CRC32(c, b) (crc_table[((int)(c) ^ (b)) & 0xff] ^ ((c) >> 8))
#define DO1(buf)  crc = CRC32(crc, *buf++)
#define DO2(buf)  DO1(buf); DO1(buf)
#define DO4(buf)  DO2(buf); DO2(buf)
#define DO8(buf)  DO4(buf); DO4(buf)

ulg crc32(ulg crc, const uch *buf, size_t len)
{
	if (buf== nullptr) return 0L;
	crc = crc ^ 0xffffffffL;
	while (len >= 8) {DO8(buf); len -= 8;}
	if (len) do {DO1(buf);} while (--len);
	return crc ^ 0xffffffffL;  // (instead of ~c for 64-bit machines)
}


class TZip
{
public:
	TZip() : pfout(nullptr),ooffset(0),oerr(false),writ(0),hasputcen(false),zfis(0),hfin(0)
	{
	}
	~TZip()
	{
	}

	// These variables say about the file we're writing into
	// We can write to pipe, file-by-handle, file-by-name, memory-to-memmapfile
	RageFile *pfout;             // if valid, we'll write here (for files or pipes)
	unsigned ooffset;         // for pfout, this is where the pointer was initially
	ZRESULT oerr;             // did a write operation give rise to an error?
	unsigned writ;            // how have we written. This is maintained by Add, not write(), to avoid confusion over seeks
	unsigned int opos;        // current pos in the mmap
	unsigned int mapsize;     // the size of the map we created
	bool hasputcen;           // have we yet placed the central directory?
	//
	TZipFileInfo *zfis;       // each file gets added onto this list, for writing the table at the end

	ZRESULT Start(RageFile *f);
	static unsigned sflush(void *param,const char *buf, unsigned *size);
	static unsigned swrite(void *param,const char *buf, unsigned size);
	unsigned int write(const char *buf,unsigned int size);
	bool oseek(unsigned int pos);
	ZRESULT Close();

	// some variables to do with the file currently being read:
	// I haven't done it object-orientedly here, just put them all
	// together, since OO didn't seem to make the design any clearer.
	ulg attr; iztimes times; ulg timestamp;  // all open_* methods set these
	long isize,ired;         // size is not set until close() on pips
	ulg crc;                                 // crc is not set until close(). iwrit is cumulative
	RageFile *hfin;           // for input files and pipes
	const char *bufin; unsigned int lenin,posin; // for memory
	// and a variable for what we've done with the input: (i.e. compressed it!)
	ulg csize;                               // compressed size, set by the compression routines
	// and this is used by some of the compression routines
	char buf[16384];


	ZRESULT open_file(const TCHAR *fn);
	ZRESULT open_dir();
	ZRESULT set_times();
	unsigned read(char *buf, unsigned size);
	ZRESULT iclose();

	ZRESULT ideflate(TZipFileInfo *zfi);
	ZRESULT istore();

	ZRESULT Add(const TCHAR *odstzn, const TCHAR *src, unsigned long flags);
	ZRESULT AddCentral();

};



ZRESULT TZip::Start(RageFile *f)
{
	if (pfout!=0 || writ!=0 || oerr!=ZR_OK || hasputcen)
		return ZR_NOTINITED;
	//
	pfout = f;
	ooffset=0;
	return ZR_OK;
}

unsigned TZip::sflush(void *param,const char *buf, unsigned *size)
{
	// static
	if (*size==0)
		return 0;
	TZip *zip = (TZip*)param;
	unsigned int writ = zip->write(buf,*size);
	if (writ!=0)
		*size=0;
	return writ;
}
unsigned TZip::swrite(void *param,const char *buf, unsigned size)
{
	// static
	if (size==0)
		return 0;
	TZip *zip=(TZip*)param;
	return zip->write(buf,size);
}
unsigned int TZip::write(const char *buf,unsigned int size)
{
	const char *srcbuf=buf;
	if (pfout != nullptr)
	{
		unsigned long writ = pfout->Write( srcbuf, size );
		return writ;
	}
	oerr=ZR_NOTINITED;
	return 0;
}

bool TZip::oseek(unsigned int pos)
{
	oerr=ZR_SEEK;
	return false;
}

ZRESULT TZip::Close()
{
	// if the directory hadn't already been added through a call to GetMemory,
	// then we do it now
	ZRESULT res=ZR_OK;
	if (!hasputcen)
		res=AddCentral();
	hasputcen=true;
	pfout->Close();
	return res;
}



#define ZIP_ATTR_READONLY 0x01
#define ZIP_ATTR_HIDDEN 0x02
#define ZIP_ATTR_SYSTEM 0x04
#define ZIP_ATTR_DIRECTORY 0x10
#define ZIP_ATTR_ARCHIVE 0x20
#define ZIP_ATTR_DIRECTORY2 0x40000000
#define ZIP_ATTR_NORMAL_FILE 0x80000000
#define ZIP_ATTR_READABLE 0x01000000
#define ZIP_ATTR_WRITEABLE 0x00800000
#define ZIP_ATTR_EXECUTABLE 0x00400000

ZRESULT TZip::open_file(const TCHAR *fn)
{
	hfin=0; bufin=0; crc=CRCVAL_INITIAL; isize=0; csize=0; ired=0;
	if (fn==0)
		return ZR_ARGS;
	hfin = new RageFile();
	if( !hfin->Open(fn) )
	{
		SAFE_DELETE( hfin );
		return ZR_NOFILE;
	}
	isize = hfin->GetFileSize();
	attr= ZIP_ATTR_NORMAL_FILE | ZIP_ATTR_READABLE | ZIP_ATTR_WRITEABLE;
	return set_times();
}

ZRESULT TZip::open_dir()
{
	hfin=0; bufin=0; crc=CRCVAL_INITIAL; isize=0; csize=0; ired=0;
	attr= ZIP_ATTR_DIRECTORY2 | ZIP_ATTR_READABLE | ZIP_ATTR_WRITEABLE | ZIP_ATTR_DIRECTORY;
	isize = 0;
	return set_times();
}


void filetime2dosdatetime(const tm st, unsigned short *dosdate,unsigned short *dostime)
{
	// date: bits 0-4 are day of month 1-31. Bits 5-8 are month 1..12. Bits 9-15 are year-1980
	// time: bits 0-4 are seconds/2, bits 5-10 are minute 0..59. Bits 11-15 are hour 0..23
	*dosdate = (unsigned short)(((st.tm_year+1900-1980)&0x7f) << 9);
	*dosdate |= (unsigned short)(((st.tm_mon+1)&0xf) << 5);
	*dosdate |= (unsigned short)((st.tm_mday&0x1f));
	*dostime = (unsigned short)((st.tm_hour&0x1f) << 11);
	*dostime |= (unsigned short)((st.tm_min&0x3f) << 5);
	*dostime |= (unsigned short)((st.tm_sec*2)&0x1f);
}

ZRESULT TZip::set_times()
{
	time_t rawtime;
	tm *ptm;
	time ( &rawtime );
	ptm = localtime ( &rawtime );

	unsigned short dosdate,dostime;
	filetime2dosdatetime(*ptm,&dosdate,&dostime);
	times.atime = time(nullptr);
	times.mtime = times.atime;
	times.ctime = times.atime;
	timestamp = (unsigned short)dostime | (((unsigned long)dosdate)<<16);
	return ZR_OK;
}

unsigned TZip::read(char *buf, unsigned size)
{
	if (bufin!=0)
	{
		if (posin>=lenin) return 0; // end of input
		ulg red = lenin-posin;
		if (red>size)
			red=size;
		memcpy(buf, bufin+posin, red);
		posin += red;
		ired += red;
		crc = crc32(crc, (uch*)buf, red);
		return red;
	}
	else if (hfin!=0)
	{
		int red = hfin->Read(buf,size);
		if (red <= 0)
			return 0;
		ired += red;
		crc = crc32(crc, (uch*)buf, red);
		return red;
	}
	else
	{
		oerr=ZR_NOTINITED;
		return 0;
	}
}

ZRESULT TZip::iclose()
{
	if (hfin!=0)
		SAFE_DELETE( hfin);
	bool mismatch = (isize!=-1 && isize!=ired);
	isize=ired; // and crc has been being updated anyway
	if (mismatch)
		return ZR_MISSIZE;
	else
		return ZR_OK;
}


ZRESULT TZip::istore()
{
	ulg size=0;
	for (;;)
	{
		unsigned int cin=read(buf,16384);
		if (cin<=0 || cin==(unsigned int)EOF)
			break;
		unsigned int cout = write(buf,cin);
		if (cout!=cin)
			return ZR_MISSIZE;
		size += cin;
	}
	csize=size;
	return ZR_OK;
}





bool has_seeded=false;
ZRESULT TZip::Add(const TCHAR *odstzn, const TCHAR *src,unsigned long flags)
{
	if (oerr)
		return ZR_FAILED;
	if (hasputcen)
		return ZR_ENDED;

	// zip has its own notion of what its names should look like: i.e. dir/file.stuff
	TCHAR dstzn[MAX_PATH]; _tcscpy(dstzn,odstzn);
	if (*dstzn==0)
		return ZR_ARGS;
	TCHAR *d=dstzn;
	while (*d!=0)
	{
		if (*d=='\\')
			*d='/';
		d++;
	}
	bool isdir = (flags==ZIP_FOLDER);
	bool needs_trailing_slash = (isdir && dstzn[_tcslen(dstzn)-1]!='/');
	int method=STORE;

	// now open whatever was our input source:
	ZRESULT openres;
	if (flags==ZIP_FILENAME)
		openres=open_file(src);
	else if (flags==ZIP_FOLDER)
		openres=open_dir();
	else
		return ZR_ARGS;
	if (openres!=ZR_OK)
		return openres;

	// A zip "entry" consists of a local header (which includes the file name),
	// then the compressed data, and possibly an extended local header.

	// Initialize the local header
	TZipFileInfo zfi; zfi.nxt=nullptr;
	strcpy(zfi.name,"");
#ifdef UNICODE
	WideCharToMultiByte(CP_UTF8,0,dstzn,-1,zfi.iname,MAX_PATH,0,0);
#else
	strcpy(zfi.iname,dstzn);
#endif
	zfi.nam=strlen(zfi.iname);
	if (needs_trailing_slash)
	{
		strcat(zfi.iname,"/");
		zfi.nam++;
	}
	strcpy(zfi.zname,"");
	zfi.extra=nullptr; zfi.ext=0;   // extra header to go after this compressed data, and its length
	zfi.cextra=nullptr; zfi.cext=0; // extra header to go in the central end-of-zip directory, and its length
	zfi.comment=nullptr; zfi.com=0; // comment, and its length
	zfi.mark = 1;
	zfi.dosflag = 0;
	zfi.att = (ush)BINARY;
	zfi.vem = (ush)0xB17; // 0xB00 is win32 os-code. 0x17 is 23 in decimal: zip 2.3
	zfi.ver = (ush)20;    // Needs PKUNZIP 2.0 to unzip it
	zfi.tim = timestamp;
	// Even though we write the header now, it will have to be rewritten, since we don't know compressed size or crc.
	zfi.crc = 0;            // to be updated later
	zfi.flg = 8;            // 8 means 'there is an extra header'. Assume for the moment that we need it.
	zfi.lflg = zfi.flg;     // to be updated later
	zfi.how = (ush)method;  // to be updated later
	zfi.siz = (ulg)(method==STORE && isize>=0 ? isize : 0); // to be updated later
	zfi.len = (ulg)(isize);  // to be updated later
	zfi.dsk = 0;
	zfi.atx = attr;
	zfi.off = writ+ooffset;         // offset within file of the start of this local record
	// stuff the 'times' structure into zfi.extra

	// nb. apparently there's a problem with PocketPC CE(zip)->CE(unzip) fails. And removing the following block fixes it up.
	char xloc[EB_L_UT_SIZE]; zfi.extra=xloc;  zfi.ext=EB_L_UT_SIZE;
	char xcen[EB_C_UT_SIZE]; zfi.cextra=xcen; zfi.cext=EB_C_UT_SIZE;
	xloc[0]  = 'U';
	xloc[1]  = 'T';
	xloc[2]  = EB_UT_LEN(3);       // length of data part of e.f.
	xloc[3]  = 0;
	xloc[4]  = EB_UT_FL_MTIME | EB_UT_FL_ATIME | EB_UT_FL_CTIME;
	xloc[5]  = (char)(times.mtime);
	xloc[6]  = (char)(times.mtime >> 8);
	xloc[7]  = (char)(times.mtime >> 16);
	xloc[8]  = (char)(times.mtime >> 24);
	xloc[9]  = (char)(times.atime);
	xloc[10] = (char)(times.atime >> 8);
	xloc[11] = (char)(times.atime >> 16);
	xloc[12] = (char)(times.atime >> 24);
	xloc[13] = (char)(times.ctime);
	xloc[14] = (char)(times.ctime >> 8);
	xloc[15] = (char)(times.ctime >> 16);
	xloc[16] = (char)(times.ctime >> 24);
	memcpy(zfi.cextra,zfi.extra,EB_C_UT_SIZE);
	zfi.cextra[EB_LEN] = EB_UT_LEN(1);


	// (1) Start by writing the local header:
	int r = putlocal(&zfi,swrite,this);
	if (r!=ZE_OK)
	{
		iclose();
		return ZR_WRITE;
	}
	writ += 4 + LOCHEAD + (unsigned int)zfi.nam + (unsigned int)zfi.ext;
	if (oerr!=ZR_OK)
	{
		iclose();
		return oerr;
	}

	//(2) Write stored file to zip file
	ZRESULT writeres=ZR_OK;
	if (!isdir && method==STORE)
		writeres=istore();
	else if (isdir)
		csize=0;
	else
		FAIL_M("deflate removed");
	iclose();
	writ += csize;
	if (oerr!=ZR_OK)
		return oerr;
	if (writeres!=ZR_OK)
		return ZR_WRITE;

	// (3) Either rewrite the local header with correct information...
	bool first_header_has_size_right = (zfi.siz==csize);
	zfi.crc = crc;
	zfi.siz = csize;
	zfi.len = isize;
	// (4) ... or put an updated header at the end
	if (zfi.how != (ush) method)
		return ZR_NOCHANGE;
	if (method==STORE && !first_header_has_size_right)
		return ZR_NOCHANGE;
	if ((r = putextended(&zfi, swrite,this)) != ZE_OK)
		return ZR_WRITE;
	writ += 16L;
	zfi.flg = zfi.lflg; // if flg modified by inflate, for the central index

	if (oerr!=ZR_OK)
		return oerr;

	// Keep a copy of the zipfileinfo, for our end-of-zip directory
	char *cextra = new char[zfi.cext]; memcpy(cextra,zfi.cextra,zfi.cext); zfi.cextra=cextra;
	TZipFileInfo *pzfi = new TZipFileInfo; memcpy(pzfi,&zfi,sizeof(zfi));
	if (zfis==nullptr)
		zfis=pzfi;
	else
	{
		TZipFileInfo *z=zfis;
		while (z->nxt!=nullptr)
			z=z->nxt;
		z->nxt=pzfi;
	}
	return ZR_OK;
}

ZRESULT TZip::AddCentral()
{
	// write central directory
	int numentries = 0;
	ulg pos_at_start_of_central = writ;
	//ulg tot_unc_size=0, tot_compressed_size=0;
	bool okay=true;
	for (TZipFileInfo *zfi=zfis; zfi!= nullptr; )
	{
		if (okay)
		{
			int res = putcentral(zfi, swrite,this);
			if (res!=ZE_OK)
				okay=false;
		}
		writ += 4 + CENHEAD + (unsigned int)zfi->nam + (unsigned int)zfi->cext + (unsigned int)zfi->com;
		//tot_unc_size += zfi->len;
		//tot_compressed_size += zfi->siz;
		numentries++;
		//
		TZipFileInfo *zfinext = zfi->nxt;
		if (zfi->cextra!=0) delete[] zfi->cextra;
		delete zfi;
		zfi = zfinext;
	}
	ulg center_size = writ - pos_at_start_of_central;
	if (okay)
	{
		int res = putend(numentries, center_size, pos_at_start_of_central+ooffset, 0, nullptr, swrite,this);
		if (res!=ZE_OK)
			okay=false;
		writ += 4 + ENDHEAD + 0;
	}
	if (!okay)
		return ZR_WRITE;
	return ZR_OK;
}





ZRESULT lasterrorZ=ZR_OK;


CreateZip::CreateZip()
{
	hz=nullptr;
}

bool CreateZip::Start( RageFile *f)
{
	hz = new TZip();
	lasterrorZ = hz->Start(f);
	return lasterrorZ == ZR_OK;
}
RString MakeDestZipFileName( RString fn )
{
	// strip leading slash
	fn.erase( fn.begin(), fn.begin()+1 );
	return fn;
}
bool CreateZip::AddFile(RString fn)
{
	lasterrorZ = hz->Add(MakeDestZipFileName(fn),fn,ZIP_FILENAME);
	return lasterrorZ == ZR_OK;
}
bool CreateZip::AddDir(RString fn)
{
	lasterrorZ = hz->Add(MakeDestZipFileName(fn),nullptr,ZIP_FOLDER);
	return lasterrorZ == ZR_OK;
}
bool CreateZip::Finish()
{
	lasterrorZ = hz->Close();
	return lasterrorZ == ZR_OK;
}

RString CreateZip::GetError()
{
	return FormatZipMessageZ( lasterrorZ );
}

