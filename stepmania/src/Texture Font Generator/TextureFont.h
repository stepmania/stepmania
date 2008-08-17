#ifndef TEXTURE_FONT_H
#define TEXTURE_FONT_H

#include <vector>
#include <map>
using namespace std;

struct FontPageDescription
{
	CString name;
	vector<wchar_t> chars;
};

struct FontPage
{
	FontPage();
	~FontPage();

	void Create( unsigned width, unsigned height );

	HBITMAP m_hPage;

	/* Width and height of this page: */
//	int Width, Height;

	int m_iFrameWidth, m_iFrameHeight;
	int m_iNumFramesX, m_iNumFramesY;
};

/* Create a bitmap font with the given parameters. */
class TextureFont
{
public:
	TextureFont();
	~TextureFont();

	vector<FontPageDescription> m_PagesToGenerate;
	void FormatFontPage( int iPage, HDC hDC );
	void FormatFontPages();
	void Save( CString sPath, CString sBitmapAppendBeforeExtension, bool bSaveMetrics, bool bSaveBitmaps, bool bExportStrokeTemplates );

	map<wchar_t, HBITMAP> m_Characters;

	/* Font generation properties: */
	bool m_bBold;				/* whether font is bold */
	bool m_bItalic;				/* whether font is italic */
	bool m_bAntiAlias;			/* antialiasing type */
	float m_fFontSizePixels;		/* font size in pixels */
	CString m_sFamily;			/* font family */
	int m_iPadding;				/* empty padding between characters */

	/* Derived properties: */
	int m_iCharDescent, m_iCharLeftOverlap, m_iCharRightOverlap, m_iCharBaseline,
		m_iCharTop, m_iCharVertSpacing;

	RECT m_BoundingRect;

	vector<FontPage *> m_apPages;

	CString m_sError, m_sWarnings;

private:
	int GetTopPadding() const;

	/* Bounds of each character, according to MeasureCharacterRanges. */
	map<wchar_t, RECT> m_RealBounds;

	map<wchar_t, ABC> m_ABC;
	void FormatCharacter( wchar_t c, HDC hDC );
};

#endif

/*
 * Copyright (c) 2003-2007 Glenn Maynard
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
