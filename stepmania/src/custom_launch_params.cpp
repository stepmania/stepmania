#include <xtl.h>
#include <stdio.h>
#include "custom_launch_params.h"
#include "SDLx-0.02\SDL\src\cdrom\xbox\undocumented.h"


CUSTOM_LAUNCH_DATA g_launchData  ;
int g_autoLaunchGame  ;
int g_launchReturnXBE  ;

//GetXbeID Borrowed from MXM and modified because CreateFile wasn't working correctly
DWORD GetXbeID( LPCTSTR szFilePath )
{
	DWORD dwReturn = 0;
	HANDLE hFile;
	DWORD dwCertificateLocation;
	DWORD dwLoadAddress;
	DWORD dwRead;
	ANSI_STRING filename;
	OBJECT_ATTRIBUTES attributes;
	IO_STATUS_BLOCK status;
	NTSTATUS error;
	
	RtlInitAnsiString(&filename,szFilePath);
	InitializeObjectAttributes(&attributes, &filename, OBJ_CASE_INSENSITIVE, NULL);
	if (NT_SUCCESS(error = NtCreateFile(&hFile, 
																GENERIC_READ |SYNCHRONIZE | FILE_READ_ATTRIBUTES, 
																&attributes, 
																&status, 
																NULL, 
																0,
																FILE_SHARE_READ,
																FILE_OPEN,	
																FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT)))
	{
		if ( SetFilePointer(	hFile,  0x104, NULL, FILE_BEGIN ) == 0x104 )
		{
			if ( ReadFile( hFile, &dwLoadAddress, 4, &dwRead, NULL ) )
			{
				if ( SetFilePointer(	hFile,  0x118, NULL, FILE_BEGIN ) == 0x118 )
				{
					if ( ReadFile( hFile, &dwCertificateLocation, 4, &dwRead, NULL ) )
					{
						dwCertificateLocation -= dwLoadAddress;
						// Add offset into file
						dwCertificateLocation += 8;
						if ( SetFilePointer(	hFile,  dwCertificateLocation, NULL, FILE_BEGIN ) == dwCertificateLocation )
						{
							dwReturn = 0;
							ReadFile( hFile, &dwReturn, sizeof(DWORD), &dwRead, NULL );
							if ( dwRead != sizeof(DWORD) )
							{
								dwReturn = 0;
							}
						}

					}
				}
			}
		}
		CloseHandle(hFile);
	}
	return dwReturn;
}

//XGetCustomLaunchData
//
//If there is a valid filename to load, then g_autoLaunchGame will be set to 1
//If there is a valid return XBE, then g_launchReturnXBE will be set to 1
int XGetCustomLaunchData()
{
	DWORD launchType;

	g_autoLaunchGame = 0 ;
	g_launchReturnXBE = 0 ;

	memset( &g_launchData, 0, sizeof( CUSTOM_LAUNCH_DATA ) ) ;


	if ( ( XGetLaunchInfo( &launchType,(PLAUNCH_DATA)&g_launchData) == ERROR_SUCCESS ) )
	{
		if  ( ( launchType == LDT_TITLE ) &&  ( g_launchData.magic == CUSTOM_LAUNCH_MAGIC ) )
		{
			if ( g_launchData.szFilename[0] ) 
				g_autoLaunchGame = 1 ;

			if ( g_launchData.szLaunchXBEOnExit[0] ) 
				g_launchReturnXBE = 1 ;

			return 1 ;
		}
		else
		{
		}


	}

	return 0 ;
}

void XReturnToLaunchingXBE( )
{
	if ( g_launchReturnXBE )
	{
		LD_LAUNCH_DASHBOARD LaunchData = { XLD_LAUNCH_DASHBOARD_MAIN_MENU };
		//XWriteTitleInfoAndRebootA( g_launchData.szLaunchXBEOnExit, g_launchData.szRemap_D_As, LDT_TITLE, 0, &LaunchData);
	}
	else
	{
		//sprintfx( "loading xboxdahs\r\n") ;
		LD_LAUNCH_DASHBOARD LaunchData = { XLD_LAUNCH_DASHBOARD_MAIN_MENU };

		/*
		LD_LAUNCH_DASHBOARD LaunchData ;
		memset( &LaunchData, 0, sizeof(LD_LAUNCH_DASHBOARD) ) ;

		LaunchData.dwContext = 0 ;
		LaunchData.dwParameter1 = 0 ;
		LaunchData.dwParameter2 = 0 ;
		LaunchData.dwReason = XLD_LAUNCH_DASHBOARD_MAIN_MENU ;*/

		//XWriteTitleInfoAndRebootA( "xboxdash.xbe", "\\Device\\Harddisk0\\Partition2", LDT_TITLE, 0, &LaunchData);
		XLaunchNewImage( NULL, (LAUNCH_DATA*)&LaunchData );
	}
}

