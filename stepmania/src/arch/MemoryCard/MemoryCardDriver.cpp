#include "global.h"
#include "MemoryCardDriver.h"
#include "PrefsManager.h"

#include "arch/arch_platform.h"

bool UsbStorageDevice::operator==(const UsbStorageDevice& other) const
{
  //  LOG->Trace( "Comparing %d %d %d %s %s to %d %d %d %s %s",
  //	      iBus, iPort, iLevel, sName.c_str(), sOsMountDir.c_str(),
  //	      other.iBus, other.iPort, other.iLevel, other.sName.c_str(), other.sOsMountDir.c_str() );
#define COMPARE(x) if( x != other.x ) return false;
  COMPARE( iBus );
  COMPARE( iPort );
  COMPARE( iLevel );
  COMPARE( sOsMountDir );
  return true;
#undef COMPARE
}

void UsbStorageDevice::SetOsMountDir( const CString &s )
{
	sOsMountDir = s;
	FixSlashesInPlace( sOsMountDir );
}


MemoryCardDriver *MakeMemoryCardDriver()
{
	if( !PREFSMAN->m_bMemoryCards )
		return new MemoryCardDriver_Null;

	MemoryCardDriver *ret = NULL;

#ifdef LINUX
	ret = new MemoryCardDriverThreaded_Linux;
#elif _WINDOWS
	ret = new MemoryCardDriverThreaded_Windows;
#endif
	if( !ret )
		ret = new MemoryCardDriver_Null;
	
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
