#include "global.h"
#include "RageSoundReader_FileReader.h"
#include "RageUtil.h"
#include "RageLog.h"

#include <set>
#ifndef NO_WAV_SUPPORT
#include "RageSoundReader_WAV.h"
#endif

#ifndef NO_MP3_SUPPORT
#include "RageSoundReader_MP3.h"
#endif

#ifndef NO_VORBIS_SUPPORT
#include "RageSoundReader_Vorbisfile.h"
#endif

SoundReader_FileReader *SoundReader_FileReader::TryOpenFile( CString filename, CString &error, CString format )
{
	SoundReader_FileReader *Sample = NULL;

#ifndef NO_WAV_SUPPORT
	if( !format.CompareNoCase("wav") )
		Sample = new RageSoundReader_WAV;
#endif
#ifndef NO_MP3_SUPPORT
	if( !format.CompareNoCase("mp3") )
		Sample = new RageSoundReader_MP3;
#endif

#ifndef NO_VORBIS_SUPPORT
	if( !format.CompareNoCase("ogg") )
		Sample = new RageSoundReader_Vorbisfile;
#endif

	if( !Sample )
		return NULL;

	OpenResult ret = Sample->Open(filename);
	if( ret == OPEN_OK )
		return Sample;

	CString err = Sample->GetError();
	delete Sample;

	LOG->Trace( "Format %s failed: %s", format.c_str(), err.c_str() );

	/* If OPEN_MATCH_BUT_FAIL, the error is important; otherwise it's probably
		* just a "unknown file"-ish error. */
	if( ret == OPEN_MATCH_BUT_FAIL )
	{
		if( error != "" )
			error += "; ";
		error += ssprintf("%s: %s", format.c_str(), err.c_str() );
	}

	return NULL;
}

SoundReader *SoundReader_FileReader::OpenFile( CString filename, CString &error )
{
	set<CString> FileTypes;
	FileTypes.insert("ogg");
	FileTypes.insert("mp3");
	FileTypes.insert("wav");

	CString format = GetExtension(filename);
	format.MakeLower();

	error = "";

	/* If the extension matches a format, try that first. */
	if( FileTypes.find(format) != FileTypes.end() )
	{
	    SoundReader_FileReader *NewSample = TryOpenFile( filename, error, format );
		if( NewSample )
			return NewSample;
		FileTypes.erase( format );
	}

	for( set<CString>::iterator it = FileTypes.begin(); it != FileTypes.end(); ++it )
	{
	    SoundReader_FileReader *NewSample = TryOpenFile( filename, error, *it );
		if( NewSample )
		{
			LOG->Warn("File \"%s\" is really %s", filename.c_str(), it->c_str());
			return NewSample;
		}
	}

	return NULL;
}
