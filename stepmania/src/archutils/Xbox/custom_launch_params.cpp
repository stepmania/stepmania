#include <xtl.h>
#include <stdio.h>
#include "custom_launch_params.h"

CUSTOM_LAUNCH_DATA g_launchData  ;
int g_autoLaunchGame  ;
int g_launchReturnXBE  ;

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
	}

	return 0 ;
}

void XReturnToLaunchingXBE( )
{
	if ( g_launchReturnXBE )
	{
		LD_LAUNCH_DASHBOARD LaunchData = { XLD_LAUNCH_DASHBOARD_MAIN_MENU };
	}
	else
	{
		LD_LAUNCH_DASHBOARD LaunchData = { XLD_LAUNCH_DASHBOARD_MAIN_MENU };

		XLaunchNewImage( NULL, (LAUNCH_DATA*)&LaunchData );
	}
}

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
