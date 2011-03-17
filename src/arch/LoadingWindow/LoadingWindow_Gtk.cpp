#include "global.h"
#include "RageLog.h"
#include "RageFileManager.h"
#include "RageUtil.h"
#include "LoadingWindow_Gtk.h"
#include "LoadingWindow_GtkModule.h"

#include <dlfcn.h>

static void *Handle = NULL;
static INIT Module_Init;
static SHUTDOWN Module_Shutdown;
static SETTEXT Module_SetText;

LoadingWindow_Gtk::LoadingWindow_Gtk()
{
}

RString LoadingWindow_Gtk::Init()
{
	ASSERT( Handle == NULL );
	
	Handle = dlopen( RageFileManagerUtil::sDirOfExecutable + "/" + "GtkModule.so", RTLD_NOW );
	if( Handle == NULL )
		return ssprintf( "dlopen(): %s", dlerror() );

	Module_Init = (INIT) dlsym(Handle, "Init");
	if( !Module_Init )
		return "Couldn't load symbol Module_Init";
	Module_Shutdown = (SHUTDOWN) dlsym(Handle, "Shutdown");
	if( !Module_Shutdown )
		return "Couldn't load symbol Module_Shutdown";
	Module_SetText = (SETTEXT) dlsym(Handle, "SetText");
	if( !Module_SetText )
		return "Couldn't load symbol Module_SetText";

	const char *ret = Module_Init( &g_argc, &g_argv );
	if( ret != NULL )
		return ret;
	return "";
}

LoadingWindow_Gtk::~LoadingWindow_Gtk()
{
	if( Module_Shutdown != NULL )
		Module_Shutdown();
	Module_Shutdown = NULL;

	if( Handle )
		dlclose( Handle );
	Handle = NULL;
}

void LoadingWindow_Gtk::SetText( RString s )
{
	Module_SetText( s );
}

/*
 * (c) 2003-2004 Glenn Maynard, Sean Burke
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
