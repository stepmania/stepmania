#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: Font

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Font.h"
#include "IniFile.h"

#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"

#include <stdio.h>
#include <assert.h>

void Font::Init()
{
	int i;

	for( i=0; i<MAX_FONT_CHARS; i++ )
	{
		m_iCharToFrameNo[i] = -1;
		m_iFrameNoToWidth[i] = -1;
	}

	m_bCapitalsOnly = false;

	m_iRefCount = 1;
}

Font::Font( const CString &sASCIITexturePath )
{
	//LOG->Trace( "Font::LoadFromFontName(%s)", sASCIITexturePath.GetString() );

	Init();

	// load texture
	m_sTexturePath = sASCIITexturePath;
	m_sTexturePath.MakeLower();
	m_pTexture = TEXTUREMAN->LoadTexture( m_sTexturePath );
	ASSERT( m_pTexture != NULL );
	if( m_pTexture->GetNumFrames() != 16*16 )
		RageException::Throw( "The font '%s' has only %d frames.  All fonts must have 16*16 frames.", m_sTexturePath.GetString() );

	int i;
	for( i=0; i<256; i++ )
		m_iCharToFrameNo[i] = i;

	// Find .ini widths path from texture path
	CString sDir, sFileName, sExtension;
	splitrelpath( sASCIITexturePath, sDir, sFileName, sExtension );
	const CString sIniPath = sDir + sFileName + ".ini";
	IniFile ini;
	ini.SetPath( sIniPath );
	ini.ReadFile();

	// load character widths
	for( i=0; i<256; i++ )
		if( !ini.GetValueI( "Char Widths", ssprintf("%d",i), m_iFrameNoToWidth[i] ) )
			RageException::Throw( "Error reading width value '%d' from '%s'.", i, sIniPath.GetString() );

	ini.GetValueB( "Char Widths", "CapitalsOnly", m_bCapitalsOnly );
	int DrawExtraPixelsLeft = 0, DrawExtraPixelsRight = 0;

	ini.GetValueI( "Char Widths", "DrawExtraPixelsLeft", DrawExtraPixelsLeft );
	ini.GetValueI( "Char Widths", "DrawExtraPixelsRight", DrawExtraPixelsRight );

	int iAddToAllWidths = 0;
	if( ini.GetValueI( "Char Widths", "AddToAllWidths", iAddToAllWidths ) )
	{
		for( int i=0; i<256; i++ )
			m_iFrameNoToWidth[i] += iAddToAllWidths;
	}

	float fScaleAllWidthsBy = 0;
	if( ini.GetValueF( "Char Widths", "ScaleAllWidthsBy", fScaleAllWidthsBy ) )
	{
		for( int i=0; i<256; i++ )
			m_iFrameNoToWidth[i] = int(roundf( m_iFrameNoToWidth[i] * fScaleAllWidthsBy ));
	}

	m_iLineSpacing = m_pTexture->GetSourceFrameHeight();
	ini.GetValueI( "Char Widths", "LineSpacing", m_iLineSpacing );

	SetTextureCoords();
	SetExtraPixels(DrawExtraPixelsLeft, DrawExtraPixelsRight);
}


Font::Font( const CString &sTexturePath, const CString& sCharacters )
{
	//LOG->Trace( "Font::LoadFromFontName(%s)", sFontFilePath.GetString() );

	Init();

	//
	// load texture
	//
	m_sTexturePath = sTexturePath;	// save
	m_sTexturePath.MakeLower();

	m_pTexture = TEXTUREMAN->LoadTexture( m_sTexturePath );
	ASSERT( m_pTexture != NULL );
	m_iLineSpacing = m_pTexture->GetSourceFrameHeight();

	//
	// find out what characters are in this font
	//
	// sanity check
	if( sCharacters.GetLength() != m_pTexture->GetNumFrames() )
		RageException::Throw( "The image '%s' doesn't have the correct number of frames.  It has %d frames but should have %d frames.",
					m_sTexturePath.GetString(), m_pTexture->GetNumFrames(), sCharacters.GetLength() );

	// set the char to frame number map
	int i;
	for( i=0; i<sCharacters.GetLength(); i++ )
	{
		char c = sCharacters[i];
		int iFrameNo = i; 

		m_iCharToFrameNo[c] = iFrameNo;
	}

	// Assume each character is the width of the frame.
	for( i=0; i<(int)m_pTexture->GetNumFrames(); i++ )
		m_iFrameNoToWidth[i] = m_pTexture->GetSourceFrameWidth();

	SetTextureCoords();
}

void Font::SetTextureCoords()
{
	// force widths to even number
	// Why do this?  It seems to just artificially widen some characters a little and
	// make it look a little worse in 640x480 ...
/*	for( i=0; i<256; i++ )
		if( m_iFrameNoToWidth[i]%2 == 1 )
			m_iFrameNoToWidth[i]++;
*/

	for(int i = 0; i < m_pTexture->GetNumFrames(); ++i)
	{
		/* Make a copy of each texture rect, reducing each to the actual dimensions
		 * of the character (most characters don't take a full block). */
		RectF r = *m_pTexture->GetTextureCoordRect(i);;

		float fPixelsToChopOff = m_pTexture->GetSourceFrameWidth() - (float)m_iFrameNoToWidth[i];
		float fTexCoordsToChopOff = fPixelsToChopOff / m_pTexture->GetSourceWidth();
		
		r.left  += fTexCoordsToChopOff/2;
		r.right -= fTexCoordsToChopOff/2;
		m_TextureCoordRects.push_back(r);

		/* Add normal widths. */
		m_Left.push_back(0);
		m_Right.push_back(m_iFrameNoToWidth[i]);
	}
}

void Font::SetExtraPixels(int DrawExtraPixelsLeft, int DrawExtraPixelsRight)
{
	/* Adjust for DrawExtraPixelsLeft and DrawExtraPixelsRight. */
	for(unsigned i = 0; i < m_TextureCoordRects.size(); ++i)
	{
		int iFrameWidth = m_pTexture->GetSourceFrameWidth();
		int iCharWidth = m_iFrameNoToWidth[i];

		/* Extra pixels to draw to the left and right. */
		int ExtraLeft = min( DrawExtraPixelsLeft, (iFrameWidth-iCharWidth)/2 );
		int ExtraRight = min( DrawExtraPixelsRight, (iFrameWidth-iCharWidth)/2 ) + ExtraLeft;

		/* Move left and expand right. */
		m_TextureCoordRects[i].left -= float(ExtraLeft) / m_pTexture->GetSourceWidth();
		m_TextureCoordRects[i].right += float(ExtraRight) / m_pTexture->GetSourceWidth();
		m_Left[i] += ExtraLeft;
		m_Right[i] += ExtraRight;
	}
}

Font::~Font()
{
	if( m_pTexture != NULL )
		TEXTUREMAN->UnloadTexture( m_sTexturePath );
}


int Font::GetLineWidthInSourcePixels( const CString &szLine )
{
	int iLineWidth = 0;
	
	for( unsigned i=0; i<szLine.size(); i++ )
	{
		const char c = szLine[i];
		const int iFrameNo = m_iCharToFrameNo[ (unsigned char)c ];
		if( iFrameNo == -1 )	// this font doesn't impelemnt this character
			RageException::Throw( "The font '%s' does not implement the character '%c'", m_sTexturePath.GetString(), c );

		iLineWidth += m_iFrameNoToWidth[iFrameNo];
	}

	return iLineWidth;
}


