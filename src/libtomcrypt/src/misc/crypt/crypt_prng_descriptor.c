/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include "tomcrypt.h"

/**
  @file crypt_prng_descriptor.c
  Stores the PRNG descriptors, Tom St Denis
*/
struct ltc_prng_descriptor prng_descriptor[TAB_SIZE] = {
{ NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};

LTC_MUTEX_GLOBAL(ltc_prng_mutex)


/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
