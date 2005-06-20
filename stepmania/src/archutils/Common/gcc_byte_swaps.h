#ifndef GCC_BYTE_SWAPS_H
#define GCC_BYTE_SWAPS_H

#if defined(CPU_X86)

inline uint32_t ArchSwap32( uint32_t n )
{
	asm(
		"xchg %b0, %h0\n"
		"rorl $16, %0\n"
		"xchg %b0, %h0":
		"=q" (n): "0" (n) );
	return n;
}

inline uint32_t ArchSwap24( uint32_t n )
{
	asm(
		"xchg %b0, %h0\n"
		"rorl $16, %0\n"
		"xchg %b0, %h0\n"
		"shrl $8, %0\n":
		"=q" (n): "0" (n) );
	return n;
}

inline uint16_t ArchSwap16( uint16_t n )
{
	asm(
		"xchg %b0, %h0\n":
		"=q" (n): "0" (n) );
	return n;
}

#define HAVE_BYTE_SWAPS
#endif

#endif
/*
 * (c) 2004 Glenn Maynard
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
