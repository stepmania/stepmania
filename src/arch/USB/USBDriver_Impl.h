#ifndef USB_DRIVER_IMPL_H
#define USB_DRIVER_IMPL_H

#include "USBDriver_Macros.h"

class USBDriver_Impl
{
public:
	static USBDriver_Impl* Create();

	/* returns true if a USB device exists with this VID/PID */
	static bool DeviceExists( uint16_t iVendorID, uint16_t iProductID );

	USBDriver_Impl();
	virtual ~USBDriver_Impl();

	virtual bool Open( int iVendorID, int iProductID ) = 0;
	virtual void Close() = 0;

	virtual int ControlMessage( int iType, int iRequest, int iValue, int iIndex, char *pData, int iSize, int iTimeout ) = 0;

	virtual int BulkRead( int iEndpoint, char *pData, int iSize, int iTimeout ) = 0;
	virtual int BulkWrite( int iEndpoint, char *pData, int iSize, int iTimeout ) = 0;

	virtual int InterruptRead( int iEndpoint, char *pData, int iSize, int iTimeout ) = 0;
	virtual int InterruptWrite( int iEndpoint, char *pData, int iSize, int iTimeout ) = 0;

	/* if something fails, return a string describing the error */
	virtual const char *GetError() const = 0;

protected:
	virtual bool SetConfiguration( int iConfig ) = 0;

	virtual bool ClaimInterface( int iInterface ) = 0;
	virtual bool ReleaseInterface( int iInterface ) = 0;
};

#endif // USB_DRIVER_IMPL_H

