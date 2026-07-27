#include "global.h"
#include "BitmapText.h"
#include "XmlFile.h"
#include "FontManager.h"
#include "RageLog.h"
#include "RageTimer.h"
#include "RageDisplay.h"
#include "ThemeManager.h"
#include "Font.h"
#include "ActorUtil.h"
#include "LuaBinding.h"

#include <cmath>
#include <cstddef>
#include <vector>


REGISTER_ACTOR_CLASS( BitmapText );

/* XXX:
 * We need some kind of font modifier string for metrics.  For example,
 * "valign=top;spacing = x+5,y+2"
 *
 * Better, we could go all the way, drop all of the actor-specific font aliases,
 * and do "font=header2;valign=top;...". */

 /* XXX: Changing a whole array of diffuse colors every frame (several times) is
 * a waste, when we're usually setting them all to the same value. Rainbow and
 * fading are annoying to optimize, but rarely used. Iterating over every
 * character in Draw() is dumb. */
#define NUM_RAINBOW_COLORS	THEME->GetMetricI("BitmapText","NumRainbowColors")
#define RAINBOW_COLOR(n)	THEME->GetMetricC("BitmapText",ssprintf("RainbowColor%i", n+1))

static std::vector<RageColor> RAINBOW_COLORS;

BitmapText::BitmapText()
{
	// Loading these theme metrics is slow, so only do it every 20th time.
	// todo: why not check to see if you need to bother updating this at all? -aj
	static int iReloadCounter = 0;
	if( iReloadCounter % 20==0 )
	{
		RAINBOW_COLORS.resize( NUM_RAINBOW_COLORS );
		for( unsigned i = 0; i < RAINBOW_COLORS.size(); ++i )
			RAINBOW_COLORS[i] = RAINBOW_COLOR(i);
	}
	iReloadCounter++;

	m_pFont = nullptr;
	m_bUppercase = false;

	m_bRainbowScroll = false;
	m_bJitter = false;
	m_fDistortion= 0.0f;
	m_bUsingDistortion= false;
	m_mult_attrs_with_diffuse= false;

	m_iWrapWidthPixels = -1;
	m_fMaxWidth = 0;
	m_fMaxHeight = 0;
	m_iVertSpacing = 0;
	m_MaxDimensionUsesZoom= false;
	m_bHasGlowAttribute = false;
	// Never, this way we dont have awkward settings between themes. -Midiman
	SetShadowLength( 0 );
	// SM4SVN r28328, "draw glow using stroke texture" forces the BitmapText to
	// glow both the inner and stroke elements. This makes BitmapText elements
	// with an invisible stroke have a glowing stroke instead. Not good. -aj
	m_TextGlowMode = TextGlowMode_Both; // Both used for compatibility with SM4
}

BitmapText::~BitmapText()
{
	if( m_pFont )
		FONT->UnloadFont( m_pFont );
}

BitmapText & BitmapText::operator=(const BitmapText &cpy)
{
	Actor::operator=(cpy);

#define CPY(a) a = cpy.a
	CPY( m_bUppercase );
	CPY( m_sText );
	CPY( m_wTextLines );
	CPY( m_iLineWidths );
	CPY( m_iWrapWidthPixels );
	CPY( m_fMaxWidth );
	CPY( m_fMaxHeight );
	CPY( m_bRainbowScroll );
	CPY( m_bJitter );
	CPY( m_fDistortion );
	CPY( m_bUsingDistortion );
	CPY( m_mult_attrs_with_diffuse );
	CPY( m_iVertSpacing );
	CPY( m_MaxDimensionUsesZoom );
	CPY( m_aVertices );
	CPY( m_vpFontPageTextures );
	CPY( m_mAttributes );
	CPY( m_bHasGlowAttribute );
	CPY( BMT_Tweens );
	CPY( BMT_current );
	CPY( BMT_start );
#undef CPY

	if( m_pFont )
		FONT->UnloadFont( m_pFont );

	if( cpy.m_pFont != nullptr )
		m_pFont = FONT->CopyFont( cpy.m_pFont );
	else
		m_pFont = nullptr;

	return *this;
}

BitmapText::BitmapText( const BitmapText &cpy ):
	Actor( cpy )
{
	m_pFont = nullptr;

	*this = cpy;
}

void BitmapText::SetCurrentTweenStart()
{
	BMT_start= BMT_current;
}

void BitmapText::EraseHeadTween()
{
	BMT_current= BMT_Tweens[0];
	BMT_Tweens.erase(BMT_Tweens.begin());
}

void BitmapText::UpdatePercentThroughTween(float between)
{
	BMT_TweenState::MakeWeightedAverage(BMT_current, BMT_start, BMT_Tweens[0],
		between);
}

void BitmapText::BeginTweening(float time, ITween* interp)
{
	Actor::BeginTweening(time, interp);
	if(!BMT_Tweens.empty())
	{
		BMT_Tweens.push_back(BMT_Tweens.back());
	}
	else
	{
		BMT_Tweens.push_back(BMT_current);
	}
}

