#pragma once
/*
-----------------------------------------------------------------------------
 File: BitmapText

 Desc: An actor that holds a Font and draws text to the screen.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Sprite.h"
#include "Font.h"


const int MAX_TEXT_LINES	=	40;
const int MAX_TEXT_CHARS	=	MAX_NUM_QUADS;

class BitmapText : public Actor
{
protected:

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

protected:
	CString m_sFontFilePath;
	Font* m_pFont;
	
	// recalculate the items below on SetText()
	TCHAR   m_szText[MAX_TEXT_CHARS];
	TCHAR*	m_szTextLines[MAX_TEXT_LINES];	// pointers into m_szText
	int		m_iLineLengths[MAX_TEXT_LINES];	// in characters
	int		m_iNumLines;
	int		m_iLineWidths[MAX_TEXT_LINES];	// in source pixels
	int		m_iWidestLineWidth;					// in source pixels

	bool m_bRainbow;
};

