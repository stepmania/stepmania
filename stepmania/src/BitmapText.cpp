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



D3DXCOLOR RAINBOW_COLORS[] = { 
	D3DXCOLOR( 1.0f, 0.2f, 0.4f, 1 ),	// red
	D3DXCOLOR( 0.8f, 0.2f, 0.6f, 1 ),	// pink
	D3DXCOLOR( 0.4f, 0.3f, 0.5f, 1 ),	// purple
	D3DXCOLOR( 0.2f, 0.6f, 1.0f, 1 ),	// sky blue
	D3DXCOLOR( 0.2f, 0.8f, 0.8f, 1 ),	// sea green
	D3DXCOLOR( 0.2f, 0.8f, 0.4f, 1 ),	// green
	D3DXCOLOR( 1.0f, 0.8f, 0.2f, 1 ),	// orange
};
const int NUM_RAINBOW_COLORS = sizeof(RAINBOW_COLORS) / sizeof(D3DXCOLOR);


BitmapText::BitmapText()
{
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
		FONT->UnloadFont( m_pFont->m_sFontFilePath );
}

bool BitmapText::Load( const CString &sFontFilePath )
{
	LOG->WriteLine( "BitmapText::LoadFromFontName(%s)", sFontFilePath );

	// load font
	m_pFont = FONT->LoadFont( sFontFilePath );

	return true;
}




