#include "global.h"
#include "RageTextureID.h"

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
}

