#include "global.h"
#include "RegistryAccess.h"
#include "RageLog.h"
#include "RageUtil.h"

/* Given "HKEY_LOCAL_MACHINE\hardware\foo", return "hardware\foo", and place
 * the HKEY_LOCAL_MACHINE constant in key. */
static bool GetRegKeyType( const CString &in, CString &out, HKEY &key )
{
	unsigned backslash = in.find( '\\' );
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
