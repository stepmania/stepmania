#include "global.h"
#include "MovieTexture.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "MovieTexture_Null.h"
#include "PrefsManager.h"

/* _WINDOWS is Windows only, where _WIN32 is Windows and Xbox, I think. Does this
 * work on the Xbox? -glenn */
#if defined(_WINDOWS)
#include "MovieTexture_DShow.h"
#define HAVE_FFMPEG
#endif

/* I don't like this. It should not be here. */
#if defined (DARWIN)
#define HAVE_FFMPEG
#endif

#ifdef HAVE_FFMPEG
#include "MovieTexture_FFMpeg.h"
#endif

#include "RageFile.h"
bool RageMovieTexture::GetFourCC( CString fn, CString &handler, CString &type )
{
	CString ignore, ext;
	splitpath( fn, ignore, ignore, ext);
	if( !ext.CompareNoCase(".mpg") ||
		!ext.CompareNoCase(".mpeg") ||
		!ext.CompareNoCase(".mpv") ||
		!ext.CompareNoCase(".mpe") )
	{
		handler = type = "MPEG";
		return true;
	}

	ifstream f;
	f.exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

	try {
		f.open(fn);

		f.seekg( 0x70, ios_base::beg );
		type = "    ";
		f.read((char *) type.c_str(), 4);
		int i;
		for( i = 0; i < 4; ++i)
			if(type[i] < 0x20 || type[i] > 0x7E) type[i] = '?';

		f.seekg( 0xBC, ios_base::beg );

		handler = "    ";
		f.read((char *) handler.c_str(), 4);
		for(i = 0; i < 4; ++i)
			if(handler[i] < 0x20 || handler[i] > 0x7E) handler[i] = '?';
	} catch(ifstream::failure e) {
		LOG->Warn("error on %s: %s", fn.c_str(), e.what() );
		handler = type = "";
		return false;
	}

	return true;
}

static void DumpAVIDebugInfo( CString fn )
{
	CString type, handler;
	if( !RageMovieTexture::GetFourCC( fn, handler, type ) )
		return;

	LOG->Info("Movie %s has handler '%s', type '%s'", fn.c_str(), handler.c_str(), type.c_str());
}

/* Try drivers in order of preference until we find one that works. */
RageMovieTexture *MakeRageMovieTexture(RageTextureID ID)
{
	DumpAVIDebugInfo( ID.filename );

	CStringArray DriversToTry;
	split(PREFSMAN->m_sMovieDrivers, ",", DriversToTry, true);
	ASSERT(DriversToTry.size() != 0);

	CString Driver;
	RageMovieTexture *ret = NULL;

	for (unsigned i=0; ret==NULL && i<DriversToTry.size(); ++i) {
		try {
			Driver = DriversToTry[i];
			LOG->Trace("Initializing driver: %s", Driver.c_str());
#ifdef _WINDOWS
			if (!Driver.CompareNoCase("DShow")) ret = new MovieTexture_DShow(ID);
#endif
#ifdef SUPPORT_MOVIETEXTURE_FFMPEG
			if (!Driver.CompareNoCase("FFMpeg")) ret = new MovieTexture_FFMpeg(ID);
#endif
			if (!Driver.CompareNoCase("Null")) ret = new MovieTexture_Null(ID);
			if (!ret)
				LOG->Warn("Unknown movie driver name: %s", Driver.c_str());
		} catch (const RageException &e) {
			LOG->Info("Couldn't load driver %s: %s", Driver.c_str(), e.what());
		}
	}
	if (!ret)
        RageException::Throw("Couldn't create a movie texture");

	LOG->Trace("Created movie texture \"%s\" with driver \"%s\"",
		ID.filename.c_str(), Driver.c_str() );
	return ret;
}

