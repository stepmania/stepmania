#include "VectorHelper.h"
#include <sys/sysctl.h>
#include <algorithm>

using std::min;
using std::max;

#if defined(USE_VEC)
#if defined(__VEC__)
#include <vecLib/vecLib.h>
#ifndef __VECLIBTYPES__
// Copy this from the header since it isn't in the 10.2.8 sdk
typedef vector unsigned char            vUInt8;
typedef vector signed char              vSInt8;
typedef vector unsigned short           vUInt16;
typedef vector signed short             vSInt16;
typedef vector unsigned int             vUInt32;
typedef vector signed int               vSInt32;
typedef vector float                    vFloat;
typedef vector bool int                 vBool32;
#endif

bool Vector::CheckForVector()
{
	int selectors[2] = { CTL_HW, HW_VECTORUNIT };
	int32_t result = 0;
	size_t length = 4;
	
	return !sysctl( selectors, 2, &result, &length, NULL, 0 ) && result;
}

/* for( size_t pos = 0; pos < size; ++pos )
 *         dest[pos] += src[pos] * volume;
 * Idea from: http://developer.apple.com/hardwaredrivers/ve/downloads/add.c */
void Vector::FastSoundWrite( int32_t *dest, const int16_t *src, unsigned size, short volume )
{
	if( size > 7 )
	{
		int index = 0;
		vUInt8 one = (vUInt8)(1);
		vUInt8 volMask = vec_lvsl( 0, &volume );
		vSInt16 vol = vec_lde( 0, &volume );
		
		vol = vec_splat( vec_perm(vol, vol, volMask), 0 );
		
		// Setup the masks.
		vUInt8 srcMask = vec_add( vec_lvsl(15, src), one );
		vUInt8 loadMask = vec_add( vec_lvsl(15, dest), one );
		vUInt8 storeMask = vec_lvsr( 0, dest ); // I have no idea why shift right for stores.
		vSInt16 load1Src = vec_ld( 0, src );
		vSInt32 load1Dest = vec_ld( 0, dest );
		vSInt32 store = (vSInt32)(0);
		
		// If dest is misaligned, pull first loop iteration out.
		if( intptr_t(dest) & 0xF )
		{
			vSInt16 load2Src  = vec_ld( 15, src );
			vSInt32 load2Dest = vec_ld( 15, dest );
			vSInt32 load3Dest = vec_ld( 31, dest );

			load1Src  = vec_perm( load1Src,  load2Src,  srcMask );
			load1Dest = vec_perm( load1Dest, load2Dest, loadMask );
			load2Dest = vec_perm( load2Dest, load3Dest, loadMask );
			
			/* Multiply the even 2-byte elements in data with those in vol to get
			 * 4-byte elements. Do the same with the odd elements then merge both
			 * high and low halves of the vectors into two new 4-element, 4-byte
			 * vectors. In this way the combined vector <first,second> contains
			 * the 8 products in the correct order. */
			vSInt32 even = vec_mule( load1Src, vol );
			vSInt32 odd  = vec_mulo( load1Src, vol );
			vSInt32 first  = vec_mergeh( even, odd );
			vSInt32 second = vec_mergel( even, odd );
			
			load1Dest = vec_add(  load1Dest, first );
			load2Dest = vec_add(  load2Dest, second );
			store     = vec_perm( load1Dest, load1Dest, storeMask );
			load1Dest = vec_perm( load1Dest, load2Dest, storeMask );
			
			while( (intptr_t(dest) + index) & 0xC )
			{
				vec_ste( store, index, dest );
				index += 4;
			}
			vec_st( load1Dest, index, dest );
			
			load1Src  = load2Src;
			load1Dest = load3Dest;
			store     = load2Dest;
			src  += 8;
			dest += 8;
			size -= 8;
			/* Incrementing the index is supposed to have the same effect
			 * as incrementing dest bust since we read from dest as well
			 * we don't want to increment twice so decrement the index. */
			index -= 16;
		}
		while( size >= 32 )
		{
			vSInt16 load2Src  = vec_ld(  15, src );
			vSInt16 load3Src  = vec_ld(  31, src );
			vSInt16 load4Src  = vec_ld(  47, src );
			vSInt16 load5Src  = vec_ld(  63, src );
			vSInt32 load2Dest = vec_ld(  15, dest );
			vSInt32 load3Dest = vec_ld(  31, dest );
			vSInt32 load4Dest = vec_ld(  47, dest );
			vSInt32 load5Dest = vec_ld(  63, dest );
			vSInt32 load6Dest = vec_ld(  79, dest );
			vSInt32 load7Dest = vec_ld(  95, dest );
			vSInt32 load8Dest = vec_ld( 111, dest );
			vSInt32 load9Dest = vec_ld( 127, dest );
			
			// Align the data
			load1Src  = vec_perm( load1Src,  load2Src,  srcMask );
			load2Src  = vec_perm( load2Src,  load3Src,  srcMask );
			load3Src  = vec_perm( load3Src,  load4Src,  srcMask );
			load4Src  = vec_perm( load4Src,  load5Src,  srcMask );
			// Not load5Src, it's untouched and used later.
			load1Dest = vec_perm( load1Dest, load2Dest, loadMask );
			load2Dest = vec_perm( load2Dest, load3Dest, loadMask );
			load3Dest = vec_perm( load3Dest, load4Dest, loadMask );
			load4Dest = vec_perm( load4Dest, load5Dest, loadMask );
			load5Dest = vec_perm( load5Dest, load6Dest, loadMask );
			load6Dest = vec_perm( load6Dest, load7Dest, loadMask );
			load7Dest = vec_perm( load7Dest, load8Dest, loadMask );
			load8Dest = vec_perm( load8Dest, load9Dest, loadMask );
			// Not load9Dest.
			
			vSInt32 even1   = vec_mule( load1Src, vol );
			vSInt32 odd1    = vec_mulo( load1Src, vol );
			vSInt32 even2   = vec_mule( load2Src, vol );
			vSInt32 odd2    = vec_mulo( load2Src, vol );
			vSInt32 even3   = vec_mule( load3Src, vol );
			vSInt32 odd3    = vec_mulo( load3Src, vol );
			vSInt32 even4   = vec_mule( load4Src, vol );
			vSInt32 odd4    = vec_mulo( load4Src, vol );
			
			vSInt32 first   = vec_mergeh( even1, odd1 );
			vSInt32 second  = vec_mergel( even1, odd1 );
			vSInt32 third   = vec_mergeh( even2, odd2 );
			vSInt32 fourth  = vec_mergel( even2, odd2 );
			vSInt32 fifth   = vec_mergeh( even3, odd3 );
			vSInt32 sixth   = vec_mergel( even3, odd3 );
			vSInt32 seventh = vec_mergeh( even4, odd4 );
			vSInt32 eighth  = vec_mergel( even4, odd4 );
			
			load1Dest = vec_add( load1Dest, first );
			load2Dest = vec_add( load2Dest, second );
			load3Dest = vec_add( load3Dest, third );
			load4Dest = vec_add( load4Dest, fourth );
			load5Dest = vec_add( load5Dest, fifth );
			load6Dest = vec_add( load6Dest, sixth );
			load7Dest = vec_add( load7Dest, seventh );
			load8Dest = vec_add( load8Dest, eighth );
			
			// Unalign results.
			store     = vec_perm( store,     load1Dest, storeMask );
			load1Dest = vec_perm( load1Dest, load2Dest, storeMask );
			load2Dest = vec_perm( load2Dest, load3Dest, storeMask );
			load3Dest = vec_perm( load3Dest, load4Dest, storeMask );
			load4Dest = vec_perm( load4Dest, load5Dest, storeMask );
			load5Dest = vec_perm( load5Dest, load6Dest, storeMask );
			load6Dest = vec_perm( load6Dest, load7Dest, storeMask );
			load7Dest = vec_perm( load7Dest, load8Dest, storeMask );

			// store the results
			vec_st( store,     index +   0, dest );
			vec_st( load1Dest, index +  16, dest );
			vec_st( load2Dest, index +  32, dest );
			vec_st( load3Dest, index +  48, dest );
			vec_st( load4Dest, index +  64, dest );
			vec_st( load5Dest, index +  80, dest );
			vec_st( load6Dest, index +  96, dest );
			vec_st( load7Dest, index + 112, dest );
			
			load1Src  = load5Src;
			load1Dest = load9Dest;
			store     = load8Dest;
			dest += 32;
			src  += 32;
			size -= 32;
		}
		/* This completely baffles gcc's loop unrolling. If I make it > 7 instead,
		 * then gcc produces 4 identical copies of the loop without scheduling them
		 * in a sane manner (hence the manual unrolling above) but this loop will
		 * never be executed more than 3 times so that code will never be used.
		 * This produces code the way gcc _should_ do it by unrolling and scheduling
		 * and then producing the rolled version. */
		while( size & ~0x7 )
		{
			vSInt16 load2Src  = vec_ld( 15, src );
			vSInt32 load2Dest = vec_ld( 15, dest );
			vSInt32 load3Dest = vec_ld( 31, dest );
			
			load1Src  = vec_perm( load1Src,  load2Src,  srcMask );
			load1Dest = vec_perm( load1Dest, load2Dest, loadMask );
			load2Dest = vec_perm( load2Dest, load3Dest, loadMask );
			
			vSInt32 even = vec_mule( load1Src, vol );
			vSInt32 odd  = vec_mulo( load1Src, vol );
			vSInt32 first  = vec_mergeh( even, odd );
			vSInt32 second = vec_mergel( even, odd );
			
			load1Dest = vec_add( load1Dest, first );
			load2Dest = vec_add( load2Dest, second );
			store     = vec_perm( store,     load1Dest, storeMask );
			load1Dest = vec_perm( load1Dest, load2Dest, storeMask );
			
			vec_st( store,     index +  0, dest );
			vec_st( load1Dest, index + 16, dest );
			
			load1Src  = load2Src;
			load1Dest = load3Dest;
			store     = load2Dest;
			src  += 8;
			dest += 8;
			size -= 8;
		}
		
		// Store the remainder of the vector, if it was misaligned.
		if( index < 0 )
		{
			store = vec_perm( store, store, storeMask );
			while( index < 0 )
			{
				vec_ste( store, index, dest );
				index += 4;
			}
		}
	}
	/* If we account for both misaligned dest and src, there is really no way to
	 * do this in vector code so do the last at most 7 elements in scalar code. */
	while( size-- )
		*(dest++) += *(src++) * volume;
}

