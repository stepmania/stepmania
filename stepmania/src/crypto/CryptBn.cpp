/*
 * Bignum routines for RSA and DH and stuff.
 */

#include "global.h"
#include "CryptBn.h"

#if defined(_MSC_VER)
#pragma warning (disable : 4244) // conversion from 'int' to 'BignumInt', possible loss of data
#endif

BignumInt bnZero[1] = { 0 };
BignumInt bnOne[2] = { 1, 1 };

/*
 * The Bignum format is an array of `BignumInt'. The first
 * element of the array counts the remaining elements. The
 * remaining elements express the actual number, base 2^BIGNUM_INT_BITS, _least_
 * significant digit first. (So it's trivial to extract the bit
 * with value 2^n for any n.)
 *
 * All Bignums in this module are positive. Negative numbers must
 * be dealt with outside it.
 *
 * INVARIANT: the most significant word of any Bignum must be
 * nonzero.
 */

Bignum Zero = bnZero, One = bnOne;

static Bignum newbn(int length)
{
	Bignum b = new BignumInt[length + 1];
	if (!b)
		abort();		       /* FIXME */
	memset(b, 0, (length + 1) * sizeof(*b));
	b[0] = length;
	return b;
}

void bn_restore_invariant(Bignum b)
{
	while (b[0] > 1 && b[b[0]] == 0)
		b[0]--;
}

Bignum copybn(Bignum orig)
{
	Bignum b = new BignumInt[orig[0] + 1];
	if (!b)
		abort();		       /* FIXME */
	memcpy(b, orig, (orig[0] + 1) * sizeof(*b));
	return b;
}

void freebn(Bignum b)
{
	/*
	 * Burn the evidence, just in case.
	 */
	memset(b, 0, sizeof(b[0]) * (b[0] + 1));
	delete [] b;
}

Bignum bn_power_2(int n)
{
	Bignum ret = newbn(n / BIGNUM_INT_BITS + 1);
	bignum_set_bit(ret, n, 1);
	return ret;
}

/*
 * Compute c = a * b.
 * Input is in the first len words of a and b.
 * Result is returned in the first 2*len words of c.
 */
static void internal_mul(BignumInt *a, BignumInt *b,
		BignumInt *c, int len)
{
	int i, j;
	BignumDblInt t;

	for (j = 0; j < 2 * len; j++)
		c[j] = 0;

	for (i = len - 1; i >= 0; i--) {
		t = 0;
		for (j = len - 1; j >= 0; j--) {
			t += MUL_WORD(a[i], (BignumDblInt) b[j]);
			t += (BignumDblInt) c[i + j + 1];
			c[i + j + 1] = (BignumInt) t;
			t = t >> BIGNUM_INT_BITS;
		}
		c[i] = (BignumInt) t;
	}
}

static void internal_add_shifted(BignumInt *number,
		unsigned n, int shift)
{
	int word = 1 + (shift / BIGNUM_INT_BITS);
	int bshift = shift % BIGNUM_INT_BITS;
	BignumDblInt addend;

	addend = (BignumDblInt)n << bshift;

	while (addend) {
		addend += number[word];
		number[word] = (BignumInt) addend & BIGNUM_INT_MASK;
		addend >>= BIGNUM_INT_BITS;
		word++;
	}
}

/*
 * Compute a = a % m.
 * Input in first alen words of a and first mlen words of m.
 * Output in first alen words of a
 * (of which first alen-mlen words will be zero).
 * The MSW of m MUST have its high bit set.
 * Quotient is accumulated in the `quotient' array, which is a Bignum
 * rather than the internal bigendian format. Quotient parts are shifted
 * left by `qshift' before adding into quot.
 */
