#pragma once
/*
-----------------------------------------------------------------------------
 File: Font

 Desc: A holder for information that is used by BitmapText objects.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageTexture.h"

const int MAX_FONT_CHARS = 256;		// only low ASCII chars are supported


class Font
{
protected:

public:
	Font( const CString &sASCIITexturePath );
	Font( const CString &sTexturePath, const CString& sChars );
	~Font();

	int GetLineWidthInSourcePixels( const char *szLine, int iLength );

	int m_iRefCount;

	CString m_sTexturePath;

	RageTexture* m_pTexture;
	bool m_bCapitalsOnly;
	int m_iDrawExtraPixelsLeft, m_iDrawExtraPixelsRight;	// for italic fonts
	int m_iLineSpacing;

	int m_iCharToFrameNo[MAX_FONT_CHARS];
	int	m_iFrameNoToWidth[MAX_FONT_CHARS];	// in soure coordinate space

protected:

};
