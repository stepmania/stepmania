#include "global.h"
#include "RageTextureID.h"

void RageTextureID::Init()
{
	/* Maximum size of the texture, per dimension. */
	iMaxSize = 2048;

	/* Number of mipmaps. (unimplemented) */
	iMipMaps = 4;

	/* Maximum number of bits for alpha.  In 16-bit modes, lowering
	 * this gives more bits for color values. (0, 1 or 4) */
	iAlphaBits = 4;

	/* If this is greater than -1, then the image will be loaded as a luma/alpha
	 * map, eg. I4A4.  At most 8 bits per pixel will be used  This only actually happens
	 * when paletted textures are supported.
	 *
	 * If the sum of alpha and grayscale bits is <= 4, and the system supports 4-bit
	 * palettes, then the image will be loaded with 4bpp.
	 *
	 * This may be set to 0, resulting in an alpha map with all pixels white. */
	iGrayscaleBits = -1;

	/* If true and color precision is being lost, dither. (slow) */
	bDither = false;
	/* If true, resize the image to fill the internal texture. (slow) */
	bStretch = false;

	/* Preferred color depth of the image.  (This is overridden for
	 * paletted images and transparencies.) */
   	iColorDepth = -1; /* default */

	/* If true, enable HOT PINK color keying. (deprecated but needed for
	 * banners) */
	bHotPinkColorKey = false;

	/* These hints will be used in addition to any in the filename. */
	AdditionalTextureHints = "";
}

bool RageTextureID::operator<(const RageTextureID &rhs) const
{
#define COMP(a) if(a<rhs.a) return true; if(a>rhs.a) return false;
	COMP(filename);
	COMP(iMaxSize);
	COMP(iMipMaps);
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
		EQUAL(iMipMaps) &&
		EQUAL(iAlphaBits) &&
		EQUAL(iGrayscaleBits) &&
		EQUAL(iColorDepth) &&
		EQUAL(bDither) &&
		EQUAL(bStretch) &&
		EQUAL(bHotPinkColorKey) &&
		EQUAL(AdditionalTextureHints);
}

