#include "global.h"
#include "RageLog.h"
#include "USBDriver_Impl_Libusb.h"
#include <cerrno>

#ifdef PRODUCT_ID_BARE
#ifdef STDSTRING_H
#define PSTRING RString
#else
#include <cstring>
#define PSTRING std::string
#endif
#else
#define PSTRING CString
#endif


extern "C" {
#include <usb.h>
}

/* static struct to ensure the USB subsystem is initialized on start */
struct USBInit
{
	USBInit() { usb_init(); usb_find_busses(); usb_find_devices(); }
};

static struct USBInit g_USBInit;

static struct usb_device *FindDevice( int iVendorID, int iProductID )
{
	for( usb_bus *bus = usb_get_busses(); bus; bus = bus->next )
		for( struct usb_device *dev = bus->devices; dev; dev = dev->next )
			if( iVendorID == dev->descriptor.idVendor && iProductID == dev->descriptor.idProduct )
				return dev;

	LOG->Trace( "FindDevice(): no match for VID 0x%04x, PID 0x%04x.", iVendorID, iProductID );
	return NULL;
}

bool USBDriver_Impl_Libusb::DeviceExists( uint16_t iVendorID, uint16_t iProductID )
{
	usb_find_busses(); usb_find_devices();
	return FindDevice(iVendorID, iProductID) != NULL;
}

USBDriver_Impl_Libusb::USBDriver_Impl_Libusb()
{
	m_pHandle = NULL;
}

USBDriver_Impl_Libusb::~USBDriver_Impl_Libusb()
{
	Close();
}

bool USBDriver_Impl_Libusb::Open( int iVendorID, int iProductID )
{
	Close();

	if( usb_find_busses() < 0 )
	{
		LOG->Warn( "Libusb: usb_find_busses: %s", usb_strerror() );
		return false;
	}

	if( usb_find_devices() < 0 )
	{
		LOG->Warn( "Libusb: usb_find_devices: %s", usb_strerror() );
		return false;
	}
	
	struct usb_device *dev = FindDevice( iVendorID, iProductID );

	if( dev == NULL )
	{
		LOG->Warn( "Libusb: no match for %04x, %04x.", iVendorID, iProductID );
		return false;
	}

	m_pHandle = usb_open( dev );

	if( m_pHandle == NULL )
	{
		LOG->Warn( "Libusb: usb_open: %s", usb_strerror() );
		return false;
	}

#ifdef LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP
	// The device may be claimed by a kernel driver. Attempt to reclaim it.

	for( unsigned iface = 0; iface < dev->config->bNumInterfaces; iface++ )
	{
		int iResult = usb_detach_kernel_driver_np( m_pHandle, iface );

		// device doesn't understand message, no attached driver, no error -- ignore these
		if( iResult == -EINVAL || iResult == -ENODATA || iResult == 0 )
			continue;

		/* we have an error we can't handle; try and get more info. */
		LOG->Warn( "usb_detach_kernel_driver_np: %s\n", usb_strerror() );


#ifdef LIBUSB_HAS_GET_DRIVER_NP
		// on EPERM, a driver exists and we can't detach - report which one
		if( iResult == -EPERM )
		{
			char szDriverName[16]="(unknown)";
		
			
			usb_get_driver_np(m_pHandle, iface, szDriverName, 16);

			LOG->Warn( "(cannot detach kernel driver \"%s\")", szDriverName );
		}
#endif	// LIBUSB_HAS_GET_DRIVER_NP

		Close();
		return false;
	}
#endif	// LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP

	if ( !SetConfiguration(dev->config->bConfigurationValue) )
	{
		LOG->Warn( "Libusb: usb_set_configuration: %s", usb_strerror() );
		Close();
		return false;
	}
	
	// attempt to claim all interfaces for this device
	for( unsigned i = 0; i < dev->config->bNumInterfaces; i++ )
	{
		if ( !ClaimInterface(i) )
		{
			LOG->Warn( "Libusb: usb_claim_interface(%i): %s", i, usb_strerror() );
			Close();
			return false;
		}
	}

	return true;
}

void USBDriver_Impl_Libusb::Close()
{
	// never opened
	if( m_pHandle == NULL )
		return;

	usb_set_altinterface( m_pHandle, 0 );
	usb_reset( m_pHandle );
	usb_close( m_pHandle );
	m_pHandle = NULL;
}

int USBDriver_Impl_Libusb::ControlMessage( int iType, int iRequest, int iValue, int iIndex, char *pData, int iSize, int iTimeout )
{
	return usb_control_msg( m_pHandle, iType, iRequest, iValue, iIndex, pData, iSize, iTimeout );
}

int USBDriver_Impl_Libusb::BulkRead( int iEndpoint, char *pData, int iSize, int iTimeout )
{
	return usb_bulk_read( m_pHandle, iEndpoint, pData, iSize, iTimeout );
}

int USBDriver_Impl_Libusb::BulkWrite( int iEndpoint, char *pData, int iSize, int iTimeout )
{
	return usb_bulk_write( m_pHandle, iEndpoint, pData, iSize, iTimeout );
}

int USBDriver_Impl_Libusb::InterruptRead( int iEndpoint, char *pData, int iSize, int iTimeout )
{
	return usb_interrupt_read( m_pHandle, iEndpoint, pData, iSize, iTimeout );
}

int USBDriver_Impl_Libusb::InterruptWrite( int iEndpoint,char *pData, int iSize, int iTimeout )
{
	return usb_interrupt_write( m_pHandle, iEndpoint, pData, iSize, iTimeout );
}

bool USBDriver_Impl_Libusb::SetConfiguration( int iConfig )
{
	return usb_set_configuration( m_pHandle, iConfig ) == 0;
}

bool USBDriver_Impl_Libusb::ClaimInterface( int iInterface )
{
	return usb_claim_interface( m_pHandle, iInterface ) == 0;
}

bool USBDriver_Impl_Libusb::ReleaseInterface( int iInterface )
{
	return usb_release_interface( m_pHandle, iInterface ) == 0;
}

const char* USBDriver_Impl_Libusb::GetError() const
{
	return usb_strerror();
}
