#include "global.h"
#include "GotoURL.h"

LONG GetRegKey(HKEY key, CString subkey, LPTSTR retdata)
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

	char *pos = _tcsstr(key, "\"%1\"");
	if (pos == NULL)
	{
		// No quotes found.  Check for %1 without quotes
		pos = strstr(key, _T("%1"));
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
