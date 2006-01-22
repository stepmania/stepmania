#include "global.h"
#include "CryptManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageFile.h"
#include "PrefsManager.h"
#include "RageFileManager.h"
#include "crypto/CryptMD5.h"

CryptManager*	CRYPTMAN	= NULL;	// global and accessable from anywhere in our program

static const RString PRIVATE_KEY_PATH = "Data/private.rsa";
static const RString PUBLIC_KEY_PATH = "Data/public.rsa";
static const RString ALTERNATE_PUBLIC_KEY_DIR = "Data/keys/";

#if !defined(HAVE_CRYPTOPP)
CryptManager::CryptManager() { }
CryptManager::~CryptManager() { }
void CryptManager::GenerateRSAKey( unsigned int keyLength, RString privFilename, RString pubFilename, RString seed ) { }
void CryptManager::SignFileToFile( RString sPath, RString sSignatureFile ) { }
bool CryptManager::VerifyFileWithFile( RString sPath, RString sSignatureFile, RString sPublicKeyFile ) { return true; }
bool CryptManager::VerifyFileWithFile( RString sPath, RString sSignatureFile )
{
	return true;
}

bool CryptManager::Verify( RString sPath, RString sSignature )
{
	return true;
}

#else

// crypt headers
#include "CryptHelpers.h"

static const int KEY_LENGTH = 1024;
#define MAX_SIGNATURE_SIZE_BYTES 1024	// 1 KB

CryptManager::CryptManager()
{
	//
	// generate keys if none are available
	//
	if( PREFSMAN->m_bSignProfileData )
	{
		if( !DoesFileExist(PRIVATE_KEY_PATH) || !DoesFileExist(PUBLIC_KEY_PATH) )
		{
			LOG->Warn( "Keys missing.  Generating new keys" );
			GenerateRSAKey( KEY_LENGTH, PRIVATE_KEY_PATH, PUBLIC_KEY_PATH, "aoksdjaksd" );
			FlushDirCache();
		}
	}
}

CryptManager::~CryptManager()
{

}

static bool WriteFile( RString sFile, RString sBuf )
{
	RageFile output;
	if( !output.Open(sFile, RageFile::WRITE) )
	{
		LOG->Warn( "WriteFile: opening %s failed: %s", sFile.c_str(), output.GetError().c_str() );
		return false;
	}
	
	if( output.Write(sBuf) == -1 || output.Flush() == -1 )
	{
		LOG->Warn( "WriteFile: writing %s failed: %s", sFile.c_str(), output.GetError().c_str() );
		output.Close();
		FILEMAN->Remove( sFile );
		return false;
	}

	return true;
}

void CryptManager::GenerateRSAKey( unsigned int keyLength, RString privFilename, RString pubFilename, RString seed )
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	RString sPubKey, sPrivKey;

	if( !CryptHelpers::GenerateRSAKey(keyLength, seed, sPubKey, sPrivKey) )
		return;

	if( !WriteFile(pubFilename, sPubKey) )
		return;

	if( !WriteFile(privFilename, sPrivKey) )
	{
		FILEMAN->Remove( privFilename );
		return;
	}
}

void CryptManager::SignFileToFile( RString sPath, RString sSignatureFile )
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	RString sPrivFilename = PRIVATE_KEY_PATH;
	RString sMessageFilename = sPath;
	if( sSignatureFile.empty() )
		sSignatureFile = sPath + SIGNATURE_APPEND;

	RString sPrivKey;
	if( !GetFileContents(sPrivFilename, sPrivKey) )
		return;

	if( !IsAFile(sMessageFilename) )
	{
		LOG->Trace( "SignFileToFile: \"%s\" doesn't exist", sMessageFilename.c_str() );
		return;
	}

	RageFile file;
	if( !file.Open(sMessageFilename) )
	{
		LOG->Warn( "VerifyFileWithFile: open(%s) failed: %s", sPath.c_str(), file.GetError().c_str() );
		return;
	}

	RString sSignature;
	RString sError;
	if( !CryptHelpers::SignFile(file, sPrivKey, sSignature, sError) )
	{
		LOG->Warn( "SignFileToFile failed: %s", sError.c_str() );
		return;
	}

	WriteFile( sSignatureFile, sSignature );
}

