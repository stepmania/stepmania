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



BitmapText::BitmapText()
{
//	m_colorTop = D3DXCOLOR(1,1,1,1);
//	m_colorBottom = D3DXCOLOR(1,1,1,1);

	for( int i=0; i<NUM_CHARS; i++ )
		m_fCharWidthsInSourcePixels[i] = 16;

//	m_bHasShadow = false;
}

bool BitmapText::LoadFromFontName( CString sFontName )
{
	RageLog( "BitmapText::LoadFromFontName(%s)", sFontName );
	
	m_sFontName = sFontName;	// save font name

	Sprite::LoadFromTexture( ssprintf("Fonts/%s 16x16.png",sFontName) );
	LoadCharWidths( ssprintf("Fonts/%s.widths",sFontName) );

	return TRUE;
}

// returns the font height in the case of a bitmap load. note does not use the real bitmap height but rather
// the height of the bitmap / 16. generally returns > 0 for success.
bool BitmapText::LoadCharWidths( CString sWidthFilePath )
{
	FILE *file;
	file = fopen( sWidthFilePath, "rb" );
	for( int i=0; i<256; i++ ) {
		BYTE widthSourcePixels;
		if( fread(&widthSourcePixels, 1, 1, file) != 1 ) 
			return false;
		m_fCharWidthsInSourcePixels[i] = widthSourcePixels;
	}
	fclose(file);
	return true;
}


// get a rectangle for the text, considering a possible text scaling.
// useful to know if some text is visible or not
float BitmapText::GetWidthInSourcePixels()
{
	float fTextWidth = 0;
	
	for( int i=0; i<m_sText.GetLength(); i++ )
		fTextWidth += m_fCharWidthsInSourcePixels[ m_sText[i] ];

	return fTextWidth;
}



// draw text at x, y using colorTop blended down to colorBottom, with size multiplied by scale
void BitmapText::Draw()
{
	// some basic properties of the text
	float fTextWidthInSourcePixels = GetWidthInSourcePixels();
	float fTextWidth = fTextWidthInSourcePixels* GetZoom();
	float fFrameWidth = GetZoomedWidth();


/*	
	if( m_bHasShadow )
	{
		// do a pass for the shadow

		float fOriginalX = GetX();
		float fOriginalY = GetY();
		D3DXCOLOR colorOriginalDiffuse[4];
		for( int i=0; i<4; i++ )
			colorOriginalDiffuse[i] = GetDiffuseColors(i);
		D3DXCOLOR colorOriginalAdd = GetAddColor();

		float fX = fOriginalX - (fTextWidth/2) + 10;
		float fY = fOriginalY + 10;

		SetY( fY );
		SetDiffuseColor( D3DXCOLOR(0,0,0,0) );	// transparent
		SetAddColor( D3DXCOLOR(0,0,0,0.5f) );	// semi-transparent black


		for( i=0; i<m_sText.GetLength(); i++ ) {
			char c = m_sText[i];
			float fCharWidthZoomed = m_fCharWidthsInSourcePixels[c] * GetZoom();
			SetState( c );
			fX += fCharWidthZoomed/2;
			SetX( fX );
			Sprite::Draw();
			fX += fCharWidthZoomed/2;
		}

		// set properties back to original
		SetX( fOriginalX );
		SetY( fOriginalY );
		for( i=0; i<4; i++ )
			SetDiffuseColors( i, colorOriginalDiffuse[i] );
		SetAddColor( colorOriginalAdd );
	}
*/


	// draw the text

	float fCenterX = GetX();
	float fX = fCenterX - (fTextWidth/2);


	for( int i=0; i<m_sText.GetLength(); i++ ) {
		char c = m_sText[i];
		float fCharWidthZoomed = m_fCharWidthsInSourcePixels[c] * GetZoom();
		SetState( c );
		fX += fCharWidthZoomed/2;
		SetX( fX );
		Sprite::Draw();
		fX += fCharWidthZoomed/2;
	}

	// set properties back to original
	SetX( fCenterX );
}
