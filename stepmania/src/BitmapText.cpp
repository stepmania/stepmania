#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: BitmapText

 Desc: See header.

 Copyright (c) 2001-2002 by the names listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "BitmapText.h"
#include "IniFile.h"
#include "FontManager.h"
#include "RageHelper.h"


BitmapText::BitmapText()
{
	m_HorizAlign = align_center;
	m_VertAlign = align_middle;

	m_pFont = NULL;

	m_iNumLines = 0;
	m_iWidestLineWidth = 0;
	m_iNumV = 0;
	
	for( int i=0; i<MAX_TEXT_LINES; i++ )
	{
		m_szTextLines[i] = '\0';
		m_iLineLengths[i] = -1;
		m_iLineWidths[i] = 1;
	}
}

BitmapText::~BitmapText()
{
	if( m_pFont )
		FONT->UnloadFont( m_pFont->m_sFontFilePath );
}

bool BitmapText::Load( const CString &sFontFilePath )
{
	HELPER.Log( "BitmapText::LoadFromFontName(%s)", sFontFilePath );

	// load font
	m_pFont = FONT->LoadFont( sFontFilePath );

	return true;
}




void BitmapText::SetText( CString sText )
{
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
}




void BitmapText::RebuildVertexBuffer()
{
	RageTexture* pTexture = m_pFont->m_pTexture;









	// make the object in logical units centered at the origin
	LPDIRECT3DVERTEXBUFFER8 pVB = SCREEN->GetVertexBuffer();
	CUSTOMVERTEX* v;
	pVB->Lock( 0, 0, (BYTE**)&v, 0 );



	m_iNumV = 0;	// the current vertex number

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
				HELPER.FatalError( ssprintf("The font '%s' does not implement the character '%c'", m_sFontFilePath, c) );
			const int iCharWidth = m_pFont->m_iFrameNoToWidth[iFrameNo];

			// HACK:
			// The right side of any italic letter is being cropped.  So, we're going to draw a little bit
			// to the right of the normal character.
			const float fPercentExtra = min( 
				0.20f, 
				(iFrameWidth-iCharWidth)/(float)iFrameWidth 
				);
			
			const float fExtraPixels = fPercentExtra * pTexture->GetSourceFrameWidth();

			// first triangle
			v[m_iNumV++].p = D3DXVECTOR3( (float)iX, iY-iHeight/2.0f,	0 );		// top left
			v[m_iNumV++].p = D3DXVECTOR3( (float)iX, iY+iHeight/2.0f,	0 );		// bottom left
			iX += iCharWidth;
			v[m_iNumV++].p = D3DXVECTOR3( iX+fExtraPixels,	iY-iHeight/2.0f, 0 );	// top right
			
			// 2nd triangle
			v[m_iNumV++].p = v[m_iNumV-1].p;										// top right
			v[m_iNumV++].p = v[m_iNumV-3].p;										// bottom left
			v[m_iNumV++].p = D3DXVECTOR3( iX+fExtraPixels,	iY+iHeight/2.0f, 0 );	// bottom right


			// set texture coordinates
			m_iNumV -= 6;

			FRECT* pTexCoordRect = pTexture->GetTextureCoordRect( iFrameNo );

			const float fExtraTexCoords = fPercentExtra * pTexture->GetTextureFrameWidth() / pTexture->GetTextureWidth();

			v[m_iNumV].tu = pTexCoordRect->left;					v[m_iNumV++].tv = pTexCoordRect->top;		// top left
			v[m_iNumV].tu = pTexCoordRect->left;					v[m_iNumV++].tv = pTexCoordRect->bottom;	// bottom left
			v[m_iNumV].tu = pTexCoordRect->right + fExtraTexCoords;	v[m_iNumV++].tv = pTexCoordRect->top;		// top right
			v[m_iNumV].tu = v[m_iNumV-1].tu;						v[m_iNumV++].tv = v[m_iNumV-1].tv;			// top right
			v[m_iNumV].tu = v[m_iNumV-3].tu;						v[m_iNumV++].tv = v[m_iNumV-3].tv;			// bottom left
			v[m_iNumV].tu = pTexCoordRect->right + fExtraTexCoords;	v[m_iNumV++].tv = pTexCoordRect->bottom;	// bottom right

/*
			v[m_iNumV].tu = 0.0f;		v[m_iNumV++].tv = 1.0f-1.0f/16;		// top left
			v[m_iNumV].tu = 0.0f;		v[m_iNumV++].tv = 1.0f;				// bottom left
			v[m_iNumV].tu = 1.0f/16;	v[m_iNumV++].tv = 1.0f-1.0f/16;		// top right
			v[m_iNumV].tu = 1.0f/16;	v[m_iNumV++].tv = 1.0f-1.0f/16;		// top right
			v[m_iNumV].tu = 0.0f;		v[m_iNumV++].tv = 1.0f;				// bottom left
			v[m_iNumV].tu = 1.0f/16;	v[m_iNumV++].tv = 1.0f;				// bottom right
*/
		}

		iY += iHeight;
	}



	pVB->Unlock();

}


