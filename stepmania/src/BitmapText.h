/*
-----------------------------------------------------------------------------
 File: BitmapText

 Desc: An actor that holds a Font and draws text to the screen.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#ifndef _BITMAPTEXT_H_
#define _BITMAPTEXT_H_


#include "Sprite.h"
#include "Font.h"


const int MAX_TEXT_LINES	=	20;
const int MAX_TEXT_CHARS	=	MAX_NUM_QUADS;

class BitmapText : public Actor
{
protected:

public:
	BitmapText();
	~BitmapText();


	bool Load( const CString &sFontName );
	void SetText( CString sText );

	int GetWidestLineWidthInSourcePixels() { return m_iWidestLineWidth; };

	void RebuildVertexBuffer();		// fill the RageScreen's vertex buffer with what we're going to draw
	virtual void RenderPrimitives();

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

	// recalculate on RebuildVertexBuffer()
//	LPDIRECT3DVERTEXBUFFER8 m_pVB;
	int		m_iNumV;		// number of verticies we filled in the vertex buffer

	bool m_bRainbow;
};


#endif