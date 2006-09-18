/*
 * RSA implementation for PuTTY.
 */

#include "global.h"

#include "RageUtil.h"
#include "CryptRSA.h"
#include "CryptSHA.h"
#include "CryptSH512.h"
#include "CryptMD5.h"
#include "CryptRand.h"
#include "CryptPrime.h"

#define GET_32BIT(cp) \
    (((unsigned long)(unsigned char)(cp)[0] << 24) | \
    ((unsigned long)(unsigned char)(cp)[1] << 16) | \
    ((unsigned long)(unsigned char)(cp)[2] << 8) | \
    ((unsigned long)(unsigned char)(cp)[3]))

#define PUT_32BIT(cp, value) { \
    (cp)[0] = (unsigned char)((value) >> 24); \
    (cp)[1] = (unsigned char)((value) >> 16); \
    (cp)[2] = (unsigned char)((value) >> 8); \
    (cp)[3] = (unsigned char)(value); }

RSAKey::RSAKey()
{
	bits = bytes = 0;
	modulus = exponent = private_exponent = p = q = iqmp = NULL;
}

RSAKey::~RSAKey()
{
	if(p)
		freebn(p);
	if(q)
		freebn(q);
	if(modulus)
		freebn(modulus);
	if(exponent)
		freebn(exponent);
	if(private_exponent)
		freebn(private_exponent);
	if(iqmp)
		freebn(iqmp);
}


int makekey(unsigned char *data, struct RSAKey *result,
		unsigned char **keystr, int order)
{
	unsigned char *p = data;
	int i;

	if (result) {
		result->bits = 0;
		for (i = 0; i < 4; i++)
			result->bits = (result->bits << 8) + *p++;
	} else
		p += 4;

	/*
	 * order=0 means exponent then modulus (the keys sent by the
	 * server). order=1 means modulus then exponent (the keys
	 * stored in a keyfile).
	 */

	if (order == 0)
		p += ssh1_read_bignum(p, result ? &result->exponent : NULL);
	if (result)
		result->bytes = (((p[0] << 8) + p[1]) + 7) / 8;
	if (keystr)
		*keystr = p + 2;
	p += ssh1_read_bignum(p, result ? &result->modulus : NULL);
	if (order == 1)
		p += ssh1_read_bignum(p, result ? &result->exponent : NULL);

	return p - data;
}

int makeprivate( const unsigned char *data, struct RSAKey *result )
{
	return ssh1_read_bignum(data, &result->private_exponent);
}

unsigned char *GetBytes( Bignum bn, int *buflen )
{
	int len = bn[0]*BIGNUM_INT_BYTES;

	unsigned char *ret = new unsigned char[len];
	unsigned char *p = ret;
	for( int i = len-1; i >= 0; --i )
		*p++ = bignum_byte( bn, i );
	return ret;
}

static void sha512_mpint(SHA512_State * s, Bignum b)
{
	unsigned char lenbuf[4];
	int len;
	len = (bignum_bitcount(b) + 8) / 8;
	PUT_32BIT(lenbuf, len);
	SHA512_Bytes(s, lenbuf, 4);
	while (len-- > 0) {
		lenbuf[0] = bignum_byte(b, len);
		SHA512_Bytes(s, lenbuf, 1);
	}
	memset(lenbuf, 0, sizeof(lenbuf));
}

/*
 * This function is a wrapper on modpow(). It has the same effect
 * as modpow(), but employs RSA blinding to protect against timing
 * attacks.
 */
