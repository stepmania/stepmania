/*
-----------------------------------------------------------------------------
 Class: FontManager

 Desc: Manages Loading and Unloading of fonts.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#ifndef _FontManager_H_
#define _FontManager_H_


#include "Font.h"


//-----------------------------------------------------------------------------
// FontManager Class Declarations
//-----------------------------------------------------------------------------
class FontManager
{
public:
	FontManager();
	~FontManager();

	Font* LoadFont( CString sFontPath );
	bool IsFontLoaded( CString sFontPath );
	void UnloadFont( CString sFontPath );

protected:
	// map from file name to a texture holder
	CTypedPtrMap<CMapStringToPtr, CString, Font*> m_mapPathToFont;
};

extern FontManager*	FONT;	// global and accessable from anywhere in our program

#endif