/* for( size_t pos = 0; pos < size; ++pos )
 *         dest[pos] = clamp( src[pos]/256, -32768, 32767 );
 */
void Vector::FastSoundRead( int16_t *dest, const int32_t *src, unsigned size )
{
	int index = 0;
	vSInt32 zero = (vSInt32)( 0 );
	vUInt32 shift = (vUInt32)( 8 );
	vSInt16 store = (vSInt16)( 0 );
	vUInt8 storeMask = vec_lvsr( 0, dest );	// Setup the store mask.

	if( intptr_t(dest) & 0xF )
	{
		index -= intptr_t(dest) & 0xF;
		store = vec_ld( 0, dest );
	}
	
	/* This is tricky. We need to divide signed 4-byte integers by 256 and stuff
	 * them into 2-byte integers. First, find the elements which are negative
	 * by comparing to zero (those less than zero will have each bit in the
	 * 32-bit element set to 1 and those at least zero will have them all set
	 * to 0). Take the absolute value (it actually subtracts the vector from zero
	 * and computes the max to do that), shift right by 8 bits, use the masks
	 * to get vectors containing only those elements which were negative and
	 * subtract twice. Use saturated arithmatic to deal with overflow. Lastly,
	 * pack the two vectors into signed 2-byte integers (again saturated). */
	while( size >= 32 )
	{
		// Use LRU load which marks the address as LRU. Does nothing on the G5.
		vSInt32 first   = vec_ldl(   0, src );
		vSInt32 second  = vec_ldl(  16, src );
		vSInt32 third   = vec_ldl(  32, src );
		vSInt32 fourth  = vec_ldl(  48, src );
		vSInt32 fifth   = vec_ldl(  64, src );
		vSInt32 sixth   = vec_ldl(  80, src );
		vSInt32 seventh = vec_ldl(  96, src );
		vSInt32 eighth  = vec_ldl( 112, src );
		
		vSInt32 temp1 = (vSInt32)vec_cmplt( first,   zero );
		vSInt32 temp2 = (vSInt32)vec_cmplt( second,  zero );
		vSInt32 temp3 = (vSInt32)vec_cmplt( third,   zero );
		vSInt32 temp4 = (vSInt32)vec_cmplt( fourth,  zero );
		vSInt32 temp5 = (vSInt32)vec_cmplt( fifth,   zero );
		vSInt32 temp6 = (vSInt32)vec_cmplt( sixth,   zero );
		vSInt32 temp7 = (vSInt32)vec_cmplt( seventh, zero );
		vSInt32 temp8 = (vSInt32)vec_cmplt( eighth,  zero );
		
		first   = vec_sr( vec_abss(first),   shift );
		second  = vec_sr( vec_abss(second),  shift );
		third   = vec_sr( vec_abss(third),   shift );
		fourth  = vec_sr( vec_abss(fourth),  shift );
		fifth   = vec_sr( vec_abss(fifth),   shift );
		sixth   = vec_sr( vec_abss(sixth),   shift );
		seventh = vec_sr( vec_abss(seventh), shift );
		eighth  = vec_sr( vec_abss(eighth),  shift );
		
		temp1 = vec_and( first,   temp1 );
		temp2 = vec_and( second,  temp2 );
		temp3 = vec_and( third,   temp3 );
		temp4 = vec_and( fourth,  temp4 );
		temp5 = vec_and( fifth,   temp5 );
		temp6 = vec_and( sixth,   temp6 );
		temp7 = vec_and( seventh, temp7 );
		temp8 = vec_and( eighth,  temp8 );
		
		first   = vec_subs( vec_sub(first,   temp1), temp1 );
		second  = vec_subs( vec_sub(second,  temp2), temp2 );
		third   = vec_subs( vec_sub(third,   temp3), temp3 );
		fourth  = vec_subs( vec_sub(fourth,  temp4), temp4 );
		fifth   = vec_subs( vec_sub(fifth,   temp5), temp5 );
		sixth   = vec_subs( vec_sub(sixth,   temp6), temp6 );
		seventh = vec_subs( vec_sub(seventh, temp7), temp7 );
		eighth  = vec_subs( vec_sub(eighth,  temp8), temp8 );
		
		vSInt16 result1 = vec_packs( first,   second );
		vSInt16 result2 = vec_packs( third,   fourth );
		vSInt16 result3 = vec_packs( fifth,   sixth  );
		vSInt16 result4 = vec_packs( seventh, eighth );
		store   = vec_perm( store,   result1, storeMask );
		result1 = vec_perm( result1, result2, storeMask );
		result2 = vec_perm( result2, result3, storeMask );
		result3 = vec_perm( result3, result4, storeMask );
		vec_st( store,    0, dest );
		vec_st( result1, 16, dest );
		vec_st( result2, 32, dest );
		vec_st( result3, 48, dest );
		
		store = result4;
		dest += 32;
		src  += 32;
		size -= 32;
	}
	// Befuddle optimizer as above.	
	while( size & ~0x7 )
	{
		vSInt32 first  = vec_ldl( 0, src );
		vSInt32 second = vec_ldl( 16, src );
		vSInt32 temp1  = (vSInt32)vec_cmplt( first,  zero );
		vSInt32 temp2  = (vSInt32)vec_cmplt( second, zero );
		
		first  = vec_sr( vec_abss(first),  shift );
		second = vec_sr( vec_abss(second), shift );
		
		temp1 = vec_and( first,  temp1 );
		temp2 = vec_and( second, temp2 );
		
		first  = vec_subs( vec_sub(first,  temp1), temp1 );
		second = vec_subs( vec_sub(second, temp2), temp2 );
		
		vSInt16 result = vec_packs( first, second );
		vec_st( vec_perm(store, result, storeMask), 0, dest );
		
		store = result;
		dest += 8;
		src  += 8;
		size -= 8;
	}
	store = vec_perm( store, store, storeMask );
	int temp = index;
	while( index < 0 )
	{
		vec_ste( store, index, dest );
		index += 2;
	}
	temp >>=1;
	dest += temp;
	src  += temp;
	size -= temp;
	while( size-- )
		*dest++ = max( -32768, min(*src++>>8, 32767) );
}

