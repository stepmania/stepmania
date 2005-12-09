#include "global.h"
#include "RegistryAccess.h"
#include "RageLog.h"
#include "RageUtil.h"

/* Given "HKEY_LOCAL_MACHINE\hardware\foo", return "hardware\foo", and place
 * the HKEY_LOCAL_MACHINE constant in key. */
static bool GetRegKeyType( const CString &sIn, CString &sOut, HKEY &key )
{
	size_t iBackslash = sIn.find( '\\' );
	if( iBackslash == sIn.npos )
	{
		LOG->Warn( "Invalid registry key: \"%s\" ", sIn.c_str() );
		return false;
	}

	CString sType = sIn.substr( 0, iBackslash );

	if( !sType.CompareNoCase( "HKEY_CLASSES_ROOT" ) )		key = HKEY_CLASSES_ROOT;
	else if( !sType.CompareNoCase( "HKEY_CURRENT_CONFIG" ) )	key = HKEY_CURRENT_CONFIG;
	else if( !sType.CompareNoCase( "HKEY_CURRENT_USER" ) )	key = HKEY_CURRENT_USER;
	else if( !sType.CompareNoCase( "HKEY_LOCAL_MACHINE" ) )	key = HKEY_LOCAL_MACHINE;
	else if( !sType.CompareNoCase( "HKEY_USERS" ) )			key = HKEY_USERS;
	else
	{
		LOG->Warn( "Invalid registry key: \"%s\" ", sIn.c_str() );
		return false;
	}

	sOut = sIn.substr( iBackslash+1 );

	return true;
}

/* Given a full key, eg. "HKEY_LOCAL_MACHINE\hardware\foo", open it and return it.
 * On error, return NULL. */
static HKEY OpenRegKey( const CString &sKey )
{
	CString sSubkey;
	HKEY hType;
	if( !GetRegKeyType(sKey, sSubkey, hType) )
		return NULL;

	HKEY hRetKey;
	LONG retval = RegOpenKeyEx( hType, sSubkey, 0, KEY_READ, &hRetKey );
	if ( retval != ERROR_SUCCESS )
	{
		LOG->Warn( werr_ssprintf(retval, "RegOpenKeyEx(%x,%s) error", hType, sSubkey.c_str()) );
		return NULL;
	}

	return hRetKey;
}

bool RegistryAccess::GetRegValue( const CString &sKey, const CString &sName, CString &sVal )
{
	HKEY hKey = OpenRegKey( sKey );
	if( hKey == NULL )
		return false;

	char sBuffer[MAX_PATH];
	DWORD iSize = sizeof(sBuffer);
	DWORD iType;
	LONG iRet = RegQueryValueEx( hKey, sName, NULL, &iType, (LPBYTE)sBuffer, &iSize );
	RegCloseKey( hKey );
	if( iRet != ERROR_SUCCESS )
		return false;

	/* Actually, CStrings are 8-bit clean, so we can accept any type of data.  Remove
	 * this if that becomes useful. */
	if( iType != REG_SZ && iType != REG_MULTI_SZ && iType != REG_EXPAND_SZ && iType != REG_BINARY )
		return false; /* type mismatch */

	if( iSize && (iType == REG_SZ || iType == REG_MULTI_SZ || iType == REG_EXPAND_SZ) )
		--iSize; /* remove nul terminator */

	sVal = CString( sBuffer, iSize );
	return true;
}

bool RegistryAccess::GetRegValue( const CString &sKey, const CString &sName, int &iVal )
{
	HKEY hKey = OpenRegKey( sKey );
	if( hKey == NULL )
		return false;

	DWORD iValue;
	DWORD iSize = sizeof(iValue);
	DWORD iType;
	LONG iRet = RegQueryValueEx( hKey, sName, NULL, &iType, (LPBYTE) &iValue, &iSize );
	RegCloseKey( hKey );
	if( iRet != ERROR_SUCCESS ) 
		return false;
	
	if( iType != REG_DWORD )
		return false; /* type mismatch */

	iVal = iValue;
	return true;
}

bool RegistryAccess::GetRegSubKeys( const CString &sKey, vector<CString> &lst, const CString &regex, bool bReturnPathToo )
{
	HKEY hKey = OpenRegKey( sKey );
	if( hKey == NULL )
		return false;

	Regex re(regex);

	bool bError = false;
	for( int index = 0; ; ++index )
	{
		FILETIME ft;
		char szBuffer[MAX_PATH];
		DWORD iSize = sizeof(szBuffer);
		LONG iRet = RegEnumKeyEx( hKey, index, szBuffer, &iSize, NULL, NULL, NULL, &ft);
		if( iRet == ERROR_NO_MORE_ITEMS )
			break;

		if( iRet != ERROR_SUCCESS )
		{
			LOG->Warn( werr_ssprintf(iRet, "GetRegSubKeys(%p,%i) error", hKey, index) );
			bError = true;
			break;
		}

		CString sStr( szBuffer, iSize );

		if( re.Compare(sStr) )
		{
			if( bReturnPathToo )
				sStr = sKey + "\\" + sStr;
			lst.push_back( sStr );
		}
	}

	RegCloseKey( hKey );

	return !bError;
}

/*
 * (c) 2004 Glenn Maynard
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
