#ifndef FONT_H
#define FONT_H
/*
-----------------------------------------------------------------------------
 File: Font

 Desc: A holder for information that is used by BitmapText objects.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageTexture.h"

const int MAX_FONT_CHARS = 256;		// only low ASCII chars are supported


class Font
{
public:
	Font( const CString &sASCIITexturePath );
	Font( const CString &sTexturePath, const CString& sChars );
	~Font();
	void Init();

	int GetLineWidthInSourcePixels( const CString &szLine );

	int m_iRefCount;

	CString m_sTexturePath;

	RageTexture* m_pTexture;
	bool m_bCapitalsOnly;
	int m_iLineSpacing;

	int m_iCharToFrameNo[MAX_FONT_CHARS];
	int	m_iFrameNoToWidth[MAX_FONT_CHARS];	// in soure coordinate space

	const RectF &GetTextureCoordRect( int frameNo ) const { return m_TextureCoordRects[frameNo]; }

	/* Source pixels to print to the left and right of each character. 
	 * m_Left[] is usually 0. */
	vector<int> m_Left, m_Right;

private:
	vector<RectF>	m_TextureCoordRects;
	void SetExtraPixels(int DrawExtraPixelsLeft, int DrawExtraPixelsRight);
	void SetTextureCoords();

};

#endif
