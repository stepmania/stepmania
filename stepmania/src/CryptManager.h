#ifndef CryptManager_H
#define CryptManager_H

class CryptManager
{
public:
	CryptManager();
	~CryptManager();

	static void GenerateRSAKey( unsigned int keyLength, CString privFilename, CString pubFilename, CString seed );

	static void SignFile( CString sPath );
	static bool VerifyFile( CString sPath );

	static CString GetFileSignature( CString sPath );
	static bool VerifyFile( CString sPath, CString sSignature );

	static void DigestFile( CString fn );

	static CString GetPublicKeyFileName();
};

extern CryptManager*	CRYPTMAN;	// global and accessable from anywhere in our program

#endif
