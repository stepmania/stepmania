#ifndef RAGE_TEXTURE_ID_H
#define RAGE_TEXTURE_ID_H

/* A unique texture is identified by a RageTextureID.  (Loading the
 * same file with two different dither settings is considered two
 * different textures, for example.)  See RageTexture.cpp for explanations
 * of these. */
struct RageTextureID
{
	CString filename;
	
	/* Maximum size of the texture, per dimension. */
	int iMaxSize;

	/* Generate mipmaps for this texture */
	bool bMipMaps;

	/* Maximum number of bits for alpha.  In 16-bit modes, lowering
	 * this gives more bits for color values. (0, 1 or 4) */
	int iAlphaBits;

	/* If this is greater than -1, then the image will be loaded as a luma/alpha
	 * map, eg. I4A4.  At most 8 bits per pixel will be used  This only actually happens
	 * when paletted textures are supported.
	 *
	 * If the sum of alpha and grayscale bits is <= 4, and the system supports 4-bit
	 * palettes, then the image will be loaded with 4bpp.
	 *
	 * This may be set to 0, resulting in an alpha map with all pixels white. */
	int iGrayscaleBits;
	
	/* Preferred color depth of the image.  (This is overridden for
	 * paletted images and transparencies.)  -1 for default. */
	int iColorDepth;
	
	/* If true and color precision is being lost, dither. (slow) */
	bool bDither;
	
	/* If true, resize the image to fill the internal texture. (slow) */
	bool bStretch;
	
	/* If true, enable HOT PINK color keying. (deprecated but needed for
	 * banners) */
	bool bHotPinkColorKey; /* #FF00FF */
	
	/* These hints will be used in addition to any in the filename. */
	CString AdditionalTextureHints;

	bool operator< (const RageTextureID &rhs) const;
	bool operator== (const RageTextureID &rhs) const;

	void Init();

	RageTextureID() { Init(); }
	RageTextureID(const CString &fn) { Init(); filename=fn; }
};

#endif
