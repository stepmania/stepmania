#include "global.h"
#include "CryptHelpers.h"

PRNGWrapper::PRNGWrapper( const struct ltc_prng_descriptor *pPRNGDescriptor )
{
	m_iPRNG = register_prng( pPRNGDescriptor );
	ASSERT( m_iPRNG >= 0 );

	int iRet = rng_make_prng( 128, m_iPRNG, &m_PRNG, nullptr );
	ASSERT_M( iRet == CRYPT_OK, error_to_string(iRet) );
}

PRNGWrapper::~PRNGWrapper()
{
	if( m_iPRNG != -1 )
		prng_descriptor[m_iPRNG].done( &m_PRNG );
}

void PRNGWrapper::AddEntropy( const void *pData, int iSize )
{
	int iRet = prng_descriptor[m_iPRNG].add_entropy( (const unsigned char *) pData, iSize, &m_PRNG );
	ASSERT_M( iRet == CRYPT_OK, error_to_string(iRet) );

	iRet = prng_descriptor[m_iPRNG].ready( &m_PRNG );
	ASSERT_M( iRet == CRYPT_OK, error_to_string(iRet) );
}

void PRNGWrapper::AddRandomEntropy()
{
	unsigned char buf[256];
	int iRet = rng_get_bytes( buf, sizeof(buf), nullptr );
	ASSERT( iRet == sizeof(buf) );

	AddEntropy( buf, sizeof(buf) );
}

RSAKeyWrapper::RSAKeyWrapper()
{
	memset( &m_Key, 0, sizeof(m_Key) );
}

RSAKeyWrapper::~RSAKeyWrapper()
{
	Unload();
}

void RSAKeyWrapper::Unload()
{
	rsa_free( &m_Key );
}

void RSAKeyWrapper::Generate( PRNGWrapper &prng, int iKeyLenBits )
{
	Unload();

	int iRet = rsa_make_key( &prng.m_PRNG, prng.m_iPRNG, iKeyLenBits / 8, 65537, &m_Key );
	ASSERT( iRet == CRYPT_OK );
}

bool RSAKeyWrapper::Load( const RString &sKey, RString &sError )
{
	Unload();

	int iRet = rsa_import( (const unsigned char *) sKey.data(), sKey.size(), &m_Key );
	if( iRet != CRYPT_OK )
	{
		memset( &m_Key, 0, sizeof(m_Key) );
		sError = error_to_string(iRet);
		return false;
	}

	return true;
}