void BitmapText::StopTweening()
{
	BMT_Tweens.clear();
	Actor::StopTweening();
}

void BitmapText::FinishTweening()
{
	if(!BMT_Tweens.empty())
	{
		BMT_current= BMT_DestTweenState();
	}
	Actor::FinishTweening();
}

void BitmapText::BMT_TweenState::MakeWeightedAverage(BMT_TweenState& out,
	BMT_TweenState const& from, BMT_TweenState const& to, float between)
{
	out.m_stroke_color.b= lerp(between, from.m_stroke_color.b, to.m_stroke_color.b);
	out.m_stroke_color.g= lerp(between, from.m_stroke_color.g, to.m_stroke_color.g);
	out.m_stroke_color.r= lerp(between, from.m_stroke_color.r, to.m_stroke_color.r);
	out.m_stroke_color.a= lerp(between, from.m_stroke_color.a, to.m_stroke_color.a);
}

void BitmapText::LoadFromNode( const XNode* node )
{
	RString text;
	node->GetAttrValue("Text", text);
	RString alt_text;
	node->GetAttrValue("AltText", alt_text);

	ThemeManager::EvaluateString(text);
	ThemeManager::EvaluateString(alt_text);

	RString font;
	// Pass optional= true so that an error will not be reported if the path
	// doesn't resolve to a file.  This way, a font can be either a path or the
	// name of a font to look up in Fonts/.  -Kyz
	if(!ActorUtil::GetAttrPath(node, "Font", font, true) &&
		!ActorUtil::GetAttrPath(node, "File", font, true))
	{
		if(!node->GetAttrValue("Font", font) &&
			!node->GetAttrValue("File", font)) // accept "File" for backward compatibility
		{
			LuaHelpers::ReportScriptErrorFmt("%s: BitmapText: Font or File attribute"
				" not found", ActorUtil::GetWhere(node).c_str());
			font = "Common Normal";
		}
		font = THEME->GetPathF("", font);
	}

	LoadFromFont(font);

	SetText(text, alt_text);
	Actor::LoadFromNode(node);
}

bool BitmapText::LoadFromFont( const RString& sFontFilePath )
{
	CHECKPOINT_M( ssprintf("BitmapText::LoadFromFont(%s)", sFontFilePath.c_str()) );

	if( m_pFont )
	{
		FONT->UnloadFont( m_pFont );
		m_pFont = nullptr;
	}

	m_pFont = FONT->LoadFont( sFontFilePath );

	this->SetStrokeColor( m_pFont->GetDefaultStrokeColor() );

	BuildChars();

	return true;
}

bool BitmapText::LoadFromTextureAndChars( const RString& sTexturePath, const RString& sChars )
{
	CHECKPOINT_M( ssprintf("BitmapText::LoadFromTextureAndChars(\"%s\",\"%s\")", sTexturePath.c_str(), sChars.c_str()) );

	if( m_pFont )
	{
		FONT->UnloadFont( m_pFont );
		m_pFont = nullptr;
	}

	m_pFont = FONT->LoadFont( sTexturePath, sChars );

	BuildChars();

	return true;
}

