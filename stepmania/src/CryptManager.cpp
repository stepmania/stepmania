#include "global.h"
#include "CryptManager.h"
#include "RageUtil.h"
#include "CryptHelpers.h"
#include "RageLog.h"

// crypt headers
#include "sha.h"
#include "hex.h"
#include "channels.h"
#include "rsa.h"
#include "md5.h"
#include "osrng.h"
#include <memory>

using namespace CryptoPP;
using namespace std;

static const CString SIGNATURE_POSPEND = ".sig.rsa";
static const CString PRIVATE_KEY_PATH = "private.rsa";
static const CString PUBLIC_KEY_PATH = "public.rsa";
static const int KEY_LENGTH = 1024;

CryptManager*	CRYPTMAN	= NULL;	// global and accessable from anywhere in our program

CryptManager::CryptManager()
{
	//
	// generate keys if none are available
	//
	if( !DoesFileExist(PRIVATE_KEY_PATH) || !DoesFileExist(PUBLIC_KEY_PATH) )
	{
		LOG->Warn( "Keys missing.  Generating new keys" );
		GenerateRSAKey( KEY_LENGTH, PRIVATE_KEY_PATH, PUBLIC_KEY_PATH, "aoksdjaksd" );
		FlushDirCache();
	}
}

CryptManager::~CryptManager()
{

}

void CryptManager::GenerateRSAKey(unsigned int keyLength, const char *privFilename, const char *pubFilename, const char *seed )
{
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
	CString sPrivFilename = PRIVATE_KEY_PATH;
	CString sMessageFilename = sPath;;
	CString sSignatureFilename = sPath + SIGNATURE_POSPEND;

	ASSERT( IsAFile(sPrivFilename) );

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
	CString sPubFilename = PUBLIC_KEY_PATH;
	CString sMessageFilename = sPath;;
	CString sSignatureFilename = sPath + SIGNATURE_POSPEND;

	ASSERT( IsAFile(sPubFilename) );

	if( !IsAFile(sSignatureFilename) )
		return false;

	// CAREFUL: These classes can throw all kinds of exceptions.  Should this
	// be wrapped in a try catch?

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

void CryptManager::DigestFile(const char *filename)
{
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
