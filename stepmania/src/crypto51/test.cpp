// test.cpp - written and placed in the public domain by Wei Dai

#include "sha.h"
#include "files.h"
#include "rng.h"
#include "rsa.h"
#include "randpool.h"

#include <iostream>
#include <time.h>

#ifdef CRYPTOPP_WIN32_AVAILABLE
#include <windows.h>
#endif

#if (_MSC_VER >= 1000)
#include <crtdbg.h>		// for the debug heap
#endif

#if defined(__MWERKS__) && defined(macintosh)
#include <console.h>
#endif

USING_NAMESPACE(CryptoPP)
USING_NAMESPACE(std)

void GenerateRSAKey(unsigned int keyLength, const char *privFilename, const char *pubFilename, const char *seed);
string RSAEncryptString(const char *pubFilename, const char *seed, const char *message);
string RSADecryptString(const char *privFilename, const char *ciphertext);
void RSASignFile(const char *privFilename, const char *messageFilename, const char *signatureFilename);
bool RSAVerifyFile(const char *pubFilename, const char *messageFilename, const char *signatureFilename);

int (*AdhocTest)(int argc, char *argv[]) = NULL;

#ifdef __BCPLUSPLUS__
int cmain(int argc, char *argv[])
#elif defined(_MSC_VER)
int __cdecl main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
#ifdef _CRTDBG_LEAK_CHECK_DF
	// Turn on leak-checking
	int tempflag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
	tempflag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag( tempflag );
#endif

#if defined(__MWERKS__) && defined(macintosh)
	argc = ccommand(&argv);
#endif

	try
	{
		std::string command, executableName, edcFilename;

		if (argc < 2)
			command = 'h';
		else
			command = argv[1];

		switch (command[0])
		{
		case 'g':
		  {
			char seed[1024], privFilename[128], pubFilename[128];
			unsigned int keyLength;

			cout << "Key length in bits: ";
			cin >> keyLength;

			cout << "\nSave private key to file: ";
			cin >> privFilename;

			cout << "\nSave public key to file: ";
			cin >> pubFilename;

			cout << "\nRandom Seed: ";
			ws(cin);
			cin.getline(seed, 1024);

			GenerateRSAKey(keyLength, privFilename, pubFilename, seed);
			return 0;
		  }
		case 'r':
		  {
			switch (argv[1][1])
			{
			case 's':
				RSASignFile(argv[2], argv[3], argv[4]);
				return 0;
			case 'v':
			  {
				bool verified = RSAVerifyFile(argv[2], argv[3], argv[4]);
				cout << (verified ? "valid signature" : "invalid signature") << endl;
				return 0;
			  }
			}
		  }
		default:
			FileSource usage("usage.dat", true, new FileSink(cout));
			return 1;
		}
	}
	catch(CryptoPP::Exception &e)
	{
		cout << "\nCryptoPP::Exception caught: " << e.what() << endl;
		return -1;
	}
	catch(std::exception &e)
	{
		cout << "\nstd::exception caught: " << e.what() << endl;
		return -2;
	}
}


RandomPool & GlobalRNG()
{
	static RandomPool randomPool;
	return randomPool;
}

void GenerateRSAKey(unsigned int keyLength, const char *privFilename, const char *pubFilename, const char *seed)
{
	RandomPool randPool;
	randPool.Put((byte *)seed, strlen(seed));

	RSAES_PKCS1v15_Decryptor priv(randPool, keyLength);
	FileSink privFile(privFilename);
	priv.DEREncode(privFile);
	privFile.MessageEnd();

	RSAES_PKCS1v15_Encryptor pub(priv);
	FileSink pubFile(pubFilename);
	pub.DEREncode(pubFile);
	pubFile.MessageEnd();
}

void RSASignFile(const char *privFilename, const char *messageFilename, const char *signatureFilename)
{
	FileSource privFile(privFilename, true);
	RSASSA_PKCS1v15_SHA_Signer priv(privFile);
	// RSASSA_PKCS1v15_SHA_Signer ignores the rng. Use a real RNG for other signature schemes!
	FileSource f(messageFilename, true, new SignerFilter(GlobalRNG(), priv, new FileSink(signatureFilename)));
}

bool RSAVerifyFile(const char *pubFilename, const char *messageFilename, const char *signatureFilename)
{
	FileSource pubFile(pubFilename, true);
	RSASSA_PKCS1v15_SHA_Verifier pub(pubFile);

	FileSource signatureFile(signatureFilename, true);
	if (signatureFile.MaxRetrievable() != pub.SignatureLength())
		return false;
	SecByteBlock signature(pub.SignatureLength());
	signatureFile.Get(signature, signature.size());

	VerifierFilter *verifierFilter = new VerifierFilter(pub);
	verifierFilter->Put(signature, pub.SignatureLength());
	FileSource f(messageFilename, true, verifierFilter);

	return verifierFilter->GetLastResult();
}

