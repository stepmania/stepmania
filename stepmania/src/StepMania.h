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

#if defined(WIN32)
extern HWND g_hWndMain;
extern int g_argc;
extern char **g_argv;
#endif

#endif
