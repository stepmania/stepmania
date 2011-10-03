#ifndef BITMAP_TEXT_H
#define BITMAP_TEXT_H

#include "Actor.h"
#include <map>

class RageTexture;
class Font;
struct FontPageTextures;
/** @brief An actor that holds a Font and draws text to the screen. */
class BitmapText : public Actor
{
public:
	BitmapText();
	BitmapText( const BitmapText &cpy );
	BitmapText &operator=(const BitmapText &cpy);
	virtual ~BitmapText();

	virtual void LoadFromNode( const XNode* pNode );
	virtual BitmapText *Copy() const;

	bool LoadFromFont( const RString& sFontName );
	bool LoadFromTextureAndChars( const RString& sTexturePath, const RString& sChars );
	virtual void SetText( const RString& sText, const RString& sAlternateText = "", int iWrapWidthPixels = -1 );
	void SetVertSpacing( int iSpacing );
	void SetMaxWidth( float fMaxWidth );
	void SetMaxHeight( float fMaxHeight );
	void SetWrapWidthPixels( int iWrapWidthPixels );
	void CropToWidth( int iWidthInSourcePixels );

	virtual bool EarlyAbortDraw() const;
	virtual void DrawPrimitives();

	void SetUppercase( bool b );
	void SetRainbowScroll( bool b )	{ m_bRainbowScroll = b; }
	void SetJitter( bool b )	{ m_bJitter = b; }

	void SetHorizAlign( float f );

	void SetStrokeColor( RageColor c )	{ m_StrokeColor = c; }
	RageColor GetStrokeColor()		{ return m_StrokeColor; }

	void SetTextGlowMode( TextGlowMode tgm )	{ m_TextGlowMode = tgm; }

	void GetLines( vector<wstring> &wTextLines ) const { wTextLines = m_wTextLines; }
	const vector<wstring> &GetLines() const { return m_wTextLines; }

	RString GetText() const { return m_sText; }
	// Return true if the string 's' will use an alternate string, if available.
	bool StringWillUseAlternate( const RString& sText, const RString& sAlternateText ) const;

	struct Attribute
	{
		Attribute() : length(-1), glow() { }
		int		length;
		RageColor	diffuse[4];
		RageColor	glow;

		void FromStack( lua_State *L, int iPos );
	};

	Attribute GetDefaultAttribute() const;
	void AddAttribute( size_t iPos, const Attribute &attr );
	void ClearAttributes();

	// Commands
	virtual void PushSelf( lua_State *L );

protected:
	Font		*m_pFont;
	bool		m_bUppercase;
	RString		m_sText;
	vector<wstring>		m_wTextLines;
	vector<int>		m_iLineWidths;	// in source pixels
	int			m_iWrapWidthPixels;		// -1 = no wrap
	float		m_fMaxWidth;			// 0 = no max
	float		m_fMaxHeight;			// 0 = no max
	bool		m_bRainbowScroll;
	bool		m_bJitter;
	int			m_iVertSpacing;

	vector<RageSpriteVertex>	m_aVertices;

	vector<FontPageTextures*>	m_vpFontPageTextures;
	map<size_t, Attribute>		m_mAttributes;
	bool				m_bHasGlowAttribute;

	RageColor		m_StrokeColor;
	TextGlowMode	m_TextGlowMode;

	// recalculate the items in SetText()
	void BuildChars();
	void DrawChars( bool bUseStrokeTexture );
	void UpdateBaseZoom();

private:
	void SetTextInternal();
};

#endif

/**
 * @file
 * @author Chris Danford, Charles Lohr, Steve Checkoway (c) 2001-2007
 * @section LICENSE
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
