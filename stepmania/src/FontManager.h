/* FontManager - Interface for loading and releasing fonts. */

#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include "RageUtil.h"

class Font;
class Game;

class FontManager
{
public:
	FontManager();
	~FontManager();

	Font* LoadFont( const CString &sFontOrTextureFilePath, CString sChars = "" );
	void UnloadFont( Font *fp );

	/* Warning: This reloads fonts completely, so all BitmapTexts need to be
	 * reset, too.  If this isn't done, best case they end up with old font
	 * metrics; worst case, they get left with stale pointers to RageTextures
	 * that'll get freed eventually and crash.  This is only used for realtime
	 * adjustment of fonts in ScreenTestFonts at the moment. */
	void ReloadFonts();

	static longchar MakeGameGlyph(wchar_t c, const Game* g);
	static bool ExtractGameGlyph(longchar ch, wchar_t &c, const Game*& g);
};

extern FontManager*	FONT;	// global and accessable from anywhere in our program


#endif

/*
 * (c) 2001-2003 Chris Danford, Glenn Maynard
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
