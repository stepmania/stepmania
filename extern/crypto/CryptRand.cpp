/*
 * cryptographic random number generator for PuTTY's ssh client
 */

#include "global.h"
#include "CryptSHA.h"
#include "CryptRand.h"

void noise_get_heavy(void (*func) (void *, int));
void noise_get_light(void (*func) (void *, int));

/*
 * `pool' itself is a pool of random data which we actually use: we
 * return bytes from `pool', at position `poolpos', until `poolpos'
 * reaches the end of the pool. At this point we generate more
 * random data, by adding noise, stirring well, and resetting
 * `poolpos' to point to just past the beginning of the pool (not
 * _the_ beginning, since otherwise we'd give away the whole
 * contents of our pool, and attackers would just have to guess the
 * next lot of noise).
 *
 * `incomingb' buffers acquired noise data, until it gets full, at
 * which point the acquired noise is SHA'ed into `incoming' and
 * `incomingb' is cleared. The noise in `incoming' is used as part
 * of the noise for each stirring of the pool, in addition to local
 * time, process listings, and other such stuff.
 */

#define HASHINPUT 64		       /* 64 bytes SHA input */
#define HASHSIZE 20		       /* 160 bits SHA output */
#define POOLSIZE 1200		       /* size of random pool */

struct RandPool
{
	unsigned char pool[POOLSIZE];
	int poolpos;

	unsigned char incoming[HASHSIZE];

	unsigned char incomingb[HASHINPUT];
	int incomingpos;
};

static struct RandPool pool;
bool random_active = false;

static void random_stir(void)
{
	unsigned block[HASHINPUT / sizeof(unsigned)];
	unsigned digest[HASHSIZE / sizeof(unsigned)];
	int i, j;

	SHATransform((unsigned *) pool.incoming, (unsigned *) pool.incomingb);
	pool.incomingpos = 0;

	/*
	 * Chunks of this code are blatantly endianness-dependent, but
	 * as it's all random bits anyway, WHO CARES?
	 */
	memcpy(digest, pool.incoming, sizeof(digest));

	/*
	 * Make two passes over the pool.
	 */
	for (i = 0; i < 2; i++) {

		/*
		 * We operate SHA in CFB mode, repeatedly adding the same
		 * block of data to the digest. But we're also fiddling
		 * with the digest-so-far, so this shouldn't be Bad or
		 * anything.
		 */
		memcpy(block, pool.pool, sizeof(block));

		/*
		 * Each pass processes the pool backwards in blocks of
		 * HASHSIZE, just so that in general we get the output of
		 * SHA before the corresponding input, in the hope that
		 * things will be that much less predictable that way
		 * round, when we subsequently return bytes ...
		 */
		for (j = POOLSIZE; (j -= HASHSIZE) >= 0;) {
			/*
			 * XOR the bit of the pool we're processing into the
			 * digest.
			 */
			unsigned k;
			for (k = 0; k < sizeof(digest) / sizeof(*digest); k++)
				digest[k] ^= ((unsigned *) (pool.pool + j))[k];

			/*
			 * Munge our unrevealed first block of the pool into
			 * it.
			 */
			SHATransform(digest, block);

			/*
			 * Stick the result back into the pool.
			 */

			for( k = 0; k < sizeof(digest) / sizeof(*digest); k++ )
				((unsigned *) (pool.pool + j))[k] = digest[k];
		}
	}

	/*
	 * Might as well save this value back into `incoming', just so
	 * there'll be some extra bizarreness there.
	 */
	SHATransform(digest, block);
	memcpy(pool.incoming, digest, sizeof(digest));

	pool.poolpos = sizeof(pool.incoming);
}

void random_add_noise( const RString &noise )
{
	unsigned char *p = (unsigned char *) noise.data();
	int i;

	if (!random_active)
		return;

	/*
	 * This function processes HASHINPUT bytes into only HASHSIZE
	 * bytes, so _if_ we were getting incredibly high entropy
	 * sources then we would be throwing away valuable stuff.
	 */
	int length = noise.size();
	while (length >= (HASHINPUT - pool.incomingpos)) {
		memcpy(pool.incomingb + pool.incomingpos, p,
				HASHINPUT - pool.incomingpos);
		p += HASHINPUT - pool.incomingpos;
		length -= HASHINPUT - pool.incomingpos;
		SHATransform((unsigned *) pool.incoming, (unsigned *) pool.incomingb);
		for (i = 0; i < HASHSIZE; i++) {
			pool.pool[pool.poolpos++] ^= pool.incomingb[i];
			if (pool.poolpos >= POOLSIZE)
				pool.poolpos = 0;
		}
		if (pool.poolpos < HASHSIZE)
			random_stir();

		pool.incomingpos = 0;
	}

	memcpy(pool.incomingb + pool.incomingpos, p, length);
	pool.incomingpos += length;
}

void random_add_heavynoise(void *noise, int length)
{
	unsigned char *p = (unsigned char *) noise;
	int i;

	while (length >= POOLSIZE) {
		for (i = 0; i < POOLSIZE; i++)
			pool.pool[i] ^= *p++;
		random_stir();
		length -= POOLSIZE;
	}

	for (i = 0; i < length; i++)
		pool.pool[i] ^= *p++;
	random_stir();
}

static void random_add_heavynoise_bitbybit(void *noise, int length)
{
	unsigned char *p = (unsigned char *) noise;
	int i;

	while (length >= POOLSIZE - pool.poolpos) {
		for (i = 0; i < POOLSIZE - pool.poolpos; i++)
			pool.pool[pool.poolpos + i] ^= *p++;
		random_stir();
		length -= POOLSIZE - pool.poolpos;
		pool.poolpos = 0;
	}

	for (i = 0; i < length; i++)
		pool.pool[i] ^= *p++;
	pool.poolpos = i;
}

void random_init(void)
{
	if( random_active )
		return;
	random_active = true;
	memset(&pool, 0, sizeof(pool));    /* just to start with */


	noise_get_heavy(random_add_heavynoise_bitbybit);
	random_stir();
}

unsigned char random_byte(void)
{
	if (pool.poolpos >= POOLSIZE)
		random_stir();

	return pool.pool[pool.poolpos++];
}

void random_get_savedata(char **data, int *len)
{
	char *buf = new char[POOLSIZE / 2];
	random_stir();
	memcpy(buf, pool.pool + pool.poolpos, POOLSIZE / 2);
	*len = POOLSIZE / 2;
	*data = buf;
	random_stir();
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
