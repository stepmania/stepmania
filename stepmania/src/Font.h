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
#include "IniFile.h"

struct glyph {
	RageTexture *Texture;

	/* Number of pixels to advance horizontally after drawing this character. */
	int hadvance, vadvance;

	/* Size of the actual rendered character. */
	float width, height;

	/* Number of pixels to offset this character when rendering. */
	float hshift, vshift;

	/* Texture coordinate rect. */
	RectF rect;
};

struct FontPageSettings {
	int DrawExtraPixelsLeft,
		DrawExtraPixelsRight,
		AddToAllWidths,
		LineSpacing;
	float ScaleAllWidthsBy;
	
	map<wchar_t,int> CharToGlyphNo;
	/* If a value is missing, the width of the texture frame is used. */
	map<int,int> GlyphWidths;

	FontPageSettings():
		DrawExtraPixelsLeft(0), DrawExtraPixelsRight(0),
		AddToAllWidths(0), 
		LineSpacing(-1),
		ScaleAllWidthsBy(1)
	{ }
};

// typedef basic_string<wchar_t> wstring;

class FontPage
{
public:
	RageTexture* m_pTexture;

	CString m_sTexturePath;

	/* All glyphs in this list will point to m_pTexture. */
	vector<glyph> glyphs;

	map<wchar_t,int> m_iCharToGlyphNo;

	FontPage();
	~FontPage();

	void Load( const CString &sASCIITexturePath, const FontPageSettings &cfg );

private:
	void SetExtraPixels(int DrawExtraPixelsLeft, int DrawExtraPixelsRight);
	void SetTextureCoords(const vector<int> &widths, int LineSpacing);
};

class Font
{
public:
	int m_iRefCount;
	CString path;
	map<wchar_t,glyph*> m_iCharToGlyph;

	Font();
	~Font();

	RageTexture *GetGlyphTexture( wchar_t c );
	const glyph &GetGlyph( wchar_t c ) const;

	int GetLineWidthInSourcePixels( const wstring &szLine ) const;
	int GetLineHeightInSourcePixels( const wstring &szLine ) const;

	/* Add a FontPage to this font. */
	void AddPage(FontPage *fp);

	/* Steal all of a font's pages. */
	void MergeFont(Font *f);

	/* Load font-wide settings. */
	void CapsOnly();

	static const wchar_t DEFAULT_GLYPH;

private:
	/* List of pages and fonts that we're responsible for freeing. */
	vector<FontPage *> pages;
	vector<Font *> merged_fonts;
};

#endif
