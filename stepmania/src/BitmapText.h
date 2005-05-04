/* BitmapText - An actor that holds a Font and draws text to the screen. */

#ifndef BITMAPTEXT_H
#define BITMAPTEXT_H

#include "Actor.h"
class RageTexture;

class Font;


template<class T>
class LunaBitmapText : public LunaActor<T>
{
public:
	LunaBitmapText() { LUA->Register( Register ); }

	static int wrapwidthpixels( T* p, lua_State *L )	{ p->SetWrapWidthPixels( IArg(1) ); return 0; }
	static int maxwidth( T* p, lua_State *L )			{ p->SetMaxWidth( FArg(1) ); return 0; }
	static int maxheight( T* p, lua_State *L )			{ p->SetMaxHeight( FArg(1) ); return 0; }
	static int settext( T* p, lua_State *L )			{ p->SetText( SArg(1) ); return 0; }
	static int GetText( T* p, lua_State *L )			{ lua_pushstring( L, p->GetText() ); return 1; }

	static void Register(lua_State *L) 
	{
		ADD_METHOD( wrapwidthpixels )
		ADD_METHOD( maxwidth )
		ADD_METHOD( maxheight )
		ADD_METHOD( settext )
		ADD_METHOD( GetText )
		LunaActor<T>::Register( L );
	}
};


class BitmapText : public Actor
{
public:
	BitmapText();
	virtual ~BitmapText();

	void LoadFromNode( const CString& sDir, const XNode* pNode );

	bool LoadFromFont( const CString& sFontName );
	bool LoadFromTextureAndChars( const CString& sTexturePath, const CString& sChars );
	void SetText( const CString& sText, const CString& sAlternateText = "", int iWrapWidthPixels = -1 );
	void SetMaxWidth( float fMaxWidth );
	void SetMaxHeight( float fMaxHeight );
	void SetWrapWidthPixels( int iWrapWidthPixels );
	void SetMaxLines( int iNumLines, int iDirection = 0 );	//Direction: 0 for crop bottom, 1 for crop top

	void CropToWidth( int iWidthInSourcePixels );

	virtual bool EarlyAbortDraw();
	virtual void DrawPrimitives();

	void TurnRainbowOn()	{ m_bRainbow = true; };
	void TurnRainbowOff()	{ m_bRainbow = false; };

	void SetHorizAlign( HorizAlign ha );
	void SetVertAlign( VertAlign va );

	void GetLines( vector<wstring> &wTextLines ) { wTextLines = m_wTextLines; }

	void SetDynamicColor( bool coloration ) { m_bColored = coloration; }

	CString GetText() const { return m_sText; }
	/* Return true if the string 's' will use an alternate string, if available. */
	bool StringWillUseAlternate(const CString& sText, const CString& sAlternateText) const;

	//
	// Commands
	//
	virtual void PushSelf( lua_State *L );

public:
	Font* m_pFont;

protected:
	
	// recalculate the items below on SetText()
	CString			m_sText;
	vector<wstring>	m_wTextLines;
	vector<int>		m_iLineWidths;			// in source pixels
	int				m_iWrapWidthPixels;	// -1 = no wrap
	float			m_fMaxWidth;
	float			m_fMaxHeight;

	bool m_bRainbow;

	bool m_bColored;

	struct ColorChange
	{
		RageColor c;	//Color to change to
		int l;			//Change Location
	};
	vector<ColorChange> m_vColors;
	vector<RageSpriteVertex> verts;
	vector<RageTexture *> tex;
	
	void AddLine( CString & sAddition, int & iWidthPixels );
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
