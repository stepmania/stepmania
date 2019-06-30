#include "global.h"
#include "RageLog.h"
#include "RageFileManager.h"
#include "RageUtil.h"
#include "LoadingWindow_Gtk.h"
#include "LoadingWindow_GtkModule.h"

#include <dlfcn.h>

static void *Handle = nullptr;
static INIT Module_Init;
static SHUTDOWN Module_Shutdown;
static SETTEXT Module_SetText;
static SETICON Module_SetIcon;
static SETSPLASH Module_SetSplash;
static SETPROGRESS Module_SetProgress;
static SETINDETERMINATE Module_SetIndeterminate;

LoadingWindow_Gtk::LoadingWindow_Gtk()
{
}

static RString ModuleError( const RString s )
{
	return ssprintf( "Couldn't load symbol Module_%s", s.c_str() );
}

RString LoadingWindow_Gtk::Init()
{
	ASSERT( Handle == nullptr );

	Handle = dlopen( RageFileManagerUtil::sDirOfExecutable + "/" + "GtkModule.so", RTLD_NOW );
	if( Handle == nullptr )
		return ssprintf( "dlopen(): %s", dlerror() );

	Module_Init = (INIT) dlsym(Handle, "Init");
	if( !Module_Init )
		return ModuleError("Init");

	Module_Shutdown = (SHUTDOWN) dlsym(Handle, "Shutdown");
	if( !Module_Shutdown )
		return ModuleError("Shutdown");

	Module_SetText = (SETTEXT) dlsym(Handle, "SetText");
	if( !Module_SetText )
		return ModuleError("SetText");

	Module_SetIcon = (SETICON) dlsym(Handle, "SetIcon");
	if( !Module_SetIcon )
		return ModuleError("SetIcon");

	Module_SetSplash = (SETSPLASH) dlsym(Handle, "SetSplash");
	if( !Module_SetSplash )
		return ModuleError("SetSplash");

	Module_SetProgress = (SETPROGRESS) dlsym(Handle, "SetProgress");
	if( !Module_SetProgress )
		return ModuleError("SetProgress");

	Module_SetIndeterminate = (SETINDETERMINATE) dlsym(Handle, "SetIndeterminate");
	if( !Module_SetIndeterminate )
		return ModuleError("SetIndeterminate");

	const char *ret = Module_Init( &g_argc, &g_argv );
	if( ret != nullptr )
		return ret;
	return "";
}

LoadingWindow_Gtk::~LoadingWindow_Gtk()
{
	if( Module_Shutdown != nullptr )
		Module_Shutdown();
	Module_Shutdown = nullptr;

	if( Handle )
		dlclose( Handle );
	Handle = nullptr;
}

void LoadingWindow_Gtk::SetText( RString s )
{
	Module_SetText( s );
}

void LoadingWindow_Gtk::SetIcon( const RageSurface *pIcon )
{
	Module_SetIcon( pIcon );
}

void LoadingWindow_Gtk::SetSplash( const RageSurface *pSplash )
{
	Module_SetSplash( pSplash );
}

void LoadingWindow_Gtk::SetProgress( const int progress )
{
	LoadingWindow::SetProgress( progress );
	Module_SetProgress( m_progress, m_totalWork );
}

void LoadingWindow_Gtk::SetTotalWork( const int totalWork )
{
	LoadingWindow::SetTotalWork( totalWork );
	Module_SetProgress( m_progress, m_totalWork );
}

void LoadingWindow_Gtk::SetIndeterminate( bool indeterminate )
{
	LoadingWindow::SetIndeterminate( indeterminate );
	Module_SetIndeterminate( m_indeterminate );
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
