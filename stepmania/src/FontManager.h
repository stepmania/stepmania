#ifndef FONTMANAGER_H
#define FONTMANAGER_H
/*
-----------------------------------------------------------------------------
 Class: FontManager

 Desc: Manages Loading and Unloading of fonts.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageUtil.h"
#include "Game.h"

#include <map>
class Font;

//-----------------------------------------------------------------------------
// FontManager Class Declarations
//-----------------------------------------------------------------------------
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

	static longchar MakeGameGlyph(wchar_t c, Game g);
	static bool ExtractGameGlyph(longchar ch, wchar_t &c, Game &g);

protected:
	// map from file name to a texture holder
	map<CString, Font*> m_mapPathToFont;
};

extern FontManager*	FONT;	// global and accessable from anywhere in our program


#endif
