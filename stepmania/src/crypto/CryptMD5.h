#ifndef SSHMD5_H
#define SSHMD5_H

typedef struct {
	uint32_t h[4];
} MD5_Core_State;

struct MD5Context {
	MD5_Core_State core;
	unsigned char block[64];
	int blkused;
	uint32_t lenhi, lenlo;
};

void MD5Init(struct MD5Context *context);
void MD5Update(struct MD5Context *context, unsigned char const *buf, unsigned len);
void MD5Final(unsigned char digest[16], struct MD5Context *context);

#endif