void BitmapText::BuildChars()
{
	// If we don't have a font yet, we'll do this when it loads.
	if( m_pFont == nullptr )
		return;

	// calculate line lengths and widths
	m_size.x = 0;

	m_iLineWidths.clear();
	for( unsigned l=0; l<m_wTextLines.size(); l++ ) // for each line
	{
		m_iLineWidths.push_back(m_pFont->GetLineWidthInSourcePixels( m_wTextLines[l] ));
		m_size.x = std::max( m_size.x, (float) m_iLineWidths.back() );
	}

	/* Ensure that the width is always even. This maintains pixel alignment;
	 * fX below will always be an integer. */
	m_size.x = QuantizeUp( m_size.x, 2.0f );

	m_aVertices.clear();
	m_vpFontPageTextures.clear();

	if( m_wTextLines.empty() )
		return;

	m_size.y = float(m_pFont->GetHeight() * m_wTextLines.size());

	// The height (from the origin to the baseline):
	int iPadding = m_pFont->GetLineSpacing() - m_pFont->GetHeight();
	iPadding += m_iVertSpacing;

	// There's padding between every line:
	m_size.y += iPadding * int(m_wTextLines.size()-1);

	// the top position of the first row of characters
	int iY = std::lrint(-m_size.y/2.0f);

	for( unsigned i=0; i<m_wTextLines.size(); i++ ) // foreach line
	{
		iY += m_pFont->GetHeight();

		std::wstring sLine = m_wTextLines[i];
		if( m_pFont->IsRightToLeft() )
			reverse( sLine.begin(), sLine.end() );
		const int iLineWidth = m_iLineWidths[i];

		float fX = SCALE( m_fHorizAlign, 0.0f, 1.0f, -m_size.x/2.0f, +m_size.x/2.0f - iLineWidth );
		int iX = std::lrint( fX );

		for( unsigned j = 0; j < sLine.size(); ++j )
		{
			RageSpriteVertex v[4];
			const glyph &g = m_pFont->GetGlyph( sLine[j] );

			// Advance the cursor early for RTL(?)
			if( m_pFont->IsRightToLeft() )
				iX -= g.m_iHadvance;

			// set vertex positions
			v[0].p = RageVector3( iX+g.m_fHshift,			iY+g.m_pPage->m_fVshift,		0 );	// top left
			v[1].p = RageVector3( iX+g.m_fHshift,			iY+g.m_pPage->m_fVshift+g.m_fHeight,	0 );	// bottom left
			v[2].p = RageVector3( iX+g.m_fHshift+g.m_fWidth,	iY+g.m_pPage->m_fVshift+g.m_fHeight,	0 );	// bottom right
			v[3].p = RageVector3( iX+g.m_fHshift+g.m_fWidth,	iY+g.m_pPage->m_fVshift,		0 );	// top right

			// Advance the cursor.
			if( !m_pFont->IsRightToLeft() )
				iX += g.m_iHadvance;

			// set texture coordinates
			v[0].t = RageVector2( g.m_TexRect.left,	g.m_TexRect.top );
			v[1].t = RageVector2( g.m_TexRect.left,	g.m_TexRect.bottom );
			v[2].t = RageVector2( g.m_TexRect.right,	g.m_TexRect.bottom );
			v[3].t = RageVector2( g.m_TexRect.right,	g.m_TexRect.top );

			m_aVertices.insert( m_aVertices.end(), &v[0], &v[4] );
			m_vpFontPageTextures.push_back( g.GetFontPageTextures() );
		}

		// The amount of padding a line needs:
		iY += iPadding;
	}

	if( m_bUsingDistortion )
	{
		int iSeed = std::lrint( RageTimer::GetTimeSinceStartFast()*500000.0f );
		RandomGen rnd( iSeed );
		for(unsigned int i= 0; i < m_aVertices.size(); i+=4)
		{
			float w= m_aVertices[i+2].p.x - m_aVertices[i].p.x;
			float h= m_aVertices[i+2].p.y - m_aVertices[i].p.y;
			for(unsigned int ioff= 0; ioff < 4; ++ioff)
			{
				m_aVertices[i+ioff].p.x += ((rnd()%9) / 8.0f - .5f) * m_fDistortion * w;
				m_aVertices[i+ioff].p.y += ((rnd()%9) / 8.0f - .5f) * m_fDistortion * h;
			}
		}
	}
}