static Bignum rsa_privkey_op(Bignum input, const RSAKey *key)
{
	Bignum random, random_encrypted, random_inverse;
	Bignum input_blinded, ret_blinded;
	Bignum ret;

	SHA512_State ss;
	unsigned char digest512[64];
	unsigned digestused = ARRAYLEN(digest512);
	int hashseq = 0;

	/*
	 * Start by inventing a random number chosen uniformly from the
	 * range 2..modulus-1. (We do this by preparing a random number
	 * of the right length and retrying if it's greater than the
	 * modulus, to prevent any potential Bleichenbacher-like
	 * attacks making use of the uneven distribution within the
	 * range that would arise from just reducing our number mod n.
	 * There are timing implications to the potential retries, of
	 * course, but all they tell you is the modulus, which you
	 * already knew.)
	 * 
	 * To preserve determinism and avoid Pageant needing to share
	 * the random number pool, we actually generate this `random'
	 * number by hashing stuff with the private key.
	 */
	while (1) {
		int bits, byte, bitsleft, v;
		random = copybn(key->modulus);
		/*
		 * Find the topmost set bit. (This function will return its
		 * index plus one.) Then we'll set all bits from that one
		 * downwards randomly.
		 */
		bits = bignum_bitcount(random);
		byte = 0;
		bitsleft = 0;
		while (bits--) {
			if (bitsleft <= 0) {
				bitsleft = 8;
				/*
				 * Conceptually the following few lines are equivalent to
				 *    byte = random_byte();
				 */
				if (digestused >= ARRAYSIZE(digest512)) {
					unsigned char seqbuf[4];
					PUT_32BIT(seqbuf, hashseq);
					SHA512_Init(&ss);
					SHA512_Bytes(&ss, "RSA deterministic blinding", 26);
					SHA512_Bytes(&ss, seqbuf, sizeof(seqbuf));
					sha512_mpint(&ss, key->private_exponent);
					SHA512_Final(&ss, digest512);
					hashseq++;

					/*
					 * Now hash that digest plus the signature
					 * input.
					 */
					SHA512_Init(&ss);
					SHA512_Bytes(&ss, digest512, sizeof(digest512));
					sha512_mpint(&ss, input);
					SHA512_Final(&ss, digest512);

					digestused = 0;
				}
				byte = digest512[digestused++];
			}
			v = byte & 1;
			byte >>= 1;
			bitsleft--;
			bignum_set_bit(random, bits, v);
		}

		/*
		 * Now check that this number is strictly greater than
		 * zero, and strictly less than modulus.
		 */
		if (bignum_cmp(random, Zero) <= 0 ||
				bignum_cmp(random, key->modulus) >= 0) {
			freebn(random);
			continue;
		} else {
			break;
		}
	}

	/*
	 * RSA blinding relies on the fact that (xy)^d mod n is equal
	 * to (x^d mod n) * (y^d mod n) mod n. We invent a random pair
	 * y and y^d; then we multiply x by y, raise to the power d mod
	 * n as usual, and divide by y^d to recover x^d. Thus an
	 * attacker can't correlate the timing of the modpow with the
	 * input, because they don't know anything about the number
	 * that was input to the actual modpow.
	 * 
	 * The clever bit is that we don't have to do a huge modpow to
	 * get y and y^d; we will use the number we just invented as
	 * _y^d_, and use the _public_ exponent to compute (y^d)^e = y
	 * from it, which is much faster to do.
	 */
	random_encrypted = modpow(random, key->exponent, key->modulus);
	random_inverse = modinv(random, key->modulus);
	input_blinded = modmul(input, random_encrypted, key->modulus);
	ret_blinded = modpow(input_blinded, key->private_exponent, key->modulus);
	ret = modmul(ret_blinded, random_inverse, key->modulus);

	freebn(ret_blinded);
	freebn(input_blinded);
	freebn(random_inverse);
	freebn(random_encrypted);
	freebn(random);

	return ret;
}

bool RSAKey::Encrypt( RString &buf ) const
{
	Bignum bn = bignum_from_bytes( (unsigned char *) buf.data(), buf.size() );
	Bignum encrypted_bn = Encrypt( bn );
	delete [] bn;

	int len;
	unsigned char *bytes = bignum_to_buffer( encrypted_bn, &len );
	buf = RString( (const char *) bytes, len );
	delete [] encrypted_bn;

	return true;
}

bool RSAKey::Decrypt( RString &buf ) const
{
	Bignum bn = bignum_from_buffer( (unsigned char *) buf.data(), buf.size() );
	if( bn == NULL )
		return false;

	Bignum decrypted_bn = Decrypt( bn );
	delete [] bn;

	int len;
	unsigned char *bytes = bignum_to_bytes( decrypted_bn, &len );
	buf = RString( (const char *) bytes, len );
	delete [] decrypted_bn;

	return true;
}