static void internal_mod(BignumInt *a, int alen,
		BignumInt *m, int mlen,
		BignumInt *quot, int qshift)
{
	BignumInt m0, m1;
	unsigned int h;
	int i, k;

	m0 = m[0];
	if (mlen > 1)
		m1 = m[1];
	else
		m1 = 0;

	for (i = 0; i <= alen - mlen; i++) {
		BignumDblInt t;
		unsigned int q, r, c, ai1;

		if (i == 0) {
			h = 0;
		} else {
			h = a[i - 1];
			a[i - 1] = 0;
		}

		if (i == alen - 1)
			ai1 = 0;
		else
			ai1 = a[i + 1];

		/* Find q = h:a[i] / m0 */
		DIVMOD_WORD(q, r, h, a[i], m0);

		/* Refine our estimate of q by looking at
h:a[i]:a[i+1] / m0:m1 */
		t = MUL_WORD(m1, q);
		if (t > ((BignumDblInt) r << BIGNUM_INT_BITS) + ai1) {
			q--;
			t -= m1;
			r = (r + m0) & BIGNUM_INT_MASK;     /* overflow? */
			if (r >= (BignumDblInt) m0 &&
					t > ((BignumDblInt) r << BIGNUM_INT_BITS) + ai1) q--;
		}

		/* Subtract q * m from a[i...] */
		c = 0;
		for (k = mlen - 1; k >= 0; k--) {
			t = MUL_WORD(q, m[k]);
			t += c;
			c = t >> BIGNUM_INT_BITS;
			if ((BignumInt) t > a[i + k])
				c++;
			a[i + k] -= (BignumInt) t;
		}

		/* Add back m in case of borrow */
		if (c != h) {
			t = 0;
			for (k = mlen - 1; k >= 0; k--) {
				t += m[k];
				t += a[i + k];
				a[i + k] = (BignumInt) t;
				t = t >> BIGNUM_INT_BITS;
			}
			q--;
		}
		if (quot)
			internal_add_shifted(quot, q, qshift + BIGNUM_INT_BITS * (alen - mlen - i));
	}
}

/*
 * Compute (base ^ exp) % mod.
 */
Bignum modpow(Bignum base_in, Bignum exp, Bignum mod)
{
	BignumInt *a, *b, *n, *m;
	int mshift;
	int i,j,mlen;
	Bignum base, result;

	/*
	 * The most significant word of mod needs to be non-zero. It
	 * should already be, but let's make sure.
	 */
	ASSERT(mod[mod[0]] != 0);

	/*
	 * Make sure the base is smaller than the modulus, by reducing
	 * it modulo the modulus if not.
	 */
	base = bigmod(base_in, mod);

	/* Allocate m of size mlen, copy mod to m */
	/* We use big endian internally */
	mlen = mod[0];
	m = new BignumInt[mlen];
	for (j = 0; j < mlen; j++)
		m[j] = mod[mod[0] - j];

	/* Shift m left to make msb bit set */
	for (mshift = 0; mshift < BIGNUM_INT_BITS-1; mshift++)
		if ((m[0] << mshift) & BIGNUM_TOP_BIT)
			break;
	if (mshift) {
		for (i = 0; i < mlen - 1; i++)
			m[i] = (m[i] << mshift) | (m[i + 1] >> (BIGNUM_INT_BITS - mshift));
		m[mlen - 1] = m[mlen - 1] << mshift;
	}

	/* Allocate n of size mlen, copy base to n */
	n = new BignumInt[mlen];
	i = mlen - base[0];
	for (j = 0; j < i; j++)
		n[j] = 0;
	for (j = 0; j < (int)base[0]; j++)
		n[i + j] = base[base[0] - j];

	/* Allocate a and b of size 2*mlen. Set a = 1 */
	a = new BignumInt[2 * mlen];
	b = new BignumInt[2 * mlen];
	for (i = 0; i < 2 * mlen; i++)
		a[i] = 0;
	a[2 * mlen - 1] = 1;

	/* Skip leading zero bits of exp. */
	i = 0;
	j = BIGNUM_INT_BITS-1;
	while (i < (int)exp[0] && (exp[exp[0] - i] & (1 << j)) == 0) {
		j--;
		if (j < 0) {
			i++;
			j = BIGNUM_INT_BITS-1;
		}
	}

	/* Main computation */
	while (i < (int)exp[0]) {
		while (j >= 0) {
			internal_mul(a + mlen, a + mlen, b, mlen);
			internal_mod(b, mlen * 2, m, mlen, NULL, 0);
			if ((exp[exp[0] - i] & (1 << j)) != 0) {
				internal_mul(b + mlen, n, a, mlen);
				internal_mod(a, mlen * 2, m, mlen, NULL, 0);
			} else {
				BignumInt *t;
				t = a;
				a = b;
				b = t;
			}
			j--;
		}
		i++;
		j = BIGNUM_INT_BITS-1;
	}

	/* Fixup result in case the modulus was shifted */
	if (mshift) {
		for (i = mlen - 1; i < 2 * mlen - 1; i++)
			a[i] = (a[i] << mshift) | (a[i + 1] >> (BIGNUM_INT_BITS - mshift));
		a[2 * mlen - 1] = a[2 * mlen - 1] << mshift;
		internal_mod(a, mlen * 2, m, mlen, NULL, 0);
		for (i = 2 * mlen - 1; i >= mlen; i--)
			a[i] = (a[i] >> mshift) | (a[i - 1] << (BIGNUM_INT_BITS - mshift));
	}

	/* Copy result to buffer */
	result = newbn(mod[0]);
	for (i = 0; i < mlen; i++)
		result[result[0] - i] = a[i + mlen];
	while (result[0] > 1 && result[result[0]] == 0)
		result[0]--;

	/* Free temporary arrays */
	for (i = 0; i < 2 * mlen; i++)
		a[i] = 0;
	delete [] a;
	for (i = 0; i < 2 * mlen; i++)
		b[i] = 0;
	delete [] b;
	for (i = 0; i < mlen; i++)
		m[i] = 0;
	delete [] m;
	for (i = 0; i < mlen; i++)
		n[i] = 0;
	delete [] n;

	freebn(base);

	return result;
}

