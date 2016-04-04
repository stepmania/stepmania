#include "global.h"
#include "MovieTexture.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "MovieTexture_Null.h"
#include "PrefsManager.h"
#include "RageFile.h"
#include "LocalizedString.h"
#include "arch/arch_default.h"

using std::vector;

void ForceToAscii( std::string &str )
{
	for (auto &ch: str)
	{
		if (ch < 0x20 || ch > 0x7E)
		{
			ch = '?';
		}
	}
}

bool RageMovieTexture::GetFourCC( std::string fn, std::string &handler, std::string &type )
{
	std::string ignore, ext;
	splitpath( fn, ignore, ignore, ext);
	Rage::ci_ascii_string ciExt{ ext.c_str() };
	if (ciExt == ".mpg" || ciExt == ".mpeg" || ciExt == ".mpv" || ciExt == ".mpe")
	{
		handler = type = "MPEG";
		return true;
	}
	if (ciExt == ".ogv")
	{
		handler = type = "Ogg";
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

DriverList RageMovieTextureDriver::m_pDriverList;

// Helper for MakeRageMovieTexture()
static void DumpAVIDebugInfo( const std::string& fn )
{
	std::string type, handler;
	if( !RageMovieTexture::GetFourCC( fn, handler, type ) )
		return;

	LOG->Trace( "Movie %s has handler '%s', type '%s'", fn.c_str(), handler.c_str(), type.c_str() );
}

static Preference<std::string> g_sMovieDrivers( "MovieDrivers", "" ); // "" == default
/* Try drivers in order of preference until we find one that works. */
static LocalizedString MOVIE_DRIVERS_EMPTY		( "Arch", "Movie Drivers cannot be empty." );
static LocalizedString COULDNT_CREATE_MOVIE_DRIVER	( "Arch", "Couldn't create a movie driver." );
RageMovieTexture *RageMovieTexture::Create( RageTextureID ID )
{
	DumpAVIDebugInfo( ID.filename );

	std::string sDrivers = g_sMovieDrivers.Get();
	if( sDrivers.empty() )
	{
		sDrivers = DEFAULT_MOVIE_DRIVER_LIST;
	}
	auto DriversToTry = Rage::split( sDrivers, ",", Rage::EmptyEntries::skip );

	if( DriversToTry.empty() )
	{
		RageException::Throw( "%s", MOVIE_DRIVERS_EMPTY.GetValue().c_str() );
	}
	RageMovieTexture *ret = nullptr;

	for (auto const &Driver: DriversToTry)
	{
		LOG->Trace( "Initializing driver: %s", Driver.c_str() );
		RageDriver *pDriverBase = RageMovieTextureDriver::m_pDriverList.Create( Driver );

		if( pDriverBase == nullptr )
		{
			LOG->Trace( "Unknown movie driver name: %s", Driver.c_str() );
			continue;
		}

		RageMovieTextureDriver *pDriver = dynamic_cast<RageMovieTextureDriver *>( pDriverBase );
		ASSERT( pDriver != nullptr );

		std::string sError;
		ret = pDriver->Create( ID, sError );
		delete pDriver;

		if( ret == nullptr )
		{
			LOG->Trace( "Couldn't load driver %s: %s", Driver.c_str(), sError.c_str() );
			Rage::safe_delete( ret );
			continue;
		}
		LOG->Trace( "Created movie texture \"%s\" with driver \"%s\"",
			    ID.filename.c_str(), Driver.c_str() );
		break;
	}
	if ( !ret )
	{
		RageException::Throw( "%s", COULDNT_CREATE_MOVIE_DRIVER.GetValue().c_str() );
	}
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
