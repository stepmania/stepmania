#ifndef RSAKEY_H
#define RSAKEY_H

#include "CryptBn.h"
struct RSAKey
{
	RSAKey();
	~RSAKey();

	int bits;
	int bytes;
	Bignum modulus;
	Bignum exponent;
	Bignum private_exponent;
	Bignum p;
	Bignum q;
	Bignum iqmp;
	CString comment;

	int Generate( int bits );
	void Fingerprint( char *str, int len ) const;
	void Sign( const CString &data, CString &out ) const;
	bool Verify( const CString &data, const CString &sig ) const;

	bool Encrypt( CString &buf ) const;
	bool Decrypt( CString &buf ) const;
	Bignum Encrypt( const Bignum input ) const;
	Bignum Decrypt( const Bignum input ) const;

	bool Check() const;
	int StrLen() const;
	void StrFmt( char *str ) const;
	char *FmtKey() const;

	void PublicBlob( CString &out ) const;
	void PrivateBlob( CString &out ) const;
	bool LoadFromPublicBlob( const CString &PublicBlob );
	bool LoadFromPrivateBlob( const CString &PrivateBlob );
};

#endif
