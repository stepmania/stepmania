#include "global.h"
#include "EmergencyShutdown.h"
#include "RageUtil.h"
#include <array>

typedef void (*Callback)();
static std::array<Callback, 5> g_pEmergencyFunc;
static unsigned g_iNumEmergencyFuncs = 0;

void RegisterEmergencyShutdownCallback( void (*pFunc)() )
{
	ASSERT( g_iNumEmergencyFuncs + 1 < g_pEmergencyFunc.size() );
	g_pEmergencyFunc[ g_iNumEmergencyFuncs++ ] = pFunc;
}

void DoEmergencyShutdown()
{
	for( unsigned i = 0; i < g_iNumEmergencyFuncs; ++i )
		g_pEmergencyFunc[i]();
}