void BitmapText::SetText( CString sText )
{
	//LOG->WriteLine( "BitmapText::SetText()" );

	if( m_pFont->m_bCapitalsOnly )
		sText.MakeUpper();

	//
	// save the string and crop if necessary
	//
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


// draw text at x, y using colorTop blended down to colorBottom, with size multiplied by scale
void BitmapText::DrawPrimitives()
{
	if( m_iNumLines == 0 )
		return;

	RageTexture* pTexture = m_pFont->m_pTexture;


	// make the object in logical units centered at the origin
	LPDIRECT3DVERTEXBUFFER8 pVB = DISPLAY->GetVertexBuffer();
	RAGEVERTEX* v;
	pVB->Lock( 0, 0, (BYTE**)&v, 0 );
	int iNumV = 0;	// the current vertex number


	const int iHeight = pTexture->GetSourceFrameHeight();	// height of a character
	const int iFrameWidth = pTexture->GetSourceFrameWidth();	// width of a character frame in logical units

	int iY;	//	 the center position of the first row of characters
	switch( m_VertAlign )
	{
	case align_top:		iY = -(m_iNumLines-1) * iHeight;	break;
	case align_middle:	iY = -(m_iNumLines-1) * iHeight/2;	break;
	case align_bottom:	iY = 0;								break;
	default:		ASSERT( false );
	}


	for( int i=0; i<m_iNumLines; i++ )		// foreach line
	{
		LPCTSTR szLine = m_szTextLines[i];
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

			// HACK:
			// The right side of any italic letter is being cropped.  So, we're going to draw a little bit
			// to the right of the normal character.
			const float fPercentExtra = min( m_pFont->m_fDrawExtraPercent, (iFrameWidth-iCharWidth)/(float)iFrameWidth );	// don't draw from the adjacent frame


			//
			// set vertex positions
			//
			const float fExtraPixels = fPercentExtra * pTexture->GetSourceFrameWidth();

			// first triangle
			v[iNumV++].p = D3DXVECTOR3( (float)iX,		 iY-iHeight/2.0f, 0 );		// top left
			v[iNumV++].p = D3DXVECTOR3( (float)iX,		 iY+iHeight/2.0f, 0 );		// bottom left
			iX += iCharWidth;
			v[iNumV++].p = D3DXVECTOR3( iX+fExtraPixels, iY-iHeight/2.0f, 0 );	// top right
			
			// 2nd triangle
			v[iNumV++].p = v[iNumV-1].p;										// top right
			v[iNumV++].p = v[iNumV-3].p;										// bottom left
			v[iNumV++].p = D3DXVECTOR3( iX+fExtraPixels, iY+iHeight/2.0f, 0 );	// bottom right


			//
			// set texture coordinates
			//
			iNumV -= 6;

			FRECT frectTexCoords = *pTexture->GetTextureCoordRect( iFrameNo );

			// Tweak the textures frame rectangles so we don't draw extra 
			// to the left and right of the character, saving us fill rate.
			float fPixelsToChopOff = pTexture->GetSourceFrameWidth() - (float)iCharWidth;
			float fTexCoordsToChopOff = fPixelsToChopOff / pTexture->GetSourceWidth();
			frectTexCoords.left  += fTexCoordsToChopOff/2;
			frectTexCoords.right -= fTexCoordsToChopOff/2;

			const float fExtraTexCoords = fPercentExtra * pTexture->GetTextureFrameWidth() / pTexture->GetTextureWidth();

			v[iNumV++].t = D3DXVECTOR2( frectTexCoords.left,					frectTexCoords.top );		// top left
			v[iNumV++].t = D3DXVECTOR2( frectTexCoords.left,					frectTexCoords.bottom );	// bottom left
			v[iNumV++].t = D3DXVECTOR2( frectTexCoords.right + fExtraTexCoords, frectTexCoords.top );		// top right
			v[iNumV++].t = v[iNumV-1].t;																	// top right
			v[iNumV++].t = v[iNumV-3].t;																	// bottom left
			v[iNumV++].t = D3DXVECTOR2( frectTexCoords.right + fExtraTexCoords, frectTexCoords.bottom );	// bottom right
		}

		iY += iHeight;
	}

	pVB->Unlock();



	// Set the stage...
	LPDIRECT3DDEVICE8 pd3dDevice = DISPLAY->GetDevice();
	pd3dDevice->SetTexture( 0, pTexture->GetD3DTexture() );

	pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  m_bBlendAdd ? D3DBLEND_ONE : D3DBLEND_SRCALPHA );
	pd3dDevice->SetRenderState( D3DRS_DESTBLEND, m_bBlendAdd ? D3DBLEND_ONE : D3DBLEND_INVSRCALPHA );

	pd3dDevice->SetVertexShader( D3DFVF_RAGEVERTEX );
	pd3dDevice->SetStreamSource( 0, pVB, sizeof(RAGEVERTEX) );



	//////////////////////
	// render the shadow
	//////////////////////
	if( m_bShadow  &&  m_temp_colorDiffuse[0].a != 0 )
	{
		DISPLAY->PushMatrix();
		DISPLAY->TranslateLocal( m_fShadowLength, m_fShadowLength, 0 );	// shift by 5 units

		DWORD dwColor = D3DXCOLOR(0,0,0,0.5f*m_temp_colorDiffuse[0].a);	// semi-transparent black

		/*
		// YUCK.  It is very common in crapier drivers that the texture factor doesn't work.
		// Too bad because using the texture factor instead of recoloring the verticies is very fast.
		pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, dwColor );

		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
		*/

		// recolor the verticies for a shadow
		pVB->Lock( 0, 0, (BYTE**)&v, 0 );
		for( int i=0; i<iNumV; i++ )
			v[i].color = dwColor;
		pVB->Unlock();

		
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );


		pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, iNumV/3 );

		DISPLAY->PopMatrix();
	}


	//////////////////////
	// render the diffuse pass
	//////////////////////

		//
	// set vertex colors for the diffuse pass
	//
	pVB->Lock( 0, 0, (BYTE**)&v, 0 );

	if( m_bRainbow )
	{
		int color_index = int(TIMER->GetTimeSinceStart() / 0.200) % NUM_RAINBOW_COLORS;
		for( int i=0; i<iNumV; i+=6 )
		{
			const D3DXCOLOR color = RAINBOW_COLORS[color_index];
			for( int j=i; j<i+6; j++ )
				v[j].color = color;

			color_index = (color_index+1)%NUM_RAINBOW_COLORS;
		}
	}
	else
	{
		for( int i=0; i<iNumV; i+=6 )
		{
			v[i  ].color = m_temp_colorDiffuse[0];	// top left
			v[i+1].color = m_temp_colorDiffuse[2];	// bottom left
			v[i+2].color = m_temp_colorDiffuse[1];	// top right
			v[i+3].color = m_temp_colorDiffuse[1];	// top right
			v[i+4].color = m_temp_colorDiffuse[2];	// bottom left
			v[i+5].color = m_temp_colorDiffuse[3];	// bottom right
		}
	}

	pVB->Unlock();

	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );

	pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, iNumV/3 );


	//////////////////////
	// render the add pass
	//////////////////////
	if( m_temp_colorAdd.a != 0 )
	{
		DWORD dwColor = m_temp_colorAdd;

		/*
		// See above comment about some cards not correctly handling the
		// texture factor render state.

		pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, dwColor );

		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
		*/

		// recolor the verticies for
		pVB->Lock( 0, 0, (BYTE**)&v, 0 );
		for( int i=0; i<iNumV; i++ )
			v[i].color = dwColor;
		pVB->Unlock();

		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );

		pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, iNumV/3 );
	}

}