/*
 * Compute (p * q) % mod.
 * The most significant word of mod MUST be non-zero.
 * We assume that the result array is the same size as the mod array.
 */
Bignum modmul(Bignum p, Bignum q, Bignum mod)
{
	BignumInt *a, *n, *m, *o;
	int mshift;
	int pqlen, mlen, rlen, i, j;
	Bignum result;

	/* Allocate m of size mlen, copy mod to m */
	/* We use big endian internally */
	mlen = mod[0];
	m = new BignumInt[mlen];
	for (j = 0; j < mlen; j++)
		m[j] = mod[mod[0] - j];

	/* Shift m left to make msb bit set */
	for (mshift = 0; mshift < BIGNUM_INT_BITS-1; mshift++)
		if ((m[0] << mshift) & BIGNUM_TOP_BIT)
			break;
	if (mshift) {
		for (i = 0; i < mlen - 1; i++)
			m[i] = (m[i] << mshift) | (m[i + 1] >> (BIGNUM_INT_BITS - mshift));
		m[mlen - 1] = m[mlen - 1] << mshift;
	}

	pqlen = (p[0] > q[0] ? p[0] : q[0]);

	/* Allocate n of size pqlen, copy p to n */
	n = new BignumInt[pqlen];
	i = pqlen - p[0];
	for (j = 0; j < i; j++)
		n[j] = 0;
	for (j = 0; j < (int)p[0]; j++)
		n[i + j] = p[p[0] - j];

	/* Allocate o of size pqlen, copy q to o */
	o = new BignumInt[pqlen];
	i = pqlen - q[0];
	for (j = 0; j < i; j++)
		o[j] = 0;
	for (j = 0; j < (int)q[0]; j++)
		o[i + j] = q[q[0] - j];

	/* Allocate a of size 2*pqlen for result */
	a = new BignumInt[2 * pqlen];

	/* Main computation */
	internal_mul(n, o, a, pqlen);
	internal_mod(a, pqlen * 2, m, mlen, NULL, 0);

	/* Fixup result in case the modulus was shifted */
	if (mshift) {
		for (i = 2 * pqlen - mlen - 1; i < 2 * pqlen - 1; i++)
			a[i] = (a[i] << mshift) | (a[i + 1] >> (BIGNUM_INT_BITS - mshift));
		a[2 * pqlen - 1] = a[2 * pqlen - 1] << mshift;
		internal_mod(a, pqlen * 2, m, mlen, NULL, 0);
		for (i = 2 * pqlen - 1; i >= 2 * pqlen - mlen; i--)
			a[i] = (a[i] >> mshift) | (a[i - 1] << (BIGNUM_INT_BITS - mshift));
	}

	/* Copy result to buffer */
	rlen = (mlen < pqlen * 2 ? mlen : pqlen * 2);
	result = newbn(rlen);
	for (i = 0; i < rlen; i++)
		result[result[0] - i] = a[i + 2 * pqlen - rlen];
	while (result[0] > 1 && result[result[0]] == 0)
		result[0]--;

	/* Free temporary arrays */
	for (i = 0; i < 2 * pqlen; i++)
		a[i] = 0;
	delete [] a;
	for (i = 0; i < mlen; i++)
		m[i] = 0;
	delete [] m;
	for (i = 0; i < pqlen; i++)
		n[i] = 0;
	delete [] n;
	for (i = 0; i < pqlen; i++)
		o[i] = 0;
	delete [] o;

	return result;
}

