#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: BitmapText.h

 Desc: A font class that draws characters from a bitmap.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "BitmapText.h"
#include "IniFile.h"
#include <stdio.h>
#include "RageTextureManager.h"



BitmapText::BitmapText()
{
//	m_colorTop = D3DXCOLOR(1,1,1,1);
//	m_colorBottom = D3DXCOLOR(1,1,1,1);

	for( int i=0; i<NUM_CHARS; i++ )
	{
		m_iCharToFrameNo[i] = -1;
		m_fFrameNoToWidth[i] = -1;
	}
	m_HorizAlign = align_center;
	m_VertAlign = align_middle;

//	m_bHasShadow = false;

	// create the vertex buffer
	HRESULT hr = SCREEN->GetDevice()->CreateVertexBuffer( 
		MAX_NUM_VERTICIES * sizeof(CUSTOMVERTEX),
		D3DUSAGE_WRITEONLY, 
		D3DFVF_CUSTOMVERTEX,
		D3DPOOL_MANAGED, 
		&m_pVB );

	if( FAILED( hr ) )
	{
		RageErrorHr( "Vertex Buffer Could Not Be Created", hr );
	}
	m_iNumV = 0;
}

BitmapText::~BitmapText()
{
	SAFE_RELEASE( m_pVB );
}

bool BitmapText::Load( CString sFontFilePath )
{
	//RageLog( "BitmapText::LoadFromFontName(%s)", sFontFilePath );
	
	m_sFontFilePath = sFontFilePath;	// save 


	// Split for the directory.  We'll need it below
	CString sFontDir, sFontFileName, sFontExtension;
	splitrelpath( sFontFilePath, sFontDir, sFontFileName, sFontExtension );



	// Read .font file
	IniFile ini;
	ini.SetPath( m_sFontFilePath );
	if( !ini.ReadFile() )
		RageError( ssprintf("Error opening Font file '%s'.", m_sFontFilePath) );



	// load texture
	CString sTextureFile = ini.GetValue( "Font", "Texture" );
	if( sTextureFile == "" )
		RageError( ssprintf("Error reading  value 'Texture' from %s.", m_sFontFilePath) );


	CString sTexturePath = sFontDir + sTextureFile;	// save the path of the new texture

	// is this the first time the texture is being loaded?
	bool bFirstTimeBeingLoaded = !TM->IsTextureLoaded( sTexturePath );

	LoadTexture( sTexturePath );

	// the size of the sprite is the size of the image before it was scaled
	SetWidth( (float)m_pTexture->GetSourceFrameWidth() );
	SetHeight( (float)m_pTexture->GetSourceFrameHeight() );		



	// find out what characters are in this font
	CString sCharacters = ini.GetValue( "Font", "Characters" );
	if( sCharacters != "" )		// the creator supplied characters
	{
		// sanity check
		if( sCharacters.GetLength() != (int)Sprite::GetNumStates() )
			RageError( ssprintf("The characters in '%s' does not match the number of frames in the texture.", m_sFontFilePath) );

		// set the char to frameno map
		for( int i=0; i<sCharacters.GetLength(); i++ )
		{
			char c = sCharacters[i];
			int iFrameNo = i; 

			m_iCharToFrameNo[c] = iFrameNo;
		}
	}
	else	// no creator supplied characters.  Assume this is a full high ASCII set
	{
		for( int i=0; i<NUM_CHARS; i++ )
			m_iCharToFrameNo[i] = i;
	}



	// and load character widths
	CString sWidthsValue = ini.GetValue( "Font", "Widths" );
	if( sWidthsValue != "" )		// the creator supplied witdths
	{
		CStringArray arrayCharWidths;
		split( sWidthsValue, ",", arrayCharWidths );

		if( arrayCharWidths.GetSize() != (int)Sprite::GetNumStates() )
			RageError( ssprintf("The number of widths specified in '%s' (%d) do not match the number of frames in the texture (%u).", m_sFontFilePath, arrayCharWidths.GetSize(), Sprite::GetNumStates()) );

		for( int i=0; i<arrayCharWidths.GetSize(); i++ )
		{
			m_fFrameNoToWidth[i] = (float)atof( arrayCharWidths[i] );
		}

	}
	else	// no creator supplied withds.  Assume each character is the width of the frame
	{
		for( int i=0; i<(int)Sprite::GetNumStates(); i++ )
		{
			m_fFrameNoToWidth[i] = (float)m_pTexture->GetSourceFrameWidth();
		}
	}


	if( bFirstTimeBeingLoaded )
	{
		// tweak the textures frame rectangles so we don't draw extra to the left and right of the character
		for( int i=0; i<(int)Sprite::GetNumStates(); i++ )
		{
			FRECT* pFrect = m_pTexture->GetTextureCoordRect( i );

			float fPixelsToChopOff = m_fFrameNoToWidth[i] - GetUnzoomedWidth();
			float fTexCoordsToChopOff = fPixelsToChopOff / m_pTexture->GetSourceWidth();

			pFrect->left  -= fTexCoordsToChopOff/2;
			pFrect->right += fTexCoordsToChopOff/2;
		}
	}

	return true;
}




