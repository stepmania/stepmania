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

#include "Game.h"

int SMmain(int argc, char* argv[]);
void ApplyGraphicOptions();
void ExitGame();
void ResetGame( bool ReturnToFirstScreen=true );
void ReadGamePrefsFromDisk( bool bSwitchToLastPlayedGame=true );
void SaveGamePrefsToDisk();
void ChangeCurrentGame( Game g );

// If successful, return filename of screenshot in sDir, else return ""
CString SaveScreenshot( CString sDir, bool bSaveCompressed, bool bMakeSignature, int iIndex = -1 );

#if defined(_WINDOWS)
#include "windows.h"
extern HWND g_hWndMain;
#endif

extern int g_argc;
extern char **g_argv;
bool GetCommandlineArgument( const CString &option, CString *argument=NULL, int iIndex=0 );

#endif
