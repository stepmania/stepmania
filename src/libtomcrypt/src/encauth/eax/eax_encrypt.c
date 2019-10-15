/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/**
   @file eax_encrypt.c
   EAX implementation, encrypt block by Tom St Denis
*/
#include "tomcrypt.h"

#ifdef LTC_EAX_MODE

/**
   Encrypt with EAX a block of data.
   @param eax        The EAX state
   @param pt         The plaintext to encrypt
   @param ct         [out] The ciphertext as encrypted
   @param length     The length of the plaintext (octets)
   @return CRYPT_OK if successful
*/
int eax_encrypt(eax_state *eax, const unsigned char *pt, unsigned char *ct,
                unsigned long length)
{
   int err;

   LTC_ARGCHK(eax != NULL);
   LTC_ARGCHK(pt  != NULL);
   LTC_ARGCHK(ct  != NULL);

   /* encrypt */
   if ((err = ctr_encrypt(pt, ct, length, &eax->ctr)) != CRYPT_OK) {
      return err;
   }

   /* omac ciphertext */
   return omac_process(&eax->ctomac, ct, length);
}

#endif


/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
