#include "global.h"
#include "RageSoundReader.h"
#include "RageUtil.h"

#ifndef NO_WAV_SUPPORT
#include "RageSoundReader_WAV.h"
#endif

#if defined(OGG_ONLY)
#include "RageSoundReader_Vorbisfile.h"
#define RageSoundReader_LowLevel RageSoundReader_Vorbisfile
#else
#include "RageSoundReader_SDL_Sound.h"
#define RageSoundReader_LowLevel SoundReader_SDL_Sound
#endif


SoundReader *SoundReader::OpenFile( CString filename, CString error )
{
    SoundReader_FileReader *NewSample = NULL;

	/* First try based on extension.  If that fails, try all other decoders.
	 * Optimally, we should have separate error returns indicating "this isn't
	 * an XXX file" vs. "this is an XXX file, but we don't support something in
	 * it", for better error string returns.
	 */
#ifndef NO_WAV_SUPPORT
	if( !GetExtension(filename).CompareNoCase("wav") )
		NewSample = new RageSoundReader_WAV;
#endif

	if( NewSample == NULL )
		NewSample = new RageSoundReader_LowLevel;
	if( !NewSample->Open(filename) )
	{
		error = NewSample->GetError();
		return NULL;
	}

	return NewSample;
}
