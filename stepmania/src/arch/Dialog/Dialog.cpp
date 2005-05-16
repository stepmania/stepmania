#include "global.h"
#include "Dialog.h"
#include "DialogDriver.h"
#include "PrefsManager.h"
#include "RageUtil.h"
#include "RageLog.h"

#include "arch/arch.h"

#include "Selector_Dialog.h"
DialogDriver *MakeDialogDriver()
{
	CString drivers = "win32,cocoa,null";
	CStringArray DriversToTry;
	split(drivers, ",", DriversToTry, true);

	ASSERT( DriversToTry.size() != 0 );

	CString Driver;
	DialogDriver *ret = NULL;

	for( unsigned i = 0; ret == NULL && i < DriversToTry.size(); ++i )
	{
		Driver = DriversToTry[i];

#ifdef USE_DIALOG_DRIVER_COCOA
		if( !DriversToTry[i].CompareNoCase("Cocoa") )	ret = new DialogDriver_Cocoa;
#endif
#ifdef USE_DIALOG_DRIVER_NULL
		if( !DriversToTry[i].CompareNoCase("Null") )	ret = new DialogDriver_Null;
#endif
#ifdef USE_DIALOG_DRIVER_WIN32
		if( !DriversToTry[i].CompareNoCase("Win32") )	ret = new DialogDriver_Win32;
#endif

		if( ret == NULL )
		{
			continue;
		}

		CString sError = ret->Init();
		if( sError != "" )
		{
			if( LOG )
				LOG->Info( "Couldn't load driver %s: %s", DriversToTry[i].c_str(), sError.c_str() );
			SAFE_DELETE( ret );
		}
	}

	return ret;
}

static DialogDriver *g_pImpl = NULL;
static DialogDriver_Null g_NullDriver;
static bool g_bWindowed = true;		// Start out true so that we'll show errors before DISPLAY is init'd.
static bool g_bIsShowingDialog = false;

void Dialog::Init()
{
	if( g_pImpl != NULL )
		return;

	g_pImpl = MakeDialogDriver();

	/* DialogDriver_Null should have worked, at least. */
	ASSERT( g_pImpl != NULL );
}

void Dialog::Shutdown()
{
	delete g_pImpl;
	g_pImpl = NULL;
}

bool Dialog::IsShowingDialog()
{
	return g_bIsShowingDialog;
}

static bool MessageIsIgnored( CString ID )
{
	vector<CString> list;
	split( PREFSMAN->m_sIgnoredMessageWindows, ",", list );
	for( unsigned i = 0; i < list.size(); ++i )
		if( !ID.CompareNoCase(list[i]) )
			return true;
	return false;
}

void Dialog::IgnoreMessage( CString ID )
{
	/* We can't ignore messages before PREFSMAN is around. */
	if( PREFSMAN == NULL )
	{
		if( ID != "" && LOG )
			LOG->Warn( "Dialog: message \"%s\" set ID too early for ignorable messages", ID.c_str() );
			
		return;
	}

	if( ID == "" )
		return;

	if( MessageIsIgnored(ID) )
		return;

	vector<CString> list;
	split( PREFSMAN->m_sIgnoredMessageWindows, ",", list );
	list.push_back( ID );
	PREFSMAN->m_sIgnoredMessageWindows.Set( join(",",list) );
	PREFSMAN->SaveGlobalPrefsToDisk();
}

void Dialog::Error( CString sMessage, CString ID )
{
	Dialog::Init();

	if( LOG )
		LOG->Trace( "Dialog: \"%s\" [%s]", sMessage.c_str(), ID.c_str() );

	if( ID != "" && MessageIsIgnored( ID ) )
		return;

	g_bIsShowingDialog = true;
	
	g_pImpl->Error( sMessage, ID );
	
	g_bIsShowingDialog = false;
}

void Dialog::SetWindowed( bool bWindowed )
{
	g_bWindowed = bWindowed;
}

void Dialog::OK( CString sMessage, CString ID )
{
	Dialog::Init();

	if( LOG )
		LOG->Trace( "Dialog: \"%s\" [%s]", sMessage.c_str(), ID.c_str() );

	if( ID != "" && MessageIsIgnored( ID ) )
		return;

	g_bIsShowingDialog = true;
	
	// only show Dialog if windowed
	if( !g_bWindowed )
		g_NullDriver.OK( sMessage, ID );
	else
		g_pImpl->OK( sMessage, ID );	// call derived version
	
	g_bIsShowingDialog = false;
}

Dialog::Result Dialog::AbortRetryIgnore( CString sMessage, CString ID )
{
	Dialog::Init();

	if( LOG )
		LOG->Trace( "Dialog: \"%s\" [%s]", sMessage.c_str(), ID.c_str() );

	if( ID != "" && MessageIsIgnored( ID ) )
		return g_NullDriver.AbortRetryIgnore( sMessage, ID );

	g_bIsShowingDialog = true;
	
	// only show Dialog if windowed
	Dialog::Result ret;
	if( !g_bWindowed )
		ret = g_NullDriver.AbortRetryIgnore( sMessage, ID );
	else
		ret = g_pImpl->AbortRetryIgnore( sMessage, ID );	// call derived version
	
	g_bIsShowingDialog = false;

	return ret;
}

Dialog::Result Dialog::AbortRetry( CString sMessage, CString ID )
{
	Dialog::Init();

	if( LOG )
		LOG->Trace( "Dialog: \"%s\" [%s]", sMessage.c_str(), ID.c_str() );

	if( ID != "" && MessageIsIgnored( ID ) )
		return g_NullDriver.AbortRetry( sMessage, ID );

	g_bIsShowingDialog = true;

	// only show Dialog if windowed
	Dialog::Result ret;
	if( !g_bWindowed )
		ret = g_NullDriver.AbortRetry( sMessage, ID );
	else
		ret = g_pImpl->AbortRetry( sMessage, ID );	// call derived version
	
	g_bIsShowingDialog = false;

	return ret;
}

/*
 * (c) 2003-2004 Glenn Maynard, Chris Danford
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