/* for( size_t pos = 0; pos < size; ++pos )
 *         dest[pos] = SCALE( float(src[pos]), -32768*256, 32767*256, -1.0f, 1.0f );
 */
void Vector::FastSoundRead( float *dest, const int32_t *src, unsigned size )
{
	/* m = -32768; M = 32767
	 * (x-2^8*m)(1-(-1))/(2^8*M-2^8*m)+(-1)
	 * = ((x-2^8*m)/(2^8))*(2/(M-m))+(-1)
	 * = ((x-2^8*m)/(2^16))*((2*2^8)/(M-m))+(-1)
	 * l1 = 2^8*m = -8388608
	 * scale = 2*2^8/(M-m) = 0.00781261921110856794
	 * l2 = -1 */
	int index = 0;
	vFloat scale = (vFloat) ( 0.00781261921110856794f );
	vSInt32 l1 = (vSInt32) ( -8388608 );
	vFloat l2 = (vFloat) ( -1.0f );
	vUInt8 storeMask = vec_lvsr( 0, dest ); // Setup the store mask.
	vFloat st = (vFloat)( 0.0f );
	
	if( intptr_t(dest) & 0xF )
	{
		index -= intptr_t(dest) & 0xF;
		st = vec_ld( 0, dest );
	}
	while( size >= 32 )
	{
		/* By far the simplest of these, we need only perform the scale
		 * operation which amounts to subtracting l1, converting to a float,
		 * multiplying by a constant, and adding l2. We can multiply and add
		 * in one instruction. The 16 in vec_ctf(X,16) divides by 2^16. */ 
		vSInt32 x1 = vec_ldl(   0, src );
		vSInt32 x2 = vec_ldl(  16, src );
		vSInt32 x3 = vec_ldl(  32, src );
		vSInt32 x4 = vec_ldl(  48, src );
		vSInt32 x5 = vec_ldl(  64, src );
		vSInt32 x6 = vec_ldl(  80, src );
		vSInt32 x7 = vec_ldl(  96, src );
		vSInt32 x8 = vec_ldl( 112, src );
		
		x1 = vec_subs( x1, l1 );
		x2 = vec_subs( x2, l1 );
		x3 = vec_subs( x3, l1 );
		x4 = vec_subs( x4, l1 );
		x5 = vec_subs( x5, l1 );
		x6 = vec_subs( x6, l1 );
		x7 = vec_subs( x7, l1 );
		x8 = vec_subs( x8, l1 );
		
		vFloat f1 = vec_ctf( x1, 16 );
		vFloat f2 = vec_ctf( x2, 16 );
		vFloat f3 = vec_ctf( x3, 16 );
		vFloat f4 = vec_ctf( x4, 16 );
		vFloat f5 = vec_ctf( x5, 16 );
		vFloat f6 = vec_ctf( x6, 16 );
		vFloat f7 = vec_ctf( x7, 16 );
		vFloat f8 = vec_ctf( x8, 16 );
		
		f1 = vec_madd( f1, scale, l2 );
		f2 = vec_madd( f2, scale, l2 );
		f3 = vec_madd( f3, scale, l2 );
		f4 = vec_madd( f4, scale, l2 );
		f5 = vec_madd( f5, scale, l2 );
		f6 = vec_madd( f6, scale, l2 );
		f7 = vec_madd( f7, scale, l2 );
		f8 = vec_madd( f8, scale, l2 );
		
		st = vec_perm( st, f1, storeMask );
		f1 = vec_perm( f1, f2, storeMask );
		f2 = vec_perm( f2, f3, storeMask );
		f3 = vec_perm( f3, f4, storeMask );
		f4 = vec_perm( f4, f5, storeMask );
		f5 = vec_perm( f5, f6, storeMask );
		f6 = vec_perm( f6, f7, storeMask );
		f7 = vec_perm( f7, f8, storeMask );
		
		vec_st( st,   0, dest );
		vec_st( f1,  16, dest );
		vec_st( f2,  32, dest );
		vec_st( f3,  48, dest );
		vec_st( f4,  64, dest );
		vec_st( f5,  80, dest );
		vec_st( f6,  96, dest );
		vec_st( f7, 112, dest );
		st = f8;
		
		dest += 32;
		src  += 32;
		size -= 32;
	}
	while( size & ~0x3 )
	{
		vFloat result = vec_ctf( vec_subs(vec_ldl(0, src), l1), 16 );
		result = vec_madd( result, scale, l2 );
		
		st = vec_perm( st, result, storeMask );
		vec_st( st, 0, dest );
		st = result;
		
		dest += 4;
		src += 4;
		size -= 4;
	}
	st = vec_perm( st, st, storeMask );
	int temp = index;
	while( index < 0 )
	{
		vec_ste( st, index, dest );
		index += 2;
	}
	temp >>= 2;
	dest += temp;
	src  += temp;
	size -= temp;
	while( size-- )
		*dest++ = float( *src++ + 32768*256 ) * 1.1921110856794079500e-7f - 1.0f;
}
#elif defined(__SSE2__)
#include <xmmintrin.h>
// This is portable to other sysems since it uses Intel's intrinsics.