void BitmapText::DrawChars( bool bUseStrokeTexture )
{
	// bail if cropped all the way
	if( m_pTempState->crop.left + m_pTempState->crop.right >= 1  ||
		m_pTempState->crop.top + m_pTempState->crop.bottom >= 1 )
		return;

	const int iNumGlyphs = m_vpFontPageTextures.size();
	int iStartGlyph = std::lrint( SCALE( m_pTempState->crop.left, 0.f, 1.f, 0, (float) iNumGlyphs ) );
	int iEndGlyph = std::lrint( SCALE( m_pTempState->crop.right, 0.f, 1.f, (float) iNumGlyphs, 0 ) );
	iStartGlyph = clamp( iStartGlyph, 0, iNumGlyphs );
	iEndGlyph = clamp( iEndGlyph, 0, iNumGlyphs );

	if( m_pTempState->fade.top > 0 ||
		m_pTempState->fade.bottom > 0 ||
		m_pTempState->fade.left > 0 ||
		m_pTempState->fade.right > 0 )
	{
		// Handle fading by tweaking the alpha values of the vertices.

		// Actual size of the fade on each side:
		const RectF &FadeDist = m_pTempState->fade;
		RectF FadeSize = FadeDist;

		// If the cropped size is less than the fade distance, clamp.
		const float fHorizRemaining = 1.0f - (m_pTempState->crop.left + m_pTempState->crop.right);
		if( FadeDist.left+FadeDist.right > 0 &&
			fHorizRemaining < FadeDist.left+FadeDist.right )
		{
			const float LeftPercent = FadeDist.left/(FadeDist.left+FadeDist.right);
			FadeSize.left = LeftPercent * fHorizRemaining;
			FadeSize.right = (1.0f-LeftPercent) * fHorizRemaining;
		}

		/* We fade from 0 to LeftColor, then from RightColor to 0. (We won't fade
		 * all the way to 0 if the crop is beyond the outer edge.) */
		const float fRightAlpha  = SCALE( FadeSize.right,  FadeDist.right,  0, 1, 0 );
		const float fLeftAlpha   = SCALE( FadeSize.left,   FadeDist.left,   0, 1, 0 );

		const float fStartFadeLeftPercent = m_pTempState->crop.left;
		const float fStopFadeLeftPercent = m_pTempState->crop.left + FadeSize.left;
		const float fLeftFadeStartGlyph = SCALE( fStartFadeLeftPercent, 0.f, 1.f, 0, (float) iNumGlyphs );
		const float fLeftFadeStopGlyph = SCALE( fStopFadeLeftPercent, 0.f, 1.f, 0, (float) iNumGlyphs );

		const float fStartFadeRightPercent = 1-(m_pTempState->crop.right + FadeSize.right);
		const float fStopFadeRightPercent = 1-(m_pTempState->crop.right);
		const float fRightFadeStartGlyph = SCALE( fStartFadeRightPercent, 0.f, 1.f, 0, (float) iNumGlyphs );
		const float fRightFadeStopGlyph = SCALE( fStopFadeRightPercent, 0.f, 1.f, 0, (float) iNumGlyphs );

		for( int start = iStartGlyph; start < iEndGlyph; ++start )
		{
			int i = start*4;

			float fAlpha = 1.0f;
			if( FadeSize.left > 0.001f )
			{
				// Add .5, so we fade wrt. the center of the vert, not the left side.
				float fPercent = SCALE( start+0.5f, fLeftFadeStartGlyph, fLeftFadeStopGlyph, 0.0f, 1.0f );
				fPercent = clamp( fPercent, 0.0f, 1.0f );
				fAlpha *= fPercent * fLeftAlpha;
			}

			if( FadeSize.right > 0.001f )
			{
				float fPercent = SCALE( start+0.5f, fRightFadeStartGlyph, fRightFadeStopGlyph, 1.0f, 0.0f );
				fPercent = clamp( fPercent, 0.0f, 1.0f );
				fAlpha *= fPercent * fRightAlpha;
			}

			for( int j = 0; j < 4; ++j )
				m_aVertices[i+j].c.a = (unsigned char)( m_aVertices[i+j].c.a * fAlpha );
		}
	}

	bool bDistanceField = m_pFont->IsDistanceField();
	if( bDistanceField )
		DISPLAY->SetEffectMode( EffectMode_DistanceField );

	for( int start = iStartGlyph; start < iEndGlyph; )
	{
		int end = start;
		while( end < iEndGlyph  &&  *m_vpFontPageTextures[end] == *m_vpFontPageTextures[start] )
			end++;

		bool bHaveATexture = !bUseStrokeTexture  ||  (bUseStrokeTexture && m_vpFontPageTextures[start]->m_pTextureStroke);
		if( bHaveATexture )
		{
			DISPLAY->ClearAllTextures();
			if( bUseStrokeTexture )
				DISPLAY->SetTexture( TextureUnit_1, m_vpFontPageTextures[start]->m_pTextureStroke->GetTexHandle() );
			else
				DISPLAY->SetTexture( TextureUnit_1, m_vpFontPageTextures[start]->m_pTextureMain->GetTexHandle() );

			// Don't bother setting texture render states for text. We never go outside of 0..1.
			/* We should call SetTextureRenderStates because it does more than just setting
			 * the texture wrapping state. If setting the wrapping state is found to be slow,
			 * there should probably be a "don't care" texture wrapping mode set in Actor. -Chris */

			// This is SLOW. We need to do something else about this. -Colby
			//Actor::SetTextureRenderStates();

			DISPLAY->DrawQuads( &m_aVertices[start*4], (end-start)*4);
		}

		start = end;
	}
	if( bDistanceField )
		DISPLAY->SetEffectMode( EffectMode_Normal );
}

/* sText is UTF-8. If not all of the characters in sText are available in the
 * font, sAlternateText will be used instead. If there are unavailable characters
 * in sAlternateText, too, just use sText. */
void BitmapText::SetText( const RString& _sText, const RString& _sAlternateText, int iWrapWidthPixels )
{
	ASSERT( m_pFont != nullptr );

	RString sNewText = StringWillUseAlternate(_sText,_sAlternateText) ? _sAlternateText : _sText;

	if( m_bUppercase )
		sNewText.MakeUpper();

	if( iWrapWidthPixels == -1 )	// wrap not specified
		iWrapWidthPixels = m_iWrapWidthPixels;

	if( m_sText == sNewText && iWrapWidthPixels==m_iWrapWidthPixels )
		return;

	m_sText = sNewText;
	m_iWrapWidthPixels = iWrapWidthPixels;
	ClearAttributes();
	SetTextInternal();
}

