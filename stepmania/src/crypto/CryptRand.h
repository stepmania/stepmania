#ifndef SSH_RAND_H
#define SSH_RAND_H

void random_init();
void random_add_noise( const CString &noise );
unsigned char random_byte();
void random_get_savedata( char **data, int *len );

#endif