bool CryptManager::VerifyFileWithFile( RString sPath, RString sSignatureFile )
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	if( VerifyFileWithFile(sPath, sSignatureFile, PUBLIC_KEY_PATH) )
		return true;

	vector<RString> asKeys;
	GetDirListing( ALTERNATE_PUBLIC_KEY_DIR, asKeys, false, true );
	for( unsigned i = 0; i < asKeys.size(); ++i )
	{
		const RString &sKey = asKeys[i];
		LOG->Trace( "Trying alternate key \"%s\" ...", sKey.c_str() );

		if( VerifyFileWithFile(sPath, sSignatureFile, sKey) )
			return true;
	}

	return false;
}

bool CryptManager::VerifyFileWithFile( RString sPath, RString sSignatureFile, RString sPublicKeyFile )
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	RString sMessageFilename = sPath;
	if( sSignatureFile.empty() )
		sSignatureFile = sPath + SIGNATURE_APPEND;

	RString sPublicKey;
	if( !GetFileContents(sPublicKeyFile, sPublicKey) )
		return false;

	int iBytes = FILEMAN->GetFileSizeInBytes( sSignatureFile );
	if( iBytes > MAX_SIGNATURE_SIZE_BYTES )
		return false;

	RString sSignature;
	if( !GetFileContents(sSignatureFile, sSignature) )
		return false;

	RageFile file;
	if( !file.Open(sPath) )
	{
		LOG->Warn( "VerifyFileWithFile: open(%s) failed: %s", sPath.c_str(), file.GetError().c_str() );
		return false;
	}

	RString sError;
	if( !CryptHelpers::VerifyFile(file, sSignature, sPublicKey, sError) )
	{
		LOG->Warn( "VerifyFile(%s) failed: %s", sPath.c_str(), sError.c_str() );
		return false;
	}

	return true;
}

bool CryptManager::Verify( RString sPath, RString sSignature )
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	RString sPublicKeyFile = PUBLIC_KEY_PATH;
	RString sMessageFilename = sPath;

	RString sPublicKey;
	if( !GetFileContents(sPublicKeyFile, sPublicKey) )
		return false;

	RageFile file;
	if( !file.Open(sPath) )
	{
		LOG->Warn( "Verify: open(%s) failed: %s", sPath.c_str(), file.GetError().c_str() );
		return false;
	}

	RString sError;
	if( !CryptHelpers::VerifyFile(file, sSignature, sPublicKey, sError) )
	{
		LOG->Warn( "Verify(%s) failed: %s", sPath.c_str(), sError.c_str() );
		return false;
	}

	return true;
}
#endif

static RString BinaryToHex( const unsigned char *string, int iNumBytes )
{
	RString s;
	for( int i=0; i<iNumBytes; i++ )
	{
		unsigned val = string[i];
		s += ssprintf( "%x", val );
	}
	return s;
}

RString CryptManager::GetMD5( RString fn )
{
       struct MD5Context md5c;
       unsigned char digest[16];
       int iBytesRead;
       unsigned char buffer[1024];

       RageFile file;
       if( !file.Open( fn, RageFile::READ ) )
       {
               LOG->Warn( "GetMD5: Failed to open file '%s'", fn.c_str() );
               return RString();
       }

       MD5Init(&md5c);
       while( !file.AtEOF() && file.GetError().empty() )
       {
               iBytesRead = file.Read( buffer, sizeof(buffer) );
               MD5Update(&md5c, buffer, iBytesRead);
       }
       MD5Final(digest, &md5c);

       return BinaryToHex( digest, sizeof(digest) );
}

RString CryptManager::GetPublicKeyFileName()
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	return PUBLIC_KEY_PATH;
}

/*
 * (c) 2004 Chris Danford
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
