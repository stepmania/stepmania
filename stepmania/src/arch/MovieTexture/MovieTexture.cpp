#include "global.h"
#include "MovieTexture.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "MovieTexture_Null.h"

/* Why is this _WINDOWS and not WIN32?
 * --steve */
#if defined(_WINDOWS)
#include "MovieTexture_DShow.h"
#define DEFAULT_MOVIE_DRIVER_LIST "DShow"
#else
#define DEFAULT_MOVIE_DRIVER_LIST "Null"
#endif

#ifdef HAVE_AVCODEC
#include "MovieTexture_AVCodec.h"
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
	split(DEFAULT_MOVIE_DRIVER_LIST, ",", DriversToTry, true);
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
#ifdef HAVE_AVCODEC
			if (!Driver.CompareNoCase("AVCodec")) ret = new MovieTexture_AVCodec(ID);
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
	return ret;
}

