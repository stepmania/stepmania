#ifndef BITMAPTEXT_H
#define BITMAPTEXT_H
/*
-----------------------------------------------------------------------------
 File: BitmapText

 Desc: An actor that holds a Font and draws text to the screen.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Sprite.h"

class Font;

class BitmapText : public Actor
{
public:
	BitmapText();
	virtual ~BitmapText();


	bool LoadFromFont( CString sFontName );
	bool LoadFromNumbers( CString sTexturePath )	{ return LoadFromTextureAndChars(sTexturePath,"0123456789%. :x"); };
	bool LoadFromTextureAndChars( CString sTexturePath, CString sChars );
	void SetText( CString sText );

	int GetWidestLineWidthInSourcePixels() { return m_iWidestLineWidth; };
	void CropToWidth( int iWidthInSourcePixels );

	virtual void DrawPrimitives();

	void TurnRainbowOn()	{ m_bRainbow = true; };
	void TurnRainbowOff()	{ m_bRainbow = false; };

	void SetHorizAlign( HorizAlign ha );
	void SetVertAlign( VertAlign va );

public:
	CString m_sFontFilePath;
	Font* m_pFont;

protected:
	
	// recalculate the items below on SetText()
	CString	m_szText;
	vector<CString> m_szTextLines;
	vector<int>		m_iLineWidths;			// in source pixels
	vector<int>		m_iLineHeights;			// in source pixels
	int				m_iWidestLineWidth;		// in source pixels

	bool m_bRainbow;

	vector<RageVertex> verts;
	vector<RageTexture *> tex;
	
	void BuildChars();
	void DrawChars();
};


#endif