// get a rectangle for the text, considering a possible text scaling.
// useful to know if some text is visible or not
float BitmapText::GetWidestLineWidthInSourcePixels()
{
	float fWidestLineWidth = 0;

	for( int i=0; i<m_sTextLines.GetSize(); i++ )
	{
		float fLineWidth = GetLineWidthInSourcePixels(i);
		
		if( fLineWidth > fWidestLineWidth )
			fWidestLineWidth = fLineWidth;
	}

	return fWidestLineWidth;
}


float BitmapText::GetLineWidthInSourcePixels( int iLineNo )
{
	CString &sLine = m_sTextLines[iLineNo];

	float fLineWidth = 0;
	
	for( int i=0; i<sLine.GetLength(); i++ )
	{
		char c = sLine[i];
		int iFrameNo = m_iCharToFrameNo[c];
		if( iFrameNo == -1 )	// this font doesn't impelemnt this character
			RageError( ssprintf("The font '%s' does not implement the character '%c'", m_sFontFilePath, c) );

		fLineWidth += m_fFrameNoToWidth[iFrameNo];
	}

	return fLineWidth;
}


void BitmapText::RebuildVertexBuffer()
{
	// make the object in logical units centered at the origin
	LPDIRECT3DVERTEXBUFFER8 pVB = m_pVB;
	CUSTOMVERTEX* v;
	pVB->Lock( 0, 0, (BYTE**)&v, 0 );




	m_iNumV = 0;	// the current vertex number

	float fHeight = GetUnzoomedHeight();
	float fY;
	switch( m_VertAlign )
	{
	case align_top:		fY = -(m_sTextLines.GetSize()) * fHeight / 2;	break;
	case align_middle:	fY = 0;											break;
	case align_bottom:	fY = (m_sTextLines.GetSize()) * fHeight / 2;	break;
	default:		ASSERT( false );
	}


	for( int i=0; i<m_sTextLines.GetSize(); i++ )		// foreach line
	{
		CString &sLine = m_sTextLines[i];

		float fLineWidth = GetLineWidthInSourcePixels(i);
		float fX;
		switch( m_HorizAlign )
		{
		case align_left:	fX = 0;					break;
		case align_center:	fX = -(fLineWidth/2);	break;
		case align_right:	fX = -fLineWidth;		break;
		default:		ASSERT( false );
		}

		for( int j=0; j<sLine.GetLength(); j++ )	// for each character in the line
		{
			char c = sLine[j];
			int iFrameNo = m_iCharToFrameNo[c];
			if( iFrameNo == -1 )	// this font doesn't impelemnt this character
				RageError( ssprintf("The font '%s' does not implement the character '%c'", m_sFontFilePath, c) );
			float fCharWidth = m_fFrameNoToWidth[iFrameNo];

			//if( c == ' ' )
			//{
			//	fX += fCharWidth;
			//	continue;
			//}


			// HACK:
			// The right side of italic letters is being cropped.  So, we're going to draw a little bit
			// to the right of the normal character.
			float fPercentExtra = 0.20f;
			
			// don't go over the frame boundary and draw part of the adjacent character
			float fPercentageOfFrame = fCharWidth / GetUnzoomedWidth();
			if( fPercentExtra > 1-fPercentageOfFrame )
				fPercentExtra = 1-fPercentageOfFrame;

			float fExtraPixels = fPercentExtra * GetUnzoomedWidth();

			// first triangle
			v[m_iNumV++].p = D3DXVECTOR3( fX,	fY-fHeight/2,	0 );				// top left
			v[m_iNumV++].p = D3DXVECTOR3( fX,	fY+fHeight/2,	0 );				// bottom left
			fX += fCharWidth;
			v[m_iNumV++].p = D3DXVECTOR3( fX+fExtraPixels,	fY-fHeight/2,	0 );	// top right
			
			// 2nd triangle
			v[m_iNumV++].p = v[m_iNumV-1].p;										// top right
			v[m_iNumV++].p = v[m_iNumV-3].p;										// bottom left
			v[m_iNumV++].p = D3DXVECTOR3( fX+fExtraPixels,	fY+fHeight/2,	0 );	// bottom right


			// set texture coordinates
			m_iNumV -= 6;

			FRECT* pTexCoordRect = m_pTexture->GetTextureCoordRect( iFrameNo );

			float fExtraTexCoords = fPercentExtra * m_pTexture->GetTextureFrameWidth() / m_pTexture->GetTextureWidth();

			v[m_iNumV].tu = pTexCoordRect->left;					v[m_iNumV++].tv = pTexCoordRect->top;		// top left
			v[m_iNumV].tu = pTexCoordRect->left;					v[m_iNumV++].tv = pTexCoordRect->bottom;	// bottom left
			v[m_iNumV].tu = pTexCoordRect->right + fExtraTexCoords;	v[m_iNumV++].tv = pTexCoordRect->top;		// top right
			v[m_iNumV].tu = v[m_iNumV-1].tu;						v[m_iNumV++].tv = v[m_iNumV-1].tv;			// top right
			v[m_iNumV].tu = v[m_iNumV-3].tu;						v[m_iNumV++].tv = v[m_iNumV-3].tv;			// bottom left
			v[m_iNumV].tu = pTexCoordRect->right + fExtraTexCoords;	v[m_iNumV++].tv = pTexCoordRect->bottom;	// bottom right


		}

		fY += fHeight;
	}



	pVB->Unlock();


}


