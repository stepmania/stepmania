#include "global.h"
#include "RestartProgram.h"
#include "windows.h"

void Win32RestartProgram()
{
	TCHAR szFullAppPath[MAX_PATH];
	GetModuleFileName(NULL, szFullAppPath, MAX_PATH);

	// Launch StepMania
	PROCESS_INFORMATION pi;
	STARTUPINFO	si;
	ZeroMemory( &si, sizeof(si) );
	CreateProcess(
		NULL,		// pointer to name of executable module
		szFullAppPath,		// pointer to command line string
		NULL,  // process security attributes
		NULL,   // thread security attributes
		false,  // handle inheritance flag
		0, // creation flags
		NULL,  // pointer to new environment block
		NULL,   // pointer to current directory name
		&si,  // pointer to STARTUPINFO
		&pi  // pointer to PROCESS_INFORMATION
	);

	ExitProcess( 0 );

	/* not reached */
}
