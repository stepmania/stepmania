#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: BitmapText

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "BitmapText.h"
#include "IniFile.h"
#include "FontManager.h"
#include "RageLog.h"
#include "RageException.h"
#include "RageTimer.h"
#include "RageDisplay.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "GameConstantsAndTypes.h"
#include "Font.h"


#define RAINBOW_COLOR(n)	THEME->GetMetricC("BitmapText",ssprintf("RainbowColor%i", n+1))

const int NUM_RAINBOW_COLORS = 7;
RageColor RAINBOW_COLORS[NUM_RAINBOW_COLORS];


BitmapText::BitmapText()
{
	// Loading these theme metrics is slow, so only do it ever 20th time.
	static int iReloadCounter = 0;
	if( iReloadCounter%20==0 )
	{
		for(int i = 0; i < NUM_RAINBOW_COLORS; ++i)
			RAINBOW_COLORS[i] = RAINBOW_COLOR(i);
	}
	iReloadCounter++;

	m_HorizAlign = align_center;
	m_VertAlign = align_middle;

	m_pFont = NULL;

	m_iWidestLineWidth = 0;
	
	m_bShadow = true;

	m_bRainbow = false;
}

BitmapText::~BitmapText()
{
	if( m_pFont )
		FONT->UnloadFont( m_pFont->m_sTexturePath );
}

bool BitmapText::LoadFromFont( CString sFontFilePath )
{
	LOG->Trace( "BitmapText::LoadFromFontName(%s)", sFontFilePath.GetString() );

	if( m_pFont ) {
		FONT->UnloadFont( m_pFont->m_sTexturePath );
		m_pFont = NULL;
	}

	// load font
	m_pFont = FONT->LoadFont( sFontFilePath );

	return true;
}


bool BitmapText::LoadFromTextureAndChars( CString sTexturePath, CString sChars )
{
	LOG->Trace( "BitmapText::LoadFromTextureAndChars(%s)", sTexturePath.GetString() );

	if( m_pFont ) {
		FONT->UnloadFont( m_pFont->m_sTexturePath );
		m_pFont = NULL;
	}

	// load font
	m_pFont = FONT->LoadFont( sTexturePath, sChars );

	return true;
}




void BitmapText::SetText( CString sText )
{
	ASSERT( m_pFont );

	if( m_pFont->m_bCapitalsOnly )
		sText.MakeUpper();

	m_szText = sText;

	/* Break the string into lines. */
	m_szTextLines.clear();
	m_iLineWidths.clear();

	split(m_szText, "\n", m_szTextLines, false);
	
	/* calculate line lengths and widths */
	m_iWidestLineWidth = 0;

	for( unsigned l=0; l<m_szTextLines.size(); l++ )	// for each line
	{
		m_iLineWidths.push_back(m_pFont->GetLineWidthInSourcePixels( m_szTextLines[l] ));
		m_iWidestLineWidth = max(m_iWidestLineWidth, m_iLineWidths.back());
	}
}

void BitmapText::CropToWidth( int iMaxWidthInSourcePixels )
{
	iMaxWidthInSourcePixels = max( 0, iMaxWidthInSourcePixels );

	for( unsigned l=0; l<m_szTextLines.size(); l++ )	// for each line
	{
		while( m_iLineWidths[l] > iMaxWidthInSourcePixels )
		{
			m_szTextLines[l].erase(m_szTextLines[l].end()-1, m_szTextLines[l].end());
			m_iLineWidths[l] = m_pFont->GetLineWidthInSourcePixels( m_szTextLines[l] );
		}
	}
}

