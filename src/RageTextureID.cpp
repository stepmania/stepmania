#include "global.h"
#include "RageTextureID.h"
#include "RageTextureManager.h"
#include "RageUtil.h"

void RageTextureID::Init()
{
	iMaxSize = 2048;
	bMipMaps = false;	// Most sprites (especially text) look worse with mip maps
	iAlphaBits = 4;
	iGrayscaleBits = -1;
	bDither = false;
	bStretch = false;
   	iColorDepth = -1; /* default */
	bHotPinkColorKey = false;
	AdditionalTextureHints = "";
	Policy = TEXTUREMAN->GetDefaultTexturePolicy();
}

bool RageTextureID::operator<(const RageTextureID &rhs) const
{
#define COMP(a) if(a<rhs.a) return true; if(a>rhs.a) return false;
	COMP(filename);
	COMP(iMaxSize);
	COMP(bMipMaps);
	COMP(iAlphaBits);
	COMP(iGrayscaleBits);
	COMP(iColorDepth);
	COMP(bDither);
	COMP(bStretch);
	COMP(bHotPinkColorKey);
	COMP(AdditionalTextureHints);
	// COMP(Policy); // don't do this
#undef COMP
	return false;
}

bool RageTextureID::operator==(const RageTextureID &rhs) const
{
#define EQUAL(a) (a==rhs.a)
	return 
		EQUAL(filename) &&
		EQUAL(iMaxSize) &&
		EQUAL(bMipMaps) &&
		EQUAL(iAlphaBits) &&
		EQUAL(iGrayscaleBits) &&
		EQUAL(iColorDepth) &&
		EQUAL(bDither) &&
		EQUAL(bStretch) &&
		EQUAL(bHotPinkColorKey) &&
		EQUAL(AdditionalTextureHints);
		// EQUAL(Policy); // don't do this
}

void RageTextureID::SetFilename( const RString &fn )
{
	filename = fn;
	CollapsePath( filename );
}

/*
 * Copyright (c) 2003-2004 Chris Danford, Glenn Maynard
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
