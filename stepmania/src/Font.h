/* Font - stores a font, used by BitmapText. */

#ifndef FONT_H
#define FONT_H

#include "RageTextureID.h"
#include "RageUtil.h"
#include "RageTypes.h"

class FontPage;
class RageTexture;
class IniFile;

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
		DefaultWidth,
		AdvanceExtraPixels;
	float ScaleAllWidthsBy;
	CString TextureHints;

	map<longchar,int> CharToGlyphNo;
	/* If a value is missing, the width of the texture frame is used. */
	map<int,int> GlyphWidths;

	FontPageSettings():
		DrawExtraPixelsLeft(0), DrawExtraPixelsRight(0),
		AddToAllWidths(0), 
		LineSpacing(-1),
		Top(-1),
		Baseline(-1),
		DefaultWidth(-1),
		AdvanceExtraPixels(1),
		ScaleAllWidthsBy(1),
		TextureHints("default")
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
	void SetTextureCoords(const vector<int> &widths, int AdvanceExtraPixels);
};

class Font
{
public:
	int m_iRefCount;
	CString path;

	Font();
	~Font();

	const glyph &GetGlyph( wchar_t c ) const;

	int GetLineWidthInSourcePixels( const wstring &szLine ) const;
	int GetLineHeightInSourcePixels( const wstring &szLine ) const;

	bool FontCompleteForString( const wstring &str ) const;

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

	static const wchar_t DEFAULT_GLYPH;

	static CString GetFontName(CString FileName);
	/* Remove filenames in 'v' that aren't in the same font as "FileName". */
	static void WeedFontNames(vector<CString> &v, const CString &FileName);

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

/*
 * (c) 2001-2004 Glenn Maynard, Chris Danford
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
