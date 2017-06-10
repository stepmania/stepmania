#include "global.h"
#include "RageLog.h"
#include "PacDrive.h"
#include "arch/USB/USBDriver_Impl.h"

/* PacDrives have PIDs 1500 - 1507, but we'll handle that later. */
const short PACDRIVE_VENDOR_ID = 0xD209;
const short PACDRIVE_PRODUCT_ID = 0x1500;

/* I/O request timeout, in microseconds (so, 10 ms) */
const unsigned REQ_TIMEOUT = 10000;

bool PacDrive::Open()
{
	for( unsigned i = 0; i < 8; ++i )
		if( OpenInternal(PACDRIVE_VENDOR_ID, PACDRIVE_PRODUCT_ID + i) )
			return true;

	return false;
}

bool PacDrive::Write( const uint16_t iData )
{
	// output is within the first 16 bits - accept a
	// 16-bit arg and cast it, for simplicity's sake.
	uint32_t data = (iData << 16);

	int iExpected = sizeof(data);

	int iResult = m_pDriver->ControlMessage(
		USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		HID_SET_REPORT, HID_IFACE_OUT, 0, (char *)&data, iExpected,
		REQ_TIMEOUT );

	if( iResult != iExpected )
		LOG->Warn( "PacDrive writing failed: %i (%s)\n", 
			iResult, m_pDriver->GetError()  );

	return iResult == iExpected;
}

/*
 * Copyright (c) 2008 BoXoRRoXoRs
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
