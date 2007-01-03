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
#include "Foreach.h"

REGISTER_ACTOR_CLASS( BitmapText )

/*
 * XXX: Changing a whole array of diffuse colors every frame (several times) is a waste,
 * when we're usually setting them all to the same value.  Rainbow and fading are annoying
 * to optimize, but rarely used.  Iterating over every character in Draw() is dumb.
 */

/* XXX:
 * We need some kind of font modifier string for metrics.  For example,
 * "valign=top;spacing = x+5,y+2"
 *
 * Better, we could go all the way, drop all of the actor-specific font aliases,
 * and do "font=header2;valign=top;...".
 */
#define RAINBOW_COLOR(n)	THEME->GetMetricC("BitmapText",ssprintf("RainbowColor%i", n+1))

static const int NUM_RAINBOW_COLORS = 7;
static RageColor RAINBOW_COLORS[NUM_RAINBOW_COLORS];

BitmapText::BitmapText()
{
	// Loading these theme metrics is slow, so only do it ever 20th time.
	static int iReloadCounter = 0;
	if( iReloadCounter%20==0 )
	{
		for( int i = 0; i < NUM_RAINBOW_COLORS; ++i )
			RAINBOW_COLORS[i] = RAINBOW_COLOR(i);
	}
	iReloadCounter++;

	m_pFont = NULL;

	m_bRainbow = false;
	m_bJitter = false;

	m_iWrapWidthPixels = -1;
	m_fMaxWidth = 0;
	m_fMaxHeight = 0;
	m_iVertSpacing = 0;

	SetShadowLength( 4 );
}

BitmapText::~BitmapText()
{
	if( m_pFont )
		FONT->UnloadFont( m_pFont );
}

BitmapText::BitmapText( const BitmapText &cpy ):
	Actor( cpy )
{
	m_pFont = NULL;
	*this = cpy;
}

BitmapText &BitmapText::operator =( const BitmapText &cpy )
{
#define CPY(a) a = cpy.a
	CPY( m_sText );
	CPY( m_wTextLines );
	CPY( m_iLineWidths );
	CPY( m_iWrapWidthPixels );
	CPY( m_fMaxWidth );
	CPY( m_fMaxHeight );
	CPY( m_bRainbow );
	CPY( m_bJitter );
	CPY( m_iVertSpacing );
	CPY( m_aVertices );
	CPY( m_pTextures );
#undef CPY

	if( m_pFont )
		FONT->UnloadFont( m_pFont );
	
	if( cpy.m_pFont != NULL )
		m_pFont = FONT->CopyFont( cpy.m_pFont );
	else
		m_pFont = NULL;

	return *this;
}

void BitmapText::LoadFromNode( const XNode* pNode )
{
	RString sText;
	pNode->GetAttrValue( "Text", sText );
	RString sAltText;
	pNode->GetAttrValue( "AltText", sAltText );

	ThemeManager::EvaluateString( sText );
	ThemeManager::EvaluateString( sAltText );

	RString sFont;
	pNode->GetAttrValue( "Font", sFont );
	if( sFont.empty() )
		pNode->GetAttrValue( "File", sFont );	// accept "File" for backward compatibility

	if( sFont == "" )
		RageException::Throw( "%s: BitmapText: missing the Font attribute",
				      ActorUtil::GetWhere(pNode).c_str() );

	LoadFromFont( THEME->GetPathF( "", sFont ) );
	SetText( sText, sAltText );

	Actor::LoadFromNode( pNode );
}


bool BitmapText::LoadFromFont( const RString& sFontFilePath )
{
	CHECKPOINT_M( ssprintf("BitmapText::LoadFromFont(%s)", sFontFilePath.c_str()) );

	if( m_pFont )
	{
		FONT->UnloadFont( m_pFont );
		m_pFont = NULL;
	}

	m_pFont = FONT->LoadFont( sFontFilePath );

	BuildChars();

	return true;
}


