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
  Source donated by Elliptic Semiconductor Inc (www.ellipticsemi.com) to the LibTom Projects
*/

#ifdef LTC_XTS_MODE

/** Terminate XTS state
   @param xts    The state to terminate
*/
void xts_done(symmetric_xts *xts)
{
   LTC_ARGCHKVD(xts != NULL);
   cipher_descriptor[xts->cipher].done(&xts->key1);
   cipher_descriptor[xts->cipher].done(&xts->key2);
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
