/*
-----------------------------------------------------------------------------
 File: BitmapText.h

 Desc: A font class that draws characters from a bitmap.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _BITMAPTEXT_H_
#define _BITMAPTEXT_H_


//#include "Actor.h"
#include "Sprite.h"
#include "BitmapText.h"

#include "BitmapFont.h"



class BitmapText : public Actor
{
public:
	BitmapText();
	~BitmapText();

	bool LoadFromFontName( CString sFontName );
	void SetText( CString sText ) { m_sText = sText; };
	CString GetText() { return m_sText; };

	void SetTopColor( D3DXCOLOR new_color ) { m_colorTop = new_color; };
	void SetBottomColor( D3DXCOLOR new_color ) { m_colorBottom = new_color; };
	void SetColor( D3DXCOLOR new_color ) { SetTopColor(new_color); SetBottomColor(new_color); };

	virtual void Draw();

protected:
	CString m_sFontName;
	CBitmapFont*	m_pFont;
	CString	m_sText;	// the string that the font is displaying
	D3DXCOLOR	m_colorTop;
	D3DXCOLOR	m_colorBottom;
};


#endif