bool BitmapText::LoadFromTextureAndChars( const RString& sTexturePath, const RString& sChars )
{
	CHECKPOINT_M( ssprintf("BitmapText::LoadFromTextureAndChars(\"%s\",\"%s\")", sTexturePath.c_str(), sChars.c_str()) );

	if( m_pFont )
	{
		FONT->UnloadFont( m_pFont );
		m_pFont = NULL;
	}

	m_pFont = FONT->LoadFont( sTexturePath, sChars );

	BuildChars();

	return true;
}

void BitmapText::BuildChars()
{
	/* If we don't have a font yet, we'll do this when it loads. */
	if( m_pFont == NULL )
		return;

	/* calculate line lengths and widths */
	m_size.x = 0;

	m_iLineWidths.clear();
	for( unsigned l=0; l<m_wTextLines.size(); l++ )	// for each line
	{
		m_iLineWidths.push_back(m_pFont->GetLineWidthInSourcePixels( m_wTextLines[l] ));
		m_size.x = max( m_size.x, m_iLineWidths.back() );
	}

	m_aVertices.clear();
	m_pTextures.clear();
	
	if( m_wTextLines.empty() )
		return;

	m_size.y = float(m_pFont->GetHeight() * m_wTextLines.size());

	/* The height (from the origin to the baseline): */
	int iPadding = m_pFont->GetLineSpacing() - m_pFont->GetHeight();
	iPadding += m_iVertSpacing;

	/* There's padding between every line: */
	m_size.y += iPadding * int(m_wTextLines.size()-1);

	int iY;	//	 the top position of the first row of characters
	switch( m_VertAlign )
	{
	case VertAlign_Top:	iY = 0;					break;
	case VertAlign_Middle:	iY = -lrintf(m_size.y/2.0f);		break;
	case VertAlign_Bottom:	iY = -(int)m_size.y;			break;
	default:		ASSERT( false );
	}

	for( unsigned i=0; i<m_wTextLines.size(); i++ )		// foreach line
	{
		iY += m_pFont->GetHeight();

		wstring sLine = m_wTextLines[i];
		if( m_pFont->IsRightToLeft() )
			reverse( sLine.begin(), sLine.end() );
		const int iLineWidth = m_iLineWidths[i];
		
		int iX;
		switch( m_HorizAlign )
		{
		case HorizAlign_Left:	iX = 0;					break;
		case HorizAlign_Center:	iX = -lrintf(iLineWidth/2.0f);	break;
		case HorizAlign_Right:	iX = -iLineWidth;			break;
		default:		ASSERT( false );
		}

		for( unsigned i = 0; i < sLine.size(); ++i )
		{
			RageSpriteVertex v[4];
			const glyph &g = m_pFont->GetGlyph( sLine[i] );

			if( m_pFont->IsRightToLeft() )
				iX -= g.m_iHadvance;

			/* set vertex positions */
			v[0].p = RageVector3( iX+g.m_fHshift,			iY+g.m_pPage->m_fVshift,		0 );	// top left
			v[1].p = RageVector3( iX+g.m_fHshift,			iY+g.m_pPage->m_fVshift+g.m_fHeight,	0 );	// bottom left
			v[2].p = RageVector3( iX+g.m_fHshift+g.m_fWidth,	iY+g.m_pPage->m_fVshift+g.m_fHeight,	0 );	// bottom right
			v[3].p = RageVector3( iX+g.m_fHshift+g.m_fWidth,	iY+g.m_pPage->m_fVshift,		0 );	// top right

			/* Advance the cursor. */
			iX += g.m_iHadvance;

			/* set texture coordinates */
			v[0].t = RageVector2( g.m_TexRect.left,		g.m_TexRect.top );
			v[1].t = RageVector2( g.m_TexRect.left,		g.m_TexRect.bottom );
			v[2].t = RageVector2( g.m_TexRect.right,	g.m_TexRect.bottom );
			v[3].t = RageVector2( g.m_TexRect.right,	g.m_TexRect.top );

			m_aVertices.insert( m_aVertices.end(), &v[0], &v[4] );
			m_pTextures.push_back( g.GetTexture() );
		}

		/* The amount of padding a line needs: */
		iY += iPadding;
	}
}

