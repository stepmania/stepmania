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
#include "PrefsManager.h"


#define RAINBOW_COLOR_1		THEME->GetMetricC("BitmapText","RainbowColor1")
#define RAINBOW_COLOR_2		THEME->GetMetricC("BitmapText","RainbowColor2")
#define RAINBOW_COLOR_3		THEME->GetMetricC("BitmapText","RainbowColor3")
#define RAINBOW_COLOR_4		THEME->GetMetricC("BitmapText","RainbowColor4")
#define RAINBOW_COLOR_5		THEME->GetMetricC("BitmapText","RainbowColor5")
#define RAINBOW_COLOR_6		THEME->GetMetricC("BitmapText","RainbowColor6")
#define RAINBOW_COLOR_7		THEME->GetMetricC("BitmapText","RainbowColor7")

const int NUM_RAINBOW_COLORS = 7;
D3DXCOLOR RAINBOW_COLORS[NUM_RAINBOW_COLORS];


BitmapText::BitmapText()
{
	// Loading these theme metrics is slow, so only do it ever 20th time.
	static int iReloadCounter = 0;
	if( iReloadCounter%20==0 )
	{
		RAINBOW_COLORS[0] = RAINBOW_COLOR_1;
		RAINBOW_COLORS[1] = RAINBOW_COLOR_2;
		RAINBOW_COLORS[2] = RAINBOW_COLOR_3;
		RAINBOW_COLORS[3] = RAINBOW_COLOR_4;
		RAINBOW_COLORS[4] = RAINBOW_COLOR_5;
		RAINBOW_COLORS[5] = RAINBOW_COLOR_6;
		RAINBOW_COLORS[6] = RAINBOW_COLOR_7;
	}
	iReloadCounter++;

	m_HorizAlign = align_center;
	m_VertAlign = align_middle;

	m_pFont = NULL;

	m_iNumLines = 0;
	m_iWidestLineWidth = 0;
	
	for( int i=0; i<MAX_TEXT_LINES; i++ )
	{
		m_szTextLines[i] = '\0';
		m_iLineLengths[i] = -1;
		m_iLineWidths[i] = 1;
	}

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
	LOG->Trace( "BitmapText::LoadFromFontName(%s)", sFontFilePath );

	// load font
	m_pFont = FONT->LoadFont( sFontFilePath );

	return true;
}


bool BitmapText::LoadFromTextureAndChars( CString sTexturePath, CString sChars )
{
	LOG->Trace( "BitmapText::LoadFromTextureAndChars(%s)", sTexturePath );

	// load font
	m_pFont = FONT->LoadFont( sTexturePath, sChars );

	return true;
}




void BitmapText::SetText( CString sText )
{
	//LOG->Trace( "BitmapText::SetText()" );

	ASSERT( m_pFont );

	if( m_pFont->m_bCapitalsOnly )
		sText.MakeUpper();

	//
	// save the string and crop if necessary
	//
	ASSERT( sText.GetLength() < MAX_TEXT_CHARS/2 );

	strncpy( m_szText, sText, MAX_TEXT_CHARS );
	m_szText[MAX_TEXT_CHARS-1] = '\0';

	int iLength = strlen( m_szText );

	//
	// strip out non-low ASCII chars
	//
	for( int i=0; i<iLength; i++ )	// for each character
	{
		if( m_szText[i] < 0  || m_szText[i] > MAX_FONT_CHARS-1 )
			m_szText[i] = ' ';
	}

	//
	// break the string into lines
	//
	m_iNumLines = 0;
	LPSTR token;

	/* Establish string and get the first token: */
	token = strtok( m_szText, "\n" );
	while( token != NULL )
	{
		m_szTextLines[m_iNumLines++] = token;
		token = strtok( NULL, "\n" );
	}
	
	//
	// calculate line lengths and widths
	//
	m_iWidestLineWidth = 0;
	
	for( int l=0; l<m_iNumLines; l++ )	// for each line
	{
		m_iLineLengths[l] = strlen( m_szTextLines[l] );
		m_iLineWidths[l] = m_pFont->GetLineWidthInSourcePixels( m_szTextLines[l], m_iLineLengths[l] );
		if( m_iLineWidths[l] > m_iWidestLineWidth )
			m_iWidestLineWidth = m_iLineWidths[l];
	}

	// fill the RageScreen's vertex buffer with what we're going to draw 
	//RebuildVertexBuffer();

}

