#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: Font

 Desc: See header.

 Copyright (c) 2001-2002 by the names listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Font.h"
#include "IniFile.h"
#include <stdio.h>
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageHelper.h"


Font::Font( const CString &sFontFilePath )
{
	//HELPER.Log( "Font::LoadFromFontName(%s)", sFontFilePath );
	for( int i=0; i<MAX_FONT_CHARS; i++ )
	{
		m_iCharToFrameNo[i] = -1;
		m_iFrameNoToWidth[i] = -1;
	}

	m_iRefCount = 1;

	m_sFontFilePath = sFontFilePath;	// save 


	//
	// Split for the directory.  We'll need it below
	//
	CString sFontDir, sFontFileName, sFontExtension;
	splitrelpath( sFontFilePath, sFontDir, sFontFileName, sFontExtension );

	//
	// Read .font file
	//
	IniFile ini;
	ini.SetPath( m_sFontFilePath );
	if( !ini.ReadFile() )
		HELPER.FatalError( ssprintf("Error opening Font file '%s'.", m_sFontFilePath) );


	//
	// load texture
	//
	CString sTextureFile = ini.GetValue( "Font", "Texture" );
	if( sTextureFile == "" )
		HELPER.FatalError( ssprintf("Error reading  value 'Texture' from %s.", m_sFontFilePath) );

	m_sTexturePath = sFontDir + sTextureFile;	// save the path of the new texture
	m_sTexturePath.MakeLower();

	// is this the first time the texture is being loaded?
	bool bFirstTimeBeingLoaded = !TEXTURE->IsTextureLoaded( m_sTexturePath );


	m_pTexture = TEXTURE->LoadTexture( m_sTexturePath, 0, false );
	assert( m_pTexture != NULL );


	//
	// find out what characters are in this font
	//
	CString sCharacters = ini.GetValue( "Font", "Characters" );
	if( sCharacters != "" )		// the creator supplied characters
	{
		// sanity check
		if( sCharacters.GetLength() != (int)m_pTexture->GetNumFrames() )
			HELPER.FatalError( ssprintf("The characters in '%s' does not match the number of frames in the texture.", m_sFontFilePath) );

		// set the char to frame number map
		for( int i=0; i<sCharacters.GetLength(); i++ )
		{
			char c = sCharacters[i];
			int iFrameNo = i; 

			m_iCharToFrameNo[c] = iFrameNo;
		}
	}
	else	// the font file creator did not supply characters.  Assume this is a full low ASCII set.
	{
		for( int i=0; i<MAX_FONT_CHARS; i++ )
			m_iCharToFrameNo[i] = i;
	}

	m_bCapitalsOnly = ("1" == ini.GetValue("Font", "CapitalsOnly"));


	//
	// load character widths
	//
	CString sWidthsValue = ini.GetValue( "Font", "Widths" );
	if( sWidthsValue != "" )		// the creator supplied witdths
	{
		CStringArray arrayCharWidths;
		split( sWidthsValue, ",", arrayCharWidths );

		if( arrayCharWidths.GetSize() != (int)m_pTexture->GetNumFrames() )
			HELPER.FatalError( ssprintf("The number of widths specified in '%s' (%d) do not match the number of frames in the texture (%u).", m_sFontFilePath, arrayCharWidths.GetSize(), m_pTexture->GetNumFrames()) );

		for( int i=0; i<arrayCharWidths.GetSize(); i++ )
		{
			m_iFrameNoToWidth[i] = atoi( arrayCharWidths[i] );
		}

	}
	else	// The font file creator didn't supply widths.  Assume each character is the width of the frame.
	{
		for( int i=0; i<(int)m_pTexture->GetNumFrames(); i++ )
		{
			m_iFrameNoToWidth[i] = m_pTexture->GetSourceFrameWidth();
		}
	}


	//
	// Tweak the textures frame rectangles so we don't draw extra 
	// to the left and right of the character, saving us fill rate.
	//
	if( bFirstTimeBeingLoaded )
	{
		for( int i=0; i<(int)m_pTexture->GetNumFrames(); i++ )
		{
			FRECT* pFrect = m_pTexture->GetTextureCoordRect( i );

			float fPixelsToChopOff = (float)m_iFrameNoToWidth[i] - m_pTexture->GetSourceFrameWidth();
			float fTexCoordsToChopOff = fPixelsToChopOff / m_pTexture->GetSourceWidth();

			pFrect->left  -= fTexCoordsToChopOff/2;
			pFrect->right += fTexCoordsToChopOff/2;
		}
	}

}


Font::~Font()
{
	if( m_pTexture != NULL )
		TEXTURE->UnloadTexture( m_sTexturePath );
}


int Font::GetLineWidthInSourcePixels( LPCTSTR szLine, int iLength )
{
	int iLineWidth = 0;
	
	for( int i=0; i<iLength; i++ )
	{
		const char c = szLine[i];
		const int iFrameNo = m_iCharToFrameNo[c];
		if( iFrameNo == -1 )	// this font doesn't impelemnt this character
			HELPER.FatalError( ssprintf("The font '%s' does not implement the character '%c'", m_sFontFilePath, c) );

		iLineWidth += m_iFrameNoToWidth[iFrameNo];
	}

	return iLineWidth;
}