void BitmapText::DrawChars()
{
	// bail if cropped all the way 
	if( m_pTempState->crop.left + m_pTempState->crop.right >= 1  || 
		m_pTempState->crop.top + m_pTempState->crop.bottom >= 1 ) 
		return; 

	const int iNumGlyphs = m_pTextures.size();
	int iStartGlyph = lrintf( SCALE( m_pTempState->crop.left, 0.f, 1.f, 0, (float) iNumGlyphs ) );
	int iEndGlyph = lrintf( SCALE( m_pTempState->crop.right, 0.f, 1.f, (float) iNumGlyphs, 0 ) );
	iStartGlyph = clamp( iStartGlyph, 0, iNumGlyphs );
	iEndGlyph = clamp( iEndGlyph, 0, iNumGlyphs );

	if( m_pTempState->fade.top > 0 ||
		m_pTempState->fade.bottom > 0 ||
		m_pTempState->fade.left > 0 ||
		m_pTempState->fade.right > 0 )
	{
		/* Handle fading by tweaking the alpha values of the vertices. */

		/* Actual size of the fade on each side: */
		const RectF &FadeDist = m_pTempState->fade;
		RectF FadeSize = FadeDist;

		/* If the cropped size is less than the fade distance, clamp. */
		const float fHorizRemaining = 1.0f - (m_pTempState->crop.left + m_pTempState->crop.right);
		if( FadeDist.left+FadeDist.right > 0 &&
			fHorizRemaining < FadeDist.left+FadeDist.right )
		{
			const float LeftPercent = FadeDist.left/(FadeDist.left+FadeDist.right);
			FadeSize.left = LeftPercent * fHorizRemaining;
			FadeSize.right = (1.0f-LeftPercent) * fHorizRemaining;
		}

		/* We fade from 0 to LeftColor, then from RightColor to 0.  (We won't fade all the way to
		 * 0 if the crop is beyond the outer edge.) */
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
				/* Add .5, so we fade wrt. the center of the vert, not the left side. */
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

	for( int start = iStartGlyph; start < iEndGlyph; )
	{
		int end = start;
		while( end < iEndGlyph && m_pTextures[end] == m_pTextures[start] )
			end++;
		DISPLAY->ClearAllTextures();
		DISPLAY->SetTexture( TextureUnit_1, m_pTextures[start]->GetTexHandle() );
		// don't bother setting texture render states for text.  We never go outside of 0..1.
		//Actor::SetTextureRenderStates();
		RageSpriteVertex &start_vertex = m_aVertices[start*4];
		int iNumVertsToDraw = (end-start)*4;
		DISPLAY->DrawQuads( &start_vertex, iNumVertsToDraw );
		
		start = end;
	}
}

/* sText is UTF-8.  If not all of the characters in sText are available in the
 * font, sAlternateText will be used instead.  If there are unavailable characters
 * in sAlternateText, too, just use sText. */
void BitmapText::SetText( const RString& _sText, const RString& _sAlternateText, int iWrapWidthPixels )
{
	ASSERT( m_pFont );

	RString sNewText = StringWillUseAlternate(_sText,_sAlternateText) ? _sAlternateText : _sText;

	if( iWrapWidthPixels == -1 )	// wrap not specified
		iWrapWidthPixels = m_iWrapWidthPixels;

	if( m_sText == sNewText && iWrapWidthPixels==m_iWrapWidthPixels )
		return;
	m_sText = sNewText;
	m_iWrapWidthPixels = iWrapWidthPixels;

	// Break the string into lines.
	//

	m_wTextLines.clear();

	if( iWrapWidthPixels == -1 )
	{
		split( RStringToWstring(m_sText), L"\n", m_wTextLines, false );
	}
	else
	{
		//
		// Break sText into lines that don't exceed iWrapWidthPixels
		// (if only one word fits on the line, it may be larger than iWrapWidthPixels).
		//
		// TODO: Investigate whether this works in all languages
		/* It doesn't.  I can add Japanese wrapping, at least.  We could handle hyphens
		 * and soft hyphens and pretty easily, too. -glenn */
		// TODO: Move this wrapping logic into Font
		vector<RString> asLines;
		split( m_sText, "\n", asLines, false );

		for( unsigned line = 0; line < asLines.size(); ++line )
		{
			vector<RString> asWords;
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
				if( iCurLineWidth + iWidthToAdd <= iWrapWidthPixels )	// will fit on current line
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

void BitmapText::UpdateBaseZoom()
{
	if( m_fMaxWidth == 0 )
	{
		this->SetBaseZoomX( 1 );
	}
	else
	{
		const float fWidth = GetUnzoomedWidth();
		if( fWidth != 0 )	// don't divide by 0
		{
			/* Never decrease the zoom. */
			const float fZoom = min( 1, m_fMaxWidth/fWidth );
			this->SetBaseZoomX( fZoom );
		}
	}

	if( m_fMaxHeight == 0 )
	{
		this->SetBaseZoomY( 1 );
	}
	else
	{
		const float fHeight = GetUnzoomedHeight();
		if( fHeight != 0 )	// don't divide by 0
		{
			/* Never decrease the zoom. */
			const float fZoom = min( 1, m_fMaxHeight/fHeight );
			this->SetBaseZoomY( fZoom );
		}
	}
}

bool BitmapText::StringWillUseAlternate( const RString& sText, const RString& sAlternateText ) const
{
	ASSERT( m_pFont );

	/* Can't use the alternate if there isn't one. */
	if( !sAlternateText.size() )
		return false;

	/* False if the alternate isn't needed. */
	if( m_pFont->FontCompleteForString(RStringToWstring(sText)) )
		return false;

	/* False if the alternate is also incomplete. */
	if( !m_pFont->FontCompleteForString(RStringToWstring(sAlternateText)) )
		return false;

	return true;
}

void BitmapText::CropToWidth( int iMaxWidthInSourcePixels )
{
	iMaxWidthInSourcePixels = max( 0, iMaxWidthInSourcePixels );

	for( unsigned l=0; l<m_wTextLines.size(); l++ )	// for each line
	{
		while( m_iLineWidths[l] > iMaxWidthInSourcePixels )
		{
			m_wTextLines[l].erase( m_wTextLines[l].end()-1, m_wTextLines[l].end() );
			m_iLineWidths[l] = m_pFont->GetLineWidthInSourcePixels( m_wTextLines[l] );
		}
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
	Actor::SetGlobalRenderStates();	// set Actor-specified render states
	DISPLAY->SetTextureModeModulate();

	/* Draw if we're not fully transparent or the zbuffer is enabled */
	if( m_pTempState->diffuse[0].a != 0 )
	{
		//
		// render the shadow
		//
		if( m_fShadowLength != 0 )
		{
			DISPLAY->PushMatrix();
			DISPLAY->TranslateWorld( m_fShadowLength, m_fShadowLength, 0 );	// shift by 5 units

			RageColor dim(0,0,0,0.5f*m_pTempState->diffuse[0].a);	// semi-transparent black

			for( unsigned i=0; i<m_aVertices.size(); i++ )
				m_aVertices[i].c = dim;
			DrawChars();

			DISPLAY->PopMatrix();
		}

		//
		// render the diffuse pass
		//
		if( m_bRainbow )
		{
			int color_index = int(RageTimer::GetTimeSinceStartFast() / 0.200) % NUM_RAINBOW_COLORS;
			for( unsigned i=0; i<m_aVertices.size(); i+=4 )
			{
				const RageColor color = RAINBOW_COLORS[color_index];
				for( unsigned j=i; j<i+4; j++ )
					m_aVertices[j].c = color;

				color_index = (color_index+1)%NUM_RAINBOW_COLORS;
			}
		}
		else
		{
			for( unsigned i=0; i<m_aVertices.size(); i+=4 )
			{
				m_aVertices[i+0].c = m_pTempState->diffuse[0];	// top left
				m_aVertices[i+1].c = m_pTempState->diffuse[2];	// bottom left
				m_aVertices[i+2].c = m_pTempState->diffuse[3];	// bottom right
				m_aVertices[i+3].c = m_pTempState->diffuse[1];	// top right
			}
		}

		// apply jitter to verts
		vector<RageVector3> vGlyphJitter;
		if( m_bJitter )
		{
			int iSeed = lrintf( RageTimer::GetTimeSinceStartFast()*8 );
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

		DrawChars();

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

	/* render the glow pass */
	if( m_pTempState->glow.a > 0.0001f )
	{
		DISPLAY->SetTextureModeGlow();

		for( unsigned i=0; i<m_aVertices.size(); i++ )
			m_aVertices[i].c = m_pTempState->glow;
		DrawChars();
	}
}

/* Rebuild when these change. */
void BitmapText::SetHorizAlign( HorizAlign ha )
{
	if( ha == m_HorizAlign )
		return;
	Actor::SetHorizAlign(ha);
	BuildChars();
}

void BitmapText::SetVertAlign( VertAlign va )
{
	if( va == m_VertAlign )
		return;
	Actor::SetVertAlign(va);
	BuildChars();
}

void BitmapText::SetWrapWidthPixels( int iWrapWidthPixels )
{
	ASSERT( m_pFont ); // always load a font first
	if( m_iWrapWidthPixels == iWrapWidthPixels )
		return;
	SetText( m_sText, "", iWrapWidthPixels );
}

ColorBitmapText::ColorBitmapText( ) : BitmapText()
{
	m_vColors.clear();
}

void ColorBitmapText::SetText( const RString& _sText, const RString& _sAlternateText, int iWrapWidthPixels )
{
	ASSERT( m_pFont );

	RString sNewText = StringWillUseAlternate(_sText,_sAlternateText) ? _sAlternateText : _sText;

	if( iWrapWidthPixels == -1 )	// wrap not specified
		iWrapWidthPixels = m_iWrapWidthPixels;

	if( m_sText == sNewText && iWrapWidthPixels==m_iWrapWidthPixels )
		return;
	m_sText = sNewText;
	m_iWrapWidthPixels = iWrapWidthPixels;

	// Set up the first color.
	m_vColors.clear();
	ColorChange change;
	change.c = RageColor ( 1, 1, 1, 1 );
	change.l = 0;
	m_vColors.push_back( change );

	m_wTextLines.clear();

	RString sCurrentLine = "";
	int		iLineWidth = 0;

	RString sCurrentWord = "";
	int		iWordWidth = 0;
	int		iGlyphsSoFar = 0;

	for( unsigned i = 0; i < m_sText.length(); i++ )
	{
		int iCharsLeft = m_sText.length() - i - 1;

		// First: Check for the special (color) case.

		RString FirstThree = m_sText.substr( i, 3 );
		if( FirstThree.CompareNoCase("|c0") == 0 && iCharsLeft > 8 )
		{
			ColorChange change;
			int k;
			sscanf( m_sText.substr( i+3, 2 ).c_str(), "%x", &k ); change.c.r = float( k ) / 255.0f;
			sscanf( m_sText.substr( i+5, 2 ).c_str(), "%x", &k ); change.c.g = float( k ) / 255.0f;
			sscanf( m_sText.substr( i+7, 2 ).c_str(), "%x", &k ); change.c.b = float( k ) / 255.0f;
			change.c.a = 1;
			change.l = iGlyphsSoFar;
			if( iGlyphsSoFar == 0 )
				m_vColors[0] = change;
			else
				m_vColors.push_back( change );
			i+=8;
			continue;
		}

		RString curCStr = m_sText.substr( i, 1 );
		char curChar = curCStr.c_str()[0];
		int iCharLen = m_pFont->GetLineWidthInSourcePixels( RStringToWstring( curCStr ) );

		switch( curChar )
		{
		case ' ':
			if( /* iLineWidth == 0 &&*/ iWordWidth == 0 )
				break;
			sCurrentLine += sCurrentWord + " ";
			iLineWidth += iWordWidth + iCharLen;
			sCurrentWord = "";
			iWordWidth = 0;
			iGlyphsSoFar++;
			break;
		case '\n':
			if( iLineWidth + iWordWidth > iWrapWidthPixels )
			{
				SimpleAddLine( sCurrentLine, iLineWidth );
				if( iWordWidth > 0 )
					iLineWidth = iWordWidth +	//Add the width of a space
						m_pFont->GetLineWidthInSourcePixels( RStringToWstring( " " ) );
				sCurrentLine = sCurrentWord + " ";
				iWordWidth = 0;
				sCurrentWord = "";
				iGlyphsSoFar++;
			} 
			else
			{
				SimpleAddLine( sCurrentLine + sCurrentWord, iLineWidth + iWordWidth );
				sCurrentLine = "";	iLineWidth = 0;
				sCurrentWord = "";	iWordWidth = 0;
			}
			break;
		default:
			if( iWordWidth + iCharLen > iWrapWidthPixels && iLineWidth == 0 )
			{
				SimpleAddLine( sCurrentWord, iWordWidth );
				sCurrentWord = curChar;	iWordWidth = iCharLen;
			}
			else if( iWordWidth + iLineWidth + iCharLen > iWrapWidthPixels )
			{
				SimpleAddLine( sCurrentLine, iLineWidth );
				sCurrentLine = ""; 
				iLineWidth = 0;
				sCurrentWord += curChar;
				iWordWidth += iCharLen;
			}
			else
			{
				sCurrentWord += curChar;
				iWordWidth += iCharLen;
			}
			iGlyphsSoFar++;
			break;
		}
	}
	
	if( iWordWidth > 0 )
	{
		sCurrentLine += sCurrentWord;
		iLineWidth += iWordWidth;
	}

	if( iLineWidth > 0 )
		SimpleAddLine( sCurrentLine, iLineWidth );

	BuildChars();
	UpdateBaseZoom();
}

void ColorBitmapText::SimpleAddLine( const RString &sAddition, const int iWidthPixels) 
{
	m_wTextLines.push_back( RStringToWstring( sAddition ) );
	m_iLineWidths.push_back( iWidthPixels );
}

void ColorBitmapText::DrawPrimitives( )
{
	Actor::SetGlobalRenderStates();	// set Actor-specified render states
	DISPLAY->SetTextureModeModulate();

	/* Draw if we're not fully transparent or the zbuffer is enabled */
	if( m_pTempState->diffuse[0].a != 0 )
	{
		//
		// render the shadow
		//
		if( m_fShadowLength != 0 )
		{
			DISPLAY->PushMatrix();
			DISPLAY->TranslateWorld( m_fShadowLength, m_fShadowLength, 0 );	// shift by 5 units

			RageColor dim(0,0,0,0.5f*m_pTempState->diffuse[0].a);	// semi-transparent black

			for( unsigned i=0; i<m_aVertices.size(); i++ )
				m_aVertices[i].c = dim;
			DrawChars();

			DISPLAY->PopMatrix();
		}

		//
		// render the diffuse pass
		//
		int loc = 0, cur = 0;
		RageColor c = m_pTempState->diffuse[0];

		for( unsigned i=0; i<m_aVertices.size(); i+=4 )
		{
			loc++;
			if( cur < (int)m_vColors.size() )
			{
				if ( loc > m_vColors[cur].l )
				{
					c = m_vColors[cur].c;
					cur++;
				}
			}
			for( unsigned j=0; j<4; j++ )
				m_aVertices[i+j].c = c;
		}

		DrawChars();
	}

	/* render the glow pass */
	if( m_pTempState->glow.a > 0.0001f )
	{
		DISPLAY->SetTextureModeGlow();

		for( unsigned i=0; i<m_aVertices.size(); i++ )
			m_aVertices[i].c = m_pTempState->glow;
		DrawChars();
	}
}

void ColorBitmapText::SetMaxLines( int iNumLines, int iDirection )
{
	iNumLines = max( 0, iNumLines );
	iNumLines = min( (int)m_wTextLines.size(), iNumLines );
	if( iDirection == 0 ) 
	{
		// Crop all bottom lines
		m_wTextLines.resize( iNumLines );
		m_iLineWidths.resize( iNumLines );
	}
	else
	{
		// Because colors are relative to the beginning, we have to crop them back
		unsigned shift = 0;

		for( unsigned i = 0; i < m_wTextLines.size() - iNumLines; i++ )
			shift += m_wTextLines[i].length();


		// When we're cutting out text, we need to maintain the last
		// color, so our text at the top doesn't become colorless.
		RageColor LastColor;

		for( unsigned i = 0; i < m_vColors.size(); i++ )
		{
			m_vColors[i].l -= shift;
			if( m_vColors[i].l < 0 )
			{
				LastColor = m_vColors[i].c;
				m_vColors.erase( m_vColors.begin() + i );
				i--;
			}
		}

		// If we already have a color set for the first char
		// do not override it.
		if( m_vColors.size() > 0 && m_vColors[0].l > 0 )
		{
			ColorChange tmp;
			tmp.c = LastColor;
			tmp.l = 0;
			m_vColors.insert( m_vColors.begin(), tmp );
		}

		m_wTextLines.erase( m_wTextLines.begin(), m_wTextLines.end() - iNumLines );
		m_iLineWidths.erase( m_iLineWidths.begin(), m_iLineWidths.end() - iNumLines );
	}
	BuildChars();
}

// lua start
#include "FontCharAliases.h"
class LunaBitmapText: public Luna<BitmapText>
{
public:
	static int wrapwidthpixels( T* p, lua_State *L )	{ p->SetWrapWidthPixels( IArg(1) ); return 0; }
	static int maxwidth( T* p, lua_State *L )		{ p->SetMaxWidth( FArg(1) ); return 0; }
	static int maxheight( T* p, lua_State *L )		{ p->SetMaxHeight( FArg(1) ); return 0; }
	static int vertspacing( T* p, lua_State *L )		{ p->SetVertSpacing( IArg(1) ); return 0; }
	static int settext( T* p, lua_State *L )
	{
		RString s = SArg(1);
		// XXX: Lua strings should simply use "\n" natively.  However, some
		// settext calls may be made from GetMetric() calls to other strings,
		// and it's confusing for :: to work in some strings and not others.
		// Eventually, all strings should be Lua expressions, but until then
		// continue to support this.
		s.Replace("::","\n");
		FontCharAliases::ReplaceMarkers( s );

		p->SetText( s ); return 0;
	}
	static int jitter( T* p, lua_State *L )			{ p->SetJitter( BArg(1) ); return 0; }
	static int GetText( T* p, lua_State *L )		{ lua_pushstring( L, p->GetText() ); return 1; }

	LunaBitmapText()
	{
		ADD_METHOD( wrapwidthpixels );
		ADD_METHOD( maxwidth );
		ADD_METHOD( maxheight );
		ADD_METHOD( vertspacing );
		ADD_METHOD( settext );
		ADD_METHOD( jitter );
		ADD_METHOD( GetText );
	}
};

LUA_REGISTER_DERIVED_CLASS( BitmapText, Actor )

// lua end

/*
 * (c) 2003-2004 Chris Danford, Charles Lohr
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