Bignum RSAKey::Encrypt(const Bignum input) const
{
	return modpow( input, this->exponent, this->modulus );
}

Bignum RSAKey::Decrypt(const Bignum input) const
{
	return rsa_privkey_op(input, this);
}

int RSAKey::StrLen() const
{
	Bignum md, ex;
	int mdlen, exlen;

	md = this->modulus;
	ex = this->exponent;
	mdlen = (bignum_bitcount(md) + 15) / 16;
	exlen = (bignum_bitcount(ex) + 15) / 16;
	return 4 * (mdlen + exlen) + 20;
}

void RSAKey::StrFmt( char *str ) const
{
	int len = 0, i, nibbles;
	static const char hex[] = "0123456789abcdef";

	len += sprintf(str + len, "0x");

	nibbles = (3 + bignum_bitcount(this->exponent)) / 4;
	if (nibbles < 1)
		nibbles = 1;
	for (i = nibbles; i--;)
		str[len++] = hex[(bignum_byte(this->exponent, i / 2) >> (4 * (i % 2))) & 0xF];

	len += sprintf(str + len, ",0x");

	nibbles = (3 + bignum_bitcount(this->modulus)) / 4;
	if (nibbles < 1)
		nibbles = 1;
	for (i = nibbles; i--;)
		str[len++] = hex[(bignum_byte(this->modulus, i / 2) >> (4 * (i % 2))) & 0xF];

	str[len] = '\0';
}

/*
 * Generate a fingerprint string for the key. Compatible with the
 * OpenSSH fingerprint code.
 */
void RSAKey::Fingerprint( char *str, int len ) const
{
	struct MD5Context md5c;
	unsigned char digest[16];
	char buffer[16 * 3 + 40];
	int numlen, slen, i;

	MD5Init(&md5c);
	numlen = ssh1_bignum_length(this->modulus) - 2;
	for (i = numlen; i--;) {
		unsigned char c = bignum_byte(this->modulus, i);
		MD5Update(&md5c, &c, 1);
	}
	numlen = ssh1_bignum_length(this->exponent) - 2;
	for (i = numlen; i--;) {
		unsigned char c = bignum_byte(this->exponent, i);
		MD5Update(&md5c, &c, 1);
	}
	MD5Final(digest, &md5c);

	sprintf(buffer, "%d ", bignum_bitcount(this->modulus));
	for (i = 0; i < 16; i++)
		sprintf(buffer + strlen(buffer), "%s%02x", i ? ":" : "",
				digest[i]);
	strncpy(str, buffer, len);
	str[len - 1] = '\0';
	slen = strlen(str);
	if (!this->comment.empty() && slen < len - 1)
	{
		str[slen] = ' ';
		strncpy(str + slen + 1, this->comment, len - slen - 1);
		str[len - 1] = '\0';
	}
}

/*
 * Verify that the public data in an RSA key matches the private
 * data. We also check the private data itself: we ensure that p >
 * q and that iqmp really is the inverse of q mod p.
 */
bool RSAKey::Check() const
{
	Bignum n, ed, pm1, qm1;
	int cmp;

	/* n must equal pq. */
	n = bigmul(this->p, this->q);
	cmp = bignum_cmp(n, this->modulus);
	freebn(n);
	if (cmp != 0)
		return 0;

	/* e * d must be congruent to 1, modulo (p-1) and modulo (q-1). */
	pm1 = copybn(this->p);
	decbn(pm1);
	ed = modmul(this->exponent, this->private_exponent, pm1);
	cmp = bignum_cmp(ed, One);
	delete [] ed;
	if (cmp != 0)
		return 0;

	qm1 = copybn(this->q);
	decbn(qm1);
	ed = modmul(this->exponent, this->private_exponent, qm1);
	cmp = bignum_cmp(ed, One);
	delete [] ed;
	if (cmp != 0)
		return 0;

	/*
	 * Ensure p > q.
	 */
	if (bignum_cmp(this->p, this->q) <= 0)
		return 0;

	/*
	 * Ensure iqmp * q is congruent to 1, modulo p.
	 */
	n = modmul(this->iqmp, this->q, this->p);
	cmp = bignum_cmp(n, One);
	delete [] n;
	if (cmp != 0)
		return 0;

	return 1;
}

