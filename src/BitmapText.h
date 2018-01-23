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

	struct BMT_TweenState
	{
		// We'd be better off not adding strokes to things we can't control
		// themewise (ScreenDebugOverlay for example). -Midiman
		BMT_TweenState();
		static void MakeWeightedAverage(BMT_TweenState& out,
			BMT_TweenState const& from, BMT_TweenState const& to, float between);
		bool operator==(BMT_TweenState const& other) const;
		bool operator!=(BMT_TweenState const& other) const { return !operator==(other); }
		void set_stroke_color(Rage::Color const& c, int id);
		Rage::Color const& GetStrokeColor() { return m_stroke_color[0]; }
		Rage::Color const& get_a_stroke(int id);
	friend class BitmapText;
	private:
		std::array<Rage::Color, NUM_DIFFUSE_COLORS> m_stroke_color;
	};

	BMT_TweenState& BMT_DestTweenState()
	{
		if(BMT_Tweens.empty())
		{ return BMT_current; }
		else
		{ return BMT_Tweens.back(); }
	}
	BMT_TweenState const& BMT_DestTweenState() const { return const_cast<BitmapText*>(this)->BMT_DestTweenState(); }

	virtual void SetCurrentTweenStart();
	virtual void EraseHeadTween();
	virtual void UpdatePercentThroughTween(float between);
	virtual void BeginTweening(float time, ITween* interp);
	// This function exists because the compiler tried to connect a call of
	// "BeginTweening(1.2f)" to the function above. -Kyz
	virtual void BeginTweening(float time, TweenType tt = TWEEN_LINEAR)
	{ Actor::BeginTweening(time, tt); }
	virtual void StopTweening();
	virtual void FinishTweening();

	bool LoadFromFont( const std::string& sFontName );
	bool LoadFromTextureAndChars( const std::string& sTexturePath, const std::string& sChars );
	virtual void SetText( const std::string& sText, const std::string& sAlternateText = "", int iWrapWidthPixels = -1 );
	void SetVertSpacing( int iSpacing );
	void SetMaxWidth( float fMaxWidth );
	void SetMaxHeight( float fMaxHeight );
	void SetMaxDimUseZoom(bool use);
	void SetWrapWidthPixels( int iWrapWidthPixels );
	void CropLineToWidth(size_t l, int width);
	void CropToWidth(int width);

	virtual bool EarlyAbortDraw() const;
	virtual void DrawPrimitives();

	void SetUppercase( bool b );
	void SetRainbowScroll( bool b )	{ m_bRainbowScroll = b; }
	void SetJitter( bool b )	{ m_bJitter = b; }
	void SetDistortion( float f );
	void UnSetDistortion();
	void set_mult_attrs_with_diffuse(bool m);
	bool get_mult_attrs_with_diffuse();

	void SetHorizAlign( float f );

	void set_stroke_color(Rage::Color const& c, int id) { BMT_DestTweenState().set_stroke_color(c, id); }
	Rage::Color const& GetStrokeColor()		{ return BMT_DestTweenState().GetStrokeColor(); }
	Rage::Color const& get_a_stroke(int id)		{ return BMT_DestTweenState().get_a_stroke(id); }
	Rage::Color const& GetCurrStrokeColor() { return BMT_current.GetStrokeColor(); }

	void SetTextGlowMode( TextGlowMode tgm )	{ m_TextGlowMode = tgm; }

	void GetLines( std::vector<std::wstring> &wTextLines ) const { wTextLines = m_wTextLines; }
	const std::vector<std::wstring> &GetLines() const { return m_wTextLines; }

	std::string GetText() const { return m_sText; }
	// Return true if the string 's' will use an alternate string, if available.
	bool StringWillUseAlternate( const std::string& sText, const std::string& sAlternateText ) const;

	struct Attribute
	{
		Attribute() : length(-1), glow() { }
		int		length;
		Rage::Color	diffuse[NUM_DIFFUSE_COLORS];
		Rage::Color	glow;
		void set_diffuse(Rage::Color const& c)
		{
			for(size_t i= 0; i < NUM_DIFFUSE_COLORS; ++i)
			{
				diffuse[i]= c;
			}
		}

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
	std::string		m_sText;
	std::vector<std::wstring>		m_wTextLines;
	std::vector<int>		m_iLineWidths;	// in source pixels
	int			m_iWrapWidthPixels;		// -1 = no wrap
	float		m_fMaxWidth;			// 0 = no max
	float		m_fMaxHeight;			// 0 = no max
	bool		m_MaxDimensionUsesZoom;
	bool		m_bRainbowScroll;
	bool		m_bJitter;
	bool		m_bUsingDistortion;
	bool m_mult_attrs_with_diffuse;
	float		m_fDistortion;
	int			m_iVertSpacing;

	std::vector<Rage::SpriteVertex>	m_aVertices;

	std::vector<FontPageTextures*>	m_vpFontPageTextures;
	// This cannot be an unordered_map because the logic for applying the
	// attributes requires them to be in order. -Kyz
	std::map<size_t, Attribute>		m_mAttributes;
	bool				m_bHasGlowAttribute;

	TextGlowMode	m_TextGlowMode;

	// recalculate the items in SetText()
	void BuildChars();
	void DrawChars( bool bUseStrokeTexture );
	void UpdateBaseZoom();

private:
	void SetTextInternal();
	std::vector<BMT_TweenState> BMT_Tweens;
	BMT_TweenState BMT_current;
	BMT_TweenState BMT_start;
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
