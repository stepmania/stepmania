#define _XOPEN_SOURCE 600
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <archutils/Darwin/VectorHelper.h>

#if 0
# error This depends on OS X and Apple's gcc.
#endif
#ifndef USE_VEC
# error Enable altivec or sse.
#endif
#define SCALE(x, l1, h1, l2, h2)	(((x) - (l1)) * ((h2) - (l2)) / ((h1) - (l1)) + (l2))

using namespace std;

// This requires that allocated memory be 16 byte aligned. Apple's new
// (and malloc) ensures this but most others only ensure that the
// memory is type size aligned. Hack in posix_memalign. We could also
// override the global new, but that's even more hassle.
#ifdef __APPLE_CC__
# define NEW(t,s) new t[(s)]
# define DELETE(x) delete[] x
#else
static void *pStupid;
# define NEW(t,s) (posix_memalign(&pStupid, 16, s*sizeof(t)), (t *)pStupid)
# define DELETE(x) free((x))
#endif

// The reference values.
static void ScalarWrite( float *pDestBuf, const float *pSrcBuf, size_t iSize )
{
	for( unsigned iPos = 0; iPos < iSize; ++iPos )
		pDestBuf[iPos] += pSrcBuf[iPos];
}

#if 0
static void ScalarRead( int16_t *pDestBuf, const int32_t *pSrcBuf, unsigned iSize )
{
	for( unsigned iPos = 0; iPos < iSize; ++iPos )
		pDestBuf[iPos] = max( -32768, min(pSrcBuf[iPos]/256, 32767) );
}
#endif

static void RandBuffer( float *pBuffer, unsigned iSize )
{
	while( iSize-- )
		*pBuffer++ = float(rand())/RAND_MAX;
}

template <typename T>
static void Diagnostic( const T *pDestBuf, const T *pRefBuf, size_t size )
{
	const int num = 10;
	for( int i = 0; i < num; ++i )
		fprintf( stderr, "%0*x ", sizeof(T)*2, pDestBuf[i] );
	puts( "" );
	for( int i = 0; i < num; ++i )
		fprintf( stderr, "%0*x ", sizeof(T)*2, pRefBuf[i] );
	puts( "" );
	for( size_t i = size - num; i < size; ++i )
		fprintf( stderr, "%0*x ", sizeof(T)*2, pDestBuf[i] );
	puts( "" );
	for( size_t i = size - num; i < size; ++i )
		fprintf( stderr, "%0*x ", sizeof(T)*2, pRefBuf[i] );
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

static bool TestWrite( float *pSrcBuf, float *pDestBuf, float *pRefBuf, size_t iSize )
{
	RandBuffer( pSrcBuf, iSize );
	memset( pDestBuf, 0, iSize * 4 );
	memset( pRefBuf, 0, iSize * 4 );
	Vector::FastSoundWrite( pDestBuf, pSrcBuf, iSize );
	ScalarWrite( pRefBuf, pSrcBuf, iSize );
	return !memcmp( pRefBuf, pDestBuf, iSize * 4 );
}

static bool CheckAlignedWrite()
{
	const size_t size = 1024;
	float *pSrcBuf  = NEW( float, size );
	float *pDestBuf = NEW( float, size );
	float *pRefBuf  = NEW( float, size );
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
	DELETE( pSrcBuf );
	DELETE( pDestBuf );
	DELETE( pRefBuf );
	return ret;
}

static bool CheckMisalignedSrcWrite()
{
	const size_t size = 1024;
	float *pSrcBuf  = NEW( float, size );
	float *pDestBuf = NEW( float, size );
	float *pRefBuf  = NEW( float, size );
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
	DELETE( pSrcBuf );
	DELETE( pDestBuf );
	DELETE( pRefBuf );
	return ret;
}

static bool CheckMisalignedDestWrite()
{
	const size_t size = 1024;
	float *pSrcBuf  = NEW( float, size );
	float *pDestBuf = NEW( float, size );
	float *pRefBuf  = NEW( float, size );
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
	DELETE( pSrcBuf );
	DELETE( pDestBuf );
	DELETE( pRefBuf );
	return ret;
}

static bool CheckMisalignedBothWrite()
{
	const size_t size = 1024;
	float *pSrcBuf  = NEW( float, size );
	float *pDestBuf = NEW( float, size );
	float *pRefBuf  = NEW( float, size );
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
	DELETE( pSrcBuf );
	DELETE( pDestBuf );
	DELETE( pRefBuf );
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
#if 0
template<typename T>
static bool CheckAlignedRead()
{
	const size_t size = 1024;
	int32_t *pSrcBuf = NEW( int32_t, size );
	T *pDestBuf      = NEW( T, size );
	T *pRefBuf       = NEW( T, size );
	bool ret = true;
	
	for( int i = 0; i < 8; ++i )
	{
		RandBuffer( pSrcBuf, size-i );
		Vector::FastSoundRead( pDestBuf, pSrcBuf, size-i );
		ScalarRead( pRefBuf, pSrcBuf, size-i );
		
		if( !(ret = cmp(pRefBuf, pDestBuf, size-i)) )
		{
			fprintf( stderr, "%d: \n", i );
			Diagnostic( pDestBuf, pRefBuf, size-i );
			break;
		}
	}

	DELETE( pSrcBuf );
	DELETE( pDestBuf );
	DELETE( pRefBuf );
	return ret;
}

template<typename T>
static bool CheckMisalignedRead()
{
	const size_t size = 1024;
	int32_t *pSrcBuf = NEW( int32_t, size );
	T *pDestBuf      = NEW( T, size );
	T *pRefBuf       = NEW( T, size );
	bool ret = true;
	
	for( int j = 0; j < 8; ++j )
	{
		for( int i = 0; i < 8; ++i )
		{
			RandBuffer( pSrcBuf, size-i );
			Vector::FastSoundRead( pDestBuf+j, pSrcBuf, size-i-j );
			ScalarRead( pRefBuf+j, pSrcBuf, size-i-j );
			
			if( !(ret = cmp(pRefBuf+j, pDestBuf+j, size-i-j)) )
			{
				fprintf( stderr, "%d, %d: \n", j, i );
				Diagnostic( pDestBuf+j, pRefBuf+j, size-i-j );
				break;
			}
		}
	}

	DELETE( pSrcBuf );
	DELETE( pDestBuf );
	DELETE( pRefBuf );
	return ret;
}
#endif

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
#if 0
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
	if( !CheckMisalignedRead<int16_t>() )
	{
		fputs( "Failed misaligned read.\n", stderr );
		return 1;
	}
	if( !CheckMisalignedRead<float>() )
	{
		fputs( "Failed misaligned float read.\n", stderr );
		return 1;
	}
#endif
	puts( "Passed." );
	return 0;
}
