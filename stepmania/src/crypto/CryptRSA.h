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

/*
 * PuTTY is copyright 1997-2001 Simon Tatham.
 * 
 * Portions copyright Robert de Bath, Joris van Rantwijk, Delian
 * Delchev, Andreas Schultz, Jeroen Massar, Wez Furlong, Nicolas Barry,
 * Justin Bradford, and CORE SDI S.A.
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
