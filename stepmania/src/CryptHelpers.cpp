#include "sha.h"
#include "files.h"
#include "hex.h"
#include "channels.h"
#include "rsa.h"
#include "md5.h"
#include <memory>

/* Pull in crypt library here. */
#ifdef _XBOX
	// FIXME
#elif defined _WINDOWS
	#ifdef DEBUG
		#pragma comment(lib, "crypto51/cryptlibd.lib")
	#else
		#pragma comment(lib, "crypto51/cryptlib.lib")
	#endif
#endif


using namespace CryptoPP;
using namespace std;

void RSASignFile(const char *privFilename, const char *messageFilename, const char *signatureFilename)
{
	FileSource privFile(privFilename, true, new HexDecoder);
	RSASSA_PKCS1v15_SHA_Signer priv(privFile);
	// RSASSA_PKCS1v15_SHA_Signer ignores the rng. Use a real RNG for other signature schemes!
	FileSource f(messageFilename, true, new SignerFilter(NullRNG(), priv, new HexEncoder(new FileSink(signatureFilename))));
}

bool RSAVerifyFile(const char *pubFilename, const char *messageFilename, const char *signatureFilename)
{
	FileSource pubFile(pubFilename, true, new HexDecoder);
	RSASSA_PKCS1v15_SHA_Verifier pub(pubFile);

	FileSource signatureFile(signatureFilename, true, new HexDecoder);
	if (signatureFile.MaxRetrievable() != pub.SignatureLength())
		return false;
	SecByteBlock signature(pub.SignatureLength());
	signatureFile.Get(signature, signature.size());

	VerifierFilter *verifierFilter = new VerifierFilter(pub);
	verifierFilter->Put(signature, pub.SignatureLength());
	FileSource f(messageFilename, true, verifierFilter);

	return verifierFilter->GetLastResult();
}

void DigestFile(const char *filename)
{
	MD5 md5;
	HashFilter md5Filter(md5);

	auto_ptr<ChannelSwitch> channelSwitch(new ChannelSwitch);
	channelSwitch->AddDefaultRoute(md5Filter);
	FileSource(filename, true, channelSwitch.release());

	HexEncoder encoder(new FileSink(cout), false);
	cout << "\nMD5: ";
	md5Filter.TransferTo(encoder);
}
