// osrng.cpp - written and placed in the public domain by Wei Dai

// Thanks to Leonard Janke for the suggestion for AutoSeededRandomPool.

#include "pch.h"
#include "osrng.h"

#ifdef OS_RNG_AVAILABLE

#ifdef CRYPTOPP_WIN32_AVAILABLE
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#include <windows.h>
#include <wincrypt.h>
#endif

#if defined(CRYPTOPP_UNIX_AVAILABLE)
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace CryptoPP {

#if defined(NONBLOCKING_RNG_AVAILABLE) || defined(BLOCKING_RNG_AVAILABLE)
OS_RNG_Err::OS_RNG_Err(const std::string &operation)
	: Exception(OTHER_ERROR, "OS_Rng: " + operation + " operation failed with error " + 
#ifdef CRYPTOPP_WIN32_AVAILABLE
		"0x" + IntToString(GetLastError(), 16)
#elif defined(CRYPTOPP_UNIX_AVAILABLE)
		IntToString(errno)
#else
		"(unknown)"
#endif
		)
{
}
#endif

#ifdef CRYPTOPP_WIN32_AVAILABLE

MicrosoftCryptoProvider::MicrosoftCryptoProvider()
{
	if(!CryptAcquireContext(&m_hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	{
		char buf[1024] = "";

		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
			0, GetLastError(), 0, buf, sizeof(buf), NULL);
		throw OS_RNG_Err( std::string("CryptAcquireContext: ") + buf);
	}
}

MicrosoftCryptoProvider::~MicrosoftCryptoProvider()
{
	CryptReleaseContext(m_hProvider, 0);
}

#endif

NonblockingRng::NonblockingRng()
{
#if defined(CRYPTOPP_UNIX_AVAILABLE)
	m_fd = open("/dev/urandom",O_RDONLY);
	if (m_fd == -1)
		throw OS_RNG_Err("open /dev/urandom");
#endif
}

NonblockingRng::~NonblockingRng()
{
#if defined(CRYPTOPP_UNIX_AVAILABLE)
	close(m_fd);
#endif
}

byte NonblockingRng::GenerateByte()
{
	byte b;
	GenerateBlock(&b, 1);
	return b;
}

void NonblockingRng::GenerateBlock(byte *output, unsigned int size)
{
#ifdef CRYPTOPP_WIN32_AVAILABLE
#	ifdef WORKAROUND_MS_BUG_Q258000
		static MicrosoftCryptoProvider m_Provider;
#	endif
	if (!CryptGenRandom(m_Provider.GetProviderHandle(), size, output))
		throw OS_RNG_Err("CryptGenRandom");
#elif defined(CRYPTOPP_UNIX_AVAILABLE)
	if (read(m_fd, output, size) != int(size))
		throw OS_RNG_Err("read /dev/urandom");
#else
#warning No crypto source
	memset( output, 0, size );
#endif
}

}

#endif
