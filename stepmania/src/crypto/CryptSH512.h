#ifndef SSH_SHA512_H
#define SSH_SHA512_H

#include "SDL_utils.h"

typedef struct
{
	unsigned long hi, lo;
} uint64, int64;

typedef struct
{
	uint64 h[8];
	unsigned char block[128];
	int blkused;
	Uint32 len[4];
} SHA512_State;

void SHA512_Init(SHA512_State * s);
void SHA512_Bytes(SHA512_State * s, const void *p, int len);
void SHA512_Final(SHA512_State * s, unsigned char *output);
void SHA512_Simple(const void *p, int len, unsigned char *output);

#endif
