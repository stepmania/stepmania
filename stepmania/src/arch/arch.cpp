/*
 * This file provides functions to create driver objects.
 */

#include "global.h"
#include "RageLog.h"
#include "RageUtil.h"

#include "PrefsManager.h"
#include "arch.h"
#include "arch_platform.h"

LoadingWindow *MakeLoadingWindow()
{
#if defined(XBOX)
	return new LoadingWindow_Xbox;
#endif

	if( !PREFSMAN->m_bShowLoadingWindow )
		return new LoadingWindow_Null;

	/* Don't load NULL by default.  On most systems, if we can't load the SDL
	 * loading window, we won't be able to init OpenGL, either, so don't bother. */
	CString drivers = "win32,cocoa,gtk,sdl";
	CStringArray DriversToTry;
	split(drivers, ",", DriversToTry, true);

	ASSERT( DriversToTry.size() != 0 );

	CString Driver;
	LoadingWindow *ret = NULL;

	for(unsigned i = 0; ret == NULL && i < DriversToTry.size(); ++i)
	{
		Driver = DriversToTry[i];

#ifdef HAVE_LOADING_WINDOW_WIN32
		if(!DriversToTry[i].CompareNoCase("Win32")) ret = new LoadingWindow_Win32;
#endif
#ifdef HAVE_LOADING_WINDOW_GTK
		if(!DriversToTry[i].CompareNoCase("Gtk")) ret = new LoadingWindow_Gtk;
#endif
#ifdef HAVE_LOADING_WINDOW_COCOA
		if(!DriversToTry[i].CompareNoCase("Cocoa")) ret = new LoadingWindow_Cocoa;
#endif
#ifdef HAVE_LOADING_WINDOW_SDL
		if(!DriversToTry[i].CompareNoCase("SDL")) ret = new LoadingWindow_SDL;
#endif
#ifdef HAVE_LOADING_WINDOW_NULL
		if(!DriversToTry[i].CompareNoCase("Null")) ret = new LoadingWindow_Null;
#endif
			
		if( ret == NULL )
			continue;

		CString sError = ret->Init();
		if( sError != "" )
		{
			LOG->Info("Couldn't load driver %s: %s", DriversToTry[i].c_str(), sError.c_str());
			SAFE_DELETE( ret );
		}
	}
	
	if(ret)
		LOG->Info("Loading window: %s", Driver.c_str());
	
	return ret;
}

#if defined(SUPPORT_OPENGL)
LowLevelWindow *MakeLowLevelWindow() { return new ARCH_LOW_LEVEL_WINDOW; }
#endif

void MakeInputHandlers(CString drivers, vector<InputHandler *> &Add)
{
	CStringArray DriversToTry;
	split(drivers, ",", DriversToTry, true);

	ASSERT( DriversToTry.size() != 0 );

	CString Driver;
	RageSoundDriver *ret = NULL;

	for(unsigned i = 0; ret == NULL && i < DriversToTry.size(); ++i)
	{
#if defined(_WINDOWS)
		if(!DriversToTry[i].CompareNoCase("DirectInput"))	Add.push_back(new InputHandler_DInput);
		if(!DriversToTry[i].CompareNoCase("Pump"))			Add.push_back(new InputHandler_Win32_Pump);
	//	if(!DriversToTry[i].CompareNoCase("Para"))			Add.push_back(new InputHandler_Win32_Para);
#elif defined(XBOX)
		if(!DriversToTry[i].CompareNoCase("Xbox"))			Add.push_back(new InputHandler_Xbox);
#endif

#if defined(SUPPORT_SDL_INPUT)
		if(!DriversToTry[i].CompareNoCase("SDL"))			Add.push_back(new InputHandler_SDL);
#endif

		if(!DriversToTry[i].CompareNoCase("MonkeyKeyboard"))Add.push_back(new InputHandler_MonkeyKeyboard);
	}
}

RageSoundDriver *MakeRageSoundDriver(CString drivers)
{
	CStringArray DriversToTry;
	split(drivers, ",", DriversToTry, true);

	ASSERT( DriversToTry.size() != 0 );

	CString Driver;
	RageSoundDriver *ret = NULL;

	for(unsigned i = 0; ret == NULL && i < DriversToTry.size(); ++i)
	{
		try {
			Driver = DriversToTry[i];
			LOG->Trace("Initializing driver: %s", DriversToTry[i].c_str());

#ifdef _WINDOWS
			if(!DriversToTry[i].CompareNoCase("DirectSound"))		ret = new RageSound_DSound;
			if(!DriversToTry[i].CompareNoCase("DirectSound-sw"))	ret = new RageSound_DSound_Software;
			if(!DriversToTry[i].CompareNoCase("WaveOut"))			ret = new RageSound_WaveOut;
#endif
#ifdef _XBOX
			if(!DriversToTry[i].CompareNoCase("DirectSound"))		ret = new RageSound_DSound;
#endif
#ifdef HAVE_ALSA
			if(!DriversToTry[i].CompareNoCase("ALSA"))				ret = new RageSound_ALSA9;
			if(!DriversToTry[i].CompareNoCase("ALSA-sw"))			ret = new RageSound_ALSA9_Software;
#endif		
#ifdef HAVE_OSS
			if(!DriversToTry[i].CompareNoCase("OSS"))				ret = new RageSound_OSS;
#endif
#ifdef RAGE_SOUND_CA
			if(!DriversToTry[i].CompareNoCase("CoreAudio"))			ret = new RageSound_CA;
#endif
#ifdef RAGE_SOUND_QT1
			if(!DriversToTry[i].CompareNoCase("QT1"))				ret = new RageSound_QT1;
#endif
			if(!DriversToTry[i].CompareNoCase("Null"))				ret = new RageSound_Null;

			if( ret == NULL )
			{
				LOG->Warn( "Unknown sound driver name: %s", DriversToTry[i].c_str() );
				continue;
			}

			CString sError = ret->Init();
			if( sError != "" )
			{
				LOG->Info( "Couldn't load driver %s: %s", DriversToTry[i].c_str(), sError.c_str() );
				SAFE_DELETE( ret );
			}
		} catch(const RageException &e) {
			LOG->Info("Couldn't load driver %s: %s", DriversToTry[i].c_str(), e.what());
		}
	}
	
	if(ret)
		LOG->Info("Sound driver: %s", Driver.c_str());
	
	return ret;
}

/*
 * (c) 2002-2004 Glenn Maynard
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
