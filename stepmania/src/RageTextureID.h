#ifndef RAGE_TEXTURE_ID_H
#define RAGE_TEXTURE_ID_H

/* A unique texture is identified by a RageTextureID.  (Loading the
 * same file with two different dither settings is considered two
 * different textures, for example.)  See RageTexture.cpp for explanations
 * of these. */
struct RageTextureID
{
	CString filename;
	int iMaxSize;
	int iMipMaps;
	int iAlphaBits;
	int iGrayscaleBits;
	int iColorDepth;
	bool bDither;
	bool bStretch;
	bool bHotPinkColorKey; /* #FF00FF */
	CString AdditionalTextureHints;

	bool operator< (const RageTextureID &rhs) const;
	bool operator== (const RageTextureID &rhs) const;

	void Init();

	RageTextureID() { Init(); }
	RageTextureID(const CString &fn) { Init(); filename=fn; }
};

#endif
