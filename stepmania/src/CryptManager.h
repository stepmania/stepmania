#ifndef CryptManager_H
#define CryptManager_H

class CryptManager
{
public:
	static void GenerateRSAKey(unsigned int keyLength, const char *privFilename, const char *pubFilename, const char *seed);

	static void SignFile( CString sPath );
	static bool VerifyFile( CString sPath );

	static void DigestFile(const char *filename);
};

#endif
