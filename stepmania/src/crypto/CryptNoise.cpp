/*
 * Noise generation for PuTTY's cryptographic random number
 * generator.
 */

#include "global.h"

#if defined(WIN32)
#define _WIN32_WINNT 0x0400		// VC6 header needs this defined.  
#include <wincrypt.h>

/*
 * This function is called once, at PuTTY startup, and will do some
 * seriously silly things like listing directories and getting disk
 * free space and a process snapshot.
 */
void noise_get_heavy(void (*func) (void *, int))
{
	static HCRYPTPROV hProv = NULL;
	if( hProv == NULL )
	{
		if( !CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) )
			RageException::Throw( "CryptAcquireContext" );
	}

	char buf[2048];
	if ( !CryptGenRandom(hProv, sizeof(buf), (BYTE*) buf) )
			RageException::Throw( "CryptGenRandom" );

	func( buf, sizeof(buf) );
}

/*
 * This function is called every time the random pool needs
 * stirring, and will acquire the system time in all available
 * forms and the battery status.
 */
void noise_get_light(void (*func) (void *, int))
{
	SYSTEMTIME systime;
	DWORD adjust[2];
	BOOL rubbish;

	GetSystemTime(&systime);
	func(&systime, sizeof(systime));

	GetSystemTimeAdjustment(&adjust[0], &adjust[1], &rubbish);
	func(&adjust, sizeof(adjust));
}

#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

static int read_dev_urandom(char *buf, int len)
{
	int fd;
	int ngot, ret;

	fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0)
		return 0;

	ngot = 0;
	while (ngot < len)
	{
		ret = read(fd, buf+ngot, len-ngot);
		if (ret < 0) {
			close(fd);
			return 0;
		}
		ngot += ret;
	}

	return 1;
}

/*
 * This function is called once, at PuTTY startup. It will do some
 * slightly silly things such as fetching an entire process listing
 * and scanning /tmp, load the saved random seed from disk, and
 * also read 32 bytes out of /dev/urandom.
 */

void noise_get_heavy(void (*func) (void *, int))
{
	char buf[512];
	FILE *fp;
	int ret;

	if (read_dev_urandom(buf, 32))
		func(buf, 32);

	fp = popen("ps -axu 2>/dev/null", "r");
	while ( (ret = fread(buf, 1, sizeof(buf), fp)) > 0)
		func(buf, ret);
	pclose(fp);

	fp = popen("ls -al /tmp 2>/dev/null", "r");
	while ( (ret = fread(buf, 1, sizeof(buf), fp)) > 0)
		func(buf, ret);
	pclose(fp);
}

/*
 * This function is called every time the random pool needs
 * stirring, and will acquire the system time.
 */
void noise_get_light(void (*func) (void *, int))
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	func(&tv, sizeof(tv));
}

#endif
