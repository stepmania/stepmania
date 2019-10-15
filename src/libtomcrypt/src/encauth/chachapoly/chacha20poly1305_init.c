/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#include "tomcrypt.h"

#ifdef LTC_CHACHA20POLY1305_MODE

/**
   Initialize an ChaCha20Poly1305 context (only the key)
   @param st        [out] The destination of the ChaCha20Poly1305 state
   @param key       The secret key
   @param keylen    The length of the secret key (octets)
   @return CRYPT_OK if successful
*/
int chacha20poly1305_init(chacha20poly1305_state *st, const unsigned char *key, unsigned long keylen)
{
   return chacha_setup(&st->chacha, key, keylen, 20);
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
