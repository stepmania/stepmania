#ifndef CryptHelpers_H
#define CryptHelpers_H

void GenerateRSAKey(unsigned int keyLength, const char *privFilename, const char *pubFilename, const char *seed);
void RSASignFile(const char *privFilename, const char *messageFilename, const char *signatureFilename);
bool RSAVerifyFile(const char *pubFilename, const char *messageFilename, const char *signatureFilename);
void DigestFile(const char *filename);


#endif
