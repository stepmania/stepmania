#include "global.h"
#include "LightsDriver.h"
#include "RageLog.h"
#include "arch/arch_default.h"

void LightsDriver::Create( const RString &driver, vector<LightsDriver *> &Add )
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
#ifdef USE_LIGHTS_DRIVER_EXPORT
	if( !driver.CompareNoCase("Export") )		ret = new LightsDriver_Export;
#endif

	if( ret == NULL && driver.CompareNoCase("Null") )
		LOG->Trace( "Unknown lights driver name: %s", driver.c_str() );
	else if( ret != NULL )
		Add.push_back( ret );

	Add.push_back( new LightsDriver_SystemMessage );
}

/*
 * (c) 2002-2005 Glenn Maynard
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
