#include "global.h"
#include "CryptManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageFile.h"
#include "PrefsManager.h"
#include "RageFileManager.h"

#include "libtomcrypt/src/headers/tomcrypt.h"

CryptManager*	CRYPTMAN	= NULL;	// global and accessable from anywhere in our program

static const RString PRIVATE_KEY_PATH = "Data/private.rsa";
static const RString PUBLIC_KEY_PATH = "Data/public.rsa";
static const RString ALTERNATE_PUBLIC_KEY_DIR = "Data/keys/";

static bool HashFile( RageFile &f, unsigned char buf_hash[20], int iHash )
{
	hash_state hash;
	int iRet = hash_descriptor[iHash].init( &hash );
	ASSERT_M( iRet == CRYPT_OK, error_to_string(iRet) );

	RString s;
	while( !f.AtEOF() )
	{
		if( f.Read(s, 1024*4) == -1 )
		{
			LOG->Warn( "Error reading %s: %s", f.GetPath().c_str(), f.GetError().c_str() );
			hash_descriptor[iHash].done( &hash, buf_hash );
			return false;
		}

		iRet = hash_descriptor[iHash].process( &hash, (const unsigned char *) s.data(), s.size() );
		ASSERT_M( iRet == CRYPT_OK, error_to_string(iRet) );
	}

	iRet = hash_descriptor[iHash].done( &hash, buf_hash );
	ASSERT_M( iRet == CRYPT_OK, error_to_string(iRet) );

	return true;
}

#if defined(DISABLE_CRYPTO)
CryptManager::CryptManager() { }
CryptManager::~CryptManager() { }
void CryptManager::GenerateRSAKey( unsigned int keyLength, RString privFilename, RString pubFilename ) { }
void CryptManager::SignFileToFile( RString sPath, RString sSignatureFile ) { }
bool CryptManager::VerifyFileWithFile( RString sPath, RString sSignatureFile, RString sPublicKeyFile ) { return true; }
bool CryptManager::VerifyFileWithFile( RString sPath, RString sSignatureFile )
{
	return true;
}

void CryptManager::GetRandomBytes( void *pData, int iBytes )
{
	uint8_t *pBuf = (uint8_t *) pData;
	while( iBytes-- )
		*pBuf++ = (uint8_t) RandomInt( 256 );
}

#else

static const int KEY_LENGTH = 1024;
#define MAX_SIGNATURE_SIZE_BYTES 1024	// 1 KB




/*
 openssl genrsa -out testing -outform DER
 openssl rsa -in testing -out testing2 -outform DER
 openssl rsa -in testing -out testing2 -pubout -outform DER
 
 openssl pkcs8 -inform DER -outform DER -nocrypt -in private.rsa -out private.der
 * 
 */


class PRNGWrapper
{
public:
	PRNGWrapper( const struct ltc_prng_descriptor *pPRNGDescriptor )
	{
		m_iPRNG = register_prng( pPRNGDescriptor );
		ASSERT( m_iPRNG >= 0 );

		int iRet = rng_make_prng( 128, m_iPRNG, &m_PRNG, NULL );
		ASSERT_M( iRet == CRYPT_OK, error_to_string(iRet) );
	}

	~PRNGWrapper()
	{
		if( m_iPRNG != -1 )
			prng_descriptor[m_iPRNG].done( &m_PRNG );
	}

	void AddEntropy( const void *pData, int iSize )
	{
		int iRet = prng_descriptor[m_iPRNG].add_entropy( (const unsigned char *) pData, iSize, &m_PRNG );
		ASSERT_M( iRet == CRYPT_OK, error_to_string(iRet) );

		iRet = prng_descriptor[m_iPRNG].ready( &m_PRNG );
		ASSERT_M( iRet == CRYPT_OK, error_to_string(iRet) );
	}

	void AddRandomEntropy()
	{
		unsigned char buf[256];
		int iRet = rng_get_bytes( buf, sizeof(buf), NULL );
		ASSERT( iRet == sizeof(buf) );

		AddEntropy( buf, sizeof(buf) );
	}

	int m_iPRNG;
	prng_state m_PRNG;
};

static PRNGWrapper *g_pPRNG = NULL;

class RSAKeyWrapper
{
public:
	RSAKeyWrapper()
	{
		memset( &m_Key, 0, sizeof(m_Key) );
	}

	~RSAKeyWrapper()
	{
		Unload();
	}

	void Unload()
	{
		rsa_free( &m_Key );
	}

	void Generate( PRNGWrapper &prng, int iKeyLenBits )
	{
		Unload();

		int iRet = rsa_make_key( &prng.m_PRNG, prng.m_iPRNG, iKeyLenBits / 8, 65537, &m_Key );
		ASSERT( iRet == CRYPT_OK );
	}

	bool Load( const RString &sKey )
	{
		Unload();

		int iRet = rsa_import( (const unsigned char *) sKey.data(), sKey.size(), &m_Key );
		if( iRet != CRYPT_OK )
		{
			memset( &m_Key, 0, sizeof(m_Key) );
			LOG->Warn( "Error loading RSA Key: %s", error_to_string(iRet) );
			return false;
		}

		return true;
	}
	
	rsa_key m_Key;
};

