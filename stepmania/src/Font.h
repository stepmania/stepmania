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
#include "RageUtil.h"
#include "IniFile.h"

class FontPage;

struct glyph {
	FontPage *fp;
	RageTexture *Texture;

	/* Number of pixels to advance horizontally after drawing this character. */
	int hadvance;

	/* Size of the actual rendered character. */
	float width, height;

	/* Number of pixels to offset this character when rendering. */
	float hshift; // , vshift;

	/* Texture coordinate rect. */
	RectF rect;
};

struct FontPageSettings {
	int DrawExtraPixelsLeft,
		DrawExtraPixelsRight,
		AddToAllWidths,
		LineSpacing,
		Top,
		Baseline;
	float ScaleAllWidthsBy;
	bool Kanji;
	
	map<longchar,int> CharToGlyphNo;
	/* If a value is missing, the width of the texture frame is used. */
	map<int,int> GlyphWidths;

	FontPageSettings():
		DrawExtraPixelsLeft(0), DrawExtraPixelsRight(0),
		AddToAllWidths(0), 
		LineSpacing(-1),
		ScaleAllWidthsBy(1),
		Top(-1),
		Baseline(-1),
		Kanji(false)
	{ }
};

class FontPage
{
public:
	RageTexture* m_pTexture;

	CString m_sTexturePath;

	/* All glyphs in this list will point to m_pTexture. */
	vector<glyph> glyphs;

	map<longchar,int> m_iCharToGlyphNo;

	FontPage();
	~FontPage();

	void Load( const CString &sASCIITexturePath, const FontPageSettings &cfg );

	/* Page-global properties. */
	int height;
	bool kanji;
	int LineSpacing, vshift;
	int GetCenter() const { return height/2; }

private:
	void SetExtraPixels(int DrawExtraPixelsLeft, int DrawExtraPixelsRight);
	void SetTextureCoords(const vector<int> &widths);
};

class Font
{
public:
	int m_iRefCount;
	CString path;
	map<longchar,glyph*> m_iCharToGlyph;

	Font();
	~Font();

	RageTexture *GetGlyphTexture( longchar c );
	const glyph &GetGlyph( longchar c ) const;

	int GetLineWidthInSourcePixels( const lstring &szLine ) const;
	int GetLineHeightInSourcePixels( const lstring &szLine ) const;
	int GetLineSpacingInSourcePixels( const lstring &szLine ) const;

	/* Add a FontPage to this font. */
	void AddPage(FontPage *fp);

	/* Steal all of a font's pages. */
	void MergeFont(Font *f);

	/* Load font-wide settings. */
	void CapsOnly();

//	int GetBaseline() const { return def->baseline; }
	int GetHeight() const { return def->height; }
	bool IsKanjiFont() const { return def->kanji; }
	int GetCenter() const { return def->GetCenter(); }
	int GetVshift() const { return def->vshift; }
	int GetLineSpacing() const { return def->LineSpacing; }

	void SetDefaultGlyph(FontPage *fp);

	static const longchar DEFAULT_GLYPH;

private:
	/* List of pages and fonts that we're responsible for freeing. */
	vector<FontPage *> pages;
	vector<Font *> merged_fonts;

	/* This is the primary fontpage of this font; font-wide baseline, ascender,
	 * etc. is pulled from it. */
	FontPage *def;
};

#endif
