#ifndef SSH_PRIME_H
#define SSH_PRIME_H

#include "CryptBn.h"
Bignum primegen(int bits, int modulus, int residue, Bignum factor, int phase);

#endif
