#ifndef STEPMANIA_H
#define STEPMANIA_H
/*
-----------------------------------------------------------------------------
 File: StepMania.h

 Desc: Objects accessable from anywhere in the program.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

void ApplyGraphicOptions();
void ExitGame();
void ResetGame();

#include "arch/ArchHooks/ArchHooks.h"
extern ArchHooks *HOOKS;	// global and accessable from anywhere in our program

#if defined(WIN32)
extern HWND g_hWndMain;
#endif

#endif
