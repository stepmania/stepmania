#ifndef CryptManager_H
#define CryptManager_H

const CString SIGNATURE_APPEND = ".sig";

class CryptManager
{
public:
	CryptManager();
	~CryptManager();

	static void GenerateRSAKey( unsigned int keyLength, CString privFilename, CString pubFilename, CString seed );

	static void SignFileToFile( CString sPath, CString sSignatureFile = "" );
	static bool VerifyFileWithFile( CString sPath, CString sSignatureFile = "" );

	static CString Sign( CString sPath );
	static bool Verify( CString sPath, CString sSignature );

	static CString GetMD5( CString fn );	// in Hex

	static CString GetPublicKeyFileName();
};

extern CryptManager*	CRYPTMAN;	// global and accessable from anywhere in our program

#endif