void BitmapText::SetTextInternal()
{
	// Break the string into lines.

	m_wTextLines.clear();

	if( m_iWrapWidthPixels == -1 )
	{
		split( RStringToWstring(m_sText), L"\n", m_wTextLines, false );
	}
	else
	{
		// Break sText into lines that don't exceed iWrapWidthPixels. (if only
		// one word fits on the line, it may be larger than iWrapWidthPixels).

		// This does not work in all languages:
		/* "...I can add Japanese wrapping, at least. We could handle hyphens
		 * and soft hyphens and pretty easily, too." -glenn */
		// TODO: Move this wrapping logic into Font.
		std::vector<RString> asLines;
		split( m_sText, "\n", asLines, false );

		for( unsigned line = 0; line < asLines.size(); ++line )
		{
			std::vector<RString> asWords;
			split( asLines[line], " ", asWords );

			RString sCurLine;
			int iCurLineWidth = 0;

			for( unsigned i=0; i<asWords.size(); i++ )
			{
				const RString &sWord = asWords[i];
				int iWidthWord = m_pFont->GetLineWidthInSourcePixels( RStringToWstring(sWord) );

				if( sCurLine.empty() )
				{
					sCurLine = sWord;
					iCurLineWidth = iWidthWord;
					continue;
				}

				RString sToAdd = " " + sWord;
				int iWidthToAdd = m_pFont->GetLineWidthInSourcePixels(L" ") + iWidthWord;
				if( iCurLineWidth + iWidthToAdd <= m_iWrapWidthPixels )	// will fit on current line
				{
					sCurLine += sToAdd;
					iCurLineWidth += iWidthToAdd;
				}
				else
				{
					m_wTextLines.push_back( RStringToWstring(sCurLine) );
					sCurLine = sWord;
					iCurLineWidth = iWidthWord;
				}
			}
			m_wTextLines.push_back( RStringToWstring(sCurLine) );
		}
	}

	BuildChars();
	UpdateBaseZoom();
}

void BitmapText::SetVertSpacing( int iSpacing )
{
	m_iVertSpacing = iSpacing;
	BuildChars();
}

void BitmapText::SetMaxWidth( float fMaxWidth )
{
	m_fMaxWidth = fMaxWidth;
	UpdateBaseZoom();
}

void BitmapText::SetMaxHeight( float fMaxHeight )
{
	m_fMaxHeight = fMaxHeight;
	UpdateBaseZoom();
}

void BitmapText::SetMaxDimUseZoom(bool use)
{
	m_MaxDimensionUsesZoom= use;
}

void BitmapText::SetUppercase( bool b )
{
	m_bUppercase = b;
	BuildChars();
}

void BitmapText::SetDistortion( float f )
{
	m_fDistortion= f;
	m_bUsingDistortion= true;
	BuildChars();
}

void BitmapText::UnSetDistortion()
{
	m_bUsingDistortion= false;
	BuildChars();
}

void BitmapText::set_mult_attrs_with_diffuse(bool m)
{
	m_mult_attrs_with_diffuse= m;
	BuildChars();
}

bool BitmapText::get_mult_attrs_with_diffuse()
{
	return m_mult_attrs_with_diffuse;
}

void BitmapText::UpdateBaseZoom()
{
	// don't divide by 0
	// Never apply a zoom greater than 1.
	// Factor in the non-base zoom so that maxwidth will be in terms of theme
	// pixels when zoom is used.
#define APPLY_DIMENSION_ZOOM(dimension_max, dimension_get, dimension_zoom_get, base_zoom_set) \
	if(dimension_max == 0) \
	{ \
		base_zoom_set(1); \
	} \
	else \
	{ \
		float dimension= dimension_get(); \
		if(m_MaxDimensionUsesZoom) \
		{ \
			dimension/= dimension_zoom_get(); \
		} \
		if(dimension != 0) \
		{ \
			const float zoom= std::fmin(1, dimension_max / dimension); \
			base_zoom_set(zoom); \
		} \
	}

	APPLY_DIMENSION_ZOOM(m_fMaxWidth, GetUnzoomedWidth, GetZoomX, SetBaseZoomX);
	APPLY_DIMENSION_ZOOM(m_fMaxHeight, GetUnzoomedHeight, GetZoomY, SetBaseZoomY);
#undef APPLY_DIMENSION_ZOOM
}

bool BitmapText::StringWillUseAlternate( const RString& sText, const RString& sAlternateText ) const
{
	ASSERT( m_pFont != nullptr );

	// Can't use the alternate if there isn't one.
	if( !sAlternateText.size() )
		return false;

	// False if the alternate isn't needed.
	if( m_pFont->FontCompleteForString(RStringToWstring(sText)) )
		return false;

	// False if the alternate is also incomplete.
	if( !m_pFont->FontCompleteForString(RStringToWstring(sAlternateText)) )
		return false;

	return true;
}

void BitmapText::CropLineToWidth(std::size_t l, int width)
{
	if(l < m_wTextLines.size())
	{
		int used_width= width;
		std::wstring& line= m_wTextLines[l];
		const std::size_t fit= m_pFont->GetGlyphsThatFit(line, &used_width);
		if(fit < line.size())
		{
			line.erase(line.begin()+fit, line.end());
		}
		m_iLineWidths[l]= used_width;
	}
}

void BitmapText::CropToWidth(int width)
{
	for(std::size_t l= 0; l < m_wTextLines.size(); ++l)
	{
		CropLineToWidth(l, width);
	}
	BuildChars();
}

