#ifndef SSHMD5_H
#define SSHMD5_H

#include "SDL_utils.h"

typedef struct {
	Uint32 h[4];
} MD5_Core_State;

struct MD5Context {
	MD5_Core_State core;
	unsigned char block[64];
	int blkused;
	Uint32 lenhi, lenlo;
};

void MD5Init(struct MD5Context *context);
void MD5Update(struct MD5Context *context, unsigned char const *buf, unsigned len);
void MD5Final(unsigned char digest[16], struct MD5Context *context);

#endif
