#include "global.h"
#include "CryptManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "PrefsManager.h"


static const CString SIGNATURE_APPEND = ".sig.rsa";
static const CString PRIVATE_KEY_PATH = "private.key.rsa";
static const CString PUBLIC_KEY_PATH = "public.key.rsa";
static const int KEY_LENGTH = 1024;

CryptManager*	CRYPTMAN	= NULL;	// global and accessable from anywhere in our program

#if 1
CryptManager::CryptManager() { }
CryptManager::~CryptManager() { }
void CryptManager::GenerateRSAKey(unsigned int keyLength, const char *privFilename, const char *pubFilename, const char *seed ) { }
void CryptManager::SignFile( CString sPath ) { }
bool CryptManager::VerifyFile( CString sPath ) { return true; }
CString CryptManager::GetFileSignature( CString sPath ) { return ""; }
bool CryptManager::VerifyFile( CString sPath, CString sSignature ) { return true; }
void CryptManager::DigestFile(const char *filename) { }
#else

// crypt headers
#include "CryptHelpers.h"
#include "crypto51/sha.h"
#include "crypto51/hex.h"
#include "crypto51/channels.h"
#include "crypto51/rsa.h"
#include "crypto51/md5.h"
#include "crypto51/osrng.h"
#include <memory>

using namespace CryptoPP;
using namespace std;

static const CString SIGNATURE_POSPEND = ".sig.rsa";
static const CString PRIVATE_KEY_PATH = "Data/private.key.rsa";
static const CString PUBLIC_KEY_PATH = "Data/public.key.rsa";
static const int KEY_LENGTH = 1024;

CryptManager*	CRYPTMAN	= NULL;	// global and accessable from anywhere in our program

CryptManager::CryptManager()
{
	//
	// generate keys if none are available
	//
	/* This is crashing in crypto51/integer.cpp CryptoPP::RecursiveInverseModPower2
	 * in Linux. -glenn */
//	if( PREFSMAN->m_bSignProfileData )
//	{
//		if( !DoesFileExist(PRIVATE_KEY_PATH) || !DoesFileExist(PUBLIC_KEY_PATH) )
//		{
//			LOG->Warn( "Keys missing.  Generating new keys" );
//			GenerateRSAKey( KEY_LENGTH, PRIVATE_KEY_PATH, PUBLIC_KEY_PATH, "aoksdjaksd" );
//			FlushDirCache();
//		}
//	}
}

CryptManager::~CryptManager()
{

}

void CryptManager::GenerateRSAKey(unsigned int keyLength, const char *privFilename, const char *pubFilename, const char *seed )
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	AutoSeededRandomPool rng;

	RSAES_OAEP_SHA_Decryptor priv(rng, keyLength);
	HexEncoder privFile(new RageFileSink(privFilename));
	priv.DEREncode(privFile);
	privFile.MessageEnd();

	RSAES_OAEP_SHA_Encryptor pub(priv);
	HexEncoder pubFile(new RageFileSink(pubFilename));
	pub.DEREncode(pubFile);
	pubFile.MessageEnd();
}

void CryptManager::SignFile( CString sPath )
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	CString sPrivFilename = PRIVATE_KEY_PATH;
	CString sMessageFilename = sPath;;
	CString sSignatureFilename = sPath + SIGNATURE_APPEND;

	if( !IsAFile(sPrivFilename) )
		return;

	if( !IsAFile(sMessageFilename) )
		return;

	// CAREFUL: These classes can throw all kinds of exceptions.  Should this
	// be wrapped in a try catch?

	RageFileSource privFile(sPrivFilename, true, new HexDecoder);
	RSASSA_PKCS1v15_SHA_Signer priv(privFile);
	AutoSeededRandomPool rng;
	RageFileSource f(sMessageFilename, true, new SignerFilter(rng, priv, new HexEncoder(new RageFileSink(sSignatureFilename))));
}

bool CryptManager::VerifyFile( CString sPath )
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	CString sPubFilename = PUBLIC_KEY_PATH;
	CString sMessageFilename = sPath;;
	CString sSignatureFilename = sPath + SIGNATURE_APPEND;

	if( !IsAFile(sPubFilename) )
		return false;

	if( !IsAFile(sSignatureFilename) )
		return false;

	// CAREFUL: These classes can throw all kinds of exceptions.  Should this
	// be wrapped in a try catch?

	/* XXX: This is opening sPubFilename for RageFile::WRITE instead of READ. */
	RageFileSource pubFile(sPubFilename, true, new HexDecoder);
	RSASSA_PKCS1v15_SHA_Verifier pub(pubFile);

	RageFileSource signatureFile(sSignatureFilename, true, new HexDecoder);
	if (signatureFile.MaxRetrievable() != pub.SignatureLength())
		return false;
	SecByteBlock signature(pub.SignatureLength());
	signatureFile.Get(signature, signature.size());

	VerifierFilter *verifierFilter = new VerifierFilter(pub);
	verifierFilter->Put(signature, pub.SignatureLength());
	RageFileSource f(sMessageFilename, true, verifierFilter);

	return verifierFilter->GetLastResult();
}

CString CryptManager::GetFileSignature( CString sPath )
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	CString sPrivFilename = PRIVATE_KEY_PATH;
	CString sMessageFilename = sPath;

	if( !IsAFile(sPrivFilename) )
		return "";

	if( !IsAFile(sMessageFilename) )
		return "";

	// CAREFUL: These classes can throw all kinds of exceptions.  Should this
	// be wrapped in a try catch?

	RageFileSource privFile(sPrivFilename, true, new HexDecoder);
	RSASSA_PKCS1v15_SHA_Signer priv(privFile);
	AutoSeededRandomPool rng;
	CString sSignature;
	RageFileSource f(sMessageFilename, true, new SignerFilter(rng, priv, new HexEncoder(new StringSink(sSignature))));
	return sSignature;
}

bool CryptManager::VerifyFile( CString sPath, CString sSignature )
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	CString sPubFilename = PUBLIC_KEY_PATH;
	CString sMessageFilename = sPath;

	if( !IsAFile(sPubFilename) )
		return false;

	// CAREFUL: These classes can throw all kinds of exceptions.  Should this
	// be wrapped in a try catch?

	RageFileSource pubFile(sPubFilename, true, new HexDecoder);
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
}


void CryptManager::DigestFile(const char *filename)
{
	ASSERT( PREFSMAN->m_bSignProfileData );

//	MD5 md5;
//	HashFilter md5Filter(md5);
//
//	auto_ptr<ChannelSwitch> channelSwitch(new ChannelSwitch);
//	channelSwitch->AddDefaultRoute(md5Filter);
//	RageFileSource(filename, true, channelSwitch.release());
//
//	HexEncoder encoder(new RageFileSink(cout), false);
//	cout << "\nMD5: ";
//	md5Filter.TransferTo(encoder);
}
#endif

CString CryptManager::GetPublicKeyFileName()
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	return PUBLIC_KEY_PATH;
}
