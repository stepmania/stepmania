#ifndef CRYPT_HELPERS_H
#define CRYPT_HELPERS_H

#if !defined(DISABLE_CRYPTO)

// tomcrypt_cfg.h redefines malloc, realloc, calloc
#pragma warning( push )
#pragma warning( disable : 4565 )
#include <tomcrypt.h>
#pragma warning ( pop )

class PRNGWrapper
{
public:
	PRNGWrapper( const struct ltc_prng_descriptor *pPRNGDescriptor );
	~PRNGWrapper();
	void AddEntropy( const void *pData, int iSize );
	void AddRandomEntropy();

	int m_iPRNG;
	prng_state m_PRNG;
};

class RSAKeyWrapper
{
public:
	RSAKeyWrapper();
	~RSAKeyWrapper();
	void Unload();
	void Generate( PRNGWrapper &prng, int iKeyLenBits );
	bool Load( const RString &sKey, RString &sError );

	rsa_key m_Key;
};

#endif

#endif