/* ----------------------------------------------------------------------
 * Implementation of the ssh-rsa signing key type. 
 */

static void getstring(const char **data, int *datalen, const char **p, int *length)
{
	*p = NULL;
	if (*datalen < 4)
		return;
	*length = GET_32BIT(*data);
	*datalen -= 4;
	*data += 4;
	if (*datalen < *length)
		return;
	*p = *data;
	*data += *length;
	*datalen -= *length;
}
static Bignum getmp(const char **data, int *datalen)
{
	const char *p;
	int length;
	Bignum b;

	getstring(data, datalen, &p, &length);
	if (!p)
		return NULL;
	b = bignum_from_bytes((unsigned char *)p, length);
	return b;
}

bool RSAKey::LoadFromPublicBlob( const RString &str )
{
	int len = str.size();
	const char *data = str.data();
	const char *p;
	int slen;
	getstring(&data, &len, &p, &slen);
	if (!p || slen != 7 || memcmp(p, "ssh-rsa", 7)) {
		return false;
	}

	struct RSAKey *rsa = new RSAKey;
	rsa->exponent = getmp(&data, &len);
	rsa->modulus = getmp(&data, &len);
	rsa->private_exponent = NULL;

	return true;
}


char *RSAKey::FmtKey() const
{
	int len = this->StrLen();
	char *p = new char[len];
	StrFmt( p );
	return p;
}

void RSAKey::PublicBlob( RString &out ) const
{
	int elen, mlen, bloblen;
	int i;
	unsigned char *blob, *p;

	elen = (bignum_bitcount(this->exponent) + 8) / 8;
	mlen = (bignum_bitcount(this->modulus) + 8) / 8;

	/*
	 * string "ssh-rsa", mpint exp, mpint mod. Total 19+elen+mlen.
	 * (three length fields, 12+7=19).
	 */
	bloblen = 19 + elen + mlen;
	blob = new unsigned char[bloblen];
	p = blob;
	PUT_32BIT(p, 7);
	p += 4;
	memcpy(p, "ssh-rsa", 7);
	p += 7;
	PUT_32BIT(p, elen);
	p += 4;
	for (i = elen; i--;)
		*p++ = bignum_byte(this->exponent, i);
	PUT_32BIT(p, mlen);
	p += 4;
	for (i = mlen; i--;)
		*p++ = bignum_byte(this->modulus, i);
	ASSERT(p == blob + bloblen);

	out = RString( (const char *) blob, bloblen );
}

void RSAKey::PrivateBlob( RString &out ) const
{
	int i;
	unsigned char *blob, *p;

	int elen = (bignum_bitcount(this->exponent) + 8) / 8;
	int mlen = (bignum_bitcount(this->modulus) + 8) / 8;
	int dlen = (bignum_bitcount(this->private_exponent) + 8) / 8;
	int plen = (bignum_bitcount(this->p) + 8) / 8;
	int qlen = (bignum_bitcount(this->q) + 8) / 8;
	int ulen = (bignum_bitcount(this->iqmp) + 8) / 8;

	/*
	 * mpint exp, mpint mod, mpint private_exp, mpint p, mpint q, mpint iqmp. Total 24 +
	 * sum of lengths.
	 */
	int bloblen = 24 + elen + mlen + dlen + plen + qlen + ulen;
	blob = new unsigned char[bloblen];
	p = blob;

	PUT_32BIT(p, elen);
	p += 4;
	for (i = elen; i--;)
		*p++ = bignum_byte(this->exponent, i);
	PUT_32BIT(p, mlen);
	p += 4;
	for (i = mlen; i--;)
		*p++ = bignum_byte(this->modulus, i);
	PUT_32BIT(p, dlen);
	p += 4;
	for (i = dlen; i--;)
		*p++ = bignum_byte(this->private_exponent, i);
	PUT_32BIT(p, plen);
	p += 4;
	for (i = plen; i--;)
		*p++ = bignum_byte(this->p, i);
	PUT_32BIT(p, qlen);
	p += 4;
	for (i = qlen; i--;)
		*p++ = bignum_byte(this->q, i);
	PUT_32BIT(p, ulen);
	p += 4;
	for (i = ulen; i--;)
		*p++ = bignum_byte(this->iqmp, i);
	ASSERT(p == blob + bloblen);

	out = RString( (const char *) blob, bloblen );
}

