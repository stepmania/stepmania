#ifndef SCREEN_EXIT_H
#define SCREEN_EXIT_H

#include "Screen.h"
#include "RageTimer.h"

class ScreenExit: public Screen
{
	bool m_Exited;
	RageTimer m_ShutdownTimer;

public:
	ScreenExit();
	void Update( float fDelta );
};


#endif