bool Vector::CheckForVector()
{
	// MMX, SSE, and SSE2 must be present, we don't use SSE3 so no need to check for it.
	return true;
}

template<typename T>
static inline void Write( T load, int32_t *&dest, const int16_t *&src,
			  unsigned &size, __m128i vol ) __attribute__((always_inline));
template<typename T>
inline void Write( T load, int32_t *&dest, const int16_t *&src, unsigned &size, __m128i vol )
{
	// There are only 8 XMM registers so no 4x unrolling
	while( size >= 8 )
        {
                __m128i data    = load( (__m128i *)src );
                __m128i hi      = _mm_mulhi_epi16( data, vol );
                __m128i low     = _mm_mullo_epi16( data, vol );
                __m128i result1 = _mm_unpacklo_epi16( low, hi );
                __m128i result2 = _mm_unpackhi_epi16( low, hi );
		
                result1 = _mm_add_epi32( result1, *(__m128i *)(dest + 0) );
                result2 = _mm_add_epi32( result2, *(__m128i *)(dest + 4) );
                _mm_store_si128( (__m128i *)(dest + 0), result1 );
                _mm_store_si128( (__m128i *)(dest + 4), result2 );
                src += 8;
                dest += 8;
                size -= 8;
        }
}	

void Vector::FastSoundWrite( int32_t *dest, const int16_t *src, unsigned size, short volume )
{
	while( (intptr_t(dest) & 0xF) && size )
        {
                // Misaligned stores are slow.
                *(dest++) += *(src++) * volume;
                --size;
        }
	
        __m128i vol = _mm_set1_epi16( volume );
	// Misaligned loads are slower so specialize to aligned loads when possible.
	if( intptr_t(src) & 0xF )
		Write( _mm_loadu_si128, dest, src, size, vol );
	else
		Write( _mm_load_si128, dest, src, size, vol );
        while( size-- )
                *(dest++) += *(src++) * volume;
}

