#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <inttypes.h>
#include <algorithm>
#include <archutils/Darwin/VectorHelper.h>

#ifndef __APPLE_CC__
# error This depends on OS X and Apple's gcc.
#endif
#ifndef USE_VEC
# error Enable altivec or sse.
#endif
#define SCALE(x, l1, h1, l2, h2)	(((x) - (l1)) * ((h2) - (l2)) / ((h1) - (l1)) + (l2))

using namespace std;

// The reference values.
static void ScalarWrite( int32_t *pDestBuf, const int16_t *pSrcBuf, unsigned iSize, int iVol )
{
	for( unsigned iPos = 0; iPos < iSize; ++iPos )
		pDestBuf[iPos] += int32_t(pSrcBuf[iPos]) * iVol;
}

static void ScalarRead( int16_t *pDestBuf, const int32_t *pSrcBuf, unsigned iSize )
{
	for( unsigned iPos = 0; iPos < iSize; ++iPos )
		pDestBuf[iPos] = max( -32768, min(pSrcBuf[iPos]/256, 32767) );
}

static void ScalarRead( float *pDestBuf, const int32_t *pSrcBuf, unsigned iSize )
{
	const int iMinimum = -32768 * 256;
	const int iMaximum = 32767  * 256;
	for( unsigned iPos = 0; iPos < iSize; ++iPos )
		pDestBuf[iPos] = SCALE( (float)pSrcBuf[iPos], iMinimum, iMaximum, -1.0f, +1.0f );
}

template <typename T>
static void RandBuffer( T *pBuffer, unsigned iSize )
{
	while( iSize-- )
		*pBuffer++ = rand() % 40000;
}

template <typename T>
static void Diagnostic( const T *pDestBuf, const T *pRefBuf, size_t size )
{
	const int num = 10;
	for( int i = 0; i < num; ++i )
		fprintf( stderr, "%x ", pDestBuf[i] );
	puts( "" );
	for( int i = 0; i < num; ++i )
		fprintf( stderr, "%x ", pRefBuf[i] );
	puts( "" );
	for( size_t i = size - num; i < size; ++i )
		fprintf( stderr, "%x ", pDestBuf[i] );
	puts( "" );
	for( size_t i = size - num; i < size; ++i )
		fprintf( stderr, "%x ", pRefBuf[i] );
	puts( "" );
}

template<>
static void Diagnostic<float>( const float *pDestBuf, const float *pRefBuf, size_t size )

{
	const int num = 10;
	for( int i = 0; i < num; ++i )
		fprintf( stderr, "%f ", pDestBuf[i] );
	puts( "" );
	for( int i = 0; i < num; ++i )
		fprintf( stderr, "%f ", pRefBuf[i] );
	puts( "" );
	for( size_t i = size - num; i < size; ++i )
		fprintf( stderr, "%f ", pDestBuf[i] );
	puts( "" );
	for( size_t i = size - num; i < size; ++i )
		fprintf( stderr, "%f ", pRefBuf[i] );
	puts( "" );
}

static bool TestWrite( int16_t *pSrcBuf, int32_t *pDestBuf, int32_t *pRefBuf, size_t iSize )
{
	const int iVol = 237;

	RandBuffer( pSrcBuf, iSize );
	memset( pDestBuf, 0, iSize * 4 );
	memset( pRefBuf, 0, iSize * 4 );
	Vector::FastSoundWrite( pDestBuf, pSrcBuf, iSize, iVol );
	ScalarWrite( pRefBuf, pSrcBuf, iSize, iVol );
	return !memcmp( pRefBuf, pDestBuf, iSize * 4 );
}

static bool CheckAlignedWrite()
{
	const size_t size = 1024;
	int16_t *pSrcBuf = new int16_t[size];
	int32_t *pDestBuf = new int32_t[size];
	int32_t *pRefBuf = new int32_t[size];
	assert( (intptr_t(pSrcBuf)  & 0xF) == 0 );
	assert( (intptr_t(pDestBuf) & 0xF) == 0 );
	assert( (intptr_t(pRefBuf)  & 0xF) == 0 );
	bool ret = true;
	size_t s;

	// Test unaligned ends
	for( int i = 0; i < 16 && ret; ++i )
	{
		s = size - i;
		ret = TestWrite( pSrcBuf, pDestBuf, pRefBuf, s );
	}
	if( !ret )
		Diagnostic( pDestBuf, pRefBuf, s );
	delete[] pSrcBuf;
	delete[] pDestBuf;
	delete[] pRefBuf;
	return ret;
}

