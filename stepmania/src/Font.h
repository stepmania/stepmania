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
	RageTexture *GetTexture() const { return const_cast<RageTexture *>(Texture); }

	/* Number of pixels to advance horizontally after drawing this character. */
	int hadvance;

	/* Size of the actual rendered character. */
	float width, height;

	/* Number of pixels to offset this character when rendering. */
	float hshift; // , vshift;

	/* Texture coordinate rect. */
	RectF rect;
};

struct FontPageSettings
{
	CString TexturePath;

	int DrawExtraPixelsLeft,
		DrawExtraPixelsRight,
		AddToAllWidths,
		LineSpacing,
		Top,
		Baseline,
		DefaultWidth;
	float ScaleAllWidthsBy;
	
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
		DefaultWidth(-1)
	{ }

	/* Map a range from a character map to glyphs.  If cnt is -1, map the
	 * whole map. Returns "" or an error message. */
	CString MapRange(CString Mapping, int map_offset, int glyph_offset, int cnt);
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

	void Load( FontPageSettings cfg );

	/* Page-global properties. */
	int height;
	int LineSpacing;
	float vshift;
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
	void MergeFont(Font &f);

	void Load(const CString &sFontOrTextureFilePath, CString sChars);
	void Unload();
	void Reload();

	/* Load font-wide settings. */
	void CapsOnly();

	int GetHeight() const { return def->height; }
	int GetCenter() const { return def->GetCenter(); }
	int GetLineSpacing() const { return def->LineSpacing; }

	void SetDefaultGlyph(FontPage *fp);

	static const longchar DEFAULT_GLYPH;

	static CString GetFontName(CString FileName);

private:
	/* List of pages and fonts that we use (and are responsible for freeing). */
	vector<FontPage *> pages;

	/* This is the primary fontpage of this font; font-wide height, center,
	 * etc. is pulled from it.  (This is one of pages[].) */
	FontPage *def;

	/* Map from characters to glyphs.  (Each glyph* is part of one of pages[].) */
	map<longchar,glyph*> m_iCharToGlyph;

	/* We keep this around only for reloading. */
	CString Chars;

	void LoadFontPageSettings(FontPageSettings &cfg, IniFile &ini, const CString &TexturePath, const CString &PageName, CString sChars);
	static void GetFontPaths(const CString &sFontOrTextureFilePath, 
							   CStringArray &TexturePaths, CString &IniPath);
	CString GetPageNameFromFileName(const CString &fn);
};

#endif