bool BitmapText::EarlyAbortDraw() const
{
	return m_wTextLines.empty();
}

// draw text at x, y using colorTop blended down to colorBottom, with size multiplied by scale
void BitmapText::DrawPrimitives()
{
	Actor::SetGlobalRenderStates(); // set Actor-specified render states
	DISPLAY->SetTextureMode( TextureUnit_1, TextureMode_Modulate );

	// Draw if we're not fully transparent or the zbuffer is enabled
	if( m_pTempState->diffuse[0].a != 0 )
	{
		// render the shadow
		if( m_fShadowLengthX != 0  ||  m_fShadowLengthY != 0 )
		{
			DISPLAY->PushMatrix();
			DISPLAY->TranslateWorld( m_fShadowLengthX, m_fShadowLengthY, 0 );

			RageColor c = m_ShadowColor;
			c.a *= m_pTempState->diffuse[0].a;
			for( unsigned i=0; i<m_aVertices.size(); i++ )
				m_aVertices[i].c = c;
			DrawChars( false );

			DISPLAY->PopMatrix();
		}

		// render the stroke
		RageColor stroke_color= GetCurrStrokeColor();
		if( stroke_color.a > 0 )
		{
			stroke_color.a *= m_pTempState->diffuse[0].a;
			for( unsigned i=0; i<m_aVertices.size(); i++ )
				m_aVertices[i].c = stroke_color;
			DrawChars( true );
		}

		// render the diffuse pass
		if( m_bRainbowScroll )
		{
			int color_index = int(RageTimer::GetTimeSinceStartFast() / 0.200) % RAINBOW_COLORS.size();
			for( unsigned i=0; i<m_aVertices.size(); i+=4 )
			{
				const RageColor color = RAINBOW_COLORS[color_index];
				for( unsigned j=i; j<i+4; j++ )
					m_aVertices[j].c = color;

				color_index = (color_index+1) % RAINBOW_COLORS.size();
			}
		}
		else
		{
			std::size_t i = 0;
			std::map<std::size_t,Attribute>::const_iterator iter = m_mAttributes.begin();
			while( i < m_aVertices.size() )
			{
				// Set the colors up to the next attribute.
				std::size_t iEnd = iter == m_mAttributes.end()? m_aVertices.size():iter->first*4;
				iEnd = std::min( iEnd, m_aVertices.size() );
				for( ; i < iEnd; i += 4 )
				{
					m_aVertices[i+0].c = m_pTempState->diffuse[0];	// top left
					m_aVertices[i+1].c = m_pTempState->diffuse[2];	// bottom left
					m_aVertices[i+2].c = m_pTempState->diffuse[3];	// bottom right
					m_aVertices[i+3].c = m_pTempState->diffuse[1];	// top right
				}
				if( iter == m_mAttributes.end() )
					break;
				// Set the colors according to this attribute.
				const Attribute &attr = iter->second;
				++iter;
				if( attr.length < 0 )
					iEnd = iter == m_mAttributes.end()? m_aVertices.size():iter->first*4;
				else
					iEnd = i + attr.length*4;
				iEnd = std::min( iEnd, m_aVertices.size() );
				std::vector<RageColor> temp_attr_diffuse(NUM_DIFFUSE_COLORS, m_internalDiffuse);
				for(std::size_t c= 0; c < NUM_DIFFUSE_COLORS; ++c)
				{
					temp_attr_diffuse[c]*= attr.diffuse[c];
					if(m_mult_attrs_with_diffuse)
					{
						temp_attr_diffuse[c]*= m_pTempState->diffuse[c];
					}
				}
				for( ; i < iEnd; i += 4 )
				{
					m_aVertices[i+0].c = temp_attr_diffuse[0];	// top left
					m_aVertices[i+1].c = temp_attr_diffuse[2];	// bottom left
					m_aVertices[i+2].c = temp_attr_diffuse[3];	// bottom right
					m_aVertices[i+3].c = temp_attr_diffuse[1];	// top right
				}
			}
		}

		// apply jitter to verts
		std::vector<RageVector3> vGlyphJitter;
		if( m_bJitter )
		{
			int iSeed = std::lrint( RageTimer::GetTimeSinceStartFast()*8 );
			RandomGen rnd( iSeed );

			for( unsigned i=0; i<m_aVertices.size(); i+=4 )
			{
				RageVector3 jitter( rnd()%2, rnd()%3, 0 );
				vGlyphJitter.push_back( jitter );

				m_aVertices[i+0].p += jitter;	// top left
				m_aVertices[i+1].p += jitter;	// bottom left
				m_aVertices[i+2].p += jitter;	// bottom right
				m_aVertices[i+3].p += jitter;	// top right
			}
		}

		DrawChars( false );

		// undo jitter to verts
		if( m_bJitter )
		{
			ASSERT( vGlyphJitter.size() == m_aVertices.size()/4 );
			for( unsigned i=0; i<m_aVertices.size(); i+=4 )
			{
				const RageVector3 &jitter = vGlyphJitter[i/4];;

				m_aVertices[i+0].p -= jitter;	// top left
				m_aVertices[i+1].p -= jitter;	// bottom left
				m_aVertices[i+2].p -= jitter;	// bottom right
				m_aVertices[i+3].p -= jitter;	// top right
			}
		}
	}

	// render the glow pass
	if( m_pTempState->glow.a > 0.0001f || m_bHasGlowAttribute )
	{
		DISPLAY->SetTextureMode( TextureUnit_1, TextureMode_Glow );

		std::size_t i = 0;
		std::map<std::size_t,Attribute>::const_iterator iter = m_mAttributes.begin();
		while( i < m_aVertices.size() )
		{
			// Set the glow up to the next attribute.
			std::size_t iEnd = iter == m_mAttributes.end()? m_aVertices.size():iter->first*4;
			iEnd = std::min( iEnd, m_aVertices.size() );
			for( ; i < iEnd; ++i )
				m_aVertices[i].c = m_pTempState->glow;
			if( iter == m_mAttributes.end() )
				break;
			// Set the glow according to this attribute.
			const Attribute &attr = iter->second;
			++iter;
			if( attr.length < 0 )
				iEnd = iter == m_mAttributes.end()? m_aVertices.size():iter->first*4;
			else
				iEnd = i + attr.length*4;
			iEnd = std::min( iEnd, m_aVertices.size() );
			for( ; i < iEnd; ++i )
			{
				if( m_internalGlow.a > 0 )
				{
					m_aVertices[i].c = attr.glow * m_internalGlow;
				}
				else
				{
					m_aVertices[i].c = attr.glow;
				}
			}
		}
		/* Draw glow using the base texture and the glow texture. Otherwise,
		 * glow looks too tame on BitmapText that has a stroke. - Chris Danford */
		/* This doesn't work well if the font is using an invisible stroke, as
		 * the invisible stroke will glow as well. Time for TextGlowMode.
		 * Only draw the strokes if the glow mode is not inner only. -aj */
		DrawChars(m_TextGlowMode != TextGlowMode_Inner);
	}
}

