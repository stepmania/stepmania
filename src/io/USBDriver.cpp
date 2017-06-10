#include "global.h"
#include "RageLog.h"

#include "io/USBDriver.h"
#include "arch/USB/USBDriver_Impl.h"

USBDriver::USBDriver()
{
	m_pDriver = NULL;
}

USBDriver::~USBDriver()
{
	delete m_pDriver;
}

bool USBDriver::OpenInternal( uint16_t iVendorID, uint16_t iProductID )
{
	Close();

	/* see if this device actually exists before trying to open it */
	if( !USBDriver_Impl::DeviceExists(iVendorID, iProductID) )
	{
		LOG->Warn( "USBDriver::OpenInternal(0x%04x, 0x%04x): device does not exist\n", iVendorID, iProductID );
		return false;
	}

	m_pDriver = USBDriver_Impl::Create();

	/* if !m_pDriver, this build cannot support USB drivers. */
	if( m_pDriver == NULL )
	{
		LOG->Warn( "USBDriver::OpenInternal(): Create failed. (No driver impl?" );
		return false;
	}

	return m_pDriver->Open( iVendorID, iProductID );
}

bool USBDriver::Open()
{
	return false;
}

void USBDriver::Close()
{
	if( m_pDriver )
		m_pDriver->Close();
}

/*
 * Copyright (c) 2008-2011 BoXoRRoXoRs
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