/*
 * Compute p % mod.
 * The most significant word of mod MUST be non-zero.
 * We assume that the result array is the same size as the mod array.
 * We optionally write out a quotient if `quotient' is non-NULL.
 * We can avoid writing out the result if `result' is NULL.
 */
static void bigdivmod(Bignum p, Bignum mod, Bignum result, Bignum quotient)
{
	BignumInt *n, *m;
	int mshift;
	int plen, mlen, i, j;

	/* Allocate m of size mlen, copy mod to m */
	/* We use big endian internally */
	mlen = mod[0];
	m = new BignumInt[mlen];
	for (j = 0; j < mlen; j++)
		m[j] = mod[mod[0] - j];

	/* Shift m left to make msb bit set */
	for (mshift = 0; mshift < BIGNUM_INT_BITS-1; mshift++)
		if ((m[0] << mshift) & BIGNUM_TOP_BIT)
			break;
	if (mshift) {
		for (i = 0; i < mlen - 1; i++)
			m[i] = (m[i] << mshift) | (m[i + 1] >> (BIGNUM_INT_BITS - mshift));
		m[mlen - 1] = m[mlen - 1] << mshift;
	}

	plen = p[0];
	/* Ensure plen > mlen */
	if (plen <= mlen)
		plen = mlen + 1;

	/* Allocate n of size plen, copy p to n */
	n = new BignumInt[plen];
	for (j = 0; j < plen; j++)
		n[j] = 0;
	for (j = 1; j <= (int)p[0]; j++)
		n[plen - j] = p[j];

	/* Main computation */
	internal_mod(n, plen, m, mlen, quotient, mshift);

	/* Fixup result in case the modulus was shifted */
	if (mshift) {
		for (i = plen - mlen - 1; i < plen - 1; i++)
			n[i] = (n[i] << mshift) | (n[i + 1] >> (BIGNUM_INT_BITS - mshift));
		n[plen - 1] = n[plen - 1] << mshift;
		internal_mod(n, plen, m, mlen, quotient, 0);
		for (i = plen - 1; i >= plen - mlen; i--)
			n[i] = (n[i] >> mshift) | (n[i - 1] << (BIGNUM_INT_BITS - mshift));
	}

	/* Copy result to buffer */
	if (result) {
		for (i = 1; i <= (int)result[0]; i++) {
			int j = plen - i;
			result[i] = j >= 0 ? n[j] : 0;
		}
	}

	/* Free temporary arrays */
	for (i = 0; i < mlen; i++)
		m[i] = 0;
	delete [] m;
	for (i = 0; i < plen; i++)
		n[i] = 0;
	delete [] n;
}

/*
 * Decrement a number.
 */
void decbn(Bignum bn)
{
	int i = 1;
	while (i < (int)bn[0] && bn[i] == 0)
		bn[i++] = BIGNUM_INT_MASK;
	bn[i]--;
}

int bignum_bytes_size( const Bignum bn )
{
	if( bn[0] == 0 )
		return 0;
	return bn[1];
}

