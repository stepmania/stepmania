#pragma once
#ifndef INPUT_HANDLER_P3IO_H
#define INPUT_HANDLER_P3IO_H

#include "ProductInfo.h" // Used to look for PRODUCT_ID_BARE which means STEPMANIA 5, NOT OITG
#include "InputHandler.h"
#include "RageThreads.h"
#include "io/P3IO.h"
#include "arch/COM/serial.h"
#include "Preference.h"

#ifdef WIN32
#include "windows.h"
#endif

#ifdef PRODUCT_ID_BARE
#include "arch/Lights/LightsDriver_Export.h"
#ifdef STDSTRING_H
#define PSTRING RString
#else
#define PSTRING std::string
#endif
#else
#include "arch/Lights/LightsDriver_External.h"
#define PSTRING CString
#endif

class InputHandler_P3IO : public InputHandler
{
public:
	InputHandler_P3IO();
	~InputHandler_P3IO();

	//sm5 compatability
#ifdef PRODUCT_ID_BARE
	void GetDevicesAndDescriptions(std::vector<InputDeviceInfo>& vDevicesOut);
#else
	void GetDevicesAndDescriptions(vector<InputDevice>& vDevicesOut, vector<PSTRING>& vDescriptionsOut);
#endif

private:
	/* Allow only one handler to control the board at a time. More than one
	* handler may be loaded due to startup and Static.ini interactions, so
	* we need this to prevent obscure I/O problems. */
	static bool s_bInitialized;

	P3IO Board;
	RageThread USBThread;

	RageThread USBBulkThread;
	void USBBulkThreadMainSD();
	void USBBulkThreadMainHD();
	static int USBBulkThreadSD_Start(void *p);
	static int USBBulkThreadHD_Start(void *p);
	static uint8_t capAt(uint8_t cap,uint8_t var);


	bool m_bFoundUSBDevice;
	bool m_bShutdown;
	uint8_t red_top_count;
	uint8_t red_bottom_count;
	uint8_t blue_top_count;
	uint8_t blue_bottom_count;
	uint8_t neon_count;



#define P3IO_NUM_STATES 22
#ifndef PRODUCT_ID_BARE
	const static int button_list[P3IO_NUM_STATES];
#else
	#ifndef STDSTRING_H

	#else
	
	const static DeviceButton button_list[P3IO_NUM_STATES];
	#endif
#endif
	bool previousStates[P3IO_NUM_STATES];
	uint8_t packetBuffer[3];


	void USBThreadMain();
	static int USBThread_Start(void *p);
	void ProcessDataFromDriver();



	//lights stuff
	void UpdateLightsUSBSD();
	void UpdateLightsUSBHD();


#define USB_OUT_P1_PANEL_LR	0x01
#define USB_OUT_P2_PANEL_LR	0x02
#define USB_OUT_P1_PANEL_UD	0x04
#define USB_OUT_P2_PANEL_UD	0x08
#define USB_OUT_MARQUEE_LR	0x10
#define USB_OUT_MARQUEE_UR	0x20
#define USB_OUT_MARQUEE_LL	0x40
#define USB_OUT_MARQUEE_UL	0x80


};

#endif // INPUT_HANDLER_P3IO

