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

void Vector::FastSoundWrite( int32_t *dest, const int16_t *src, unsigned size, short volume )
{
	if( unlikely(size == 0) )
		return;
	ASSERT_M( (unsigned(dest) &0xF) == 0, ssprintf("dest = %p", dest) );
	
	vUInt8 splat_mask = vec_lvsl( 0, &volume );
	vSInt16 vol = vec_lde( 0, &volume );
	
	vol = vec_splat( vec_perm(vol, vol, splat_mask), 0 );
	
	vSInt16 MSQ = vec_ld( 0, src );
	vSInt16 LSQ;
	vUInt8 mask = vec_add( vec_lvsl(15, src), vec_splat_u8(1) );
	
	while( size & ~0x7 )
	{
		// Deal with unaligned data.
		LSQ = vec_ld( 15, src );
		
		vSInt16 data = vec_perm( MSQ, LSQ, mask );
		vSInt32 result1 = vec_ld( 0, dest );
		vSInt32 result2 = vec_ld( 16, dest );
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
		// Deal with the remaining samples but be careful while storing.
		// Deal with unaligned data.
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
			vec_st( result1, 0, dest );
			dest += 4;
			size -= 4;
			while( size-- )
				vec_ste( result2, 0, dest++ );
		}
		else
		{
			while( size-- )
				vec_ste( result1, 0, dest++ );
		}
	}
}

void Vector::FastSoundRead( int16_t *dest, const int32_t *src, unsigned size )
{
	ASSERT_M( (unsigned(dest) & 0xF) == 0, ssprintf("dest = %p", dest) );
	ASSERT_M( (unsigned(src) & 0xF) == 0, ssprintf("src = %p", src) );
	
	vSInt32 zero = (vSInt32)( 0 );
	vUInt32 shift = (vUInt32)( 8 );
	
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
		// Deal with the remaining samples but be careful while storing.
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

void Vector::FastSoundRead( float *dest, const int32_t *src, unsigned size )
{
	ASSERT_M( (unsigned(dest) &0xF) == 0, ssprintf("dest = %p", dest) );
	ASSERT_M( (unsigned(src) & 0xF) == 0, ssprintf("src = %p", src) );
	
	vFloat scale = (vFloat) ( 32767.5f );
	vSInt32 l1 = (vSInt32) ( -8388608 );
	vFloat l2 = (vFloat) ( -1.0f );
	
	while( size & ~0x3 )
	{
		vFloat result = vec_ctf( vec_subs(vec_ldl(0, src), l1), 8 );
		
		vec_st( vec_madd(result, scale, l2), 0, dest );
		dest += 4;
		src += 4;
		size -= 4;
	}
	if( size )
	{
		// Deal with the remaining samples but be careful while storing.
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