CryptManager::CryptManager()
{
	ltc_mp = ltm_desc;

	g_pPRNG = new PRNGWrapper( &yarrow_desc );

	if( !PREFSMAN->m_bSignProfileData )
		return;
	//
	// generate keys if none are available
	//
	bool bGenerate = false;
	RSAKeyWrapper key;
	RString sKey;
	if( !DoesFileExist(PRIVATE_KEY_PATH) ||
	    !GetFileContents(PRIVATE_KEY_PATH, sKey) ||
	    !key.Load(sKey) )
		bGenerate = true;
	if( !DoesFileExist(PUBLIC_KEY_PATH) ||
	    !GetFileContents(PUBLIC_KEY_PATH, sKey) ||
	    !key.Load(sKey) )
		bGenerate = true;

	if( bGenerate )
	{
		LOG->Warn( "Keys missing or failed to load.  Generating new keys" );
		GenerateRSAKey( KEY_LENGTH, PRIVATE_KEY_PATH, PUBLIC_KEY_PATH );
		FlushDirCache();
	}
}

CryptManager::~CryptManager()
{
	SAFE_DELETE( g_pPRNG );
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

void CryptManager::GenerateRSAKey( unsigned int keyLength, RString privFilename, RString pubFilename )
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	int iRet;

	rsa_key key;
	iRet = rsa_make_key( &g_pPRNG->m_PRNG, g_pPRNG->m_iPRNG, keyLength / 8, 65537, &key );
	if( iRet != CRYPT_OK )
	{
		LOG->Warn( "GenerateRSAKey(%i) error: %s", keyLength, error_to_string(iRet) );
		return;
	}

	unsigned char buf[1024];
	unsigned long iSize = sizeof(buf);
	iRet = rsa_export( buf, &iSize, PK_PUBLIC, &key );
	if( iRet != CRYPT_OK )
	{
		LOG->Warn( "Export error: %s", error_to_string(iRet) );
		return;
	}

	RString sPubKey( (const char *) buf, iSize );

	iSize = sizeof(buf);
	iRet = rsa_export( buf, &iSize, PK_PRIVATE, &key );
	if( iRet != CRYPT_OK )
	{
		LOG->Warn( "Export error: %s", error_to_string(iRet) );
		return;
	}

	RString sPrivKey( (const char *) buf, iSize );

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
		LOG->Warn( "SignFileToFile: open(%s) failed: %s", sPath.c_str(), file.GetError().c_str() );
		return;
	}

	RSAKeyWrapper key;
	if( !key.Load(sPrivKey) )
		return;

	int iHash = register_hash( &sha1_desc );
	ASSERT( iHash >= 0 );

	unsigned char buf_hash[20];
	if( !HashFile(file, buf_hash, iHash) )
		return;

	unsigned char signature[256];
	unsigned long signature_len = sizeof(signature);

	int iRet = rsa_sign_hash_ex(
			buf_hash, sizeof(buf_hash),
			signature, &signature_len,
			LTC_PKCS_1_V1_5, &g_pPRNG->m_PRNG, g_pPRNG->m_iPRNG, iHash,
			0, &key.m_Key);
	if( iRet != CRYPT_OK )
	{
		LOG->Warn( "SignFileToFile error: %s", error_to_string(iRet) );
		return;
	}

	RString sSignature( (const char *) signature, signature_len );
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

	return Verify( sPath, sSignature, sPublicKey );
}

bool CryptManager::Verify( RString sPath, RString sSignature, RString sPublicKey )
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	RageFile file;
	if( !file.Open(sPath) )
	{
		LOG->Warn( "Verify: open(%s) failed: %s", sPath.c_str(), file.GetError().c_str() );
		return false;
	}

	RSAKeyWrapper key;
	if( !key.Load(sPublicKey) )
		return false;

	int iHash = register_hash( &sha1_desc );
	ASSERT( iHash >= 0 );

	unsigned char buf_hash[20];
	HashFile( file, buf_hash, iHash );

	int iMatch;
	int iRet = rsa_verify_hash_ex( (const unsigned char *) sSignature.data(), sSignature.size(),
			buf_hash, sizeof(buf_hash),
			LTC_PKCS_1_EMSA, iHash, 0, &iMatch, &key.m_Key );

	if( iRet != CRYPT_OK )
	{
		LOG->Warn( "Verify(%s) failed: %s", sPath.c_str(), error_to_string(iRet) );
		return false;
	}

	if( !iMatch )
	{
		LOG->Warn( "Verify(%s) failed: signature mismatch", sPath.c_str() );
		return false;
	}

	return true;
}

void CryptManager::GetRandomBytes( void *pData, int iBytes )
{
	int iRet = prng_descriptor[g_pPRNG->m_iPRNG].read( (unsigned char *) pData, iBytes, &g_pPRNG->m_PRNG );
	ASSERT( iRet == iBytes );
}
#endif

RString CryptManager::GetMD5ForFile( RString fn )
{
	RageFile file;
	if( !file.Open( fn, RageFile::READ ) )
	{
		LOG->Warn( "GetMD5: Failed to open file '%s'", fn.c_str() );
		return RString();
	}
	int iHash = register_hash( &md5_desc );
	ASSERT( iHash >= 0 );

	unsigned char digest[16];
	HashFile( file, digest, iHash );

	return BinaryToHex( digest, sizeof(digest) );
}

RString CryptManager::GetMD5ForString( RString sData )
{
	unsigned char digest[16];

	int iHash = register_hash( &md5_desc );

	hash_state hash;
	int iRet = hash_descriptor[iHash].init( &hash );
	iRet = hash_descriptor[iHash].process( &hash, (const unsigned char *) sData.data(), sData.size() );
	iRet = hash_descriptor[iHash].done( &hash, digest );

	return BinaryToHex( digest, sizeof(digest) );
}

RString CryptManager::GetPublicKeyFileName()
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	return PUBLIC_KEY_PATH;
}

/*
 * (c) 2004-2007 Chris Danford, Glenn Maynard
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
