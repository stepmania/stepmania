#include "global.h"
#include "RageSoundDriver.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "LocalizedString.h"
#include "Foreach.h"

map<istring, CreateSoundDriverFn> *RegisterSoundDriver::g_pRegistrees;
RegisterSoundDriver::RegisterSoundDriver( const istring &sName, CreateSoundDriverFn pfn )
{
	if( g_pRegistrees == NULL )
		g_pRegistrees = new map<istring, CreateSoundDriverFn>;
	
	ASSERT( g_pRegistrees->find(sName) == g_pRegistrees->end() );
	(*g_pRegistrees)[sName] = pfn;
}

static LocalizedString SOUND_DRIVERS_CANNOT_EMPTY( "Arch", "Sound Drivers cannot be empty." );
RageSoundDriver *MakeRageSoundDriver( const RString &drivers )
{
	vector<RString> DriversToTry;
	split( drivers, ",", DriversToTry, true );
	
	if( DriversToTry.empty() )
		RageException::Throw( "%s", SOUND_DRIVERS_CANNOT_EMPTY.GetValue().c_str() );
	
	FOREACH_CONST( RString, DriversToTry, Driver )
	{
		map<istring, CreateSoundDriverFn>::const_iterator iter = RegisterSoundDriver::g_pRegistrees->find( istring(*Driver) );
		
		if( iter == RegisterSoundDriver::g_pRegistrees->end() )
		{
			LOG->Trace( "Unknown sound driver: %s", Driver->c_str() );
			continue;
		}
		
		RageSoundDriver *pRet = (iter->second)();
		DEBUG_ASSERT( pRet );
		const RString sError = pRet->Init();
		
		if( sError.empty() )
		{
			LOG->Info( "Sound driver: %s", Driver->c_str() );
			return pRet;
		}
		LOG->Info( "Couldn't load driver %s: %s", Driver->c_str(), sError.c_str() );
		SAFE_DELETE( pRet );
	}
	return NULL;
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
