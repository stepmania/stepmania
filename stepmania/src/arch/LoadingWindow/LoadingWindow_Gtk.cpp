#include "global.h"
#include "RageLog.h"
#include "StepMania.h"
#include "RageFileManager.h"
#include "LoadingWindow_Gtk.h"
#include "LoadingWindow_GtkModule.h"

#include <dlfcn.h>

static void *Handle = NULL;
static INIT Module_Init;
static SHUTDOWN Module_Shutdown;
static SETTEXT Module_SetText;

LoadingWindow_Gtk::LoadingWindow_Gtk()
{
try {
	ASSERT( Handle == NULL );
	
	Handle = dlopen( DirOfExecutable + "/" + "GtkModule.so", RTLD_NOW );
	if( Handle == NULL )
		RageException::ThrowNonfatal("dlopen(): %s", dlerror());

	Module_Init = (INIT) dlsym(Handle, "Init");
	if( !Module_Init )
		RageException::ThrowNonfatal( "Couldn't load symbol Module_Init" );
	Module_Shutdown = (SHUTDOWN) dlsym(Handle, "Shutdown");
	if( !Module_Shutdown )
		RageException::ThrowNonfatal( "Couldn't load symbol Module_Shutdown" );
	Module_SetText = (SETTEXT) dlsym(Handle, "SetText");
	if( !Module_SetText )
		RageException::ThrowNonfatal( "Couldn't load symbol Module_SetText" );

	const char *ret = Module_Init( &g_argc, &g_argv );
	if( ret != NULL )
		RageException::ThrowNonfatal( ret );
} catch(...) {
	if( Handle )
		dlclose( Handle );
	Handle = NULL;
	throw;
}
}

LoadingWindow_Gtk::~LoadingWindow_Gtk()
{
	Module_Shutdown();

	dlclose( Handle );
	Handle = NULL;
}

void LoadingWindow_Gtk::SetText( CString s )
{
	Module_SetText( s );
}
