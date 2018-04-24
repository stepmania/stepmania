/* USBDriver_Impl_WinUSB: (not fully complete) USB control system for
 * Windows XP SP2 and up. Libusb has problems trying to hack around 
 * how terribly Windows handles I/O devices, so we have a native API 
 * implementation (in the works). */

#ifndef USB_DRIVER_IMPL_WINUSB_H
#define USB_DRIVER_IMPL_WINUSB_H

#include "USBDriver_Impl.h"
struct WINUSB_INTERFACE_HANDLE;

#define HAS_USBDRIVER_IMPL_WINUSB

class USBDriver_Impl_WinUSB : public USBDriver_Impl
{
public:
	static bool DeviceExists( uint16_t iVendorID, uint16_t iProductID );

	USBDriver_Impl();
	~USBDriver_Impl();

	bool Open( int iVendorID, int iProductID );
	void Close();

	int ControlMessage( int iType, int iRequest, int iValue, int iIndex, char *pData, int iSize, int iTimeout );

	int BulkRead( int iEndpoint, char *pData, int iSize, int iTimeout );
	int BulkWrite( int iEndpoint, char *pData, int iSize, int iTimeout );

	int InterruptRead( int iEndpoint, char *pData, int iSize, int iTimeout );
	int InterruptWrite( int iEndpoint, char *pData, int iSize, int iTimeout );

protected:
	bool SetConfiguration( int iConfig );

	bool ClaimInterface( int iInterface );
	bool ReleaseInterface( int iInterface );

private:
	WINUSB_INTERFACE_HANDLE m_hDevice;
};

#endif // USBDRIVER_IMPL_WINUSB_H
