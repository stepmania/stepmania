/* LuaDriverHandle_USB: a set of USB API functions extended into Lua space.
 * The backing driver used for this is libusb-1.0 (http://www.libusb.org). */

#ifndef LUA_DRIVER_HANDLE_USB_H
#define LUA_DRIVER_HANDLE_USB_H

#include "LuaDriverHandle.h"

/* a bunch of forward declarations for our linking pleasure */
struct libusb_context;
struct libusb_device_handle;
struct libusb_device_descriptor;
struct libusb_config_descriptor;
struct libusb_interface_descriptor;
struct libusb_endpoint_descriptor;

class LuaDriverHandle_USB : public LuaDriverHandle
{
public:
	LuaDriverHandle_USB();
	virtual ~LuaDriverHandle_USB();

	bool Open( uint16_t iVendorID, uint16_t iProductID );
	bool IsOpen() const;
	void Close();

	/* these are set by global const ints in the .cpp. */
	int GetRevisionMajor() const;
	int GetRevisionMinor() const;

	int GetError() const { return m_iError; }
	const char* GetErrorStr( int error ) const;

	void PushSelf( lua_State *L );
protected:
	/* the last error that was encountered */
	int m_iError;

	libusb_context *m_pContext;
	libusb_device_handle *m_pHandle;

	friend class LunaLuaDriverHandle_USB;

	/* USB device handling functions - see libusb documentation at
	 * http://libusb.sourceforge.net/api-1.0/group__dev.html */
	int GetConfiguration();
	bool SetConfiguration( int config );

	bool ClaimInterface( int interface );
	bool ReleaseInterface( int interface );

	/* USB I/O functions - for more info, see libusb documentation at
	 * http://libusb.sourceforge.net/api-1.0/group__syncio.html */
	int ControlTransfer( uint8_t bmRequestType, uint8_t bRequest,
		uint16_t wValue, uint16_t wIndex, uint8_t *data,
		uint16_t wLength, unsigned int timeout );

	int BulkTransfer( uint8_t endpoint, uint8_t *data, uint32_t length,
		int* transferred, unsigned int timeout );

	int InterruptTransfer( uint8_t endpoint, uint8_t *data, uint32_t length,
		int* transferred, unsigned int timeout );

	const libusb_device_descriptor* GetDeviceDescriptor() const;
	const libusb_config_descriptor* GetConfigDescriptor( int config ) const;

	/* Pushes tables representing various libusb structs, documented at
	 * http://libusb.sourceforge.net/api-1.0/group__desc.html; if
	 * bPushSubtables is true, then the following happen recursively:
	 *
	 * Device: configurations are pushed into a "Configurations" subtable
	 * Config: interfaces are pushed into an "Interfaces" subtable
	 * Interface: endpoints are pushed into an "Endpoints" subtable
	 */

	void PushDeviceDescriptor( lua_State *L,
		const libusb_device_descriptor *desc, bool bPushSubtables = false );

	void PushConfigDescriptor( lua_State *L,
		const libusb_config_descriptor *desc, bool bPushSubtables = false );

	void PushInterfaceDescriptor( lua_State *L,
		const libusb_interface_descriptor *desc, bool bPushSubtables = false );

	void PushEndpointDescriptor( lua_State *L,
		const libusb_endpoint_descriptor *desc );
};

#endif // LUA_DRIVER_HANDLE_USB_H

/*
 * (c) 2011 Mark Cannon
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