unsigned char *bignum_to_buffer( const Bignum bn, int *nbytes )
{
	if( bn[0] == 0 )
	{
		/* Special case: zero length. */
		*nbytes = bn[0];

		unsigned char *ret = new unsigned char[1];
		return ret;
	}

	*nbytes = bn[1];

	unsigned char *ret = new unsigned char[bn[1]];
	unsigned char *p = ret;
	for( int i = bn[1]-1; i >= 0; --i )
		*p++ = bignum_byte( bn, i+BIGNUM_INT_BYTES );
	return ret;
}

Bignum bignum_from_buffer(const unsigned char *data, int nbytes)
{
	Bignum result;
	int w, i;

	w = (nbytes + BIGNUM_INT_BYTES - 1) / BIGNUM_INT_BYTES; /* bytes->words */
	w += 1; /* size */

	result = newbn(w);
	for (i = 1; i <= w; i++)
		result[i] = 0;

	result[1] = nbytes;
	for (i = nbytes; i--;) {
		unsigned char byte = *data++;
		result[2 + i / BIGNUM_INT_BYTES] |= byte << (8*i % BIGNUM_INT_BITS);
	}

	while (result[0] > 1 && result[result[0]] == 0)
		result[0]--;
	return result;
}

unsigned char *bignum_to_bytes( const Bignum bn, int *nbytes )
{
	if( bn[0] == 0 )
	{
		/* Special case: zero length. */
		*nbytes = 0;
		unsigned char *ret = new unsigned char[1];
		ret[0] = 0;
		return ret;
	}

	int len = *nbytes = bn[0] * BIGNUM_INT_BYTES;

	unsigned char *ret = new unsigned char[len];
	unsigned char *p = ret;
	for( int i = len-1; i >= 0; --i )
		*p++ = bignum_byte( bn, i );
	return ret;
}

/* Convert a sequence of bytes to a bignum.  Any sequence is valid. */
Bignum bignum_from_bytes(const unsigned char *data, int nbytes)
{
	Bignum result;
	int w, i;

	w = (nbytes + BIGNUM_INT_BYTES - 1) / BIGNUM_INT_BYTES; /* bytes->words */

	result = newbn(w);
	for (i = 1; i <= w; i++)
		result[i] = 0;

	for (i = nbytes; i--;) {
		unsigned char byte = *data++;
		result[1 + i / BIGNUM_INT_BYTES] |= byte << (8*i % BIGNUM_INT_BITS);
	}

	while (result[0] > 1 && result[result[0]] == 0)
		result[0]--;
	return result;
}

/*
 * Read an ssh1-format bignum from a data buffer. Return the number
 * of bytes consumed.
 */
int ssh1_read_bignum(const unsigned char *data, Bignum * result)
{
	const unsigned char *p = data;
	int i;
	int w, b;

	w = 0;
	for (i = 0; i < 2; i++)
		w = (w << 8) + *p++;
	b = (w + 7) / 8;		       /* bits -> bytes */

	if (!result)		       /* just return length */
		return b + 2;

	*result = bignum_from_bytes(p, b);

	return p + b - data;
}

/*
 * Return the bit count of a bignum, for ssh1 encoding.
 */
int bignum_bitcount(Bignum bn)
{
	int bitcount = bn[0] * BIGNUM_INT_BITS - 1;
	while (bitcount >= 0
			&& (bn[bitcount / BIGNUM_INT_BITS + 1] >> (bitcount % BIGNUM_INT_BITS)) == 0) bitcount--;
	return bitcount + 1;
}

/*
 * Return the byte length of a bignum when ssh1 encoded.
 */
int ssh1_bignum_length(Bignum bn)
{
	return 2 + (bignum_bitcount(bn) + 7) / 8;
}

/*
 * Return the byte length of a bignum when ssh2 encoded.
 */
int ssh2_bignum_length(Bignum bn)
{
	return 4 + (bignum_bitcount(bn) + 8) / 8;
}

/*
 * Return a byte from a bignum; 0 is least significant, etc.
 */
unsigned char bignum_byte( const Bignum bn, int i )
{
	if (i >= BIGNUM_INT_BYTES * (int)bn[0])
		return 0;		       /* beyond the end */
	else
		return (bn[i / BIGNUM_INT_BYTES + 1] >>
				((i % BIGNUM_INT_BYTES)*8)) & 0xFF;
}

