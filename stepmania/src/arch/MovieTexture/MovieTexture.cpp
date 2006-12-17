#include "global.h"
#include "MovieTexture.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "MovieTexture_Null.h"
#include "PrefsManager.h"
#include "RageFile.h"
#include "LocalizedString.h"
#include "Foreach.h"
#include "arch/arch_default.h"

void ForceToAscii( RString &str )
{
	for( unsigned i=0; i<str.size(); ++i )
		if( str[i] < 0x20 || str[i] > 0x7E )
			str[i] = '?';
}

bool RageMovieTexture::GetFourCC( RString fn, RString &handler, RString &type )
{
	RString ignore, ext;
	splitpath( fn, ignore, ignore, ext);
	if( !ext.CompareNoCase(".mpg") ||
		!ext.CompareNoCase(".mpeg") ||
		!ext.CompareNoCase(".mpv") ||
		!ext.CompareNoCase(".mpe") )
	{
		handler = type = "MPEG";
		return true;
	}

	//Not very pretty but should do all the same error checking without iostream
#define HANDLE_ERROR(x) { \
		LOG->Warn( "Error reading %s: %s", fn.c_str(), x ); \
		handler = type = ""; \
		return false; \
	}

	RageFile file;
	if( !file.Open(fn) )
		HANDLE_ERROR("Could not open file.");
	if( !file.Seek(0x70) )
		HANDLE_ERROR("Could not seek.");
	type = "    ";
	if( file.Read((char *)type.c_str(), 4) != 4 )
		HANDLE_ERROR("Could not read.");
	ForceToAscii( type );
	
	if( file.Seek(0xBC) != 0xBC )
		HANDLE_ERROR("Could not seek.");
	handler = "    ";
	if( file.Read((char *)handler.c_str(), 4) != 4 )
		HANDLE_ERROR("Could not read.");
	ForceToAscii( handler );

	return true;
#undef HANDLE_ERROR
}

static DriverList g_pRegistrees;
RegisterMovieTextureDriver::RegisterMovieTextureDriver( const istring &sName, CreateRageDriverFn pfn )
{
	g_pRegistrees.Add( sName, pfn );
}

// Helper for MakeRageMovieTexture()
static void DumpAVIDebugInfo( const RString& fn )
{
	RString type, handler;
	if( !RageMovieTexture::GetFourCC( fn, handler, type ) )
		return;
	
	LOG->Trace( "Movie %s has handler '%s', type '%s'", fn.c_str(), handler.c_str(), type.c_str() );
}

static Preference<RString> g_sMovieDrivers( "MovieDrivers", "" ); // "" == default
/* Try drivers in order of preference until we find one that works. */
static LocalizedString MOVIE_DRIVERS_EMPTY		( "Arch", "Movie Drivers cannot be empty." );
static LocalizedString COULDNT_CREATE_MOVIE_DRIVER	( "Arch", "Couldn't create a movie driver." );
RageMovieTexture *MakeRageMovieTexture( RageTextureID ID )
{
	DumpAVIDebugInfo( ID.filename );
	
	RString sDrivers = g_sMovieDrivers;
	if( sDrivers.empty() )
		sDrivers = DEFAULT_MOVIE_DRIVER_LIST;
	
	vector<RString> DriversToTry;
	split( sDrivers, ",", DriversToTry, true );
	
	if( DriversToTry.empty() )
		RageException::Throw( "%s", MOVIE_DRIVERS_EMPTY.GetValue().c_str() );
	
	RageMovieTexture *ret = NULL;
	
	FOREACH_CONST( RString, DriversToTry, Driver )
	{
		LOG->Trace( "Initializing driver: %s", Driver->c_str() );
		RageDriver *pDriverBase = g_pRegistrees.Create( *Driver );
		
		if( pDriverBase == NULL )
		{
			LOG->Trace( "Unknown movie driver name: %s", Driver->c_str() );
			continue;
		}
		
		RageMovieTextureDriver *pDriver = dynamic_cast<RageMovieTextureDriver *>( pDriverBase );
		ASSERT( pDriver );

		RString sError;
		ret = pDriver->Create( ID, sError );
		delete pDriver;

		if( ret == NULL )
		{
			LOG->Info( "Couldn't load driver %s: %s", Driver->c_str(), sError.c_str() );
			SAFE_DELETE( ret );
			continue;
		}
		LOG->Trace( "Created movie texture \"%s\" with driver \"%s\"",
			    ID.filename.c_str(), Driver->c_str() );
		break;
	}
	if ( !ret )
		RageException::Throw( "%s", COULDNT_CREATE_MOVIE_DRIVER.GetValue().c_str() );
	
	return ret;
}


/*
 * (c) 2003-2004 Glenn Maynard
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
