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

/* for( unsigned pos = 0; pos < size; ++pos )
 *         dest[pos] += src[pos];
 * Idea from: http://developer.apple.com/hardwaredrivers/ve/downloads/add.c */
void Vector::FastSoundWrite( float *dest, const float *src, unsigned size )
{
	if( size > 4 )
	{
		int index = 0;
		vUInt8 one = (vUInt8)(1);
		vUInt8 srcMask = vec_add( vec_lvsl(15, src), one );
		vUInt8 destMask = vec_add(vec_lvsl(15, dest), one );
		vUInt8 storeMask = vec_lvsr( 0, dest );
		vFloat load1Src = vec_ld( 0, src );
		vFloat load1Dest = vec_ld( 0, dest );
		vFloat store = (vFloat)(0.0f);
		
		// If dest is misaligned, pull the first loop iteration out.
		if( intptr_t(dest) & 0xF )
		{
			vFloat load2Src = vec_ld( 15, src );
			vFloat load2Dest = vec_ld( 15, dest );
			
			load1Src  = vec_perm( load1Src,  load2Src,  srcMask  );
			load1Dest = vec_perm( load1Dest, load2Dest, destMask );
			
			load1Dest = vec_add(  load1Dest, load1Src );
			store     = vec_perm( load1Dest, load1Dest, storeMask );
			
			while( (intptr_t(dest) + index) & 0xC )
			{
				vec_ste( store, index, dest );
				index += 4;
			}
			load1Src  = load2Src;
			store     = load1Dest;
			load1Dest = load2Dest;
			src  += 4;
			dest += 4;
			size -= 4;
			/* Incrementing the index is supposed to have the same effect
			 * as incrementing dest but since we read from dest as well
			 * we don't want to increment twice so decrement the index. */
			// XXX: What in the world did I mean here?
			index -= 16;			
		}
		while( size >= 32 )
		{
			vFloat load2Src  = vec_ld(  15, src  );
			vFloat load3Src  = vec_ld(  31, src  );
			vFloat load4Src  = vec_ld(  47, src  );
			vFloat load5Src  = vec_ld(  63, src  );
			vFloat load6Src  = vec_ld(  79, src  );
			vFloat load7Src  = vec_ld(  95, src  );
			vFloat load8Src  = vec_ld( 111, src  );
			vFloat load9Src  = vec_ld( 127, src  );
			vFloat load2Dest = vec_ld(  15, dest );
			vFloat load3Dest = vec_ld(  31, dest );
			vFloat load4Dest = vec_ld(  47, dest );
			vFloat load5Dest = vec_ld(  63, dest );
			vFloat load6Dest = vec_ld(  79, dest );
			vFloat load7Dest = vec_ld(  95, dest );
			vFloat load8Dest = vec_ld( 111, dest );
			vFloat load9Dest = vec_ld( 127, dest );

			// Align the data.
			load1Src  = vec_perm( load1Src,  load2Src,  srcMask );
			load2Src  = vec_perm( load2Src,  load3Src,  srcMask );
			load3Src  = vec_perm( load3Src,  load4Src,  srcMask );
			load4Src  = vec_perm( load4Src,  load5Src,  srcMask );
			load5Src  = vec_perm( load5Src,  load6Src,  srcMask );
			load6Src  = vec_perm( load6Src,  load7Src,  srcMask );
			load7Src  = vec_perm( load7Src,  load8Src,  srcMask );
			load8Src  = vec_perm( load8Src,  load9Src,  srcMask );
			// Not load5Src, it's untouched and used later.
			load1Dest = vec_perm( load1Dest, load2Dest, destMask );
			load2Dest = vec_perm( load2Dest, load3Dest, destMask );
			load3Dest = vec_perm( load3Dest, load4Dest, destMask );
			load4Dest = vec_perm( load4Dest, load5Dest, destMask );
			load5Dest = vec_perm( load5Dest, load6Dest, destMask );
			load6Dest = vec_perm( load6Dest, load7Dest, destMask );
			load7Dest = vec_perm( load7Dest, load8Dest, destMask );
			load8Dest = vec_perm( load8Dest, load9Dest, destMask );
			// Not load9Dest.

			load1Dest = vec_add( load1Dest, load1Src );
			load2Dest = vec_add( load2Dest, load2Src );
			load3Dest = vec_add( load3Dest, load3Src );
			load4Dest = vec_add( load4Dest, load4Src );
			load5Dest = vec_add( load5Dest, load5Src );
			load6Dest = vec_add( load6Dest, load6Src );
			load7Dest = vec_add( load7Dest, load7Src );
			load8Dest = vec_add( load8Dest, load8Src );

			// Unalign the results.
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

			load1Src  = load9Src;
			load1Dest = load9Dest;
			store     = load8Dest;
			dest += 32;
			src  += 32;
			size -= 32;
		}

		/* This completely baffles gcc's loop unrolling. If I make it > 3 instead,
		 * then gcc produces 4 identical copies of the loop without scheduling them
		 * in a sane manner (hence the manual unrolling above) but this loop will
		 * never be executed more than 3 times so that code will never be used.
		 * This produces code the way gcc _should_ do it by unrolling and scheduling
		 * and then producing the rolled version. */
		while( size & ~0x3 )
		{
			vFloat load2Src  = vec_ld( 15, src );
			vFloat load2Dest = vec_ld( 15, dest );
			
			load1Src  = vec_perm( load1Src,  load2Src,  srcMask );
			load1Dest = vec_perm( load1Dest, load2Dest, destMask );
			load1Dest = vec_add(  load1Dest, load1Src );

			store     = vec_perm( store,     load1Dest, storeMask );
			vec_st( store, index, dest );

			load1Src  = load2Src;
			store     = load1Dest;
			load1Dest = load2Dest;
			src  += 4;
			dest += 4;
			size -= 4;
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
	 * do this in vector code so do the last at most 3 elements in scalar code. */
	while( size-- )
		*(dest++) += *(src++);
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
static inline void __attribute__((always_inline)) Write( T load, float *&dest, const float *&src, unsigned &size )
{
	// There are only 8 XMM registers so no 8x unrolling. Let's do 2 though.
	while( size >= 8 )
	{
		__m128 data1 = load( src + 0 );
		__m128 data2 = load( src + 4 );

		data1 = _mm_add_ps( data1, *(__m128 *)(dest + 0) );
		data2 = _mm_add_ps( data2, *(__m128 *)(dest + 4) );
		_mm_store_ps( dest + 0, data1 );
		_mm_store_ps( dest + 4, data2 );
		src  += 8;
		dest += 8;
		size -= 8;
	}
}

void Vector::FastSoundWrite( float *dest, const float *src, unsigned size )
{
	while( (intptr_t(dest) & 0xF) && size )
        {
                // Misaligned stores are slow.
                *(dest++) += *(src++);
                --size;
        }
	
	// Misaligned loads are slower so specialize to aligned loads when possible.
	if( intptr_t(src) & 0xF )
		Write( _mm_loadu_ps, dest, src, size );
	else
		Write( _mm_load_ps, dest, src, size );
        while( size-- )
                *(dest++) += *(src++);
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