// draw text at x, y using colorTop blended down to colorBottom, with size multiplied by scale
void BitmapText::RenderPrimitives()
{
	if( m_iNumLines == 0 )
		return;

	RageTexture* pTexture = m_pFont->m_pTexture;


	// fill the RageScreen's vertex buffer with what we're going to draw 
	RebuildVertexBuffer();


	LPDIRECT3DVERTEXBUFFER8 pVB = SCREEN->GetVertexBuffer();
	CUSTOMVERTEX* v;


	// Set the stage...
	LPDIRECT3DDEVICE8 pd3dDevice = SCREEN->GetDevice();
	pd3dDevice->SetTexture( 0, pTexture->GetD3DTexture() );

	pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	//pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  bBlendAdd ? D3DBLEND_ONE : D3DBLEND_SRCALPHA );
	//pd3dDevice->SetRenderState( D3DRS_DESTBLEND, bBlendAdd ? D3DBLEND_ONE : D3DBLEND_INVSRCALPHA );
	pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  m_bBlendAdd ? D3DBLEND_ONE : D3DBLEND_SRCALPHA );
	pd3dDevice->SetRenderState( D3DRS_DESTBLEND, m_bBlendAdd ? D3DBLEND_ONE : D3DBLEND_INVSRCALPHA );

	pd3dDevice->SetVertexShader( D3DFVF_CUSTOMVERTEX );
	pd3dDevice->SetStreamSource( 0, pVB, sizeof(CUSTOMVERTEX) );



	if( m_temp_colorDiffuse[0].a != 0 )
	{
		//////////////////////
		// render the shadow
		//////////////////////
		if( m_bShadow )
		{
			SCREEN->PushMatrix();
			SCREEN->TranslateLocal( m_fShadowLength, m_fShadowLength, 0 );	// shift by 5 units

			pVB->Lock( 0, 0, (BYTE**)&v, 0 );

			for( int i=0; i<m_iNumV; i++ )
				v[i].color = D3DXCOLOR(0,0,0,0.5f*m_temp_colorDiffuse[0].a);	// semi-transparent black
			
			pVB->Unlock();

			pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
			
			pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, m_iNumV/3 );

			SCREEN->PopMatrix();
		}


		//////////////////////
		// render the diffuse pass
		//////////////////////
		pVB->Lock( 0, 0, (BYTE**)&v, 0 );

		for( int i=0; i<m_iNumV; i++ )
		{
			if( i%2 == 0 )	// this is a bottom vertex
				v[i].color = m_temp_colorDiffuse[0];
			else				// this is a top vertex
				v[i].color = m_temp_colorDiffuse[1];
		}
		
		
		pVB->Unlock();

		
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );//bBlendAdd ? D3DTOP_ADD : D3DTOP_MODULATE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );//bBlendAdd ? D3DTOP_ADD : D3DTOP_MODULATE );

		pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, m_iNumV/3 );
	}


	//////////////////////
	// render the add pass
	//////////////////////
	if( m_temp_colorAdd.a != 0 )
	{
		pVB->Lock( 0, 0, (BYTE**)&v, 0 );

		for( int i=0; i<m_iNumV; i++ )
			v[i].color = m_temp_colorAdd;
		
		pVB->Unlock();

		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
		
		pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, m_iNumV/3 );
	}

}