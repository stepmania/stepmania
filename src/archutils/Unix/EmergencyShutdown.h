#ifndef EMERGENCY_SHUTDOWN_H
#define EMERGENCY_SHUTDOWN_H

void RegisterEmergencyShutdownCallback( void (*pFunc)() );
void DoEmergencyShutdown();

#endif

