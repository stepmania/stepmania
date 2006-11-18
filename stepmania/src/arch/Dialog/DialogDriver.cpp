#include "global.h"
#include "DialogDriver.h"
#include "Foreach.h"
#include "RageLog.h"

map<istring, CreateDialogDriverFn> *RegisterDialogDriver::g_pRegistrees;
RegisterDialogDriver::RegisterDialogDriver( const istring &sName, CreateDialogDriverFn pfn )
{
	if( g_pRegistrees == NULL )
		g_pRegistrees = new map<istring, CreateDialogDriverFn>;
	
	ASSERT( g_pRegistrees->find(sName) == g_pRegistrees->end() );
	(*g_pRegistrees)[sName] = pfn;
}

REGISTER_DIALOG_DRIVER_CLASS( Null );

DialogDriver *MakeDialogDriver()
{
	RString sDrivers = "win32,cocoa,null";
	vector<RString> asDriversToTry;
	split( sDrivers, ",", asDriversToTry, true );
	
	ASSERT( asDriversToTry.size() != 0 );
	
	RString sDriver;
	DialogDriver *pRet = NULL;
	
	FOREACH_CONST( RString, asDriversToTry, Driver )
	{
		map<istring, CreateDialogDriverFn>::const_iterator iter = RegisterDialogDriver::g_pRegistrees->find( istring(*Driver) );
		
		if( iter == RegisterDialogDriver::g_pRegistrees->end() )
			continue;
		
		pRet = (iter->second)();
		DEBUG_ASSERT( pRet );
		const RString sError = pRet->Init();
		
		if( sError != "" )
		{
			if( LOG )
				LOG->Info( "Couldn't load driver %s: %s", Driver->c_str(), sError.c_str() );
			SAFE_DELETE( pRet );
			continue;
		}
		break;
	}
	return pRet;
}


/*
 * (c) 2002-2006 Gleen Maynard, Steve Checkoway
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
