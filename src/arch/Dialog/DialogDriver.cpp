#include "global.h"
#include "DialogDriver.h"
#include "RageLog.h"
#include "RageUtil.hpp"

using std::vector;

std::map<Rage::ci_ascii_string, CreateDialogDriverFn> *RegisterDialogDriver::g_pRegistrees;
RegisterDialogDriver::RegisterDialogDriver( const Rage::ci_ascii_string &sName, CreateDialogDriverFn pfn )
{
	if( g_pRegistrees == nullptr )
	{
		g_pRegistrees = new std::map<Rage::ci_ascii_string, CreateDialogDriverFn>;
	}
	ASSERT( g_pRegistrees->find(sName) == g_pRegistrees->end() );
	(*g_pRegistrees)[sName] = pfn;
}

REGISTER_DIALOG_DRIVER_CLASS( Null );

DialogDriver *DialogDriver::Create()
{
	std::string sDrivers = "win32,macosx,null";
	auto asDriversToTry = Rage::split( sDrivers, ",", Rage::EmptyEntries::skip );

	ASSERT( asDriversToTry.size() != 0 );

	for (auto const &Driver: asDriversToTry)
	{
		auto iter = RegisterDialogDriver::g_pRegistrees->find( Rage::ci_ascii_string{Driver.c_str()} );

		if( iter == RegisterDialogDriver::g_pRegistrees->end() )
		{
			continue;
		}
		DialogDriver *pRet = (iter->second)();
		DEBUG_ASSERT( pRet );
		const std::string sError = pRet->Init();

		if( sError.empty() )
		{	return pRet;
		}
		if( LOG )
		{	LOG->Info( "Couldn't load driver %s: %s", Driver.c_str(), sError.c_str() );
		}
		Rage::safe_delete( pRet );
	}
	return nullptr;
}


/*
 * (c) 2002-2006 Glenn Maynard, Steve Checkoway
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
