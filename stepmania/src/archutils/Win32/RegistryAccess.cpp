#include "global.h"
#include "RegistryAccess.h"
#include "RageLog.h"
#include "RageUtil.h"

/* Given "HKEY_LOCAL_MACHINE\hardware\foo", return "hardware\foo", and place
 * the HKEY_LOCAL_MACHINE constant in key. */
static bool GetRegKeyType( const CString &in, CString &out, HKEY &key )
{
	size_t backslash = in.find( '\\' );
	if( backslash == in.npos )
	{
		LOG->Warn( "Invalid registry key: \"%s\" ", in.c_str() );
		return false;
	}

	CString type = in.substr( 0, backslash );

	if( !type.CompareNoCase( "HKEY_CLASSES_ROOT" ) )		key = HKEY_CLASSES_ROOT;
	else if( !type.CompareNoCase( "HKEY_CURRENT_CONFIG" ) )	key = HKEY_CURRENT_CONFIG;
	else if( !type.CompareNoCase( "HKEY_CURRENT_USER" ) )	key = HKEY_CURRENT_USER;
	else if( !type.CompareNoCase( "HKEY_LOCAL_MACHINE" ) )	key = HKEY_LOCAL_MACHINE;
	else if( !type.CompareNoCase( "HKEY_USERS" ) )			key = HKEY_USERS;
	else
	{
		LOG->Warn( "Invalid registry key: \"%s\" ", in.c_str() );
		return false;
	}

	out = in.substr( backslash+1 );

	return true;
}

/* Given a full key, eg. "HKEY_LOCAL_MACHINE\hardware\foo", open it and return it.
 * On error, return NULL. */
static HKEY OpenRegKey( const CString &key )
{
	CString subkey;
	HKEY type;
	if( !GetRegKeyType( key, subkey, type ) )
		return NULL;

	HKEY retkey;
	LONG retval = RegOpenKeyEx( type, subkey, 0, KEY_READ, &retkey );
	if ( retval != ERROR_SUCCESS )
	{
		LOG->Warn( werr_ssprintf(retval, "RegOpenKeyEx(%i,%s) error", type, subkey.c_str()) );
		return NULL;
	}

	return retkey;
}

bool GetRegValue( const CString &key, const CString &sName, CString &val )
{
	HKEY hkey = OpenRegKey( key );
	if( hkey == NULL )
		return false;

	char sBuffer[MAX_PATH];
	DWORD nSize = sizeof(sBuffer);
	DWORD Type;
	LONG ret = RegQueryValueEx( hkey, sName, NULL, &Type, (LPBYTE)sBuffer, &nSize );
	RegCloseKey( hkey );
	if( ret != ERROR_SUCCESS )
		return false;

	/* Actually, CStrings are 8-bit clean, so we can accept any type of data.  Remove
	 * this if that becomes useful. */
	if( Type != REG_SZ && Type != REG_MULTI_SZ && Type != REG_EXPAND_SZ && Type != REG_BINARY )
		return false; /* type mismatch */

	if( nSize && (Type == REG_SZ || Type == REG_MULTI_SZ || Type == REG_EXPAND_SZ) )
		--nSize; /* remove nul terminator */

	val = CString( sBuffer, nSize );
	return true;
}

bool GetRegValue( const CString &key, CString sName, int &val )
{
	HKEY hkey = OpenRegKey( key );
	if( hkey == NULL )
		return false;

	DWORD value;
	DWORD nSize = sizeof(value);
	DWORD Type;
	LONG ret = RegQueryValueEx( hkey, sName, NULL, &Type, (LPBYTE) &value, &nSize );
	RegCloseKey( hkey );
	if( ret != ERROR_SUCCESS ) 
		return false;
	
	if( Type != REG_DWORD )
		return false; /* type mismatch */

	val = value;
	return true;
}

bool GetRegSubKeys( const CString &key, vector<CString> &lst, const CString &regex, bool bReturnPathToo )
{
	HKEY hkey = OpenRegKey( key );
	if ( hkey == NULL )
		return false;

	Regex re(regex);

	bool bError = false;
	for( int index = 0; ; ++index )
	{
		FILETIME ft;
		char szBuffer[MAX_PATH];
		DWORD nSize = sizeof(szBuffer);
		LONG ret = RegEnumKeyEx( hkey, index, szBuffer, &nSize, NULL, NULL, NULL, &ft);
		if( ret == ERROR_NO_MORE_ITEMS )
			break;

		if( ret != ERROR_SUCCESS )
		{
			LOG->Warn( werr_ssprintf(ret, "GetRegSubKeys(%p,%i) error", hkey, index) );
			bError = true;
			break;
		}

		CString str( szBuffer, nSize );

		if( re.Compare(str) )
		{
			if( bReturnPathToo )
				str = key + "\\" + str;
			lst.push_back( str );
		}
	}

	RegCloseKey( hkey );

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
