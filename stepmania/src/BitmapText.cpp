#include "stdafx.h"
//-----------------------------------------------------------------------------
// File: BitmapText.cpp
//
// Desc: A font class that draws from a bitmap.
//
// Copyright (c) 2001 Chris Danford.  All rights reserved.
//-----------------------------------------------------------------------------


#include "BitmapText.h"
#include <assert.h>
#include "IniFile.h"



BitmapText::BitmapText()
{

}



BOOL BitmapText::LoadFromFontFile( CString sFontFilePath )
{
	RageLog( "BitmapText::LoadFromFontFile(%s)", sFontFilePath );

	m_sFontFilePath = sFontFilePath;

	// read font file
	IniFile ini;
	ini.SetPath( m_sFontFilePath );
	if( !ini.ReadFile() )
		RageError( ssprintf("Error opening Font file: %s.", m_sFontFilePath) );

	CString sTexturePath = ini.GetValue( "Font", "Texture" );
	if( sTexturePath == "" )
		RageError( ssprintf("Error reading value 'Texture' from %s.", m_sFontFilePath) );

	this->LoadFromTexture( sTexturePath );

	// fill in our map from characters to frame no
	CString sCharacters = ini.GetValue( "Font", "Characters" );
	if( sCharacters == "" )
		RageError( ssprintf("Error reading value 'Characters' from %s.", m_sFontFilePath) );

	for( int i=0; i<sCharacters.GetLength(); i++ )
	{
		TCHAR c = sCharacters[i];
		m_mapCharToFrameNo[c] = i;
	}

	// Validate that the number of characters we read in is the same as 
	// the number of frames in the texture.
	if( sCharacters.GetLength() != (int)this->GetNumStates() )
		RageError( ssprintf("The Font %s specifies %d characters, but the Texture has %d frames.",
						   m_sFontFilePath, sCharacters.GetLength(), (int)this->GetNumStates()) );

	ResetWidthAndHeight();

	return TRUE;
}


void BitmapText::SetText( CString sText )
{
	m_sText = sText;

	ResetWidthAndHeight();
}


void BitmapText::Draw()
{
	//RageLog( "BitmapText::Draw()" );
	// UGLY!!!!

	TCHAR c;
	UINT uFrameNo;
	int iNumChars = m_sText.GetLength();
	int iLeftEdge = this->GetLeftEdge();
	int iOriginalCenterX = (int)this->GetX();
	FLOAT fOriginalWidth = m_size.x;

	int iFrameWidth = (int)( m_pTexture->GetFrameWidth() * this->GetZoom() );
	m_size.x = (FLOAT)iFrameWidth;
	
	// draw each character in the string
	for( int i=0; i<iNumChars; i++ )
	{
		c = m_sText[i];

		// Get what frame in the animation this character is.
		if( !m_mapCharToFrameNo.Lookup(c, uFrameNo) )
			uFrameNo = 0;
		
		this->SetState( uFrameNo );
		this->SetX( (int)(iLeftEdge + iFrameWidth*(i+0.5f)) );

		Sprite::Draw();
 	}

	this->SetX( iOriginalCenterX );
	m_size.x = fOriginalWidth;
}


void BitmapText::ResetWidthAndHeight()
{
	m_size.x = (FLOAT)m_pTexture->GetFrameWidth() * m_sText.GetLength();
	m_size.y = (FLOAT)m_pTexture->GetFrameHeight();
}
