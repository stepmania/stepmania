/*
 * Noise generation for PuTTY's cryptographic random number
 * generator.
 */

#include "global.h"

#if defined(_XBOX)
void noise_get_heavy(void (*func) (void *, int))
{
}

void noise_get_light(void (*func) (void *, int))
{
}

#elif defined(_WINDOWS)
#define _WIN32_WINNT 0x0400		// VC6 header needs this defined.  
#include <windows.h>
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

	if (read_dev_urandom(buf, 32))
		func(buf, 32);

#if defined(UNIX)
	FILE *fp;
	int ret;

	fp = popen("ps -axu 2>/dev/null", "r");
	while ( (ret = fread(buf, 1, sizeof(buf), fp)) > 0)
		func(buf, ret);
	pclose(fp);

	fp = popen("ls -al /tmp 2>/dev/null", "r");
	while ( (ret = fread(buf, 1, sizeof(buf), fp)) > 0)
		func(buf, ret);
	pclose(fp);
#endif
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

/*
 * PuTTY is copyright 1997-2001 Simon Tatham.
 * 
 * Portions copyright Robert de Bath, Joris van Rantwijk, Delian
 * Delchev, Andreas Schultz, Jeroen Massar, Wez Furlong, Nicolas Barry,
 * Justin Bradford, and CORE SDI S.A.
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
