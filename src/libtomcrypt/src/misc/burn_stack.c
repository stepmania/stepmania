/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tomstdenis@gmail.com, http://libtomcrypt.com
 */
#include "tomcrypt.h"

/**
   @file burn_stack.c
   Burn stack, Tom St Denis
*/

/**
   Burn some stack memory
   @param len amount of stack to burn in bytes
*/
void burn_stack(unsigned long len)
{
   unsigned char buf[32];
   zeromem(buf, sizeof(buf));
   if (len > (unsigned long)sizeof(buf))
      burn_stack(len - sizeof(buf));
}



/* $Source$ */
/* $Revision: 24838 $ */
/* $Date: 2007-01-23 23:16:57 -0600 (Tue, 23 Jan 2007) $ */