bool RSAKey::LoadFromPrivateBlob( const RString &str )
{
	int len = str.size();
	const char *data = str.data();

	this->exponent = getmp(&data, &len);
	this->modulus = getmp(&data, &len);
	this->private_exponent = getmp(&data, &len);
	this->p = getmp(&data, &len);
	this->q = getmp(&data, &len);
	this->iqmp = getmp(&data, &len);

	if (!this->Check())
		return false;

	return true;
}

/* static struct RSAKey *rsa2_openssh_createkey(unsigned char **blob, int *len)
{
	const char **b = (const char **) blob;
	struct RSAKey *rsa = new RSAKey;

	rsa->comment = NULL;
	rsa->modulus = getmp(b, len);
	rsa->exponent = getmp(b, len);
	rsa->private_exponent = getmp(b, len);
	rsa->iqmp = getmp(b, len);
	rsa->p = getmp(b, len);
	rsa->q = getmp(b, len);

	if (!rsa->modulus || !rsa->exponent || !rsa->private_exponent ||
			!rsa->iqmp || !rsa->p || !rsa->q)
	{
		delete rsa;
		return NULL;
	}

	return rsa;
}

static int rsa2_openssh_fmtkey( struct RSAKey *rsa, unsigned char *blob, int len )
{
	int bloblen =
		ssh2_bignum_length(rsa->modulus) +
		ssh2_bignum_length(rsa->exponent) +
		ssh2_bignum_length(rsa->private_exponent) +
		ssh2_bignum_length(rsa->iqmp) +
		ssh2_bignum_length(rsa->p) + ssh2_bignum_length(rsa->q);

	if (bloblen > len)
		return bloblen;

	bloblen = 0;
#define ENC(x) \
	PUT_32BIT(blob+bloblen, ssh2_bignum_length((x))-4); bloblen += 4; \
		for (i = ssh2_bignum_length((x))-4; i-- ;) blob[bloblen++]=bignum_byte((x),i);
	int i;
	ENC(rsa->modulus);
	ENC(rsa->exponent);
	ENC(rsa->private_exponent);
	ENC(rsa->iqmp);
	ENC(rsa->p);
	ENC(rsa->q);

	return bloblen;
}
*/
int rsa2_pubkey_bits( const RString &blob )
{
	RSAKey rsa;
	if( !rsa.LoadFromPublicBlob( blob ) )
		return 0;

	return bignum_bitcount(rsa.modulus);
}

char *rsa2_fingerprint( struct RSAKey *rsa )
{
	struct MD5Context md5c;
	unsigned char digest[16], lenbuf[4];
	char buffer[16 * 3 + 40];
	char *ret;
	int numlen, i;

	MD5Init(&md5c);
	MD5Update(&md5c, (unsigned char *)"\0\0\0\7ssh-rsa", 11);

#define ADD_BIGNUM(bignum) \
	numlen = (bignum_bitcount(bignum)+8)/8; \
		PUT_32BIT(lenbuf, numlen); MD5Update(&md5c, lenbuf, 4); \
		for (i = numlen; i-- ;) { \
			unsigned char c = bignum_byte(bignum, i); \
				MD5Update(&md5c, &c, 1); \
		}
	ADD_BIGNUM(rsa->exponent);
	ADD_BIGNUM(rsa->modulus);
#undef ADD_BIGNUM

	MD5Final(digest, &md5c);

	sprintf(buffer, "ssh-rsa %d ", bignum_bitcount(rsa->modulus));
	for (i = 0; i < 16; i++)
		sprintf(buffer + strlen(buffer), "%s%02x", i ? ":" : "",
				digest[i]);
	ret = new char[strlen(buffer) + 1];
	if (ret)
		strcpy(ret, buffer);
	return ret;
}

