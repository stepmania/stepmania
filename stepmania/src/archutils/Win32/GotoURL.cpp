#include "global.h"
#include "GotoURL.h"
#include "windows.h"
#include "shellapi.h"

/* This is called from the crash handler; don't use RegistryAccess, since it's
 * not crash-conditions safe. */
static LONG GetRegKey(HKEY key, CString subkey, LPTSTR retdata)
{
	HKEY hkey;
    LONG retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

    if (retval != ERROR_SUCCESS)
		return retval;

	long datasize = MAX_PATH;
	TCHAR data[MAX_PATH];
	RegQueryValue(hkey, "emulation", data, &datasize);
	strcpy(retdata,data);
	RegCloseKey(hkey);

    return ERROR_SUCCESS;
}


bool GotoURL(CString url)
{
	// First try ShellExecute()
	int result = (int) ShellExecute(NULL, "open", url, NULL,NULL, SW_SHOWDEFAULT);

	// If it failed, get the .htm regkey and lookup the program
	if (result > 32)
		return true;

	char key[2*MAX_PATH];
	if (GetRegKey(HKEY_CLASSES_ROOT, ".htm", key) != ERROR_SUCCESS)
		return false;

	strcpy(key, "\\shell\\open\\command");

	if (GetRegKey(HKEY_CLASSES_ROOT,key,key) != ERROR_SUCCESS)
		return false;

	char *pos = strstr( key, "\"%1\"" );
	if (pos == NULL)
	{
		// No quotes found.  Check for %1 without quotes
		pos = strstr( key, "%1" );
		if (pos == NULL)                   
			pos = key+lstrlen(key)-1; // No parameter.
		else
			*pos = '\0';                 // Remove the parameter
	}
	else
		*pos = '\0';                       // Remove the parameter

	strcat(pos, " ");
	strcat(pos, url);

	return WinExec(key,SW_SHOWDEFAULT) > 32;
}

/*
 * (c) 2002-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
