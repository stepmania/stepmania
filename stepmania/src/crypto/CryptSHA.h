#ifndef SSHSHA_H
#define SSHSHA_H

#include "SDL_utils.h"

typedef struct {
	Uint32 h[5];
	unsigned char block[64];
	int blkused;
	Uint32 lenhi, lenlo;
} SHA_State;

void SHA_Init(SHA_State * s);
void SHA_Bytes(SHA_State * s, const void *p, int len);
void SHA_Final(SHA_State * s, unsigned char *output);
void SHA_Simple(const void *p, int len, unsigned char *output);
void SHATransform(unsigned int *digest, unsigned int *block);

#endif
