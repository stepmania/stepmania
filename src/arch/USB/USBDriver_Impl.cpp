#include "global.h"
#include "USBDriver_Impl.h"
#include "arch/arch_default.h"

USBDriver_Impl::USBDriver_Impl()
{
	/* no-op */
}

USBDriver_Impl::~USBDriver_Impl()
{
	/* no-op */
}

/* libusb has working implementations on the three major systems - Linux,
 * Mac, and Windows - but they may not perform as well as the native APIs.
 * If we have a different API, prefer that first. */

USBDriver_Impl* USBDriver_Impl::Create()
{
#if defined(HAS_USBDRIVER_IMPL_WINUSB)
	return new USBDriver_Impl_WinUSB;
#elif defined(HAS_USBDRIVER_IMPL_LIBUSB)
	return new USBDriver_Impl_Libusb;
#else
	return NULL;
#endif
}

/* XXX: can we do this better? */
bool USBDriver_Impl::DeviceExists( uint16_t iVendorID, uint16_t iProductID )
{
#if defined(HAS_USBDRIVER_IMPL_WINUSB)
	return USBDriver_Impl_WinUSB::DeviceExists( iVendorID, iProductID );
#elif defined(HAS_USBDRIVER_IMPL_LIBUSB)
	return USBDriver_Impl_Libusb::DeviceExists( iVendorID, iProductID );
#else
	return false;
#endif
}
