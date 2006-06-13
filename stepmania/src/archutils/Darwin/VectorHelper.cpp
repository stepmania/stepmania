#include "global.h"
#include "VectorHelper.h"
#include "RageUtil.h"
#include <sys/sysctl.h>

#ifdef USE_VEC
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
	if( size == 0 )
		return;
	ASSERT_M( (intptr_t(dest) & 0x7) == 0, ssprintf("dest = %p", dest) );
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
		
		// If dest is unaligned, pull first loop iteration out.
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
			
			load1Dest = vec_add( load1Dest, first );
			load2Dest = vec_add( load2Dest, second );
			store     = vec_perm( load1Dest, load1Dest, storeMask );
			load1Dest = vec_perm( load1Dest, load2Dest, storeMask );
			
			while( (intptr_t(dest) + index) & 0xC )
			{
				vec_ste( store, index, dest );
				index += 4;
			}
			vec_st( load1Dest, index, dest );
			
			load1Src = load2Src;
			load1Dest = load3Dest;
			store = load2Dest;
			src += 8;
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
			vec_st( store,     index,       dest );
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
			
			vec_st( store,     index,      dest );
			vec_st( load1Dest, index + 16, dest );
			
			load1Src  = load2Src;
			load1Dest = load3Dest;
			store     = load2Dest;
			src  += 8;
			dest += 8;
			size -= 8;
		}
		
		// Store the remainder of the vector, if it was unaligned.
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
	/* If we account for both unaligned dest and src, there is really no way to
	 * do this in vector code so do the last at most 7 elements in scalar code. */
	while( size-- )
		*(dest++) += *(src++) * volume;
}

/* for( size_t pos = 0; pos < size; ++pos )
 *         dest[pos] = clamp( src[pos]/256, -32768, 32767 );
 */
