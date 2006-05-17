#include "global.h"
#include "VectorHelper.h"
#include "RageUtil.h"

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
	long cpuAttributes;
	OSErr err = Gestalt( gestaltPowerPCProcessorFeatures, &cpuAttributes );
	
	return err == noErr && ( cpuAttributes & (1 << gestaltPowerPCHasVectorInstructions) );
}

/* for( size_t pos = 0; pos < size; ++pos )
 *         dest[pos] += src[pos] * volume;
 */
void Vector::FastSoundWrite( int32_t *dest, const int16_t *src, unsigned size, short volume )
{
	if( size == 0 )
		return;
	ASSERT_M( (unsigned(dest) &0xF) == 0, ssprintf("dest = %p", dest) );
	/* We want to splat the volume across a vector but it is unlikely to be aligned in
	 * the same location every time so we are going to load it into some offset in
	 * the vector, permute it to the first word and splat it. */
	vUInt8 mask = vec_lvsl( 0, &volume );
	vSInt16 vol = vec_lde( 0, &volume );
	
	vol = vec_splat( vec_perm(vol, vol, mask), 0 );
	
	/* To handle src being unaligned we're going to load an initial vector and each
	 * iteration of the loop, we'll load 15 bytes past the first to pick up the next
	 * vector of data (note that if we can load one element of a vector, it's always
	 * safe to load the whole vector) unless src is actually aligned in which case
	 * we'll end up with the same vector. To handle that, be tricky with the
	 * permutation mask to load from LSQ otherwise every iteration but the first we
	 * will be loading the wrong data. */
	vSInt16 MSQ = vec_ld( 0, src );
	vSInt16 LSQ;
	
	mask = vec_add( vec_lvsl(15, src), vec_splat_u8(1) );

	while( size >= 32 )
	{
		vSInt16 data1, data2, data3, data4;
		
		LSQ = vec_ld( 15, src );
		data1 = vec_perm( MSQ, LSQ, mask );
		MSQ = vec_ld( 31, src );
		data2 = vec_perm( LSQ, MSQ, mask ); // Reverse MSQ and LSQ.
		LSQ = vec_ld( 47, src );
		data3 = vec_perm( MSQ, LSQ, mask );
		MSQ = vec_ld( 63, src );
		data4 = vec_perm( LSQ, MSQ, mask ); // Reversed.
		
		// Load early, let gcc schedule them where it wants.
		vSInt32 result1 = vec_ld(   0, dest );
		vSInt32 result2 = vec_ld(  16, dest );
		vSInt32 result3 = vec_ld(  32, dest );
		vSInt32 result4 = vec_ld(  48, dest );
		vSInt32 result5 = vec_ld(  64, dest );
		vSInt32 result6 = vec_ld(  80, dest );
		vSInt32 result7 = vec_ld(  96, dest );
		vSInt32 result8 = vec_ld( 112, dest );
		
		vSInt32 even1   = vec_mule( data1, vol );
		vSInt32 odd1    = vec_mulo( data1, vol );
		vSInt32 even2   = vec_mule( data2, vol );
		vSInt32 odd2    = vec_mulo( data2, vol );
		vSInt32 even3   = vec_mule( data3, vol );
		vSInt32 odd3    = vec_mulo( data3, vol );
		vSInt32 even4   = vec_mule( data4, vol );
		vSInt32 odd4    = vec_mulo( data4, vol );
		vSInt32 first   = vec_mergeh( even1, odd1 );
		vSInt32 second  = vec_mergel( even1, odd1 );
		vSInt32 third   = vec_mergeh( even2, odd2 );
		vSInt32 fourth  = vec_mergel( even2, odd2 );
		vSInt32 fifth   = vec_mergeh( even3, odd3 );
		vSInt32 sixth   = vec_mergel( even3, odd3 );
		vSInt32 seventh = vec_mergeh( even4, odd4 );
		vSInt32 eighth  = vec_mergel( even4, odd4 );
		
		result1 = vec_add( result1, first );
		result2 = vec_add( result2, second );
		result3 = vec_add( result3, third );
		result4 = vec_add( result4, fourth );
		result5 = vec_add( result5, fifth );
		result6 = vec_add( result6, sixth );
		result7 = vec_add( result7, seventh );
		result8 = vec_add( result8, eighth );
		
		vec_st( result1,   0, dest );
		vec_st( result2,  16, dest );
		vec_st( result3,  32, dest );
		vec_st( result4,  48, dest );
		vec_st( result5,  64, dest );
		vec_st( result6,  80, dest );
		vec_st( result7,  96, dest );
		vec_st( result8, 112, dest );
		dest += 32;
		src += 32;
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
		// Load next 16 bytes.
		LSQ = vec_ld( 15, src );
		
		vSInt16 data = vec_perm( MSQ, LSQ, mask );
		vSInt32 result1 = vec_ld( 0, dest );
		vSInt32 result2 = vec_ld( 16, dest );
		
		/* Multiply the even 2-byte elements in data with those in vol to get
		 * 4-byte elements. Do the same with the odd elements then merge both
		 * high and low halves of the vectors into two new 4-element, 4-byte
		 * vectors. In this way the combined vector <first,second> contains
		 * the 8 products in the correct order. */
		vSInt32 even = vec_mule( data, vol );
		vSInt32 odd = vec_mulo( data, vol );
		vSInt32 first = vec_mergeh( even, odd );
		vSInt32 second = vec_mergel( even, odd );
		
		vec_st( vec_add(result1, first), 0, dest );
		vec_st( vec_add(result2, second), 16, dest );
		dest += 8;
		src += 8;
		size -= 8;
		MSQ = LSQ;
	}
	if( size )
	{
		/* There is more data left but fewer than 14 bytes (7 2-byte elements)
		 * to be written. Handle it exactly the same as above but be careful
		 * to only store size 4-byte products. */
		LSQ = vec_ld( 15, src );
		
		vSInt16 data = vec_perm( MSQ, LSQ, mask );
		vSInt32 result1 = vec_ld( 0, dest );
		vSInt32 result2 = vec_ld( 16, dest );
		vSInt32 even = vec_mule( data, vol );
		vSInt32 odd = vec_mulo( data, vol );
		vSInt32 first = vec_mergeh( even, odd );
		vSInt32 second = vec_mergel( even, odd );
		
		result1 = vec_add( result1, first );
		result2 = vec_add( result2, second );
		if( size >= 4 )
		{
			/* We can store the first 4 in one store instruction. Store
			 * the size-4 others one at a time. */
			vec_st( result1, 0, dest );
			dest += 4;
			size -= 4;
			while( size-- )
				vec_ste( result2, 0, dest++ );
		}
		else
		{
			/* We have fewer than 4 so store them one at a time. */
			while( size-- )
				vec_ste( result1, 0, dest++ );
		}
	}
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
	while( size > 7 )
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