template<typename T>
static inline void Read( T load, int16_t *&dest, const int32_t *&src, unsigned &size ) __attribute__((always_inline));
template<typename T>
static inline void Read( T load, int16_t *&dest, const int32_t *&src, unsigned &size )
{
	__m128i zero = _mm_setzero_si128();
	while( size >= 8 )
	{
		__m128i data1 = load( (__m128i *)(src + 0) );
		__m128i data2 = load( (__m128i *)(src + 4) );
		__m128i mask1 = _mm_cmplt_epi32( data1, zero );
		__m128i mask2 = _mm_cmplt_epi32( data2, zero );
		__m128i t1    = _mm_srai_epi32(  data1, 31 );
		__m128i t2    = _mm_srai_epi32(  data2, 31 );
		
		// We can't do 32 bit saturating arithmetic but that's unlikely to be a problem
		data1 = _mm_sub_epi32(  _mm_xor_si128(data1, t1), t1 );
		data2 = _mm_sub_epi32(  _mm_xor_si128(data2, t2), t2 );
		data1 = _mm_srai_epi32( data1, 8 );
		data2 = _mm_srai_epi32( data2, 8 );
		mask1 = _mm_and_si128(  mask1, data1 ); // destructive logic, we want data1 still
		mask2 = _mm_and_si128(  mask2, data2 ); // destructive logic
		data1 = _mm_sub_epi32( _mm_sub_epi32(data1, mask1), mask1 );
		data2 = _mm_sub_epi32( _mm_sub_epi32(data2, mask2), mask2 );
		data1 = _mm_packs_epi32( data1, data2 );
		_mm_store_si128( (__m128i *)dest, data1 );
		src += 8;
		dest += 8;
		size -= 8;
	}
	if( size )
	{
		__m128i data1 = load( (__m128i *)(src + 0) );
		__m128i data2 = size > 4 ? load( (__m128i *)(src + 4) ) : zero;
		__m128i mask1 = _mm_cmplt_epi32( data1, zero );
		__m128i mask2 = _mm_cmplt_epi32( data2, zero );
		__m128i t1    = _mm_srai_epi32(  data1, 31 );
		__m128i t2    = _mm_srai_epi32(  data2, 31 );
		
		// We can't do 32 bit saturating arithmetic but that's unlikely to be a problem
		data1 = _mm_sub_epi32(  _mm_xor_si128(data1, t1), t1 );
		data2 = _mm_sub_epi32(  _mm_xor_si128(data2, t2), t2 );
		data1 = _mm_srai_epi32( data1, 8 );
		data2 = _mm_srai_epi32( data2, 8 );
		mask1 = _mm_and_si128(  mask1, data1 ); // destructive logic, we want data1 still
		mask2 = _mm_and_si128(  mask2, data2 ); // destructive logic
		data1 = _mm_sub_epi32( _mm_sub_epi32(data1, mask1), mask1 );
		data2 = _mm_sub_epi32( _mm_sub_epi32(data2, mask2), mask2 );
		data1 = _mm_packs_epi32( data1, data2 );
#define X(x) (-(size >= (x)))
		data2 = _mm_set_epi8( 0, 0, X(7), X(7), X(6), X(6), X(5), X(5),
				      X(4), X(4), X(3), X(3), X(2), X(2), -1, -1 );
#undef X
		_mm_maskmoveu_si128( data1, data2, (char *)dest );
	}
}
	
