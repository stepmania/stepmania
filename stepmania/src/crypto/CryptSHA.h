#ifndef SSHSHA_H
#define SSHSHA_H

typedef struct {
	uint32_t h[5];
	unsigned char block[64];
	int blkused;
	uint32_t lenhi, lenlo;
} SHA_State;

void SHA_Init(SHA_State * s);
void SHA_Bytes(SHA_State * s, const void *p, int len);
void SHA_Final(SHA_State * s, unsigned char *output);
void SHA_Simple(const void *p, int len, unsigned char *output);
void SHATransform(unsigned int *digest, unsigned int *block);

#endif