void XLaunchNewImageWithParams( char *szXBEName, char *szMap_D_As, PCUSTOM_LAUNCH_DATA pLaunchData )
{
	ANSI_STRING filename;
	OBJECT_ATTRIBUTES attributes;
	IO_STATUS_BLOCK status;
	HANDLE hDevice;
	NTSTATUS error;
	char szDevice[MAX_PATH] ;
	DWORD dwTitleID, dwRead  ;


	sprintf( szDevice, "%s\\%s", szMap_D_As, szXBEName ) ;

	dwTitleID = GetXbeID( szDevice ) ;

	//XWriteTitleInfoAndRebootA( szXBEName, szMap_D_As, LDT_TITLE, dwTitleID, pLaunchData); 
}


/*

Example of how to call XLaunchNewImageWithParams()


	CUSTOM_LAUNCH_DATA ldata ;


	memset( &ldata, 0, sizeof(CUSTOM_LAUNCH_DATA) ) ;

	ldata.magic = CUSTOM_LAUNCH_MAGIC ;  //Must set this to our magic number for recognition!

	//This is the filename to launch when the launched XBE starts up.
	//Note that the filename path is *relative* to the D: that is being mapped
	//in the 2nd parameter passed to XLaunchNewImageWithParams
	//In this case, XLaunchNewImageWithParams is passed a value of 
	//"\\Device\\Harddisk0\\Partition1\\Games" which is equivalent to "E:\\Games"
	//which gets mapped to "D:\".  Therefore, if our rom directory is
	//E:\GAMES\SMSROMS, then we have to use a value of "D:\\SMSROMS\\FILENAME.ZIP"
	//in this ldata.szFilename

	//PCSXBox also recognizes the standard mappings for the C, E, F, X, Y, and Z
	//drives.  The R drive can be used to indicate a file on the DVD-ROM drive.
	//e.g. R:\\cds\\castle.bin
	//You can also pass it a name like SMB:\\gamename.bin to indicate it should
	//load the game on the samba share defined in PCSXBox.  (Probably not useful
	//to frontends since you won't know what the share is mapped to, but the
	//syntax is available to you.)

	strcpy( ldata.szFilename, "d:\\psxcds\\castle.bin" );

	//This is *only* the XBE filename.  Do not include a path in this value.
	strcpy( ldata.szLaunchXBEOnExit, "dashboard.xbe" ) ;

	//This is what D:\ shall be mapped to when we launch the return XBE.
	//In this case, D:\ will be mapped to E:\DASHBOARDS
	strcpy( ldata.szRemap_D_As, "\\Device\\Harddisk0\\Partition1\\DASHBOARDS" ) ;

	//The following CUSTOM_LAUNCH_DATA field is the only other one used by PCSXBox:

	//ldata.executionType = 0;  //This means "launch game in HLE mode"
	ldata.executionType = 1;  //This means "launch game in BIOS mode"

	//The following will launch E:\GAMES\PCSXBOX.XBE ( if "E:" is mapped to "\\Device\\Harddisk0\\Partition1" )
	XLaunchNewImageWithParams( "PCSXBOX.XBE", "\\Device\\Harddisk0\\Partition1\\GAMES", &ldata ) ;
*/

/*  Example for command-line launching FCEUltra

CUSTOM_LAUNCH_DATA ldata ;


memset( &ldata, 0, sizeof(CUSTOM_LAUNCH_DATA) ) ;

ldata.magic = CUSTOM_LAUNCH_MAGIC ;  //Must set this to our magic number for recognition!

strcpy( ldata.szFilename, "e:\\games\\nesroms\\Contra (U).nes" );

//This is *only* the XBE filename.  Do not include a path in this value.
strcpy( ldata.szLaunchXBEOnExit, "dashboard.xbe" ) ;

//This is what D:\ shall be mapped to when we launch the return XBE.
//In this case, D:\ will be mapped to E:\DASHBOARDS
strcpy( ldata.szRemap_D_As, "\\Device\\Harddisk0\\Partition1\\DASHBOARDS" ) ;


//The following will launch E:\GAMES\FCE.XBE ( if "E:" is mapped to "\\Device\\Harddisk0\\Partition1" )
XLaunchNewImageWithParams( "FCE.XBE", "\\Device\\Harddisk0\\Partition1\\GAMES", &ldata ) ;
*/

#ifdef  __cplusplus
extern "C" {
#endif

void doReturn() 
{
	XGetCustomLaunchData();
	XReturnToLaunchingXBE( );
}

#ifdef  __cplusplus
}
#endif
