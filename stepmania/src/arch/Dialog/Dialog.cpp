#include "global.h"
#include "Dialog.h"
#include "DialogDriver.h"
#if !defined(SMPACKAGE)
#include "PrefsManager.h"
#endif
#include "RageUtil.h"
#include "RageLog.h"
#include "arch/arch.h"
#include "RageThreads.h"

#if !defined(SMPACKAGE)
static Preference<RString> g_sIgnoredDialogs( "IgnoredDialogs", "" );
#endif

DialogDriver *MakeDialogDriver()
{
	RString sDrivers = "win32,cocoa,null";
	vector<RString> asDriversToTry;
	split( sDrivers, ",", asDriversToTry, true );

	ASSERT( asDriversToTry.size() != 0 );

	RString sDriver;
	DialogDriver *pRet = NULL;

	for( unsigned i = 0; pRet == NULL && i < asDriversToTry.size(); ++i )
	{
		sDriver = asDriversToTry[i];

#ifdef USE_DIALOG_DRIVER_COCOA
		if( !asDriversToTry[i].CompareNoCase("Cocoa") )	pRet = new DialogDriver_Cocoa;
#endif
#ifdef USE_DIALOG_DRIVER_NULL
		if( !asDriversToTry[i].CompareNoCase("Null") )	pRet = new DialogDriver_Null;
#endif
#ifdef USE_DIALOG_DRIVER_WIN32
		if( !asDriversToTry[i].CompareNoCase("Win32") )	pRet = new DialogDriver_Win32;
#endif

		if( pRet == NULL )
		{
			continue;
		}

		RString sError = pRet->Init();
		if( sError != "" )
		{
			if( LOG )
				LOG->Info( "Couldn't load driver %s: %s", asDriversToTry[i].c_str(), sError.c_str() );
			SAFE_DELETE( pRet );
		}
	}

	return pRet;
}

static DialogDriver *g_pImpl = NULL;
static DialogDriver_Null g_NullDriver;
static bool g_bWindowed = true;		// Start out true so that we'll show errors before DISPLAY is init'd.

static bool DialogsEnabled()
{
	return g_bWindowed;
}

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

static bool MessageIsIgnored( RString sID )
{
#if !defined(SMPACKAGE)
	vector<RString> asList;
	split( g_sIgnoredDialogs, ",", asList );
	for( unsigned i = 0; i < asList.size(); ++i )
		if( !sID.CompareNoCase(asList[i]) )
			return true;
#endif
	return false;
}

void Dialog::IgnoreMessage( RString sID )
{
	/* We can't ignore messages before PREFSMAN is around. */
#if !defined(SMPACKAGE)
	if( PREFSMAN == NULL )
	{
		if( sID != "" && LOG )
			LOG->Warn( "Dialog: message \"%s\" set ID too early for ignorable messages", sID.c_str() );		
		return;
	}

	if( sID == "" )
		return;

	if( MessageIsIgnored(sID) )
		return;

	vector<RString> asList;
	split( g_sIgnoredDialogs, ",", asList );
	asList.push_back( sID );
	g_sIgnoredDialogs.Set( join(",",asList) );
	PREFSMAN->SavePrefsToDisk();
#endif
}

void Dialog::Error( RString sMessage, RString sID )
{
	Dialog::Init();

	if( LOG )
		LOG->Trace( "Dialog: \"%s\" [%s]", sMessage.c_str(), sID.c_str() );

	if( sID != "" && MessageIsIgnored(sID) )
		return;

	RageThread::SetIsShowingDialog( true );
	
	g_pImpl->Error( sMessage, sID );
	
	RageThread::SetIsShowingDialog( false );
}

void Dialog::SetWindowed( bool bWindowed )
{
	g_bWindowed = bWindowed;
}

void Dialog::OK( RString sMessage, RString sID )
{
	Dialog::Init();

	if( LOG )
		LOG->Trace( "Dialog: \"%s\" [%s]", sMessage.c_str(), sID.c_str() );

	if( sID != "" && MessageIsIgnored(sID) )
		return;

	RageThread::SetIsShowingDialog( true );
	
	// only show Dialog if windowed
	if( DialogsEnabled() )
		g_pImpl->OK( sMessage, sID );	// call derived version
	else
		g_NullDriver.OK( sMessage, sID );
	
	RageThread::SetIsShowingDialog( false );
}

Dialog::Result Dialog::AbortRetryIgnore( RString sMessage, RString sID )
{
	Dialog::Init();

	if( LOG )
		LOG->Trace( "Dialog: \"%s\" [%s]", sMessage.c_str(), sID.c_str() );

	if( sID != "" && MessageIsIgnored(sID) )
		return g_NullDriver.AbortRetryIgnore( sMessage, sID );

	RageThread::SetIsShowingDialog( true );
	
	// only show Dialog if windowed
	Dialog::Result ret;
	if( DialogsEnabled() )
		ret = g_pImpl->AbortRetryIgnore( sMessage, sID );	// call derived version
	else
		ret = g_NullDriver.AbortRetryIgnore( sMessage, sID );
	
	RageThread::SetIsShowingDialog( false );

	return ret;
}

Dialog::Result Dialog::AbortRetry( RString sMessage, RString sID )
{
	Dialog::Init();

	if( LOG )
		LOG->Trace( "Dialog: \"%s\" [%s]", sMessage.c_str(), sID.c_str() );

	if( sID != "" && MessageIsIgnored(sID) )
		return g_NullDriver.AbortRetry( sMessage, sID );

	RageThread::SetIsShowingDialog( true );

	// only show Dialog if windowed
	Dialog::Result ret;
	if( DialogsEnabled() )
		ret = g_pImpl->AbortRetry( sMessage, sID );	// call derived version
	else
		ret = g_NullDriver.AbortRetry( sMessage, sID );
	
	RageThread::SetIsShowingDialog( false );

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