bool RSAKey::Verify( const RString &data, const RString &sig ) const
{
	Bignum in, out;
	int bytes, i, j;
	unsigned char hash[20];

	in = bignum_from_bytes( (const unsigned char *) sig.data(), sig.size() );

	/* Base (in) must be smaller than the modulus. */
	if( bignum_cmp(in, this->modulus) >= 0 )
	{
		freebn(in);
		return false;
	}
	out = modpow(in, this->exponent, this->modulus);
	freebn(in);

	bool ret = true;

	bytes = (bignum_bitcount(this->modulus)+7) / 8;
	/* Top (partial) byte should be zero. */
	if (bignum_byte(out, bytes - 1) != 0)
		ret = 0;
	/* First whole byte should be 1. */
	if (bignum_byte(out, bytes - 2) != 1)
		ret = 0;
	/* Most of the rest should be FF. */
	for (i = bytes - 3; i >= 20; i--) {
		if (bignum_byte(out, i) != 0xFF)
			ret = 0;
	}
	/* Finally, we expect to see the SHA-1 hash of the signed data. */
	SHA_Simple( data.data(), data.size(), hash );
	for (i = 19, j = 0; i >= 0; i--, j++) {
		if (bignum_byte(out, i) != hash[j])
			ret = false;
	}
	freebn(out);

	return ret;
}

void RSAKey::Sign( const RString &data, RString &out ) const
{
	Bignum in;
	{
		unsigned char hash[20];
		SHA_Simple(data.data(), data.size(), hash);

		int nbytes = (bignum_bitcount(this->modulus) - 1) / 8;
		unsigned char *bytes = new unsigned char[nbytes];

		memset( bytes, 0xFF, nbytes );
		bytes[0] = 1;
		memcpy( bytes + nbytes - 20, hash, 20 );

		in = bignum_from_bytes(bytes, nbytes);
		delete [] bytes;
	}

	Bignum outnum = rsa_privkey_op(in, this);
	delete [] in;

	int siglen;
	unsigned char *bytes = bignum_to_bytes( outnum, &siglen );
	delete [] outnum;

	out = RString( (const char *) bytes, siglen );
	delete [] bytes;
}

#define RSA_EXPONENT 37		       /* we like this prime */

int RSAKey::Generate( int bits )
{
	Bignum pm1, qm1, phi_n;

	/*
	 * We don't generate e; we just use a standard one always.
	 */
	this->exponent = bignum_from_long(RSA_EXPONENT);

	/*
	 * Generate p and q: primes with combined length `bits', not
	 * congruent to 1 modulo e. (Strictly speaking, we wanted (p-1)
	 * and e to be coprime, and (q-1) and e to be coprime, but in
	 * general that's slightly more fiddly to arrange. By choosing
	 * a prime e, we can simplify the criterion.)
	 */
	this->p = primegen(bits / 2, RSA_EXPONENT, 1, NULL, 1);
	this->q = primegen(bits - bits / 2, RSA_EXPONENT, 1, NULL, 2);

	/*
	 * Ensure p > q, by swapping them if not.
	 */
	if (bignum_cmp(this->p, this->q) < 0)
		swap( p, q );

	/*
	 * Now we have p, q and e. All we need to do now is work out
	 * the other helpful quantities: n=pq, d=e^-1 mod (p-1)(q-1),
	 * and (q^-1 mod p).
	 */
	this->modulus = bigmul(this->p, this->q);
	pm1 = copybn(this->p);
	decbn(pm1);
	qm1 = copybn(this->q);
	decbn(qm1);
	phi_n = bigmul(pm1, qm1);
	freebn(pm1);
	freebn(qm1);
	this->private_exponent = modinv(this->exponent, phi_n);
	this->iqmp = modinv(this->q, this->p);

	/*
	 * Clean up temporary numbers.
	 */
	freebn(phi_n);

	return 1;
}

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
