/* Font - stores a font, used by BitmapText. */

#ifndef FONT_H
#define FONT_H

#include "RageTextureID.h"
#include "RageUtil.h"
#include "RageTypes.h"
#include <map>

class FontPage;
class RageTexture;
class IniFile;

struct glyph
{
	FontPage *m_pPage;
	RageTexture *m_pTexture;
	RageTexture *GetTexture() const { return const_cast<RageTexture *>(m_pTexture); }

	/* Number of pixels to advance horizontally after drawing this character. */
	int m_iHadvance;

	/* Size of the actual rendered character. */
	float m_fWidth, m_fHeight;

	/* Number of pixels to offset this character when rendering. */
	float m_fHshift; // , vshift;

	/* Texture coordinate rect. */
	RectF m_TexRect;
};

struct FontPageSettings
{
	CString m_sTexturePath;

	int m_iDrawExtraPixelsLeft,
		m_iDrawExtraPixelsRight,
		m_iAddToAllWidths,
		m_iLineSpacing,
		m_iTop,
		m_iBaseline,
		m_iDefaultWidth,
		m_iAdvanceExtraPixels;
	float m_fScaleAllWidthsBy;
	CString m_sTextureHints;

	map<longchar,int> CharToGlyphNo;
	/* If a value is missing, the width of the texture frame is used. */
	map<int,int> m_mapGlyphWidths;

	FontPageSettings():
		m_iDrawExtraPixelsLeft(0), m_iDrawExtraPixelsRight(0),
		m_iAddToAllWidths(0), 
		m_iLineSpacing(-1),
		m_iTop(-1),
		m_iBaseline(-1),
		m_iDefaultWidth(-1),
		m_iAdvanceExtraPixels(1),
		m_fScaleAllWidthsBy(1),
		m_sTextureHints("default")
	{ }

	/* Map a range from a character map to glyphs.  If cnt is -1, map the
	 * whole map. Returns "" or an error message. */
	CString MapRange( CString sMapping, int iMapOffset, int iGlyphOffset, int iCount );
};

class FontPage
{
public:
	FontPage();
	~FontPage();

	void Load( const FontPageSettings &cfg );

	/* Page-global properties. */
	int m_iHeight;
	int m_iLineSpacing;
	float m_fVshift;
	int GetCenter() const { return m_iHeight/2; }

	/* Remember these only for GetLineWidthInSourcePixels. */
	int m_iDrawExtraPixelsLeft, m_iDrawExtraPixelsRight;

	RageTexture* m_pTexture;

	// XXX remove?
	CString m_sTexturePath;

	/* All glyphs in this list will point to m_pTexture. */
	vector<glyph> m_aGlyphs;

	map<longchar,int> m_iCharToGlyphNo;

private:
	void SetExtraPixels( int iDrawExtraPixelsLeft, int DrawExtraPixelsRight );
	void SetTextureCoords( const vector<int> &aiWidths, int iAdvanceExtraPixels );
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

	int GetHeight() const { return m_pDefault->m_iHeight; }
	int GetCenter() const { return m_pDefault->GetCenter(); }
	int GetLineSpacing() const { return m_pDefault->m_iLineSpacing; }

	void SetDefaultGlyph( FontPage *pPage );

	static const wchar_t DEFAULT_GLYPH;

	bool IsRightToLeft() const { return m_bRightToLeft; };

private:
	/* List of pages and fonts that we use (and are responsible for freeing). */
	vector<FontPage *> m_apPages;

	/* This is the primary fontpage of this font; font-wide height, center,
	 * etc. is pulled from it.  (This is one of pages[].) */
	FontPage *m_pDefault;

	/* Map from characters to glyphs.  (Each glyph* is part of one of pages[].) */
	map<longchar,glyph*> m_iCharToGlyph;
	glyph *m_iCharToGlyphCache[128];

	// True for Hebrew, Arabic, Urdu fonts.
	// This will also change the way glyphs from the default FontPage are rendered. 
	// There may be a better way to handle this.
	bool m_bRightToLeft;

	/* We keep this around only for reloading. */
	CString m_sChars;

	void LoadFontPageSettings( FontPageSettings &cfg, IniFile &ini, const CString &sTexturePath, const CString &PageName, CString sChars );
	static void GetFontPaths( const CString &sFontOrTextureFilePath, CStringArray &sTexturePaths );
	CString GetPageNameFromFileName( const CString &sFilename );
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
