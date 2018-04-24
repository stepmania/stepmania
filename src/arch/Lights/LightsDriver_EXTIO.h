#pragma once
#ifndef LightsDriver_EXTIO_H
#define LightsDriver_EXTIO_H

#include "ProductInfo.h" // Used to look for PRODUCT_ID_BARE which means STEPMANIA 5, NOT OITG
#include "LightsDriver.h"
#include "arch/COM/serial.h"
#include "RageLog.h"
#include "GameState.h"
#include "RageThreads.h"
#include "Preference.h"

#ifdef PRODUCT_ID_BARE
#ifdef STDSTRING_H
#define PSTRING RString
#else
#define PSTRING std::string
#endif
#else
#define PSTRING CString
#endif

//extio bits
#define EXTIO_OUT_B1	0x80// 8
#define EXTIO_OUT_UP	0x40// 7
#define EXTIO_OUT_DOWN	0x20// 6
#define EXTIO_OUT_LEFT	0x10// 5
#define EXTIO_OUT_RIGHT	0x08// 4
#define EXTIO_OUT_NEON	0x40// 7

class LightsDriver_EXTIO : public LightsDriver
{
public:
	LightsDriver_EXTIO();
	~LightsDriver_EXTIO();
	virtual void Set(const LightsState *ls);
	static LightsState GetState();
	

private:
	/* Allow only one handler to control the board at a time. More than one
	* handler may be loaded due to startup and Static.ini interactions, so
	* we need this to prevent obscure I/O problems. */
	static bool s_bInitialized;
	
	static bool lightsUpdated;
	static bool m_bShutdown;

	static serial::Serial extio;
	void UpdateLightsEXTIO();
	void ExtioSetPlayerPanel(int player, uint8_t panel, int state);
	void WriteExtioPacket();



	RageThread EXTIOThread;
	static RageMutex m_Lock;
	static LightsState m_State;
	void EXTIOThreadMain();
	static int EXTIOThread_Start(void *p);
	static bool AreLightsUpdated();


};

#endif // LightsDriver_EXTIO_H

