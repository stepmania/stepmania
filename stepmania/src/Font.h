/*
-----------------------------------------------------------------------------
 File: Font

 Desc: A holder for information that is used by BitmapText objects.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#ifndef _Font_H_
#define _Font_H_


#include "RageTexture.h"

const int MAX_FONT_CHARS = 256;		// only low ASCII chars are supported


class Font
{
protected:

public:
	Font( const CString &sFontPath );
	~Font();

	int GetLineWidthInSourcePixels( LPCTSTR szLine, int iLength );

	int m_iRefCount;

	CString m_sFontFilePath;
	CString m_sTexturePath;

	RageTexture* m_pTexture;
	bool m_bCapitalsOnly;
	float m_fDrawExtraPercent;	// for italic fonts

	int m_iCharToFrameNo[MAX_FONT_CHARS];
	int	m_iFrameNoToWidth[MAX_FONT_CHARS];	// in soure coordinate space

protected:

};


#endif