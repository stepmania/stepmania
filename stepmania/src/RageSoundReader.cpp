#include "global.h"
#include "RageSoundReader.h"
#include "RageUtil.h"
#include "RageLog.h"

#ifndef NO_WAV_SUPPORT
#include "RageSoundReader_WAV.h"
#endif

#ifndef NO_MP3_SUPPORT
#include "RageSoundReader_MP3.h"
#endif

#ifndef NO_VORBIS_SUPPORT
#include "RageSoundReader_Vorbisfile.h"
#endif


SoundReader *SoundReader::OpenFile( CString filename, CString &error )
{
    SoundReader_FileReader *NewSample = NULL;

	/* XXX: First try based on extension.  If that fails, try all other decoders.
	 * Optimally, we should have separate error returns indicating "this isn't
	 * an XXX file" vs. "this is an XXX file, but we don't support something in
	 * it", for better error string returns. */
#ifndef NO_WAV_SUPPORT
	if( !GetExtension(filename).CompareNoCase("wav") )
		NewSample = new RageSoundReader_WAV;
#endif
#ifndef NO_MP3_SUPPORT
	if( !GetExtension(filename).CompareNoCase("mp3") )
		NewSample = new RageSoundReader_MP3;
#endif

#ifndef NO_VORBIS_SUPPORT
	if( !GetExtension(filename).CompareNoCase("ogg") )
		NewSample = new RageSoundReader_Vorbisfile;
#endif

	ASSERT( NewSample );

	if( !NewSample->Open(filename) )
	{
		error = NewSample->GetError();
		return NULL;
	}

	return NewSample;
}
