// Cross-platform, libusb-based driver for outputting lights
// via a PacDrive (http://www.ultimarc.com/pacdrive.html)

#include "global.h"
#include "RageLog.h"
#include "LightsDriver_LinuxPacDrive.h"

#include <cstdint>

extern "C" {
#include <usb.h>
}

REGISTER_LIGHTS_DRIVER_CLASS( LinuxPacDrive );

#define USB_DIR_OUT	0x00
#define USB_DIR_IN	0x80

#define USB_TYPE_STANDARD	(0x00 << 5)
#define USB_TYPE_CLASS		(0x01 << 5)
#define USB_TYPE_VENDOR		(0x02 << 5)
#define USB_TYPE_RESERVED	(0x03 << 5)

#define USB_RECIP_DEVICE	0x00
#define USB_RECIP_INTERFACE	0x01
#define USB_RECIP_ENDPOINT	0x02
#define USB_RECIP_OTHER		0x03

#define HID_GET_REPORT	0x01
#define HID_SET_REPORT	0x09
#define HID_IFACE_IN	256
#define HID_IFACE_OUT	512


/* PacDrives have PIDs 1500 - 1507, but we'll handle that later. */
const int PACDRIVE_VENDOR_ID = 0xD209;
const int PACDRIVE_PRODUCT_ID = 0x1500;

/* I/O request timeout, in microseconds (so, 10 ms) */
const unsigned PACDRIVE_TIMEOUT = 10000;

/* static struct to ensure the USB subsystem is initialized on start */
struct USBInit
{
	USBInit() { usb_init(); usb_find_busses(); usb_find_devices(); }
};

static struct USBInit g_USBInit;

//Adds new preference to allow for different light wiring setups
static Preference<RString> g_sPacDriveLightOrdering("PacDriveLightOrdering", "openitg");
int iLightingOrder = 0;

LightsDriver_LinuxPacDrive::LightsDriver_LinuxPacDrive()
{
	Device = NULL;
	DeviceHandle = NULL;

	FindDevice();
	OpenDevice();

	// clear all lights
	WriteDevice( 0 );

	RString lightOrder = g_sPacDriveLightOrdering.Get();
	if (lightOrder.CompareNoCase("lumenar") == 0 || lightOrder.CompareNoCase("openitg") == 0) {
		iLightingOrder = 1;
	}
}

LightsDriver_LinuxPacDrive::~LightsDriver_LinuxPacDrive()
{
	// clear all lights and close the connection
	WriteDevice( 0 );
	CloseDevice();
}

void LightsDriver_LinuxPacDrive::Set( const LightsState *ls )
{
	if ( !DeviceHandle ) return;

	std::uint16_t outb = 0;

	switch (iLightingOrder) {
	case 1:
		//Sets the cabinet light values to follow LumenAR/OpenITG wiring standards

		/*
		 * OpenITG PacDrive Order:
		 * Taken from LightsDriver_PacDrive::SetLightsMappings() in openitg.
		 *
		 * 0: Marquee UL
		 * 1: Marquee UR
		 * 2: Marquee DL
		 * 3: Marquee DR
		 *
		 * 4: P1 Button
		 * 5: P2 Button
		 *
		 * 6: Bass Left
		 * 7: Bass Right
		 *
		 * 8,9,10,11: P1 L R U D
		 * 12,13,14,15: P2 L R U D
		 */

		if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT]) outb |= BIT(0);
		if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_RIGHT]) outb |= BIT(1);
		if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT]) outb |= BIT(2);
		if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_RIGHT]) outb |= BIT(3);

		if (ls->m_bGameButtonLights[GameController_1][GAME_BUTTON_START]) outb |= BIT(4);
		if (ls->m_bGameButtonLights[GameController_2][GAME_BUTTON_START]) outb |= BIT(5);

		//Most PacDrive/Cabinet setups only have *one* bass light, so mux them together here.
		if (ls->m_bCabinetLights[LIGHT_BASS_LEFT] || ls->m_bCabinetLights[LIGHT_BASS_RIGHT]) outb |= BIT(6);
		if (ls->m_bCabinetLights[LIGHT_BASS_LEFT] || ls->m_bCabinetLights[LIGHT_BASS_RIGHT]) outb |= BIT(7);

		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_LEFT]) outb |= BIT(8);
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_RIGHT]) outb |= BIT(9);
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_UP]) outb |= BIT(10);
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_DOWN]) outb |= BIT(11);

		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_LEFT]) outb |= BIT(12);
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_RIGHT]) outb |= BIT(13);
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_UP]) outb |= BIT(14);
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_DOWN]) outb |= BIT(15);

		break;

	case 0:
	default:
		//If all else fails, falls back to original order
		//reference page 7
		//http://www.peeweepower.com/stepmania/sm509pacdriveinfo.pdf

		if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT]) outb |= BIT(0);
		if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_RIGHT]) outb |= BIT(1);
		if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT]) outb |= BIT(2);
		if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_RIGHT]) outb |= BIT(3);

		if (ls->m_bCabinetLights[LIGHT_BASS_LEFT] || ls->m_bCabinetLights[LIGHT_BASS_RIGHT]) outb |= BIT(4);

		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_LEFT]) outb |= BIT(5);
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_RIGHT]) outb |= BIT(6);
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_UP]) outb |= BIT(7);
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_DOWN]) outb |= BIT(8);
		if (ls->m_bGameButtonLights[GameController_1][GAME_BUTTON_START]) outb |= BIT(9);

		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_LEFT]) outb |= BIT(10);
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_RIGHT]) outb |= BIT(11);
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_UP]) outb |= BIT(12);
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_DOWN]) outb |= BIT(13);
		if (ls->m_bGameButtonLights[GameController_2][GAME_BUTTON_START]) outb |= BIT(14);
		//Bit index 15 is unused.

		break;
}


	WriteDevice(outb);
}

