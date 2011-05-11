/*
 * SHA1 hash algorithm. Used in SSH2 as a MAC, and the transform is
 * also used as a `stirring' function for the PuTTY random number
 * pool. Implemented directly from the specification by Simon
 * Tatham.
 */

#include "global.h"
#include "CryptSHA.h"

/* ----------------------------------------------------------------------
 * Core SHA algorithm: processes 16-word blocks into a message digest.
 */

#define rol(x,y) ( ((x) << (y)) | (((uint32_t)x) >> (32-y)) )

static void SHA_Core_Init(uint32_t h[5])
{
	h[0] = 0x67452301;
	h[1] = 0xefcdab89;
	h[2] = 0x98badcfe;
	h[3] = 0x10325476;
	h[4] = 0xc3d2e1f0;
}

void SHATransform(unsigned int *digest, unsigned int *block)
{
	unsigned int w[80];
	unsigned int a, b, c, d, e;
	int t;

	for (t = 0; t < 16; t++)
		w[t] = block[t];

	for (t = 16; t < 80; t++) {
		unsigned int tmp = w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16];
		w[t] = rol(tmp, 1);
	}

	a = digest[0];
	b = digest[1];
	c = digest[2];
	d = digest[3];
	e = digest[4];

	for (t = 0; t < 20; t++) {
		unsigned int tmp =
			rol(a, 5) + ((b & c) | (d & ~b)) + e + w[t] + 0x5a827999;
		e = d;
		d = c;
		c = rol(b, 30);
		b = a;
		a = tmp;
	}
	for (t = 20; t < 40; t++) {
		unsigned int tmp = rol(a, 5) + (b ^ c ^ d) + e + w[t] + 0x6ed9eba1;
		e = d;
		d = c;
		c = rol(b, 30);
		b = a;
		a = tmp;
	}
	for (t = 40; t < 60; t++) {
		unsigned int tmp = rol(a,
				5) + ((b & c) | (b & d) | (c & d)) + e + w[t] +
					0x8f1bbcdc;
				e = d;
				d = c;
				c = rol(b, 30);
				b = a;
				a = tmp;
	}
	for (t = 60; t < 80; t++) {
		unsigned int tmp = rol(a, 5) + (b ^ c ^ d) + e + w[t] + 0xca62c1d6;
		e = d;
		d = c;
		c = rol(b, 30);
		b = a;
		a = tmp;
	}

	digest[0] += a;
	digest[1] += b;
	digest[2] += c;
	digest[3] += d;
	digest[4] += e;
}

/* ----------------------------------------------------------------------
 * Outer SHA algorithm: take an arbitrary length byte string,
 * convert it into 16-word blocks with the prescribed padding at
 * the end, and pass those blocks to the core SHA algorithm.
 */

void SHA_Init(SHA_State * s)
{
	SHA_Core_Init(s->h);
	s->blkused = 0;
	s->lenhi = s->lenlo = 0;
}

void SHA_Bytes(SHA_State * s, const void *p, int len)
{
	unsigned char *q = (unsigned char *) p;
	uint32_t wordblock[16];
	uint32_t lenw = len;
	int i;

	/*
	 * Update the length field.
	 */
	s->lenlo += lenw;
	s->lenhi += (s->lenlo < lenw);

	if (s->blkused && s->blkused + len < 64) {
		/*
		 * Trivial case: just add to the block.
		 */
		memcpy(s->block + s->blkused, q, len);
		s->blkused += len;
	} else {
		/*
		 * We must complete and process at least one block.
		 */
		while (s->blkused + len >= 64) {
			memcpy(s->block + s->blkused, q, 64 - s->blkused);
			q += 64 - s->blkused;
			len -= 64 - s->blkused;
			/* Now process the block. Gather bytes big-endian into words */
			for (i = 0; i < 16; i++) {
				wordblock[i] =
					(((uint32_t) s->block[i * 4 + 0]) << 24) |
					(((uint32_t) s->block[i * 4 + 1]) << 16) |
					(((uint32_t) s->block[i * 4 + 2]) << 8) |
					(((uint32_t) s->block[i * 4 + 3]) << 0);
			}
			SHATransform((unsigned int *)s->h, (unsigned int *)wordblock);
			s->blkused = 0;
		}
		memcpy(s->block, q, len);
		s->blkused = len;
	}
}

void SHA_Final(SHA_State * s, unsigned char *output)
{
	int i;
	int pad;
	unsigned char c[64];
	uint32_t lenhi, lenlo;

	if (s->blkused >= 56)
		pad = 56 + 64 - s->blkused;
	else
		pad = 56 - s->blkused;

	lenhi = (s->lenhi << 3) | (s->lenlo >> (32 - 3));
	lenlo = (s->lenlo << 3);

	memset(c, 0, pad);
	c[0] = 0x80;
	SHA_Bytes(s, &c, pad);

	c[0] = char((lenhi >> 24) & 0xFF);
	c[1] = char((lenhi >> 16) & 0xFF);
	c[2] = char((lenhi >> 8) & 0xFF);
	c[3] = char((lenhi >> 0) & 0xFF);
	c[4] = char((lenlo >> 24) & 0xFF);
	c[5] = char((lenlo >> 16) & 0xFF);
	c[6] = char((lenlo >> 8) & 0xFF);
	c[7] = char((lenlo >> 0) & 0xFF);

	SHA_Bytes(s, &c, 8);

	for (i = 0; i < 5; i++)
	{
		output[i * 4] = char((s->h[i] >> 24) & 0xFF);
		output[i * 4 + 1] = char((s->h[i] >> 16) & 0xFF);
		output[i * 4 + 2] = char((s->h[i] >> 8) & 0xFF);
		output[i * 4 + 3] = char((s->h[i]) & 0xFF);
	}
}

void SHA_Simple(const void *p, int len, unsigned char *output)
{
	SHA_State s;

	SHA_Init(&s);
	SHA_Bytes(&s, p, len);
	SHA_Final(&s, output);
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