void BitmapText::CropToWidth( int iMaxWidthInSourcePixels )
{
	iMaxWidthInSourcePixels = max( 0, iMaxWidthInSourcePixels );

	for( int l=0; l<m_iNumLines; l++ )	// for each line
	{
		while( m_iLineWidths[l] > iMaxWidthInSourcePixels )
		{
			m_iLineLengths[l]--;
			m_iLineWidths[l] = m_pFont->GetLineWidthInSourcePixels( m_szTextLines[l], m_iLineLengths[l] );
		}
	}
	
}

// draw text at x, y using colorTop blended down to colorBottom, with size multiplied by scale
void BitmapText::DrawPrimitives()
{
	// offset so that pixels are aligned to texels
	if( PREFSMAN->m_iDisplayResolution == 320 )
		DISPLAY->TranslateLocal( -1, -1, 0 );
	else
		DISPLAY->TranslateLocal( -0.5f, -0.5f, 0 );

	if( m_iNumLines == 0 )
		return;

	RageTexture* pTexture = m_pFont->m_pTexture;


	// make the object in logical units centered at the origin
	static RAGEVERTEX v[4000];
	int iNumV = 0;	// the current vertex number


	const int iHeight = pTexture->GetSourceFrameHeight();	// height of a character
	const int iLineSpacing = m_pFont->m_iLineSpacing;			// spacing between lines
	const int iFrameWidth = pTexture->GetSourceFrameWidth();	// width of a character frame in logical units

	int iY;	//	 the center position of the first row of characters
	switch( m_VertAlign )
	{
	case align_bottom:	iY = -(m_iNumLines)	  * iLineSpacing		+ iLineSpacing/2;	break;
	case align_middle:	iY = -(m_iNumLines-1) * iLineSpacing/2;					break;
	case align_top:		iY =								+ iLineSpacing/2;	break;
	default:		ASSERT( false );
	}


	for( int i=0; i<m_iNumLines; i++ )		// foreach line
	{
		const char *szLine = m_szTextLines[i];
		const int iLineLength = m_iLineLengths[i];
		const int iLineWidth = m_iLineWidths[i];
		
		int iX;
		switch( m_HorizAlign )
		{
		case align_left:	iX = 0;					break;
		case align_center:	iX = -(iLineWidth/2);	break;
		case align_right:	iX = -iLineWidth;		break;
		default:			ASSERT( false );
		}

		for( int j=0; j<iLineLength; j++ )	// for each character in the line
		{
			const char c = szLine[j];
			const int iFrameNo = m_pFont->m_iCharToFrameNo[c];
			if( iFrameNo == -1 )	// this font doesn't impelemnt this character
				throw RageException( "The font '%s' does not implement the character '%c'", m_sFontFilePath, c );
			const int iCharWidth = m_pFont->m_iFrameNoToWidth[iFrameNo];

			// The right side of any italic letter is being cropped.  So, we're going to draw a little bit
			// to the right of the normal character.
			const int iDrawExtraPixelsLeft = min( m_pFont->m_iDrawExtraPixelsLeft, (iFrameWidth-iCharWidth)/2 );
			const int iDrawExtraPixelsRight = min( m_pFont->m_iDrawExtraPixelsRight, (iFrameWidth-iCharWidth)/2 );

			//
			// set vertex positions
			//
			v[iNumV++].p = D3DXVECTOR3( (float)iX-iDrawExtraPixelsLeft,										iY-iHeight/2.0f, 0 );	// top left
			v[iNumV++].p = D3DXVECTOR3( (float)iX-iDrawExtraPixelsLeft+iCharWidth+iDrawExtraPixelsRight,	iY-iHeight/2.0f, 0 );	// top right
			v[iNumV++].p = D3DXVECTOR3( (float)iX-iDrawExtraPixelsLeft,										iY+iHeight/2.0f, 0 );	// bottom left
			v[iNumV++].p = D3DXVECTOR3( (float)iX-iDrawExtraPixelsLeft+iCharWidth+iDrawExtraPixelsRight,	iY+iHeight/2.0f, 0 );	// bottom right

			iX += iCharWidth;

			//
			// set texture coordinates
			//
			iNumV -= 4;

			FRECT frectTexCoords = *pTexture->GetTextureCoordRect( iFrameNo );

			// Tweak the textures frame rectangles so we don't draw extra 
			// to the left and right of the character, saving us fill rate.
			float fPixelsToChopOff = pTexture->GetSourceFrameWidth() - (float)iCharWidth;
			float fTexCoordsToChopOff = fPixelsToChopOff / pTexture->GetSourceWidth();
			frectTexCoords.left  += fTexCoordsToChopOff/2;
			frectTexCoords.right -= fTexCoordsToChopOff/2;

			const float fExtraTexCoordsLeft = iDrawExtraPixelsLeft / (float)pTexture->GetSourceWidth();
			const float fExtraTexCoordsRight = iDrawExtraPixelsRight / (float)pTexture->GetSourceWidth();

			v[iNumV++].t = D3DXVECTOR2( frectTexCoords.left  - fExtraTexCoordsLeft,	 frectTexCoords.top );		// top left
			v[iNumV++].t = D3DXVECTOR2( frectTexCoords.right + fExtraTexCoordsRight, frectTexCoords.top );		// top right
			v[iNumV++].t = D3DXVECTOR2( frectTexCoords.left  - fExtraTexCoordsLeft,	 frectTexCoords.bottom );	// bottom left
			v[iNumV++].t = D3DXVECTOR2( frectTexCoords.right + fExtraTexCoordsRight, frectTexCoords.bottom );	// bottom right
		}

		iY += iLineSpacing;
	}


	DISPLAY->SetTexture( pTexture );

	DISPLAY->SetColorTextureMultDiffuse();
	DISPLAY->SetAlphaTextureMultDiffuse();

	if( m_bBlendAdd )
		DISPLAY->SetBlendModeAdd();
	else
		DISPLAY->SetBlendModeNormal();


	if( m_temp.diffuse[0].a != 0 )
	{
		//////////////////////
		// render the shadow
		//////////////////////
		if( m_bShadow )
		{
			DISPLAY->PushMatrix();
			DISPLAY->TranslateLocal( m_fShadowLength, m_fShadowLength, 0 );	// shift by 5 units

			DWORD dwColor = D3DXCOLOR(0,0,0,0.5f*m_temp.diffuse[0].a);	// semi-transparent black

			int i;
			for( i=0; i<iNumV; i++ )
				v[i].color = dwColor;
			for( i=0; i<iNumV; i+=4 )
				DISPLAY->AddQuad( &v[i] );

			DISPLAY->PopMatrix();
		}

		//////////////////////
		// render the diffuse pass
		//////////////////////
		if( m_bRainbow )
		{
			int color_index = int(TIMER->GetTimeSinceStart() / 0.200) % NUM_RAINBOW_COLORS;
			for( int i=0; i<iNumV; i+=4 )
			{
				const D3DXCOLOR color = RAINBOW_COLORS[color_index];
				for( int j=i; j<i+4; j++ )
					v[j].color = color;

				color_index = (color_index+1)%NUM_RAINBOW_COLORS;
			}
		}
		else
		{
			for( int i=0; i<iNumV; i+=4 )
			{
				v[i+0].color = m_temp.diffuse[0];	// top left
				v[i+1].color = m_temp.diffuse[1];	// top right
				v[i+2].color = m_temp.diffuse[2];	// bottom left
				v[i+3].color = m_temp.diffuse[3];	// bottom right
			}
		}

		for( int i=0; i<iNumV; i+=4 )
			DISPLAY->AddQuad( &v[i] );
	}

	//////////////////////
	// render the glow pass
	//////////////////////
	if( m_temp.glow.a != 0 )
	{
		DISPLAY->SetColorDiffuse();

		int i;
		for( i=0; i<iNumV; i++ )
			v[i].color = m_temp.glow;
		for( i=0; i<iNumV; i+=4 )
			DISPLAY->AddQuad( &v[i] );
	}

}