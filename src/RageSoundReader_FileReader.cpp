#include "global.h"
#include "RageSoundReader_FileReader.h"
#include "RageFile.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "ActorUtil.h"

#include <set>
#if defined(HAS_WAV)
#include "RageSoundReader_WAV.h"
#endif

#if defined(HAS_MP3)
#include "RageSoundReader_MP3.h"
#endif

#if defined(HAS_OGG)
#include "RageSoundReader_Vorbisfile.h"
#endif

RageSoundReader_FileReader *RageSoundReader_FileReader::TryOpenFile( RageFileBasic *pFile, RString &error, RString format, bool &bKeepTrying )
{
	RageSoundReader_FileReader *Sample = NULL;

#if defined(HAS_WAV)
	if( !format.CompareNoCase("wav") )
		Sample = new RageSoundReader_WAV;
#endif

#if defined(HAS_MP3)
	if( !format.CompareNoCase("mp3") )
		Sample = new RageSoundReader_MP3;
#endif

#if defined(HAS_OGG)
	if( !format.CompareNoCase("oga") || !format.CompareNoCase("ogg") )
		Sample = new RageSoundReader_Vorbisfile;
#endif

	if( !Sample )
		return NULL;

	OpenResult ret = Sample->Open( pFile );
	pFile = NULL; // Sample owns it now
	if( ret == OPEN_OK )
		return Sample;

	RString err = Sample->GetRSRError();
	delete Sample;

	LOG->Trace( "Format %s failed: %s", format.c_str(), err.c_str() );

	/*
	 * The file failed to open, or failed to read.  This indicates a problem that will
	 * affect all readers, so don't waste time trying more readers. (OPEN_IO_ERROR)
	 *
	 * Errors fall in two categories:
	 * OPEN_UNKNOWN_FILE_FORMAT: Data was successfully read from the file, but it's the
	 * wrong file format.  The error message always looks like "unknown file format" or
	 * "Not Vorbis data"; ignore it so we always give a consistent error message, and
	 * continue trying other file formats.
	 *
	 * OPEN_FATAL_ERROR: Either the file was opened successfully and appears to be the
	 * correct format, but a fatal format-specific error was encountered that will probably
	 * not be fixed by using a different reader (for example, an Ogg file that doesn't
	 * actually contain any audio streams); or the file failed to open or read ("I/O
	 * error", "permission denied"), in which case all other readers will probably fail,
	 * too.  The returned error is used, and no other formats will be tried.
	 */
	bKeepTrying = (ret != OPEN_FATAL_ERROR);
	switch( ret )
	{
		case OPEN_UNKNOWN_FILE_FORMAT:
			bKeepTrying = true;
			error = "Unknown file format";
			break;

		case OPEN_FATAL_ERROR:
			/* The file matched, but failed to load.  We know it's this type of data;
			 * don't bother trying the other file types. */
			bKeepTrying = false;
			error = err;
			break;
		default: break;
	}

	return NULL;
}

#include "RageFileDriverMemory.h"

RageSoundReader_FileReader *RageSoundReader_FileReader::OpenFile( RString filename, RString &error, bool *pPrebuffer )
{
	HiddenPtr<RageFileBasic> pFile;
	{
		RageFile *pFileOpen = new RageFile;
		if( !pFileOpen->Open(filename) )
		{
			error = pFileOpen->GetError();
			delete pFileOpen;
			return NULL;
		}
		pFile = pFileOpen;
	}

	if( pPrebuffer )
	{
		if( pFile->GetFileSize() < 1024*50 )
		{
			RageFileObjMem *pMem = new RageFileObjMem;
			bool bRet = FileCopy( *pFile, *pMem, error, NULL );
			if( !bRet )
			{
				delete pMem;
				return NULL;
			}

			pFile = pMem;
			pFile->Seek( 0 );
			*pPrebuffer = true;
		}
		else
		{
			*pPrebuffer = false;
		}
	}
	set<RString> FileTypes;
	vector<RString> const& sound_exts= ActorUtil::GetTypeExtensionList(FT_Sound);
	for(vector<RString>::const_iterator curr= sound_exts.begin();
			curr != sound_exts.end(); ++curr)
	{
		FileTypes.insert(*curr);
	}

	RString format = GetExtension( filename );
	format.MakeLower();

	error = "";

	bool bKeepTrying = true;

	/* If the extension matches a format, try that first. */
	if( FileTypes.find(format) != FileTypes.end() )
	{
		RageSoundReader_FileReader *NewSample = TryOpenFile( pFile->Copy(), error, format, bKeepTrying );
		if( NewSample )
			return NewSample;
		FileTypes.erase( format );
	}

	for( set<RString>::iterator it = FileTypes.begin(); bKeepTrying && it != FileTypes.end(); ++it )
	{
		RageSoundReader_FileReader *NewSample = TryOpenFile( pFile->Copy(), error, *it, bKeepTrying );
		if( NewSample )
		{
			LOG->UserLog( "Sound file", pFile->GetDisplayPath(), "is really %s.", it->c_str() );
			return NewSample;
		}
	}

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
