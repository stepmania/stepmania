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

Font::Font( const CString &sASCIITexturePath )
{
	//LOG->Trace( "Font::LoadFromFontName(%s)", sASCIITexturePath );

	int i;

	for( i=0; i<MAX_FONT_CHARS; i++ )
	{
		m_iCharToFrameNo[i] = -1;
		m_iFrameNoToWidth[i] = -1;
	}

	m_iRefCount = 1;

	// load texture
	m_sTexturePath = sASCIITexturePath;
	m_sTexturePath.MakeLower();
	m_pTexture = TEXTUREMAN->LoadTexture( m_sTexturePath );
	ASSERT( m_pTexture != NULL );
	if( m_pTexture->GetNumFrames() != 16*16 )
		throw RageException( "The font '%s' has only %d frames.  All fonts must have 16*16 frames.", m_sTexturePath );

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
			throw RageException( "Error reading 'Char Widths from '%s'.", sIniPath );

	m_bCapitalsOnly = false;
	ini.GetValueB( "Char Widths", "CapitalsOnly", m_bCapitalsOnly );

	m_iDrawExtraPixelsLeft = 0;
	ini.GetValueI( "Char Widths", "DrawExtraPixelsLeft", m_iDrawExtraPixelsLeft );

	m_iDrawExtraPixelsRight = 0;
	ini.GetValueI( "Char Widths", "DrawExtraPixelsRight", m_iDrawExtraPixelsRight );

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

	// force widths to even number
	for( i=0; i<256; i++ )
		if( m_iFrameNoToWidth[i]%2 == 1 )
			m_iFrameNoToWidth[i]++;
}


Font::Font( const CString &sTexturePath, const CString& sCharacters )
{
	//LOG->Trace( "Font::LoadFromFontName(%s)", sFontFilePath );
	int i;
	for( i=0; i<MAX_FONT_CHARS; i++ )
	{
		m_iCharToFrameNo[i] = -1;
		m_iFrameNoToWidth[i] = -1;
	}

	m_bCapitalsOnly = false;
	m_iDrawExtraPixelsLeft = 0;
	m_iDrawExtraPixelsRight = 0;

	m_iRefCount = 1;

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
		throw RageException( "The image '%s' doesn't have the correct number of frames.  It has %d frames but should have %d frames.",
					m_sTexturePath, m_pTexture->GetNumFrames(), sCharacters.GetLength() );

	// set the char to frame number map
	for( i=0; i<sCharacters.GetLength(); i++ )
	{
		char c = sCharacters[i];
		int iFrameNo = i; 

		m_iCharToFrameNo[c] = iFrameNo;
	}

	// Assume each character is the width of the frame.
	for( i=0; i<(int)m_pTexture->GetNumFrames(); i++ )
		m_iFrameNoToWidth[i] = m_pTexture->GetSourceFrameWidth();
}


Font::~Font()
{
	if( m_pTexture != NULL )
		TEXTUREMAN->UnloadTexture( m_sTexturePath );
}


int Font::GetLineWidthInSourcePixels( const char *szLine, int iLength )
{
	int iLineWidth = 0;
	
	for( int i=0; i<iLength; i++ )
	{
		const char c = szLine[i];
		const int iFrameNo = m_iCharToFrameNo[c];
		if( iFrameNo == -1 )	// this font doesn't impelemnt this character
			throw RageException( "The font '%s' does not implement the character '%c'", m_sTexturePath, c );

		iLineWidth += m_iFrameNoToWidth[iFrameNo];
	}

	return iLineWidth;
}