/*
 * Return a bit from a bignum; 0 is least significant, etc.
 */
int bignum_bit(Bignum bn, int i)
{
	if (i >= BIGNUM_INT_BITS * (int)bn[0])
		return 0;		       /* beyond the end */
	else
		return (bn[i / BIGNUM_INT_BITS + 1] >> (i % BIGNUM_INT_BITS)) & 1;
}

/*
 * Set a bit in a bignum; 0 is least significant, etc.
 */
void bignum_set_bit(Bignum bn, int bitnum, int value)
{
	if (bitnum >= BIGNUM_INT_BITS * (int)bn[0])
		abort();		       /* beyond the end */
	else {
		int v = bitnum / BIGNUM_INT_BITS + 1;
		int mask = 1 << (bitnum % BIGNUM_INT_BITS);
		if (value)
			bn[v] |= mask;
		else
			bn[v] &= ~mask;
	}
}

/*
 * Write a ssh1-format bignum into a buffer. It is assumed the
 * buffer is big enough. Returns the number of bytes used.
 */
int ssh1_write_bignum(void *data, Bignum bn)
{
	unsigned char *p = (unsigned char *) data;
	int len = ssh1_bignum_length(bn);
	int i;
	int bitc = bignum_bitcount(bn);

	*p++ = (bitc >> 8) & 0xFF;
	*p++ = (bitc) & 0xFF;
	for (i = len - 2; i--;)
		*p++ = bignum_byte(bn, i);
	return len;
}

/*
 * Compare two bignums. Returns like strcmp.
 */
int bignum_cmp(Bignum a, Bignum b)
{
	int amax = a[0], bmax = b[0];
	int i = (amax > bmax ? amax : bmax);
	while (i) {
		BignumInt aval = (i > amax ? 0 : a[i]);
		BignumInt bval = (i > bmax ? 0 : b[i]);
		if (aval < bval)
			return -1;
		if (aval > bval)
			return +1;
		i--;
	}
	return 0;
}

/*
 * Right-shift one bignum to form another.
 */
Bignum bignum_rshift(Bignum a, int shift)
{
	Bignum ret;
	int i, shiftw, shiftb, shiftbb, bits;
	BignumInt ai, ai1;

	bits = bignum_bitcount(a) - shift;
	ret = newbn((bits + BIGNUM_INT_BITS - 1) / BIGNUM_INT_BITS);

	if (ret) {
		shiftw = shift / BIGNUM_INT_BITS;
		shiftb = shift % BIGNUM_INT_BITS;
		shiftbb = BIGNUM_INT_BITS - shiftb;

		ai1 = a[shiftw + 1];
		for (i = 1; i <= (int)ret[0]; i++) {
			ai = ai1;
			ai1 = (i + shiftw + 1 <= (int)a[0] ? a[i + shiftw + 1] : 0);
			ret[i] = ((ai >> shiftb) | (ai1 << shiftbb)) & BIGNUM_INT_MASK;
		}
	}

	return ret;
}

/*
 * Non-modular multiplication and addition.
 */
Bignum bigmuladd(Bignum a, Bignum b, Bignum addend)
{
	int alen = a[0], blen = b[0];
	int mlen = (alen > blen ? alen : blen);
	int rlen, i, maxspot;
	BignumInt *workspace;
	Bignum ret;

	/* mlen space for a, mlen space for b, 2*mlen for result */
	workspace = new BignumInt[mlen * 4];
	for (i = 0; i < mlen; i++) {
		workspace[0 * mlen + i] = (mlen - i <= (int)a[0] ? a[mlen - i] : 0);
		workspace[1 * mlen + i] = (mlen - i <= (int)b[0] ? b[mlen - i] : 0);
	}

	internal_mul(workspace + 0 * mlen, workspace + 1 * mlen,
			workspace + 2 * mlen, mlen);

	/* now just copy the result back */
	rlen = alen + blen + 1;
	if (addend && rlen <= (int)addend[0])
		rlen = addend[0] + 1;
	ret = newbn(rlen);
	maxspot = 0;
	for (i = 1; i <= (int)ret[0]; i++) {
		ret[i] = (i <= 2 * mlen ? workspace[4 * mlen - i] : 0);
		if (ret[i] != 0)
			maxspot = i;
	}
	ret[0] = maxspot;

	/* now add in the addend, if any */
	if (addend) {
		BignumDblInt carry = 0;
		for (i = 1; i <= rlen; i++) {
			carry += (i <= (int)ret[0] ? ret[i] : 0);
			carry += (i <= (int)addend[0] ? addend[i] : 0);
			ret[i] = (BignumInt) carry & BIGNUM_INT_MASK;
			carry >>= BIGNUM_INT_BITS;
			if (ret[i] != 0 && i > maxspot)
				maxspot = i;
		}
	}
	ret[0] = maxspot;

	delete [] workspace;
	return ret;
}

