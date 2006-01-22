/*
 * This file provides functions to create driver objects.
 */

#include "global.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "arch.h"
#include "arch_platform.h"
#include "Foreach.h"
#include "LocalizedString.h"
#include "arch/arch_default.h"

#include "InputHandler/Selector_InputHandler.h"
static LocalizedString INPUT_HANDLERS_EMPTY( "Arch", "Input Handlers cannot be empty." );
void MakeInputHandlers( CString drivers, vector<InputHandler *> &Add )
{
	vector<CString> DriversToTry;
	split( drivers, ",", DriversToTry, true );

	if( DriversToTry.empty() )
		RageException::Throw( INPUT_HANDLERS_EMPTY.GetValue() );

	CString Driver;

	FOREACH_CONST( CString, DriversToTry, s )
	{
		InputHandler *ret = NULL;

#ifdef USE_INPUT_HANDLER_DIRECTINPUT
		if( !s->CompareNoCase("DirectInput") )	ret = new InputHandler_DInput;
#endif
#ifdef USE_INPUT_HANDLER_LINUX_JOYSTICK
		if( !s->CompareNoCase("Joystick") )	ret = new InputHandler_Linux_Joystick;
#endif
#ifdef USE_INPUT_HANDLER_LINUX_TTY
		if( !s->CompareNoCase("tty") )		ret = new InputHandler_Linux_tty;
#endif
#ifdef USE_INPUT_HANDLER_SDL
		if( !s->CompareNoCase("SDL") )		ret = new InputHandler_SDL;
#endif
#ifdef USE_INPUT_HANDLER_WIN32_PARA
		if( !s->CompareNoCase("Para") )		ret = new InputHandler_Win32_Para;
#endif
#ifdef USE_INPUT_HANDLER_WIN32_PUMP
		if( !s->CompareNoCase("Pump") )		ret = new InputHandler_Win32_Pump;
#endif
#ifdef USE_INPUT_HANDLER_WIN32_MIDI
		if( !s->CompareNoCase("MIDI") )		ret = new InputHandler_Win32_MIDI;
#endif
#ifdef USE_INPUT_HANDLER_X11
		if( !s->CompareNoCase("X11") )		ret = new InputHandler_X11;
#endif
#ifdef USE_INPUT_HANDLER_XBOX
		if( !s->CompareNoCase("Xbox") )		ret = new InputHandler_Xbox;
#endif
#ifdef USE_INPUT_HANDLER_CARBON
		if( !s->CompareNoCase("Carbon") )	ret = new InputHandler_Carbon;
#endif

		if( ret == NULL )
			LOG->Trace( "Unknown Input Handler name: %s", s->c_str() );
		else
			Add.push_back( ret );
	}

	// Always add
	Add.push_back( new InputHandler_MonkeyKeyboard );

}

#include "Lights/Selector_LightsDriver.h"
void MakeLightsDrivers( CString driver, vector<LightsDriver *> &Add )
{
	LOG->Trace( "Initializing lights driver: %s", driver.c_str() );

	LightsDriver *ret = NULL;

#ifdef USE_LIGHTS_DRIVER_LINUX_PARALLEL
	if( !driver.CompareNoCase("LinuxParallel") )	ret = new LightsDriver_LinuxParallel;
#endif
#ifdef USE_LIGHTS_DRIVER_LINUX_WEEDTECH
	if( !driver.CompareNoCase("WeedTech") )		ret = new LightsDriver_LinuxWeedTech;
#endif
#ifdef USE_LIGHTS_DRIVER_WIN32_PARALLEL
	if( !driver.CompareNoCase("Parallel") )		ret = new LightsDriver_Win32Parallel;
#endif

	if( ret == NULL && driver.CompareNoCase("Null") )
		LOG->Trace( "Unknown lights driver name: %s", driver.c_str() );
	else if( ret != NULL )
		Add.push_back( ret );

	Add.push_back( new LightsDriver_SystemMessage );
}

