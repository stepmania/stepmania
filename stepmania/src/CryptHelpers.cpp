#include "sha.h"
#include "files.h"
#include "hex.h"
#include "channels.h"
#include "rsa.h"
#include "md5.h"
#include "randpool.h"
#include <memory>

using namespace CryptoPP;
using namespace std;

void GenerateRSAKey(unsigned int keyLength, const char *privFilename, const char *pubFilename, const char *seed)
{
	RandomPool randPool;
	randPool.Put((byte *)seed, strlen(seed));

	RSAES_OAEP_SHA_Decryptor priv(randPool, keyLength);
	HexEncoder privFile(new FileSink(privFilename));
	priv.DEREncode(privFile);
	privFile.MessageEnd();

	RSAES_OAEP_SHA_Encryptor pub(priv);
	HexEncoder pubFile(new FileSink(pubFilename));
	pub.DEREncode(pubFile);
	pubFile.MessageEnd();
}

void RSASignFile(const char *privFilename, const char *messageFilename, const char *signatureFilename)
{
	FileSource privFile(privFilename, true, new HexDecoder);
	RSASSA_PKCS1v15_SHA_Signer priv(privFile);
	RandomNumberGenerator &rng = RandomPool();
	FileSource f(messageFilename, true, new SignerFilter(rng, priv, new HexEncoder(new FileSink(signatureFilename))));
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
