/* BitmapText - An actor that holds a Font and draws text to the screen. */

#ifndef BITMAPTEXT_H
#define BITMAPTEXT_H

#include "Sprite.h"

class Font;

class BitmapText : public Actor
{
public:
	BitmapText();
	virtual ~BitmapText();


	bool LoadFromFont( CString sFontName );
	bool LoadFromTextureAndChars( CString sTexturePath, CString sChars );
	void SetText( CString sText, CString sAlternateText = "", int iWrapWidthPixels = -1 );
	void SetMaxWidth( float MaxWidth );
	void SetWrapWidthPixels( int iWrapWidthPixels );

	void CropToWidth( int iWidthInSourcePixels );

	virtual bool EarlyAbortDraw();
	virtual void DrawPrimitives();

	void TurnRainbowOn()	{ m_bRainbow = true; };
	void TurnRainbowOff()	{ m_bRainbow = false; };

	void SetHorizAlign( HorizAlign ha );
	void SetVertAlign( VertAlign va );

	void GetLines( vector<wstring> &wTextLines ) { wTextLines = m_wTextLines; }

	CString GetText() const { return m_sText; }
	/* Return true if the string 's' will use an alternate string, if available. */
	bool StringWillUseAlternate(CString sText, CString sAlternateText) const;

	virtual void HandleCommand( const ParsedCommand &command );

public:
	Font* m_pFont;

protected:
	
	// recalculate the items below on SetText()
	CString			m_sText;
	vector<wstring>	m_wTextLines;
	vector<int>		m_iLineWidths;			// in source pixels
	int				m_iWrapWidthPixels;	// -1 = no wrap
	float			m_fMaxWidth;

	bool m_bRainbow;

	vector<RageSpriteVertex> verts;
	vector<RageTexture *> tex;
	
	void BuildChars();
	void DrawChars();
	void UpdateBaseZoom();
};


#endif

/*
 * (c) 2001-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