// Rebuild when these change.
void BitmapText::SetHorizAlign( float f )
{
	float fHorizAlign = m_fHorizAlign;
	Actor::SetHorizAlign(f);
	if( fHorizAlign == m_fHorizAlign )
		return;
	BuildChars();
}

void BitmapText::SetWrapWidthPixels( int iWrapWidthPixels )
{
	ASSERT( m_pFont != nullptr ); // always load a font first
	if( m_iWrapWidthPixels == iWrapWidthPixels )
		return;
	m_iWrapWidthPixels = iWrapWidthPixels;
	SetTextInternal();
}

BitmapText::Attribute BitmapText::GetDefaultAttribute() const
{
	Attribute attr;
	for( int i = 0; i < 4; ++i )
		attr.diffuse[i] = GetDiffuses( i );
	attr.glow = GetGlow();
	return attr;
}

void BitmapText::AddAttribute( std::size_t iPos, const Attribute &attr )
{
	// Fixup position for new lines.
	int iLines = 0;
	std::size_t iAdjustedPos = iPos;

	for (std::wstring const & line : m_wTextLines)
	{
		std::size_t length = line.length();
		if( length >= iAdjustedPos )
			break;
		iAdjustedPos -= length;
		++iLines;
	}
	m_mAttributes[iPos-iLines] = attr;
	m_bHasGlowAttribute = m_bHasGlowAttribute || attr.glow.a > 0.0001f;
}

void BitmapText::ClearAttributes()
{
	m_mAttributes.clear();
	m_bHasGlowAttribute = false;
}

void BitmapText::Attribute::FromStack( lua_State *L, int iPos )
{
	if( lua_type(L, iPos) != LUA_TTABLE )
		return;

	lua_pushvalue( L, iPos );
	const int iTab = lua_gettop( L );

	// Get the length.
	lua_getfield( L, iTab, "Length" );
	length = lua_tointeger( L, -1 );
	lua_settop( L, iTab );

	// Get the diffuse colors.
	lua_getfield( L, iTab, "Diffuses" );
	if( !lua_isnil(L, -1) )
	{
		for( int i = 1; i <= NUM_DIFFUSE_COLORS; ++i )
		{
			lua_rawgeti( L, -i, i );
			diffuse[i-1].FromStack( L, -1 );
		}
	}
	lua_settop( L, iTab );

	// Get a single diffuse color.
	lua_getfield( L, iTab, "Diffuse" );
	if( !lua_isnil(L, -1) )
	{
		diffuse[0].FromStack( L, -1 );
		diffuse[1] = diffuse[2] = diffuse[3] = diffuse[0];
	}
	lua_settop( L, iTab );

	// Get the glow color.
	lua_getfield( L, iTab, "Glow" );
	glow.FromStack( L, -1 );

	lua_settop( L, iTab - 1 );
}

