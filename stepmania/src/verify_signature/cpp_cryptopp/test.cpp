// Based on test.cpp from crypto++
// Parameters:
//    public_key - X.509 standard SubjectPublicKeyInfo key in binary format
//    data_file - the data file whose signature to verify
//    signature - the signature of data_file in binary format
// test.cpp - written and placed in the public domain by Wei Dai

#include "sha.h"
#include "files.h"
#include "rsa.h"

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

		if (argc != 4)
		{
			cout << "\nUsage:    RSAPubKeyData.exe publickey_fn data_fn signature_fn" << endl;
			return -1;
		}

		if( RSAVerifyFile(argv[2], argv[3], argv[4]) )
		{
			cout << "The signature is valid." << endl;
		}
		else
		{
			cout << "The signature is not valid." << endl;
		}
		return 0;
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