static bool CheckMisalignedSrcWrite()
{
	const size_t size = 1024;
	int16_t *pSrcBuf = new int16_t[size];
	int32_t *pDestBuf = new int32_t[size];
	int32_t *pRefBuf = new int32_t[size];
	assert( (intptr_t(pSrcBuf)  & 0xF) == 0 );
	assert( (intptr_t(pDestBuf) & 0xF) == 0 );
	assert( (intptr_t(pRefBuf)  & 0xF) == 0 );
	bool ret = true;

	for( int j = 0; j < 8 && ret; ++j )
	{
		size_t s;
		for( int i = 0; i < 8 && ret; ++i )
		{
			s = size - i - j; // Source buffer is shrinking.
			ret = TestWrite( pSrcBuf+j, pDestBuf, pRefBuf, s );
		}
		if( !ret )
			Diagnostic( pDestBuf, pRefBuf, s );
	}
	delete[] pSrcBuf;
	delete[] pDestBuf;
	delete[] pRefBuf;
	return ret;
}

static bool CheckMisalignedDestWrite()
{
	const size_t size = 1024;
	int16_t *pSrcBuf = new int16_t[size];
	int32_t *pDestBuf = new int32_t[size];
	int32_t *pRefBuf = new int32_t[size];
	assert( (intptr_t(pSrcBuf)  & 0xF) == 0 );
	assert( (intptr_t(pDestBuf) & 0xF) == 0 );
	assert( (intptr_t(pRefBuf)  & 0xF) == 0 );
	bool ret = true;

	for( int j = 0; j < 4 && ret; ++j )
	{
		size_t s;
		for( int i = 0; i < 8 && ret; ++i )
		{
			s = size - i - j; // Dest buffer is shrinking.
			ret = TestWrite( pSrcBuf, pDestBuf+j, pRefBuf+j, s );
		}
		if( !ret )
			Diagnostic( pDestBuf+j, pRefBuf+j, s );
	}
	delete[] pSrcBuf;
	delete[] pDestBuf;
	delete[] pRefBuf;
	return ret;
}

static bool CheckMisalignedBothWrite()
{
	const size_t size = 1024;
	int16_t *pSrcBuf = new int16_t[size];
	int32_t *pDestBuf = new int32_t[size];
	int32_t *pRefBuf = new int32_t[size];
	assert( (intptr_t(pSrcBuf)  & 0xF) == 0 );
	assert( (intptr_t(pDestBuf) & 0xF) == 0 );
	assert( (intptr_t(pRefBuf)  & 0xF) == 0 );
	bool ret = true;
	size_t s;

	for( int j = 0; j < 4 && ret; ++j )
	{
		for( int i = 0; i < 8 && ret; ++i )
		{
			for( int k = 0; k < 8 && ret; ++k )
			{
				s = size - i - j - k; // Both buffers are shrinking.
				ret = TestWrite( pSrcBuf+i, pDestBuf+j, pRefBuf+j, s );
			}
		}
		if( !ret )
			Diagnostic( pDestBuf+j, pRefBuf+j, s );
	}
	delete[] pSrcBuf;
	delete[] pDestBuf;
	delete[] pRefBuf;
	return ret;
}

static bool cmp( const int16_t *p1, const int16_t *p2, size_t size )
{
	return !memcmp( p1, p2, size * 2 );
}

static bool cmp( const float *p1, const float *p2, size_t size )
{
	const float epsilon = 0.000001;
	++size;
	while( --size )
		if( fabs(*p1++ - *p2++) >= epsilon )
			return false;
	return true;
}

template<typename T>
static bool CheckAlignedRead()
{
	const size_t size = 1024;
	int32_t *pSrcBuf = new int32_t[size];
	T *pDestBuf = new T[size];
	T *pRefBuf = new T[size];
	assert( (intptr_t(pSrcBuf)  & 0xF) == 0 );
	assert( (intptr_t(pDestBuf) & 0xF) == 0 );
	assert( (intptr_t(pRefBuf)  & 0xF) == 0 );
	RandBuffer( pSrcBuf, size );
	Vector::FastSoundRead( pDestBuf, pSrcBuf, size );
	ScalarRead( pRefBuf, pSrcBuf, size );
	bool ret = cmp( pRefBuf, pDestBuf, size );

	if( !ret )
		Diagnostic( pDestBuf, pRefBuf, size );
	delete[] pSrcBuf;
	delete[] pDestBuf;
	delete[] pRefBuf;
	return ret;
}

int main()
{
	srand( time(NULL) );
	if( !Vector::CheckForVector() )
	{
		fputs( "No vector unit accessable.\n", stderr );
		return 1;
	}
	if( !CheckAlignedWrite() )
	{
		fputs( "Failed aligned write.\n", stderr );
		return 1;
	}
	if( !CheckMisalignedSrcWrite() )
	{
		fputs( "Failed misaligned source write.\n", stderr );
		return 1;
	}
	if( !CheckMisalignedDestWrite() )
	{
		fputs( "Failed misaligned destination write\n", stderr );
		return 1;
	}
	if( !CheckMisalignedBothWrite() )
	{
		fputs( "Failed misaligned source and destination write.\n", stderr );
		return 1;
	}
	if( !CheckAlignedRead<int16_t>() )
	{
		fputs( "Failed aligned read.\n", stderr );
		return 1;
	}
	if( !CheckAlignedRead<float>() )
	{
		fputs( "Failed aligned float read.\n", stderr );
		return 1;
	}
	puts( "Passed." );
	return 0;
}