void LightsDriver_LinuxPacDrive::FindDevice()
{
	if ( usb_find_busses() < 0 )
	{
		LOG->Warn( "libusb: usb_find_busses: %s", usb_strerror() );
		return;
	}

	if ( usb_find_devices() < 0 )
	{
		LOG->Warn( "libusb: usb_find_devices: %s", usb_strerror() );
		return;
	}

	for ( usb_bus *bus = usb_get_busses(); bus; bus = bus->next )
		for ( struct usb_device *dev = bus->devices; dev; dev = dev->next )
			if ( PACDRIVE_VENDOR_ID == dev->descriptor.idVendor &&
				 PACDRIVE_PRODUCT_ID <= dev->descriptor.idProduct &&
				 PACDRIVE_PRODUCT_ID + 8 > dev->descriptor.idProduct ) {
				Device = dev;
				LOG->Info( "PacDrive device was found vid: 0x%04x pid: 0x%04x", dev->descriptor.idVendor, dev->descriptor.idProduct );
				return;
			}

	LOG->Warn( "PacDrive was not found!" );
	Device = NULL;
}

void LightsDriver_LinuxPacDrive::OpenDevice()
{
	CloseDevice();

	if ( !Device ) return;

	DeviceHandle = usb_open( Device );

	if ( DeviceHandle == NULL ) {
		LOG->Warn( "libusb: usb_open: %s", usb_strerror() );
		return;
	}

	// The device may be claimed by a kernel driver. Attempt to reclaim it.
	for ( unsigned iface = 0; iface < Device->config->bNumInterfaces; iface++ )
	{
		int result = usb_detach_kernel_driver_np( DeviceHandle, iface );

		// device doesn't understand message, no attached driver, no error -- ignore these
		if( result == -EINVAL || result == -ENODATA || result == 0 )
			continue;

		/* we have an error we can't handle; try and get more info. */
		LOG->Warn( "usb_detach_kernel_driver_np: %s\n", usb_strerror() );

		// on EPERM, a driver exists and we can't detach - report which one
		if ( result == -EPERM )
		{
			char szDriverName[16];
			strcpy( szDriverName, "(unknown)" );
			usb_get_driver_np(DeviceHandle, iface, szDriverName, 16);

			LOG->Warn( "(cannot detach kernel driver \"%s\")", szDriverName );
		}

		CloseDevice();
		return;
	}

	if ( usb_set_configuration( DeviceHandle, Device->config->bConfigurationValue) ) {
		LOG->Warn( "libusb: usb_set_configuration: %s", usb_strerror() );
		CloseDevice();
		return;
	}

	// attempt to claim all interfaces for this device
	for ( unsigned i = 0; i < Device->config->bNumInterfaces; i++ )
	{
		if ( usb_claim_interface( DeviceHandle, i ) ) {
			LOG->Warn( "Libusb: usb_claim_interface(%i): %s", i, usb_strerror() );
			CloseDevice();
			return;
		}
	}
}

void LightsDriver_LinuxPacDrive::WriteDevice(std::uint16_t out)
{
	if ( !DeviceHandle ) return;

	// output is within the first 16 bits - accept a
	// 16-bit arg and cast it, for simplicity's sake.
	std::uint32_t data = (out << 16);
	int expected = sizeof(data);

	int result = usb_control_msg( DeviceHandle, USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
							  HID_SET_REPORT, HID_IFACE_OUT, 0, (char *)&data, expected,
							  PACDRIVE_TIMEOUT );

	if( result != expected ) {
		LOG->Warn( "PacDrive writing failed: %i (%s)\n", result, usb_strerror() );
		CloseDevice();
	}
}

void LightsDriver_LinuxPacDrive::CloseDevice()
{
	if ( !DeviceHandle )
		return;

	usb_set_altinterface( DeviceHandle, 0 );
	usb_reset( DeviceHandle );
	usb_close( DeviceHandle );
	DeviceHandle = NULL;
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
