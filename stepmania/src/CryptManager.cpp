#include "global.h"
#include "CryptManager.h"
#include "RageUtil.h"
#include "CryptHelpers.h"

// crypt headers
#include "sha.h"
#include "hex.h"
#include "channels.h"
#include "rsa.h"
#include "md5.h"
#include "randpool.h"
#include <memory>

using namespace CryptoPP;
using namespace std;

static const CString SIGNATURE_POSPEND = ".sig.rsa";
static const CString PRIVATE_KEY_PATH = "private.rsa";
static const CString PUBLIC_KEY_PATH = "public.rsa";









void CryptManager::GenerateRSAKey(unsigned int keyLength, const char *privFilename, const char *pubFilename, const char *seed)
{
	RandomPool randPool;
	randPool.Put((byte *)seed, strlen(seed));

	RSAES_OAEP_SHA_Decryptor priv(randPool, keyLength);
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

	if( !IsAFile(sMessageFilename) || !IsAFile(sPrivFilename) )
		return;

	// CAREFUL: These classes can throw all kinds of exceptions.  Should this
	// be wrapped in a try catch?

	// TODO: use RageFile here
	RageFileSource privFile(sPrivFilename, true, new HexDecoder);
	RSASSA_PKCS1v15_SHA_Signer priv(privFile);
	RandomNumberGenerator &rng = RandomPool();
	RageFileSource f(sMessageFilename, true, new SignerFilter(rng, priv, new HexEncoder(new RageFileSink(sSignatureFilename))));
}

bool CryptManager::VerifyFile( CString sPath )
{
	CString sPubFilename = PUBLIC_KEY_PATH;
	CString sMessageFilename = sPath;;
	CString sSignatureFilename = sPath + SIGNATURE_POSPEND;

	if( !IsAFile(sSignatureFilename) || !IsAFile(sPubFilename) )
		return false;

	// CAREFUL: These classes can throw all kinds of exceptions.  Should this
	// be wrapped in a try catch?

	// TODO: use RageFile here
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