void Vector::FastSoundRead( int16_t *dest, const int32_t *src, unsigned size )
{
	ASSERT_M( (unsigned(dest) & 0xF) == 0, ssprintf("dest = %p", dest) );
	ASSERT_M( (unsigned(src) & 0xF) == 0, ssprintf("src = %p", src) );
	
	vSInt32 zero = (vSInt32)( 0 );
	vUInt32 shift = (vUInt32)( 8 );
	
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
		
		vBool32 b1 = vec_cmplt( first,   zero );
		vBool32 b2 = vec_cmplt( second,  zero );
		vBool32 b3 = vec_cmplt( third,   zero );
		vBool32 b4 = vec_cmplt( fourth,  zero );
		vBool32 b5 = vec_cmplt( fifth,   zero );
		vBool32 b6 = vec_cmplt( sixth,   zero );
		vBool32 b7 = vec_cmplt( seventh, zero );
		vBool32 b8 = vec_cmplt( eighth,  zero );
		
		first   = vec_sr( vec_abss(first),   shift );
		second  = vec_sr( vec_abss(second),  shift );
		third   = vec_sr( vec_abss(third),   shift );
		fourth  = vec_sr( vec_abss(fourth),  shift );
		fifth   = vec_sr( vec_abss(fifth),   shift );
		sixth   = vec_sr( vec_abss(sixth),   shift );
		seventh = vec_sr( vec_abss(seventh), shift );
		eighth  = vec_sr( vec_abss(eighth),  shift );
		
		vSInt32 temp1 = vec_and( first,   (vSInt32)b1 );
		vSInt32 temp2 = vec_and( second,  (vSInt32)b2 );
		vSInt32 temp3 = vec_and( third,   (vSInt32)b3 );
		vSInt32 temp4 = vec_and( fourth,  (vSInt32)b4 );
		vSInt32 temp5 = vec_and( fifth,   (vSInt32)b5 );
		vSInt32 temp6 = vec_and( sixth,   (vSInt32)b6 );
		vSInt32 temp7 = vec_and( seventh, (vSInt32)b7 );
		vSInt32 temp8 = vec_and( eighth,  (vSInt32)b8 );
		
		first   = vec_subs( vec_sub(first,   temp1), temp1 );
		second  = vec_subs( vec_sub(second,  temp2), temp2 );
		third   = vec_subs( vec_sub(third,   temp3), temp3 );
		fourth  = vec_subs( vec_sub(fourth,  temp4), temp4 );
		fifth   = vec_subs( vec_sub(fifth,   temp5), temp5 );
		sixth   = vec_subs( vec_sub(sixth,   temp6), temp6 );
		seventh = vec_subs( vec_sub(seventh, temp7), temp7 );
		eighth  = vec_subs( vec_sub(eighth,  temp8), temp8 );
		
		vec_st( vec_packs(first,   second),  0, dest );
		vec_st( vec_packs(third,   fourth), 16, dest );
		vec_st( vec_packs(fifth,   sixth),  32, dest );
		vec_st( vec_packs(seventh, eighth), 48, dest );
		
		dest += 32;
		src += 32;
		size -= 32;
	}
	// Befuddle optimizer as above.	
	while( size & ~0x7 )
	{
		vSInt32 first = vec_ldl( 0, src );
		vSInt32 second = vec_ldl( 16, src );
		vBool32 b1 = vec_cmplt( first, zero );
		vBool32 b2 = vec_cmplt( second, zero );
		
		first = vec_abss( first );
		second = vec_abss( second );
		first = vec_sr( first, shift );
		second = vec_sr( second, shift );
		
		vSInt32 temp1 = vec_and( first, (vSInt32)b1 );
		vSInt32 temp2 = vec_and( second, (vSInt32) b2 );
		
		first = vec_subs( first, temp1 );
		second = vec_subs( second, temp2 );
		first = vec_subs( first, temp1 );
		second = vec_sub( second, temp2 );
		
		vec_st( vec_packs(first, second), 0, dest );
		dest += 8;
		src += 8;
		size -= 8;
	}
	if( size )
	{
		// Deal with the remaining samples but be careful while storing as above.
		vSInt32 first = vec_ldl( 0, src );
		vSInt32 second = vec_ldl( 16, src );
		vBool32 b1 = vec_cmplt( first, zero );
		vBool32 b2 = vec_cmplt( second, zero );
		
		first = vec_abss( first );
		second = vec_abss( second );
		first = vec_sr( first, shift );
		second = vec_sr( second, shift );
		
		vSInt32 temp1 = vec_and( first, (vSInt32)b1 );
		vSInt32 temp2 = vec_and( second, (vSInt32) b2 );
		
		first = vec_subs( first, temp1 );
		second = vec_subs( second, temp2 );
		first = vec_subs( first, temp1 );
		second = vec_sub( second, temp2 );
		
		vSInt16 result = vec_packs( first, second );
		while( size-- )
			vec_ste( result, 0, dest++ );
	}
}

/* for( size_t pos = 0; pos < size; ++pos )
 *         dest[pos] = SCALE( float(src[pos]), -32768*256, 32767*256, -1.0f, 1.0f );
 */
void Vector::FastSoundRead( float *dest, const int32_t *src, unsigned size )
{
	ASSERT_M( (unsigned(dest) &0xF) == 0, ssprintf("dest = %p", dest) );
	ASSERT_M( (unsigned(src) & 0xF) == 0, ssprintf("src = %p", src) );
	
	vFloat scale = (vFloat) ( 32767.5f );
	vSInt32 l1 = (vSInt32) ( -8388608 );
	vFloat l2 = (vFloat) ( -1.0f );
	
	while( size > 3 )
	{
		/* By far the simplest of these, we need only perform the scale
		 * operation which amounts to subtracting l1, converting to a float,
		 * multiplying by a constant, and adding l1. We can multiply and add
		 * in one instruction. */
		vFloat result = vec_ctf( vec_subs(vec_ldl(0, src), l1), 8 );
		
		vec_st( vec_madd(result, scale, l2), 0, dest );
		dest += 4;
		src += 4;
		size -= 4;
	}
	if( size )
	{
		// Deal with the remaining samples but be careful while storing as above.
		vFloat result = vec_madd( vec_ctf(vec_subs(vec_ldl(0, src), l1), 8), scale, l2 );
		
		while( size-- )
			vec_ste( result, 0, dest++ );
	}
}
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