/*
 * Non-modular multiplication.
 */
Bignum bigmul(Bignum a, Bignum b)
{
	return bigmuladd(a, b, NULL);
}

/*
 * Create a bignum which is the bitmask covering another one. That
 * is, the smallest integer which is >= N and is also one less than
 * a power of two.
 */
Bignum bignum_bitmask(Bignum n)
{
	Bignum ret = copybn(n);
	int i;
	BignumInt j;

	i = ret[0];
	while (n[i] == 0 && i > 0)
		i--;
	if (i <= 0)
		return ret;		       /* input was zero */
	j = 1;
	while (j < n[i])
		j = 2 * j + 1;
	ret[i] = j;
	while (--i > 0)
		ret[i] = BIGNUM_INT_MASK;
	return ret;
}

/*
 * Convert a (max 32-bit) long into a bignum.
 */
Bignum bignum_from_long(unsigned long nn)
{
	Bignum ret;
	BignumDblInt n = nn;

	ret = newbn(3);
	ret[1] = (BignumInt)(n & BIGNUM_INT_MASK);
	ret[2] = (BignumInt)((n >> BIGNUM_INT_BITS) & BIGNUM_INT_MASK);
	ret[3] = 0;
	ret[0] = (ret[2]  ? 2 : 1);
	return ret;
}

/*
 * Add a long to a bignum.
 */
Bignum bignum_add_long(Bignum number, unsigned long addendx)
{
	Bignum ret = newbn(number[0] + 1);
	int i, maxspot = 0;
	BignumDblInt carry = 0, addend = addendx;

	for (i = 1; i <= (int)ret[0]; i++) {
		carry += addend & BIGNUM_INT_MASK;
		carry += (i <= (int)number[0] ? number[i] : 0);
		addend >>= BIGNUM_INT_BITS;
		ret[i] = (BignumInt) carry & BIGNUM_INT_MASK;
		carry >>= BIGNUM_INT_BITS;
		if (ret[i] != 0)
			maxspot = i;
	}
	ret[0] = maxspot;
	return ret;
}

/*
 * Compute the residue of a bignum, modulo a (max 16-bit) short.
 */
unsigned short bignum_mod_short(Bignum number, unsigned short modulus)
{
	BignumDblInt mod, r;
	int i;

	r = 0;
	mod = modulus;
	for (i = number[0]; i > 0; i--)
		r = (r * (BIGNUM_TOP_BIT % mod) * 2 + number[i] % mod) % mod;
	return (unsigned short) r;
}


/*
 * Simple division.
 */
Bignum bigdiv(Bignum a, Bignum b)
{
	Bignum q = newbn(a[0]);
	bigdivmod(a, b, NULL, q);
	return q;
}

/*
 * Simple remainder.
 */
Bignum bigmod(Bignum a, Bignum b)
{
	Bignum r = newbn(b[0]);
	bigdivmod(a, b, r, NULL);
	return r;
}

/*
 * Greatest common divisor.
 */
Bignum biggcd(Bignum av, Bignum bv)
{
	Bignum a = copybn(av);
	Bignum b = copybn(bv);

	while (bignum_cmp(b, Zero) != 0) {
		Bignum t = newbn(b[0]);
		bigdivmod(a, b, t, NULL);
		while (t[0] > 1 && t[t[0]] == 0)
			t[0]--;
		freebn(a);
		a = b;
		b = t;
	}

	freebn(b);
	return a;
}