void Vector::FastSoundRead( int16_t *dest, const int32_t *src, unsigned size )
{
	while( (intptr_t(dest) & 0xF) && size )
	{
		// Misaligned stores are very slow.
		*dest++ = max( -32768, min(*src++>>8, 32767) );
		--size;
	}
	// Specialize loads.
	if( intptr_t(src) & 0xF )
		Read( _mm_loadu_si128, dest, src, size );
	else
		Read( _mm_load_si128, dest, src, size );
	while( size-- )
		*dest++ = max( -32768, min(*src++>>8, 32767) );
}

template<typename T>
static inline void Read( T load, float *&dest, const int32_t *&src, unsigned &size ) __attribute__((always_inline));
template<typename T>
static inline void Read( T load, float *&dest, const int32_t *&src, unsigned &size )
{
	/* m = -32768; M = 32767
	* (x-2^8*m)(1-(-1))/(2^8*M-2^8*m)+(-1)
	* (x-2^8*m)/(2^7*(M-m))+(-1)
	* l1 = 2^8*m = -8388608
	* scale = 1/(2^7*(M-m)) = 0.00000011921110856794
	* l2 = -1 */
	__m128 scale = _mm_set1_ps( 0.00000011921110856794f );
	__m128i l1   = _mm_set1_epi32( -8388608 );
	__m128 l2    = _mm_set1_ps( -1.0f );
	
	while( size >= 4 )
	{
		__m128i data = _mm_sub_epi32( load((__m128i *)src), l1 );
		__m128 result = _mm_cvtepi32_ps( data );
		
		result = _mm_add_ps( _mm_mul_ps(result, scale), l2 );
		_mm_store_ps( dest, result );
		src += 4;
		dest += 4;
		size -= 4;
	}
	if( size )
	{
#define X(x) (-(size >= (x)))
		__m128i storeMask = _mm_set_epi8( 0, 0, 0, 0, X(3), X(3), X(3), X(3),
						  X(2), X(2), X(2), X(2), -1, -1, -1, -1 );
#undef X
		__m128i data = _mm_sub_epi32( load((__m128i *)src), l1 );
		__m128 result = _mm_cvtepi32_ps( data );
		
		result = _mm_add_ps( _mm_mul_ps(result, scale), l2 );
		// This might not be valid.
		_mm_maskmoveu_si128( (__m128i)result, storeMask, (char *)dest );
	}
}	

void Vector::FastSoundRead( float *dest, const int32_t *src, unsigned size )
{
	while( (intptr_t(dest) & 0xF) && size )
	{
		// Misaligned stores are very slow.
		*dest++ = float( *src++ + 32768*256 ) * 1.1921110856794079500e-7f - 1.0f;
		--size;
	}
	// Specialize loads.
	if( intptr_t(src) & 0xF )
		Read( _mm_loadu_si128, dest, src, size );
	else
		Read( _mm_load_si128, dest, src, size );
	while( size-- )
		*dest++ = float( *src++ + 32768*256 ) * 1.1921110856794079500e-7f - 1.0f;;	
}
#else
#error huh?
#endif
#endif


/*
 * (c) 2006 Steve Checkoway
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
