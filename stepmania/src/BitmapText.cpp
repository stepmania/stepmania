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
float BitmapText::GetTextWidthInSourcePixels()
{
	float fTextWidth = 0;
	
	for( int i=0; i<m_sText.GetLength(); i++ )
		fTextWidth += m_fCharWidthsInSourcePixels[ m_sText[i] ];

	return fTextWidth;
}



// draw text at x, y using colorTop blended down to colorBottom, with size multiplied by scale
void BitmapText::Draw()
{
	float fCenterX = GetX();
	float fTextWidthInSourcePixels = GetTextWidthInSourcePixels();
	float fTextWidth = fTextWidthInSourcePixels* GetZoom();
	float fFrameWidth = GetZoomedWidth();
	float fX = fCenterX - (fTextWidth/2);


	for( int i=0; i<m_sText.GetLength(); i++ ) {
		char c = m_sText[i];
		SetState( c );
		SetX( fX );		// set X acording to offset
		Sprite::Draw();
		fX += m_fCharWidthsInSourcePixels[c] * GetZoom();
	}

	// set properties back to original
	SetX( fCenterX );
}