// draw text at x, y using colorTop blended down to colorBottom, with size multiplied by scale
void BitmapText::DrawPrimitives()
{
	if( m_szTextLines.empty() )
		return;

	RageTexture* pTexture = m_pFont->m_pTexture;

	static RageVertex *v = NULL;
	static int vcnt = 0;
	{
		int charcnt = 0;
		for( unsigned i=0; i<m_szTextLines.size(); i++ )
			charcnt += m_szTextLines[i].size();

		charcnt *= 4; /* 4 vertices per char */
		if(charcnt > vcnt)
		{
			vcnt = charcnt;
			delete [] v;
			v = new RageVertex[vcnt];
		}
	}

	int iNumV = 0;	// the current vertex number

	// make the object in logical units centered at the origin
	const int iHeight = pTexture->GetSourceFrameHeight();	// height of a character
	const int iLineSpacing = m_pFont->m_iLineSpacing;			// spacing between lines

	int iY;	//	 the center position of the first row of characters
	switch( m_VertAlign )
	{
	case align_bottom:	iY = -(int(m_szTextLines.size()))	 * iLineSpacing		+ iLineSpacing/2;	break;
	case align_middle:	iY = -(int(m_szTextLines.size())-1)	 * iLineSpacing/2;					break;
	case align_top:		iY =						 	     + iLineSpacing/2;	break;
	default:		ASSERT( false );	return;
	}

	for( unsigned i=0; i<m_szTextLines.size(); i++ )		// foreach line
	{
		const CString &szLine = m_szTextLines[i];
		const int iLineWidth = m_iLineWidths[i];
		
		int iX;
		switch( m_HorizAlign )
		{
		case align_left:	iX = 0;					break;
		case align_center:	iX = -(iLineWidth/2);	break;
		case align_right:	iX = -iLineWidth;		break;
		default:			ASSERT( false );		return;
		}

		for( unsigned j=0; j<szLine.size(); j++ )	// for each character in the line
		{
			const char c = szLine[j];
			if(m_pFont->m_iCharToFrameNo.find(c) == m_pFont->m_iCharToFrameNo.end())
				RageException::Throw( "The font '%s' does not implement the character '%c'", m_sFontFilePath.GetString(), c );

			const int iFrameNo = m_pFont->m_iCharToFrameNo[ (unsigned char)c ];

			const glyph &g = m_pFont->GetGlyph(iFrameNo);

			/* set vertex positions */
			v[iNumV++].p = RageVector3( (float)iX-g.left,	iY-iHeight/2.0f, 0 );	// top left
			v[iNumV++].p = RageVector3( (float)iX-g.left,	iY+iHeight/2.0f, 0 );	// bottom left
			v[iNumV++].p = RageVector3( (float)iX+g.right,	iY+iHeight/2.0f, 0 );	// bottom right
			v[iNumV++].p = RageVector3( (float)iX+g.right,	iY-iHeight/2.0f, 0 );	// top right

			/* Advance the cursor. */
			iX += g.width;

			/* set texture coordinates */
			iNumV -= 4;

			v[iNumV++].t = RageVector2( g.rect.left,	g.rect.top );		// top left
			v[iNumV++].t = RageVector2( g.rect.left,	g.rect.bottom );	// bottom left
			v[iNumV++].t = RageVector2( g.rect.right,	g.rect.bottom );	// bottom right
			v[iNumV++].t = RageVector2( g.rect.right,	g.rect.top );		// top right
		}

		iY += iLineSpacing;
	}


	DISPLAY->SetTexture( pTexture );
	DISPLAY->SetTextureModeModulate();
	if( m_bBlendAdd )
		DISPLAY->SetBlendModeAdd();
	else
		DISPLAY->SetBlendModeNormal();

	/* Draw if we're not fully transparent or the zbuffer is enabled (which ignores
	 * alpha). */
	if( m_temp.diffuse[0].a != 0 || DISPLAY->ZBufferEnabled())
	{
		//////////////////////
		// render the shadow
		//////////////////////
		if( m_bShadow )
		{
			DISPLAY->PushMatrix();
			DISPLAY->TranslateLocal( m_fShadowLength, m_fShadowLength, 0 );	// shift by 5 units

			RageColor dim(0,0,0,0.5f*m_temp.diffuse[0].a);	// semi-transparent black

			for( int i=0; i<iNumV; i++ )
				v[i].c = dim;
			DISPLAY->DrawQuads( v, iNumV );

			DISPLAY->PopMatrix();
		}

		//////////////////////
		// render the diffuse pass
		//////////////////////
		if( m_bRainbow )
		{
			int color_index = int(RageTimer::GetTimeSinceStart() / 0.200) % NUM_RAINBOW_COLORS;
			for( int i=0; i<iNumV; i+=4 )
			{
				const RageColor color = RAINBOW_COLORS[color_index];
				for( int j=i; j<i+4; j++ )
					v[j].c = color;

				color_index = (color_index+1)%NUM_RAINBOW_COLORS;
			}
		}
		else
		{
			for( int i=0; i<iNumV; i+=4 )
			{
				v[i+0].c = m_temp.diffuse[0];	// top left
				v[i+1].c = m_temp.diffuse[2];	// bottom left
				v[i+2].c = m_temp.diffuse[3];	// bottom right
				v[i+3].c = m_temp.diffuse[1];	// top right
			}
		}

		DISPLAY->DrawQuads( v, iNumV );
	}

	/* render the glow pass */
	if( m_temp.glow.a != 0 )
	{
		DISPLAY->SetTextureModeGlow();

		int i;
		for( i=0; i<iNumV; i++ )
			v[i].c = m_temp.glow;
		DISPLAY->DrawQuads( v, iNumV );
	}
}
