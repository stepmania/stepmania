#include "global.h"
#include "MovieTexture.h"
#include "RageUtil.h"
#include "RageLog.h"

#if defined(WIN32)
#include "MovieTexture_DShow.h"
#endif

#include <fstream>
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
/* Well, eventually; for now it's DShow or bust.w */
RageMovieTexture *MakeRageMovieTexture(RageTextureID ID)
{
	DumpAVIDebugInfo( ID.filename );

#if defined(WIN32)
	return new MovieTexture_DShow(ID);
#else
	/* XXX: need a simple null movie texture object */
	RageException::Throw("xxx");
#endif
}

