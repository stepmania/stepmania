#include "global.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "USBDriver_Impl_WinUSB.h"

#include <winusb.h>
#include <Usb100.h>
#include <Setupapi.h>

#pragma comment (lib, "setupapi.lib" )
#pragma comment (lib, "winusb.lib" )

bool USBDriver_Impl_WinUsb::DeviceExists( uint16_t iVendorID, uint16_t iProductID )
{
	/* currently not implemented */
	return false;
}

USBDriver_Impl_WinUSB::USBDriver_Impl_WinUSB()
{
	m_hDevice = INVALID_HANDLE_VALUE;
}

USBDriver_Impl_WinUSB::~USBDriver_Impl_WinUSB()
{
}

/* There are not words for how much I hate Microsoft APIs. -- vyhd */
bool USBDriver_Impl_WinUSB::Open( int iVendorID, int iProductID )
{
	// get a set of all the devices currently on the system
	HDEVINFO hDeviceInfo = SetupDiGetClassDevs( NULL, NULL, NULL, DIGCF_PRESENT );

	if( hDeviceInfo == INVALID_HANDLE_VALUE )
	{
		LOG->Trace( "WinUSB: SetupDiGetClassDevs failed (error %i)", GetLastError() );
		return false;
	}

	SP_DEVINFO_DATA DeviceInfoData;
	SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
	PSP_DEVICE_INTERFACE_DATA pInterfaceDetailData = NULL;

	LPTSTR lpDevicePath = NULL;

	for( int i = 0; SetupDiEnumDeviceInfo(hDeviceInfo, i, &DeviceInfoData); ++i )
	{
		if( lpDevicePath )
			LocalFree( lpDevicePath );
		if( pDeviceInterfaceData )
			LocalFree( pDeviceInterfaceData );

		DeviceInterfaceData.czSize = sizeof(SP_DEVICE_INTERFACE_DATA);

		bool bSuccess = SetupDiEnumDeviceInterfaces( hDeviceInfo, &DeviceInfoData, NULL, i, &DeviceInterfaceData );

		if( GetLastError() == ERROR_NO_MORE_ITEMS )
			break;

		if( !bSuccess )
		{
			LOG->Warn( "SetupDiEnumDeviceInterfaces failed with %d", GetLastError() );
			break;
		}

		int iRequiredLength = -1;

		bResult = SetupDiGetDeviceInterfaceDetail( hDeviceInfo, &DeviceInterfaceData, NULL, 0, &iRequiredLength, NULL );

		if( !bResult && GetLastError() == ERROR_INSUFFICIENT_BUFFER )
		{
			pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LPTR, iRequiredLength);

			if( !pInterfaceDetailData )
			{
				LOG->Warn( "Error allocating pInterfaceDetailData." );
				break;
			}
		}

		if( pInterfaceDetailSize )
			pInterfaceDetailSize->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		else
			LOG->Warn( "XXX pInterfaceDetailSize is NULL, assumed not NULL" );

		bResult = SetupDiGetDeviceInterfaceDetail( hDeviceInfo, &DeviceInterfaceData, pInterfaceDetailData, iRequiredLength, NULL, &DeviceInfoData );

		if( !bResult )
		{
			LOG->Warn( "SetupDiGetDeviceInterfaceDetai: %d", GetLastError() );
			break;
		}

		
	return SetupDiDestroyDeviceInfoList( hDeviceInfo );
}

void USBDriver_Impl_WinUSB::Close()
{
}

int USBDriver_Impl_WinUSB::ControlMessage( int iType, int iRequest, int iValue, int iIndex, char *pData, int iSize, int iTimeout )
{
	// TODO: use WinUsb_SetPipePolicy to set timeout?
	WINUSB_SETUP_PACKET msg;
	msg.RequestType = iType;
	msg.Request = iRequest;
	msg.Value = iValue;
	msg.Index = iIndex;
	msg.Length = iSize;

	int iRet = -1;

	// TODO: are we sure that iRet will stay -1 when the call fails?
	WinUsb_ControlTransfer( m_pDevice, &msg, pData, iSize, &iRet );
	return iRet;
}

int USBDriver_Impl_WinUSB::BulkRead( int iEndpoint, char *pData, int iSize, int iTimeout )
{
	int iRet = -1;
	WinUsb_ReadPipe( m_hDevice, iEndpoint, pData, iSize );
	return iRet;
}

int USBDriver_Impl_WinUSB::BulkWrite( int iEndpoint, char *pData, int iSize, int iTimeout )
{
	int iRet = -1;
	WinUsb_WritePipe( m_hDevice, iEndpoint, pData, iSize );
	return iRet;
}

int USBDriver_Impl_WinUSB::InterruptRead( int iEndpoint, char *pData, int iSize, int iTimeout )
{
	return 0;
}

int USBDriver_Impl_WinUSB::InterruptWrite( int iEndpoint, char *pData, int iSize, int iTimeout )
{
	return 0;
}

bool USBDriver_Impl_WinUSB::SetConfiguration( int iConfig )
{
	return false;
}

bool USBDriver_Impl_WinUSB::ClaimInterface( int iInterface )
{
	return false;
}

bool USBDriver_Impl_WinUSB::ReleaseInterface( int iInterface )
{
	return false;
}

