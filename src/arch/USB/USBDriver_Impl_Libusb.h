/* USBDriver_Impl_Libusb: a low-level USB control system for any system
 * which implements the libusb-0.1 API calls (Windows, Linux, Mac). */

#ifndef USB_DRIVER_IMPL_LIBUSB_H
#define USB_DRIVER_IMPL_LIBUSB_H

#include "USBDriver_Impl.h"
struct usb_dev_handle;

#define HAS_USBDRIVER_IMPL_LIBUSB

class USBDriver_Impl_Libusb : public USBDriver_Impl
{
public:
	static bool DeviceExists( uint16_t iVendorID, uint16_t iProductID );

	USBDriver_Impl_Libusb();
	~USBDriver_Impl_Libusb();

	bool Open( int iVendorID, int iProductID );
	void Close();

	int ControlMessage( int iType, int iRequest, int iValue, int iIndex, char *pData, int iSize, int iTimeout );

	int BulkRead( int iEndpoint, char *pData, int iSize, int iTimeout );
	int BulkWrite( int iEndpoint, char *pData, int iSize, int iTimeout );

	int InterruptRead( int iEndpoint, char *pData, int iSize, int iTimeout );
	int InterruptWrite( int iEndpoint, char *pData, int iSize, int iTimeout );

	virtual const char *GetError() const;

protected:
	bool SetConfiguration( int iConfig );

	bool ClaimInterface( int iInterface );
	bool ReleaseInterface( int iInterface );

private:
	usb_dev_handle *m_pHandle;
};

#endif // USB_DRIVER_LIBUSB_H