// lua start
#include "FontCharAliases.h"

/** @brief Allow Lua to have access to the BitmapText. */
class LunaBitmapText: public Luna<BitmapText>
{
public:
	static int wrapwidthpixels( T* p, lua_State *L )	{ p->SetWrapWidthPixels( IArg(1) ); COMMON_RETURN_SELF; }
#define MAX_DIMENSION(maxdimension, SetMaxDimension) \
	static int maxdimension( T* p, lua_State *L ) \
	{ p->SetMaxDimension(FArg(1)); COMMON_RETURN_SELF; }
	MAX_DIMENSION(maxwidth, SetMaxWidth);
	MAX_DIMENSION(maxheight, SetMaxHeight);
#undef MAX_DIMENSION
	static int max_dimension_use_zoom(T* p, lua_State* L)
	{
		p->SetMaxDimUseZoom(lua_toboolean(L, 1));
		COMMON_RETURN_SELF;
	}
	static int vertspacing( T* p, lua_State *L )		{ p->SetVertSpacing( IArg(1) ); COMMON_RETURN_SELF; }
	static int settext( T* p, lua_State *L )
	{
		RString s = SArg(1);
		RString sAlt;
		/* XXX: Lua strings should simply use "\n" natively. However, some
		 * settext calls may be made from GetMetric() calls to other strings, and
		 * it's confusing for :: to work in some strings and not others.
		 * Eventually, all strings should be Lua expressions, but until then,
		 * continue to support this. */
		s.Replace("::","\n");
		FontCharAliases::ReplaceMarkers( s );

		if( lua_gettop(L) > 1 )
		{
			sAlt = SArg(2);
			sAlt.Replace("::","\n");
			FontCharAliases::ReplaceMarkers( sAlt );
		}

		p->SetText( s, sAlt );
		COMMON_RETURN_SELF;
	}
	static int rainbowscroll( T* p, lua_State *L )		{ p->SetRainbowScroll( BArg(1) ); COMMON_RETURN_SELF; }
	static int jitter( T* p, lua_State *L )			{ p->SetJitter( BArg(1) ); COMMON_RETURN_SELF; }
	static int distort( T* p, lua_State *L) { p->SetDistortion( FArg(1) ); COMMON_RETURN_SELF; }
	static int undistort( T* p, lua_State *L) { p->UnSetDistortion(); COMMON_RETURN_SELF; }
	GETTER_SETTER_BOOL_METHOD(mult_attrs_with_diffuse);
	static int GetText( T* p, lua_State *L )		{ lua_pushstring( L, p->GetText() ); return 1; }
	static int AddAttribute( T* p, lua_State *L )
	{
		std::size_t iPos = IArg(1);
		BitmapText::Attribute attr = p->GetDefaultAttribute();

		attr.FromStack( L, 2 );
		p->AddAttribute( iPos, attr );
		COMMON_RETURN_SELF;
	}
	static int ClearAttributes( T* p, lua_State *L )	{ p->ClearAttributes(); COMMON_RETURN_SELF; }
	static int strokecolor( T* p, lua_State *L )		{ RageColor c; c.FromStackCompat( L, 1 ); p->SetStrokeColor( c ); COMMON_RETURN_SELF; }
	DEFINE_METHOD(getstrokecolor, GetStrokeColor());
	static int uppercase( T* p, lua_State *L )		{ p->SetUppercase( BArg(1) ); COMMON_RETURN_SELF; }
	static int textglowmode( T* p, lua_State *L )	{ p->SetTextGlowMode( Enum::Check<TextGlowMode>(L, 1) ); COMMON_RETURN_SELF; }

	LunaBitmapText()
	{
		ADD_METHOD( wrapwidthpixels );
		ADD_METHOD( maxwidth );
		ADD_METHOD( maxheight );
		ADD_METHOD( max_dimension_use_zoom );
		ADD_METHOD( vertspacing );
		ADD_METHOD( settext );
		ADD_METHOD( rainbowscroll );
		ADD_METHOD( jitter );
		ADD_METHOD( distort );
		ADD_METHOD( undistort );
		ADD_GET_SET_METHODS(mult_attrs_with_diffuse);
		ADD_METHOD( GetText );
		ADD_METHOD( AddAttribute );
		ADD_METHOD( ClearAttributes );
		ADD_METHOD( strokecolor );
		ADD_METHOD( getstrokecolor );
		ADD_METHOD( uppercase );
		ADD_METHOD( textglowmode );
		//ADD_METHOD( LoadFromFont );
		//ADD_METHOD( LoadFromTextureAndChars );
	}
};

LUA_REGISTER_DERIVED_CLASS( BitmapText, Actor )

// lua end

/*
 * (c) 2003-2007 Chris Danford, Charles Lohr, Steve Checkoway
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
