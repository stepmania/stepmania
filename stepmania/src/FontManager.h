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

#include "Font.h"

#include <map>

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

	static bool MatchesFont(CString FontName, CString FileName);

protected:
	// map from file name to a texture holder
	map<CString, Font*> m_mapPathToFont;
	static Font *LoadFontInt(const CString &sFontOrTextureFilePath, CString sChars);
	static CString GetPageNameFromFileName(const CString &fn);
	static void LoadFontPageSettings(FontPageSettings &cfg, IniFile &ini, const CString &PageName, int NumFrames);
	static void GetFontPaths(const CString &sFontOrTextureFilePath, 
							   CStringArray &TexturePaths, CString &IniPath);
};

extern FontManager*	FONT;	// global and accessable from anywhere in our program


#endif