// draw text at x, y using colorTop blended down to colorBottom, with size multiplied by scale
void BitmapText::RenderPrimitives()
{
	if( m_sTextLines.GetSize() == 0 )
		return;

	if( m_pTexture == NULL )
		return;


	
	D3DXCOLOR	colorDiffuse[4];
	for(int i=0; i<4; i++)
		colorDiffuse[i] = m_colorDiffuse[i];
	D3DXCOLOR	colorAdd		= m_colorAdd;

	// update properties based on SpriteEffects 
	switch( m_Effect )
	{
	case no_effect:
		break;
	case blinking:
		{
		for(int i=0; i<4; i++)
			colorDiffuse[i] = m_bTweeningTowardEndColor ? m_effect_colorDiffuse1 : m_effect_colorDiffuse2;
		}
		break;
	case camelion:
		{
		for(int i=0; i<4; i++)
			colorDiffuse[i] = m_effect_colorDiffuse1*m_fPercentBetweenColors + m_effect_colorDiffuse2*(1.0f-m_fPercentBetweenColors);
		}
		break;
	case glowing:
		colorAdd = m_effect_colorAdd1*m_fPercentBetweenColors + m_effect_colorAdd2*(1.0f-m_fPercentBetweenColors);
		break;
	case wagging:
		break;
	case spinning:
		// nothing special needed
		break;
	case vibrating:
		break;
	case flickering:
		m_bVisibleThisFrame = !m_bVisibleThisFrame;
		if( !m_bVisibleThisFrame )
			for(int i=0; i<4; i++)
				colorDiffuse[i] = D3DXCOLOR(0,0,0,0);		// don't draw the frame
		break;
	}




	RebuildVertexBuffer();


	LPDIRECT3DVERTEXBUFFER8 pVB = m_pVB;
	CUSTOMVERTEX* v;


	// Set the stage...
	LPDIRECT3DDEVICE8 pd3dDevice = SCREEN->GetDevice();
	pd3dDevice->SetTexture( 0, m_pTexture->GetD3DTexture() );

	pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	//pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  bBlendAdd ? D3DBLEND_ONE : D3DBLEND_SRCALPHA );
	//pd3dDevice->SetRenderState( D3DRS_DESTBLEND, bBlendAdd ? D3DBLEND_ONE : D3DBLEND_INVSRCALPHA );
	pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  m_bBlendAdd ? D3DBLEND_ONE : D3DBLEND_SRCALPHA );
	pd3dDevice->SetRenderState( D3DRS_DESTBLEND, m_bBlendAdd ? D3DBLEND_ONE : D3DBLEND_INVSRCALPHA );

	pd3dDevice->SetVertexShader( D3DFVF_CUSTOMVERTEX );
	pd3dDevice->SetStreamSource( 0, pVB, sizeof(CUSTOMVERTEX) );



	if( colorDiffuse[0].a != 0 )
	{
		//////////////////////
		// render the shadow
		//////////////////////
		if( m_bHasShadow )
		{
			SCREEN->PushMatrix();
			SCREEN->Translate( 5, 5, 0 );	// shift by 5 units

			pVB->Lock( 0, 0, (BYTE**)&v, 0 );

			for( int i=0; i<m_iNumV; i++ )
				v[i].color = D3DXCOLOR(0,0,0,0.5f*colorDiffuse[0].a);	// semi-transparent black
			
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
				v[i].color = colorDiffuse[0];
			else				// this is a top vertex
				v[i].color = colorDiffuse[1];
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
	if( colorAdd.a != 0 )
	{
		pVB->Lock( 0, 0, (BYTE**)&v, 0 );

		for( int i=0; i<m_iNumV; i++ )
			v[i].color = colorAdd;
		
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

void BitmapText::SetText( CString sText )
{ 
	if( sText.GetLength() > MAX_NUM_VERTICIES )
		sText = sText.Left( MAX_NUM_VERTICIES );

	// strip out foreign chars
	for( int i=0; i<sText.GetLength(); i++ )	// for each character
		if( sText[i] < 0  || sText[i] > NUM_CHARS-1 )
			sText.SetAt( i, ' ' );


	m_sTextLines.RemoveAll(); 
	split( sText, "\n", m_sTextLines, false );


	RebuildVertexBuffer();
}

CString BitmapText::GetText() 
{ 
	return join( "\n", m_sTextLines ); 
}