/*
 * Modular inverse, using Euclid's extended algorithm.
 */
Bignum modinv(Bignum number, Bignum modulus)
{
	Bignum a = copybn(modulus);
	Bignum b = copybn(number);
	Bignum xp = copybn(Zero);
	Bignum x = copybn(One);
	int sign = +1;

	while (bignum_cmp(b, One) != 0) {
		Bignum t = newbn(b[0]);
		Bignum q = newbn(a[0]);
		bigdivmod(a, b, t, q);
		while (t[0] > 1 && t[t[0]] == 0)
			t[0]--;
		freebn(a);
		a = b;
		b = t;
		t = xp;
		xp = x;
		x = bigmuladd(q, xp, t);
		sign = -sign;
		freebn(t);
		freebn(q);
	}

	freebn(b);
	freebn(a);
	freebn(xp);

	/* now we know that sign * x == 1, and that x < modulus */
	if (sign < 0) {
		/* set a new x to be modulus - x */
		Bignum newx = newbn(modulus[0]);
		BignumInt carry = 0;
		int maxspot = 1;
		int i;

		for (i = 1; i <= (int)newx[0]; i++) {
			BignumInt aword = (i <= (int)modulus[0] ? modulus[i] : 0);
			BignumInt bword = (i <= (int)x[0] ? x[i] : 0);
			newx[i] = aword - bword - carry;
			bword = ~bword;
			carry = carry ? (newx[i] >= bword) : (newx[i] > bword);
			if (newx[i] != 0)
				maxspot = i;
		}
		newx[0] = maxspot;
		freebn(x);
		x = newx;
	}

	/* and return. */
	return x;
}

/*
 * Render a bignum into decimal. Return a malloced string holding
 * the decimal representation.
 */
char *bignum_decimal(Bignum x)
{
	int ndigits, ndigit;
	int i, iszero;
	BignumDblInt carry;
	char *ret;
	BignumInt *workspace;

	/*
	 * First, estimate the number of digits. Since log(10)/log(2)
	 * is just greater than 93/28 (the joys of continued fraction
	 * approximations...) we know that for every 93 bits, we need
	 * at most 28 digits. This will tell us how much to malloc.
	 *
	 * Formally: if x has i bits, that means x is strictly less
	 * than 2^i. Since 2 is less than 10^(28/93), this is less than
	 * 10^(28i/93). We need an integer power of ten, so we must
	 * round up (rounding down might make it less than x again).
	 * Therefore if we multiply the bit count by 28/93, rounding
	 * up, we will have enough digits.
	 */
	i = bignum_bitcount(x);
	ndigits = (28 * i + 92) / 93;      /* multiply by 28/93 and round up */
	ndigits++;			       /* allow for trailing \0 */
	ret = new char[ndigits];

	/*
	 * Now allocate some workspace to hold the binary form as we
	 * repeatedly divide it by ten. Initialise this to the
	 * big-endian form of the number.
	 */
	workspace = new BignumInt[x[0]];
	for (i = 0; i < (int)x[0]; i++)
		workspace[i] = x[x[0] - i];

	/*
	 * Next, write the decimal number starting with the last digit.
	 * We use ordinary short division, dividing 10 into the
	 * workspace.
	 */
	ndigit = ndigits - 1;
	ret[ndigit] = '\0';
	do {
		iszero = 1;
		carry = 0;
		for (i = 0; i < (int)x[0]; i++) {
			carry = (carry << BIGNUM_INT_BITS) + workspace[i];
			workspace[i] = (BignumInt) (carry / 10);
			if (workspace[i])
				iszero = 0;
			carry %= 10;
		}
		ret[--ndigit] = (char) (carry + '0');
	} while (!iszero);

	/*
	 * There's a chance we've fallen short of the start of the
	 * string. Correct if so.
	 */
	if (ndigit > 0)
		memmove(ret, ret + ndigit, ndigits - ndigit);

	/*
	 * Done.
	 */
	delete [] workspace;
	return ret;
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
