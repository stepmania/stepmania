#pragma once
#ifndef LightsDriver_Satellite_H
#define LightsDriver_Satellite_H

#include "ProductInfo.h" // Used to look for PRODUCT_ID_BARE which means STEPMANIA 5, NOT OITG
#include "LightsDriver.h"
#include "arch/COM/serial.h"
#include "RageLog.h"
#include "GameState.h"
#include "RageThreads.h"
#include "Preference.h"
#include "arch/ACIO/ACIO.h"

#ifdef PRODUCT_ID_BARE
#ifdef STDSTRING_H
#define PSTRING RString
#else
#define PSTRING std::string
#endif
#else
#define PSTRING CString
#endif

#define BIT(i) (1<<(i))
#define BIT_IS_SET(v,i) ((v&BIT(i))!=0)
class LightsDriver_Satellite : public LightsDriver
{
public:
	LightsDriver_Satellite();
	~LightsDriver_Satellite();
	virtual void Set(const LightsState *ls);
	static LightsState GetState();
	

private:
	/* Allow only one handler to control the board at a time. More than one
	* handler may be loaded due to startup and Static.ini interactions, so
	* we need this to prevent obscure I/O problems. */
	static bool s_bInitialized;
	
	static bool lightsUpdated;
	static bool m_bShutdown;

	static serial::Serial satellites;
	static uint8_t acio_request[256];
	static uint8_t acio_response[256];


	//static int get_expected_reply_bytes_from_request();
	//static int read_acio_packet();
	//static int write_acio_packet(int r_length);
	//static int assemble_init_packet_to_write(int j, bool isInitPacket);

	static void sendSatelliteSolid(int u, int sli, int slm, int slo, int sri, int srm, int sro);
	static void sendSatelliteTest(int u, int sli, int slm, int slo, int sri, int srm, int sro);


	RageThread SatelliteThread;
	static RageMutex m_Lock;
	static LightsState m_State;
	static int SatelliteThread_Start(void *p);
	void SatelliteThreadMain();
	void UpdateLightsSatellite();
	static bool AreLightsUpdated(); // checks if the set method was called so we spam the com port needlessly

	#define SATELLITE_X 0 //nothing
	#define SATELLITE_R 1 //red
	#define SATELLITE_G 2 //green
	#define SATELLITE_B 3 //blue
	#define SATELLITE_W 4 //white

	#define	SATELLITE_U 0 //monitor underlay
	#define SATELLITE_I 1 //inner
	#define SATELLITE_M 2 //mid
	#define SATELLITE_O 3 //outer

};

#endif // LightsDriver_Satellite_H

