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
	void ReloadFonts();

	/* Return true if FileName is a part of the font "FontName". */
	static bool MatchesFont(CString FontName, CString FileName);

	static longchar MakeGameGlyph(longchar c, Game g);
	static bool ExtractGameGlyph(longchar ch, int &c, Game &g);

	typedef map<CString, longchar, StdStringLessNoCase> aliasmap;
	static aliasmap CharAliases;
	static map<CString,CString> CharAliasRepl;

	static void ReplaceMarkers( CString &sText );

protected:
	// map from file name to a texture holder
	map<CString, Font*> m_mapPathToFont;
	static void InitCharAliases();
};

extern FontManager*	FONT;	// global and accessable from anywhere in our program


#endif
