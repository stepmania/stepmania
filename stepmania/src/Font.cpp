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
	m_bCapitalsOnly = false;
	m_iRefCount = 1;
}

void Font::Load( const CString &TexturePath, IniFile &ini )
{
	m_sTexturePath = TexturePath;

	// load texture
	m_sTexturePath.MakeLower();
	m_pTexture = TEXTUREMAN->LoadTexture( m_sTexturePath );
	ASSERT( m_pTexture != NULL );

	m_iLineSpacing = m_pTexture->GetSourceFrameHeight();

	// load character widths
	vector<int> FrameWidths;
	int i;
	// Assume each character is the width of the frame by default.
	for( i=0; i<256; i++ )
		FrameWidths.push_back(m_pTexture->GetSourceFrameWidth());

	{
		/* Iterate over all keys. */
		const IniFile::key *k = ini.GetKey("main");
		if(k)
		{
			for(IniFile::key::const_iterator key = k->begin(); key != k->end(); ++key)
			{
				CString val = key->first;
				CString data = key->second;

				val.MakeUpper();

				/* If val is an integer, it's a width, eg. "10=27". */
				if(IsAnInt(val))
				{
					FrameWidths[atoi(val)] = atoi(data);
					continue;
				}
			}
		}
	}

	ini.GetValueB( "main", "CapitalsOnly", m_bCapitalsOnly );
	int DrawExtraPixelsLeft = 0, DrawExtraPixelsRight = 0;

	ini.GetValueI( "main", "DrawExtraPixelsLeft", DrawExtraPixelsLeft );
	ini.GetValueI( "main", "DrawExtraPixelsRight", DrawExtraPixelsRight );

	int iAddToAllWidths = 0;
	if( ini.GetValueI( "main", "AddToAllWidths", iAddToAllWidths ) )
	{
		for( int i=0; i<256; i++ )
			FrameWidths[i] += iAddToAllWidths;
	}

	float fScaleAllWidthsBy = 0;
	if( ini.GetValueF( "main", "ScaleAllWidthsBy", fScaleAllWidthsBy ) )
	{
		for( int i=0; i<256; i++ )
			FrameWidths[i] = int(roundf( FrameWidths[i] * fScaleAllWidthsBy ));
	}

	ini.GetValueI( "main", "LineSpacing", m_iLineSpacing );

	SetTextureCoords(FrameWidths);
	SetExtraPixels(DrawExtraPixelsLeft, DrawExtraPixelsRight);
}

Font::Font( const CString &sASCIITexturePath )
{
	//LOG->Trace( "Font::LoadFromFontName(%s)", sASCIITexturePath.GetString() );

	Init();

	/* Default to 0..255. */
	for( int i=0; i<256; i++ )
		m_iCharToFrameNo[i] = i;

	// Find .ini widths path from texture path
	CString sDir, sFileName, sExtension;
	splitrelpath( sASCIITexturePath, sDir, sFileName, sExtension );
	const CString sIniPath = sDir + sFileName + ".ini";

	IniFile ini;
	ini.SetPath( sIniPath );
	ini.ReadFile();
	ini.RenameKey("Char Widths", "main");
	Load(sASCIITexturePath, ini);

	if( m_pTexture->GetNumFrames() != 16*16 )
		RageException::Throw( "The font '%s' has only %d frames.  All fonts must have 16*16 frames.", m_sTexturePath.GetString() );
}


Font::Font( const CString &sTexturePath, const CString& sCharacters )
{
	//LOG->Trace( "Font::LoadFromFontName(%s)", sFontFilePath.GetString() );

	Init();

	/* Map characters to frames; we don't actually have an INI. */
	for( int i=0; i<sCharacters.GetLength(); i++ )
	{
		char c = sCharacters[i];
		m_iCharToFrameNo[c] = i;
	}

	/* load texture */
	IniFile ini;
	Load(sTexturePath, ini);

	// sanity check
	if( sCharacters.GetLength() != m_pTexture->GetNumFrames() )
		RageException::Throw( "The image '%s' doesn't have the correct number of frames.  It has %d frames but should have %d frames.",
					m_sTexturePath.GetString(), m_pTexture->GetNumFrames(), sCharacters.GetLength() );
}

void Font::SetTextureCoords(const vector<int> &widths)
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
		glyph g;
		g.left = g.right = 0;

		/* Make a copy of each texture rect, reducing each to the actual dimensions
		 * of the character (most characters don't take a full block). */
		g.rect = *m_pTexture->GetTextureCoordRect(i);;

		float fPixelsToChopOff = m_pTexture->GetSourceFrameWidth() - (float)widths[i];
		float fTexCoordsToChopOff = fPixelsToChopOff / m_pTexture->GetSourceWidth();
		
		g.rect.left  += fTexCoordsToChopOff/2;
		g.rect.right -= fTexCoordsToChopOff/2;

		/* Add normal widths. */
		g.left = 0;
		g.right = g.width = widths[i];

		glyphs.push_back(g);
	}
}

void Font::SetExtraPixels(int DrawExtraPixelsLeft, int DrawExtraPixelsRight)
{
	/* Adjust for DrawExtraPixelsLeft and DrawExtraPixelsRight. */
	for(unsigned i = 0; i < glyphs.size(); ++i)
	{
		int iFrameWidth = m_pTexture->GetSourceFrameWidth();
		int iCharWidth = glyphs[i].width;

		/* Extra pixels to draw to the left and right. */
		int ExtraLeft = min( DrawExtraPixelsLeft, (iFrameWidth-iCharWidth)/2 );
		int ExtraRight = min( DrawExtraPixelsRight, (iFrameWidth-iCharWidth)/2 ) + ExtraLeft;

		/* Move left and expand right. */
		glyphs[i].rect.left -= float(ExtraLeft) / m_pTexture->GetSourceWidth();
		glyphs[i].rect.right += float(ExtraRight) / m_pTexture->GetSourceWidth();
		glyphs[i].left += ExtraLeft;
		glyphs[i].right += ExtraRight;
	}
}

Font::~Font()
{
	if( m_pTexture != NULL )
		TEXTUREMAN->UnloadTexture( m_pTexture );
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

		iLineWidth += glyphs[iFrameNo].width;
	}

	return iLineWidth;
}


