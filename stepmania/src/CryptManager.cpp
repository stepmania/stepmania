#include "global.h"
#include "CryptManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "PrefsManager.h"

// crypt headers
#include "CryptHelpers.h"
#include "crypto51/sha.h"
#include "crypto51/rsa.h"
#include "crypto51/osrng.h"
#include "crypto/CryptMD5.h"
#include <memory>

using namespace CryptoPP;
using namespace std;

#ifdef WIN32
//	#ifdef DEBUG
//		#pragma comment(lib, "crypto51/Debug/cryptlib.lib")
//	#else
		#pragma comment(lib, "crypto51/Release/cryptlib.lib")
//	#endif
#endif

static const CString PRIVATE_KEY_PATH = "Data/private.key.rsa";
static const CString PUBLIC_KEY_PATH = "Data/public.key.rsa";
static const int KEY_LENGTH = 1024;

CryptManager*	CRYPTMAN	= NULL;	// global and accessable from anywhere in our program

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

void CryptManager::GenerateRSAKey( unsigned int keyLength, CString privFilename, CString pubFilename, CString seed )
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	AutoSeededRandomPool rng;

	RSAES_OAEP_SHA_Decryptor priv(rng, keyLength);
	RageFileSink privFile(privFilename);
	priv.DEREncode(privFile);
	privFile.MessageEnd();

	RSAES_OAEP_SHA_Encryptor pub(priv);
	RageFileSink pubFile(pubFilename);
	pub.DEREncode(pubFile);
	pubFile.MessageEnd();
}

void CryptManager::SignFileToFile( CString sPath, CString sSignatureFile )
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	CString sPrivFilename = PRIVATE_KEY_PATH;
	CString sMessageFilename = sPath;
	if( sSignatureFile.empty() )
		sSignatureFile = sPath + SIGNATURE_APPEND;

	if( !IsAFile(sPrivFilename) )
		return;

	if( !IsAFile(sMessageFilename) )
		return;

	try {
		RageFileSource privFile(sPrivFilename, true);
		RSASSA_PKCS1v15_SHA_Signer priv(privFile);
		AutoSeededRandomPool rng;

		RageFileSource f(sMessageFilename, true, new SignerFilter(rng, priv, new RageFileSink(sSignatureFile)));
	} catch( const CryptoPP::Exception &s ) {
		LOG->Warn( "SignFileToFile failed: %s", s.what() );
	}
}

bool CryptManager::VerifyFileWithFile( CString sPath, CString sSignatureFile )
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	CString sPubFilename = PUBLIC_KEY_PATH;
	CString sMessageFilename = sPath;
	if( sSignatureFile.empty() )
		sSignatureFile = sPath + SIGNATURE_APPEND;

	if( !IsAFile(sPubFilename) )
		return false;

	if( !IsAFile(sSignatureFile) )
		return false;

	try {
		RageFileSource pubFile(sPubFilename, true);
		RSASSA_PKCS1v15_SHA_Verifier pub(pubFile);

		RageFileSource signatureFile(sSignatureFile, true);
		if (signatureFile.MaxRetrievable() != pub.SignatureLength())
			return false;
		SecByteBlock signature(pub.SignatureLength());
		signatureFile.Get(signature, signature.size());

		VerifierFilter *verifierFilter = new VerifierFilter(pub);
		verifierFilter->Put(signature, pub.SignatureLength());
		RageFileSource f(sMessageFilename, true, verifierFilter);

		return verifierFilter->GetLastResult();
	} catch( const CryptoPP::Exception &s ) {
		LOG->Warn( "VerifyFileWithFile(%s,%s) failed: %s", sPath.c_str(), sSignatureFile.c_str(), s.what() );
		return false;
	}
}

bool CryptManager::Verify( CString sPath, CString sSignature )
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	CString sPubFilename = PUBLIC_KEY_PATH;
	CString sMessageFilename = sPath;

	if( !IsAFile(sPubFilename) )
		return false;

	try {
		RageFileSource pubFile(sPubFilename, true);
		RSASSA_PKCS1v15_SHA_Verifier pub(pubFile);

		StringSource signatureFile(sSignature, true);
		if (signatureFile.MaxRetrievable() != pub.SignatureLength())
			return false;
		SecByteBlock signature(pub.SignatureLength());
		signatureFile.Get(signature, signature.size());

		VerifierFilter *verifierFilter = new VerifierFilter(pub);
		verifierFilter->Put(signature, pub.SignatureLength());
		RageFileSource f(sMessageFilename, true, verifierFilter);

		return verifierFilter->GetLastResult();
	} catch( const CryptoPP::Exception &s ) {
		LOG->Warn( "Verify(%s,sig) failed: %s", sPath.c_str(), s.what() );
		return false;
	}
}

static CString BinaryToHex( const unsigned char *string, int iNumBytes )
{
	CString s;
	for( int i=0; i<iNumBytes; i++ )
	{
		unsigned val = string[i];
		s += ssprintf( "%x", val );
	}
	return s;
}

CString CryptManager::GetMD5( CString fn )
{
       struct MD5Context md5c;
       unsigned char digest[16];
       int iBytesRead;
       unsigned char buffer[1024];

       RageFile file;
       if( !file.Open( fn, RageFile::READ ) )
       {
               LOG->Warn( "GetMD5: Failed to open file '%s'", fn.c_str() );
               return "";
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

CString CryptManager::GetPublicKeyFileName()
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	return PUBLIC_KEY_PATH;
}