#include "LoadingWindow/Selector_LoadingWindow.h"
LoadingWindow *MakeLoadingWindow()
{
	if( !PREFSMAN->m_bShowLoadingWindow )
		return new LoadingWindow_Null;

	/* Don't load NULL by default. */
	const CString drivers = "xbox,win32,cocoa,gtk";
	vector<CString> DriversToTry;
	split( drivers, ",", DriversToTry, true );

	ASSERT( DriversToTry.size() != 0 );

	CString Driver;
	LoadingWindow *ret = NULL;

	for( unsigned i = 0; ret == NULL && i < DriversToTry.size(); ++i )
	{
		Driver = DriversToTry[i];

#ifdef USE_LOADING_WINDOW_COCOA
		if( !DriversToTry[i].CompareNoCase("Cocoa") )	ret = new LoadingWindow_Cocoa;
#endif
#ifdef USE_LOADING_WINDOW_GTK
		if( !DriversToTry[i].CompareNoCase("Gtk") )	ret = new LoadingWindow_Gtk;
#endif
#ifdef USE_LOADING_WINDOW_NULL
		if( !DriversToTry[i].CompareNoCase("Null") )	ret = new LoadingWindow_Null;
#endif
#ifdef USE_LOADING_WINDOW_WIN32
		if( !DriversToTry[i].CompareNoCase("Win32") )	ret = new LoadingWindow_Win32;
#endif
#ifdef USE_LOADING_WINDOW_XBOX
		if( !DriversToTry[i].CompareNoCase("Xbox") )	ret = new LoadingWindow_Xbox;
#endif

			
		if( ret == NULL )
			continue;

		CString sError = ret->Init();
		if( sError != "" )
		{
			LOG->Info( "Couldn't load driver %s: %s", DriversToTry[i].c_str(), sError.c_str() );
			SAFE_DELETE( ret );
		}
	}
	
	if(ret)
		LOG->Info( "Loading window: %s", Driver.c_str() );
	
	return ret;
}

#if defined(SUPPORT_OPENGL)
#include "LowLevelWindow/Selector_LowLevelWindow.h"
LowLevelWindow *MakeLowLevelWindow()
{
	return new ARCH_LOW_LEVEL_WINDOW;
}
#endif

#include "MemoryCard/MemoryCardDriver_Null.h"
#include "MemoryCard/Selector_MemoryCardDriver.h"
MemoryCardDriver *MakeMemoryCardDriver()
{
	if( !PREFSMAN->m_bMemoryCards )
		return new MemoryCardDriver_Null;

	MemoryCardDriver *ret = NULL;

#ifdef ARCH_MEMORY_CARD_DRIVER
	ret = new ARCH_MEMORY_CARD_DRIVER;
#endif

	if( !ret )
		ret = new MemoryCardDriver_Null;
	
	return ret;
}

static Preference<CString> g_sMovieDrivers( "MovieDrivers", "" ); // "" == default
#include "MovieTexture/Selector_MovieTexture.h"
static void DumpAVIDebugInfo( const CString& fn );
/* Try drivers in order of preference until we find one that works. */
static LocalizedString MOVIE_DRIVERS_EMPTY		( "Arch", "Movie Drivers cannot be empty." );
static LocalizedString COULDNT_CREATE_MOVIE_DRIVER	( "Arch", "Couldn't create a movie driver." );
RageMovieTexture *MakeRageMovieTexture( RageTextureID ID )
{
	DumpAVIDebugInfo( ID.filename );

	RString sDrivers = g_sMovieDrivers;
	if( sDrivers.empty() )
		sDrivers = DEFAULT_MOVIE_DRIVER_LIST;

	vector<CString> DriversToTry;
	split( sDrivers, ",", DriversToTry, true );
	
	if( DriversToTry.empty() )
		RageException::Throw( MOVIE_DRIVERS_EMPTY.GetValue() );

	CString Driver;
	RageMovieTexture *ret = NULL;

	for( unsigned i=0; ret==NULL && i<DriversToTry.size(); ++i )
	{
		Driver = DriversToTry[i];
		LOG->Trace( "Initializing driver: %s", Driver.c_str() );
#ifdef USE_MOVIE_TEXTURE_THEORA
		if( !Driver.CompareNoCase("Theora") ) ret = new MovieTexture_Theora(ID);
#endif
#ifdef USE_MOVIE_TEXTURE_DSHOW
		if( !Driver.CompareNoCase("DShow") ) ret = new MovieTexture_DShow(ID);
#endif
#ifdef USE_MOVIE_TEXTURE_FFMPEG
		if( !Driver.CompareNoCase("FFMpeg") ) ret = new MovieTexture_FFMpeg(ID);
#endif
#ifdef USE_MOVIE_TEXTURE_NULL
		if( !Driver.CompareNoCase("Null") ) ret = new MovieTexture_Null(ID);
#endif
		if( ret == NULL )
		{
			LOG->Trace( "Unknown movie driver name: %s", Driver.c_str() );
			continue;
		}

		CString sError = ret->Init();
		if( sError != "" )
		{
			LOG->Info( "Couldn't load driver %s: %s", Driver.c_str(), sError.c_str() );
			SAFE_DELETE( ret );
		}
	}
	if (!ret)
		RageException::Throw( COULDNT_CREATE_MOVIE_DRIVER.GetValue() );

	LOG->Trace( "Created movie texture \"%s\" with driver \"%s\"",
		    ID.filename.c_str(), Driver.c_str() );
	return ret;
}

