#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: Font

 Desc: See header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "Font.h"
#include "IniFile.h"

#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"
#include "FontManager.h"
#include "GameState.h"

const longchar Font::DEFAULT_GLYPH = 0xFFFF;

FontPage::FontPage()
{
	m_pTexture = NULL;
}

void FontPage::Load( const CString &TexturePath, const FontPageSettings &cfg )
{
	m_sTexturePath = TexturePath;

	// load texture
	m_sTexturePath.MakeLower();
	m_pTexture = TEXTUREMAN->LoadTexture( m_sTexturePath );
	ASSERT( m_pTexture != NULL );

	// load character widths
	vector<int> FrameWidths;
	int i;
	// Assume each character is the width of the frame by default.
	for( i=0; i<m_pTexture->GetNumFrames(); i++ )
	{
		map<int,int>::const_iterator it = cfg.GlyphWidths.find(i);
		if(it != cfg.GlyphWidths.end())
		{
			FrameWidths.push_back(it->second);
		} else {
			FrameWidths.push_back(m_pTexture->GetSourceFrameWidth());
		}
	}

	if( cfg.AddToAllWidths )
	{
		for( int i=0; i<256; i++ )
			FrameWidths[i] += cfg.AddToAllWidths;
	}

	if( cfg.ScaleAllWidthsBy != 1 )
	{
		for( int i=0; i<256; i++ )
			FrameWidths[i] = int(roundf( FrameWidths[i] * cfg.ScaleAllWidthsBy ));
	}

	m_iCharToGlyphNo = cfg.CharToGlyphNo;

	/* All characters on a page have the same vertical spacing. */
	int LineSpacing = cfg.LineSpacing;
	if(LineSpacing == -1)
		LineSpacing = m_pTexture->GetSourceFrameHeight();

	SetTextureCoords(FrameWidths, LineSpacing);
	SetExtraPixels(cfg.DrawExtraPixelsLeft, cfg.DrawExtraPixelsRight);
}

void FontPage::SetTextureCoords(const vector<int> &widths, int LineSpacing)
{
	for(int i = 0; i < m_pTexture->GetNumFrames(); ++i)
	{
		glyph g;

		/* Make a copy of each texture rect, reducing each to the actual dimensions
		 * of the character (most characters don't take a full block). */
		g.rect = *m_pTexture->GetTextureCoordRect(i);;

		/* Set the width and height to the width and line spacing, respectively. */
		g.width = float(widths[i]);
		g.height = float(m_pTexture->GetSourceFrameHeight());

		/* By default, advance one pixel more than the width.  (This could be
		 * an option.) */
		g.hadvance = int(g.width + 1);

		/* Shift the character up so the top of the rendered quad is at the top
		 * of the character. */
		g.vshift = -(m_pTexture->GetSourceFrameHeight() - LineSpacing)/2.0f;

		/* Do the same thing with X.  Do this by changing the actual rendered
		 * rect, instead of shifting it, so we don't render more than we need to. */
		g.hshift = 0;
		{
			int iPixelsToChopOff = m_pTexture->GetSourceFrameWidth() - widths[i];
			if((iPixelsToChopOff % 2) == 1)
			{
				/* We don't want to chop off an odd number of pixels, since that'll
				 * put our texture coordinates between texels and make things blurrier. */
				iPixelsToChopOff--;
				g.width++;
			}
			float fTexCoordsToChopOff = float(iPixelsToChopOff) / m_pTexture->GetTextureWidth();

			g.rect.left  += fTexCoordsToChopOff/2;
			g.rect.right -= fTexCoordsToChopOff/2;
		}

		g.vadvance = LineSpacing;
		g.Texture = m_pTexture;

		glyphs.push_back(g);
	}
}

