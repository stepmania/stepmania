/*
-----------------------------------------------------------------------------
 File: BitmapText.h

 Desc: A font class that draws characters from a bitmap.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _BITMAPTEXT_H_
#define _BITMAPTEXT_H_


#include "Sprite.h"
#include "BitmapText.h"


const int NUM_CHARS = 256;


class BitmapText : public Sprite
{
protected:

public:
	BitmapText();

	bool LoadFromFontName( CString sFontName );
	void SetText( CString sText ) { m_sText = sText; };
	CString GetText() { return m_sText; };

	bool LoadFontWidths( CString sFilePath );
	//float GetWidthZoomed();

//	void SetTopColor( D3DXCOLOR new_color ) { m_colorTop = new_color; };
//	void SetBottomColor( D3DXCOLOR new_color ) { m_colorBottom = new_color; };
//	void SetColor( D3DXCOLOR new_color ) { SetTopColor(new_color); SetBottomColor(new_color); };

	void Draw();

	float GetWidthInSourcePixels();	// in logical, pre-scaled units

protected:
	bool LoadCharWidths( CString sWidthFilePath );

	CString m_sFontName;
	float m_fCharWidthsInSourcePixels[NUM_CHARS];	// in soure coordinate space

	CString	m_sText;	// the string that the font is displaying
//	D3DXCOLOR	m_colorTop;
//	D3DXCOLOR	m_colorBottom;
//	bool	m_bHasShadow;
};


#endif