#include "Sound/Selector_RageSoundDriver.h"
static LocalizedString SOUND_DRIVERS_CANNOT_EMPTY( "Arch", "Sound Drivers cannot be empty." );
RageSoundDriver *MakeRageSoundDriver( CString drivers )
{
	vector<CString> DriversToTry;
	split( drivers, ",", DriversToTry, true );

	if( DriversToTry.empty() )
		RageException::Throw( SOUND_DRIVERS_CANNOT_EMPTY.GetValue() );

	CString Driver;
	RageSoundDriver *ret = NULL;

	for( unsigned i = 0; ret == NULL && i < DriversToTry.size(); ++i )
	{
		Driver = DriversToTry[i];
		LOG->Trace( "Initializing driver: %s", DriversToTry[i].c_str() );

#ifdef USE_RAGE_SOUND_ALSA9
		if( !DriversToTry[i].CompareNoCase("ALSA") )		ret = new RageSound_ALSA9;
#endif
#ifdef USE_RAGE_SOUND_ALSA9_SOFTWARE
		if( !DriversToTry[i].CompareNoCase("ALSA-sw") )		ret = new RageSound_ALSA9_Software;
#endif
#ifdef USE_RAGE_SOUND_CA
		if( !DriversToTry[i].CompareNoCase("CoreAudio") )	ret = new RageSound_CA;
#endif
#ifdef USE_RAGE_SOUND_DSOUND
		if( !DriversToTry[i].CompareNoCase("DirectSound") )	ret = new RageSound_DSound;
#endif
#ifdef USE_RAGE_SOUND_DSOUND_SOFTWARE
		if( !DriversToTry[i].CompareNoCase("DirectSound-sw") )	ret = new RageSound_DSound_Software;
#endif
#ifdef USE_RAGE_SOUND_NULL
		if( !DriversToTry[i].CompareNoCase("Null") )		ret = new RageSound_Null;
#endif
#ifdef USE_RAGE_SOUND_OSS
		if( !DriversToTry[i].CompareNoCase("OSS") )		ret = new RageSound_OSS;
#endif
#ifdef USE_RAGE_SOUND_WAVE_OUT
		if( !DriversToTry[i].CompareNoCase("WaveOut") )		ret = new RageSound_WaveOut;
#endif

		if( ret == NULL )
		{
			LOG->Trace( "Unknown sound driver name: %s", DriversToTry[i].c_str() );
			continue;
		}

		CString sError = ret->Init();
		if( sError != "" )
		{
			LOG->Info( "Couldn't load driver %s: %s", DriversToTry[i].c_str(), sError.c_str() );
			SAFE_DELETE( ret );
		}
	}
	
	if(ret)
		LOG->Info( "Sound driver: %s", Driver.c_str() );
	
	return ret;
}

// Helper for MakeRageMovieTexture()
static void DumpAVIDebugInfo( const CString& fn )
{
	CString type, handler;
	if( !RageMovieTexture::GetFourCC( fn, handler, type ) )
		return;

	LOG->Trace( "Movie %s has handler '%s', type '%s'", fn.c_str(), handler.c_str(), type.c_str() );
}

/*
 * (c) 2002-2005 Glenn Maynard, Ben Anderson
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