void FontPage::SetExtraPixels(int DrawExtraPixelsLeft, int DrawExtraPixelsRight)
{
	/* Hack: do one more than we were asked to; I think a lot of fonts are one
	 * too low. */
	DrawExtraPixelsRight++;
	DrawExtraPixelsLeft++;

	if((DrawExtraPixelsLeft % 2) == 1)
		DrawExtraPixelsLeft++;

	/* Adjust for DrawExtraPixelsLeft and DrawExtraPixelsRight. */
	for(unsigned i = 0; i < glyphs.size(); ++i)
	{
		int iFrameWidth = m_pTexture->GetSourceFrameWidth();
		float iCharWidth = glyphs[i].width;

		/* Extra pixels to draw to the left and right.  We don't have to
		 * worry about alignment here; CharWidth is always even (by
		 * SetTextureCoords) and iFrameWidth are almost always even. */
		float ExtraLeft = min( float(DrawExtraPixelsLeft), (iFrameWidth-iCharWidth)/2.0f );
		float ExtraRight = min( float(DrawExtraPixelsRight), (iFrameWidth-iCharWidth)/2.0f );

		/* Move left and expand right. */
		glyphs[i].rect.left -= ExtraLeft / m_pTexture->GetTextureWidth();
		glyphs[i].rect.right += ExtraRight / m_pTexture->GetTextureWidth();
		glyphs[i].hshift -= ExtraLeft;
		glyphs[i].width += ExtraLeft + ExtraRight;
	}
}

FontPage::~FontPage()
{
	if( m_pTexture != NULL )
		TEXTUREMAN->UnloadTexture( m_pTexture );
}


int Font::GetLineWidthInSourcePixels( const lstring &szLine ) const
{
	int LineWidth = 0;
	
	for( unsigned i=0; i<szLine.size(); i++ )
		LineWidth += GetGlyph(szLine[i]).hadvance;

	return LineWidth;
}

int Font::GetLineHeightInSourcePixels( const lstring &szLine ) const
{
	int iLineSpacing = 0;

	/* The spacing of a line is the spacing of its tallest used font page. */
	for( unsigned i=0; i<szLine.size(); i++ )
		iLineSpacing = max(iLineSpacing, GetGlyph(szLine[i]).vadvance);

	return iLineSpacing;
}


Font::Font()
{
	//LOG->Trace( "Font::LoadFromFontName(%s)", sASCIITexturePath.GetString() );

	m_iRefCount = 1;
}

Font::~Font()
{
	for(unsigned i = 0; i < pages.size(); ++i)
		delete pages[i];

	/* Free any fonts that we merged into us. */
	for(unsigned i = 0; i < merged_fonts.size(); ++i)
		FONT->UnloadFont(merged_fonts[i]);
}

void Font::AddPage(FontPage *fp)
{
	pages.push_back(fp);

	for(map<longchar,int>::const_iterator it = fp->m_iCharToGlyphNo.begin();
		it != fp->m_iCharToGlyphNo.end(); ++it)
	{
		m_iCharToGlyph[it->first] = &fp->glyphs[it->second];
	}
}

void Font::MergeFont(Font *f)
{
	for(map<longchar,glyph*>::iterator it = f->m_iCharToGlyph.begin();
		it != f->m_iCharToGlyph.end(); ++it)
	{
		m_iCharToGlyph[it->first] = it->second;
	}

	/* We now have ownership of f.  Mark it to be freed. */
	merged_fonts.push_back(f);
}

const glyph &Font::GetGlyph( longchar c ) const
{
	/* See if there's a game-specific version of this character. */
	int gc = FontManager::MakeGameGlyph(c, GAMESTATE->m_CurGame);
	map<longchar,glyph*>::const_iterator it = m_iCharToGlyph.find(gc);

	/* If there isn't, try the regular character. */
	if(it == m_iCharToGlyph.end()) it = m_iCharToGlyph.find(c);

	/* If *that's* missing, use the default glyph. */
	if(it == m_iCharToGlyph.end()) it = m_iCharToGlyph.find(DEFAULT_GLYPH);

	if(it == m_iCharToGlyph.end()) 
		RageException::Throw( "The default glyph is missing from the font '%s'", path.GetString() );
	
	return *it->second;
}

RageTexture *Font::GetGlyphTexture( longchar c )
{
	glyph &g = const_cast<glyph &> (GetGlyph(c));
	return g.Texture;
}

void Font::CapsOnly()
{
	/* For each uppercase character that we have a mapping for, add
     * a lowercase one. */
	for(char c = 'A'; c <= 'Z'; ++c)
	{
		map<longchar,glyph*>::const_iterator it = m_iCharToGlyph.find(c);

		if(it == m_iCharToGlyph.end())
			continue;

		m_iCharToGlyph[(char) tolower(c)] = it->second;
	}
}
