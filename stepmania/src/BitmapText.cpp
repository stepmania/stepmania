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
	CString &sText = m_sTextLines[iLineNo];

	float fLineWidth = 0;
	
	for( int j=0; j<sText.GetLength(); j++ )
		fLineWidth += m_fCharWidthsInSourcePixels[ sText[j] ];

	return fLineWidth;
}



// draw text at x, y using colorTop blended down to colorBottom, with size multiplied by scale
void BitmapText::Draw()
{
	if( m_sTextLines.GetSize() == 0 )
		return;

	// save the properties now so we can restore them at the end
	float fOriginalX = GetX();
	float fOriginalY = GetY();

	


	float fHeight = Sprite::GetZoomedHeight();

	for( int i=0; i<m_sTextLines.GetSize(); i++ )
	{
		CString &sText = m_sTextLines[i];
		float fY = fOriginalY + fHeight * i - (m_sTextLines.GetSize()-1) * fHeight / 2;
		float fLineWidth = GetLineWidthInSourcePixels(i) * GetZoomX();

		float fX = fOriginalX - (fLineWidth/2);

		SetY( fY );

		for( int j=0; j<sText.GetLength(); j++ ) 
		{
			char c = sText[j];
			float fCharWidthZoomed = m_fCharWidthsInSourcePixels[c] * GetZoomX();
			SetState( c );
			fX += fCharWidthZoomed/2;
			SetX( fX );
			Sprite::Draw();
			fX += fCharWidthZoomed/2;
		}

	}

	// reset properties to what they were before this function 
	SetX( fOriginalX );
	SetY( fOriginalY );

}

void BitmapText::SetText( CString sText )
{ 
	m_sTextLines.RemoveAll(); 
	split( sText, "\n", m_sTextLines, false );
}

CString BitmapText::GetText() 
{ 
	return join( "\n", m_sTextLines ); 
}
