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
  @file crypt_find_hash_oid.c
  Find a hash, Tom St Denis
*/

int find_hash_oid(const unsigned long *ID, unsigned long IDlen)
{
   int x;
   LTC_ARGCHK(ID != NULL);
   LTC_MUTEX_LOCK(&ltc_hash_mutex);
   for (x = 0; x < TAB_SIZE; x++) {
       if (hash_descriptor[x].name != NULL && hash_descriptor[x].OIDlen == IDlen && !XMEMCMP(hash_descriptor[x].OID, ID, sizeof(unsigned long) * IDlen)) {
          LTC_MUTEX_UNLOCK(&ltc_hash_mutex);
          return x;
       }
   }
   LTC_MUTEX_UNLOCK(&ltc_hash_mutex);
   return -1;
}

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
