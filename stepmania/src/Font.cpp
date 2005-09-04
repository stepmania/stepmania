#include "global.h"
#include "Font.h"
#include "IniFile.h"

#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"
#include "FontManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "GameManager.h"
#include "FontCharmaps.h"
#include "FontCharAliases.h"

/* Last private-use Unicode character: */
const wchar_t Font::DEFAULT_GLYPH = 0xF8FF;

FontPage::FontPage()
{
	m_pTexture = NULL;
	m_iDrawExtraPixelsLeft = m_iDrawExtraPixelsRight = 0;
}

void FontPage::Load( const FontPageSettings &cfg )
{
	m_sTexturePath = cfg.m_sTexturePath;

	// load texture
	RageTextureID ID( m_sTexturePath );
	if( cfg.m_sTextureHints != "default" )
		ID.AdditionalTextureHints = cfg.m_sTextureHints;
	else
		ID.AdditionalTextureHints = "16bpp";

	m_pTexture = TEXTUREMAN->LoadTexture( ID );
	ASSERT( m_pTexture != NULL );

	// load character widths
	vector<int> aiFrameWidths;

	int default_width = m_pTexture->GetSourceFrameWidth();
	if( cfg.m_iDefaultWidth != -1 )
		default_width = cfg.m_iDefaultWidth;

	// Assume each character is the width of the frame by default.
	for( int i=0; i<m_pTexture->GetNumFrames(); i++ )
	{
		map<int,int>::const_iterator it = cfg.m_mapGlyphWidths.find(i);
		if( it != cfg.m_mapGlyphWidths.end() )
			aiFrameWidths.push_back( it->second );
		else
			aiFrameWidths.push_back( default_width );
	}

	if( cfg.m_iAddToAllWidths )
	{
		for( int i=0; i<m_pTexture->GetNumFrames(); i++ )
			aiFrameWidths[i] += cfg.m_iAddToAllWidths;
	}

	if( cfg.m_fScaleAllWidthsBy != 1 )
	{
		for( int i=0; i<m_pTexture->GetNumFrames(); i++ )
			aiFrameWidths[i] = int(roundf( aiFrameWidths[i] * cfg.m_fScaleAllWidthsBy ));
	}

	m_iCharToGlyphNo = cfg.CharToGlyphNo;

	m_iLineSpacing = cfg.m_iLineSpacing;
	if( m_iLineSpacing == -1 )
		m_iLineSpacing = m_pTexture->GetSourceFrameHeight();

	int iBaseline=0;
	/* If we don't have a top and/or baseline, assume we're centered in the
	 * frame, and that LineSpacing is the total height. */
	iBaseline = cfg.m_iBaseline;
	if( iBaseline == -1 )
	{
		float center = m_pTexture->GetSourceFrameHeight()/2.0f;
		iBaseline = int( center + m_iLineSpacing/2 );
	}

	int iTop = cfg.m_iTop;
	if( iTop == -1 )
	{
		float center = m_pTexture->GetSourceFrameHeight()/2.0f;
		iTop = int( center - m_iLineSpacing/2 );
	}
	m_iHeight = iBaseline - iTop;
	m_iDrawExtraPixelsLeft = cfg.m_iDrawExtraPixelsLeft;
	m_iDrawExtraPixelsRight = cfg.m_iDrawExtraPixelsRight;

	/* Shift the character up so the top will be rendered at the baseline. */
	m_fVshift = (float) -iBaseline;

	SetTextureCoords( aiFrameWidths, cfg.m_iAdvanceExtraPixels );
	SetExtraPixels( cfg.m_iDrawExtraPixelsLeft, cfg.m_iDrawExtraPixelsRight );

//	LOG->Trace("Font %s: height %i, baseline %i ( == top %i)",
//		   m_sTexturePath.c_str(), height, baseline, baseline-height);
}

void FontPage::SetTextureCoords( const vector<int> &widths, int iAdvanceExtraPixels )
{
	for(int i = 0; i < m_pTexture->GetNumFrames(); ++i)
	{
		glyph g;

		g.m_pPage = this;

		/* Make a copy of each texture rect, reducing each to the actual dimensions
		 * of the character (most characters don't take a full block). */
		g.m_TexRect = *m_pTexture->GetTextureCoordRect(i);

		/* Set the width and height to the width and line spacing, respectively. */
		g.m_fWidth = float( widths[i] );
		g.m_fHeight = float(m_pTexture->GetSourceFrameHeight());

		g.m_iHadvance = int(g.m_fWidth) + iAdvanceExtraPixels;

		/* Do the same thing with X.  Do this by changing the actual rendered
		 * m_TexRect, instead of shifting it, so we don't render more than we
		 * need to. */
		g.m_fHshift = 0;
		{
			int iSourcePixelsToChopOff = m_pTexture->GetSourceFrameWidth() - widths[i];
			if( (iSourcePixelsToChopOff % 2) == 1 )
			{
				/* We don't want to chop off an odd number of pixels, since that'll
				 * put our texture coordinates between texels and make things blurrier. 
				 * Note that, since we set m_iHadvance above, this merely expands what
				 * we render; it doesn't advance the cursor further.  So, glyphs
				 * that have an odd width should err to being a pixel offcenter left,
				 * not right. */
				--iSourcePixelsToChopOff;
				++g.m_fWidth;
			}

			const float fTexCoordsToChopOff = iSourcePixelsToChopOff * m_pTexture->GetSourceToTexCoordsRatio();

			g.m_TexRect.left  += fTexCoordsToChopOff/2;
			g.m_TexRect.right -= fTexCoordsToChopOff/2;
		}

		g.m_pTexture = m_pTexture;

		m_aGlyphs.push_back(g);
	}
}

void FontPage::SetExtraPixels( int iDrawExtraPixelsLeft, int iDrawExtraPixelsRight )
{
	/* Hack: do one more than we were asked to; I think a lot of fonts are one
	 * too low. */
	iDrawExtraPixelsRight++;
	iDrawExtraPixelsLeft++;

	if( (iDrawExtraPixelsLeft % 2) == 1 )
		++iDrawExtraPixelsLeft;

	/* Adjust for iDrawExtraPixelsLeft and iDrawExtraPixelsRight. */
	for( unsigned i = 0; i < m_aGlyphs.size(); ++i )
	{
		int iFrameWidth = m_pTexture->GetSourceFrameWidth();
		float fCharWidth = m_aGlyphs[i].m_fWidth;

		/* Extra pixels to draw to the left and right.  We don't have to
		 * worry about alignment here; fCharWidth is always even (by
		 * SetTextureCoords) and iFrameWidth are almost always even. */
		float fExtraLeft = min( float(iDrawExtraPixelsLeft), (iFrameWidth-fCharWidth)/2.0f );
		float fExtraRight = min( float(iDrawExtraPixelsRight), (iFrameWidth-fCharWidth)/2.0f );

		/* Move left and expand right. */
		m_aGlyphs[i].m_TexRect.left -= fExtraLeft * m_pTexture->GetSourceToTexCoordsRatio();
		m_aGlyphs[i].m_TexRect.right += fExtraRight * m_pTexture->GetSourceToTexCoordsRatio();
		m_aGlyphs[i].m_fHshift -= fExtraLeft;
		m_aGlyphs[i].m_fWidth += fExtraLeft + fExtraRight;
	}
}

FontPage::~FontPage()
{
	if( m_pTexture != NULL )
		TEXTUREMAN->UnloadTexture( m_pTexture );
}

int Font::GetLineWidthInSourcePixels( const wstring &szLine ) const
{
	int iLineWidth = 0;
	
	for( unsigned i=0; i<szLine.size(); i++ )
		iLineWidth += GetGlyph(szLine[i]).m_iHadvance;

	if( szLine.size() > 0 )
	{
		/* Add overdraw. */
		iLineWidth += GetGlyph(szLine[0]).m_pPage->m_iDrawExtraPixelsLeft;
		iLineWidth += GetGlyph(szLine[szLine.size()-1]).m_pPage->m_iDrawExtraPixelsRight;
	}

	return iLineWidth;
}

int Font::GetLineHeightInSourcePixels( const wstring &szLine ) const
{
	int iLineHeight = 0;

	/* The height of a line is the height of its tallest used font page. */
	for( unsigned i=0; i<szLine.size(); i++ )
		iLineHeight = max( iLineHeight, GetGlyph(szLine[i]).m_pPage->m_iHeight );

	return iLineHeight;
}


Font::Font()
{
	//LOG->Trace( "Font::LoadFromFontName(%s)", sASCIITexturePath.c_str() );

	m_iRefCount = 1;
	m_pDefault = NULL;
}

Font::~Font()
{
	Unload();
}

void Font::Unload()
{
	for( unsigned i = 0; i < m_apPages.size(); ++i )
		delete m_apPages[i];
	m_apPages.clear();
	
	m_iCharToGlyph.clear();
	m_pDefault = NULL;

	/* Don't clear the refcount.  We've unloaded, but that doesn't mean things
	 * aren't still pointing to us. */
}

void Font::Reload()
{
	Unload();
	ASSERT( !path.empty() );
	Load( path, m_sChars );
}


void Font::AddPage( FontPage *m_pPage )
{
	m_apPages.push_back( m_pPage );

	for( map<longchar,int>::const_iterator it = m_pPage->m_iCharToGlyphNo.begin();
		it != m_pPage->m_iCharToGlyphNo.end(); ++it )
	{
		m_iCharToGlyph[it->first] = &m_pPage->m_aGlyphs[it->second];
	}
}

void Font::MergeFont(Font &f)
{
	/* If we don't have a font page yet, and f does, grab the default font
	 * page.  It'll usually be overridden later on by one of our own font
	 * pages; this will be used only if we don't have any font pages at
	 * all. */
	if( m_pDefault == NULL )
		m_pDefault = f.m_pDefault;

	for(map<longchar,glyph*>::iterator it = f.m_iCharToGlyph.begin();
		it != f.m_iCharToGlyph.end(); ++it)
	{
		m_iCharToGlyph[it->first] = it->second;
	}

	m_apPages.insert( m_apPages.end(), f.m_apPages.begin(), f.m_apPages.end() );

	f.m_apPages.clear();
}

const glyph &Font::GetGlyph( wchar_t c ) const
{
	ASSERT(c >= 0 && c <= 0xFFFFFF);

	/* Fast path: */
	if( c < (int) ARRAYSIZE(m_iCharToGlyphCache) && m_iCharToGlyphCache[c] )
		return *m_iCharToGlyphCache[c];

	/* Try the regular character. */
	map<longchar,glyph*>::const_iterator it = m_iCharToGlyph.find(c);

	/* If that's missing, use the default glyph. */
	if(it == m_iCharToGlyph.end()) it = m_iCharToGlyph.find(DEFAULT_GLYPH);

	if(it == m_iCharToGlyph.end()) 
		RageException::Throw( "The default glyph is missing from the font '%s'", path.c_str() );
	
	return *it->second;
}

bool Font::FontCompleteForString( const wstring &str ) const
{
	map<longchar,glyph*>::const_iterator m_pDefault = m_iCharToGlyph.find( DEFAULT_GLYPH );
	if( m_pDefault == m_iCharToGlyph.end() )
		RageException::Throw( "The default glyph is missing from the font '%s'", path.c_str() );

	for( unsigned i = 0; i < str.size(); ++i )
	{
		/* If the glyph for this character is the default glyph, we're incomplete. */
		const glyph &g = GetGlyph( str[i] );
		if( &g == m_pDefault->second )
			return false;
	}
	return true;
}

void Font::CapsOnly()
{
	/* For each uppercase character that we have a mapping for, add
     * a lowercase one. */
	for(char c = 'A'; c <= 'Z'; ++c)
	{
		map<longchar,glyph*>::const_iterator it = m_iCharToGlyph.find(c);

		if(it == m_iCharToGlyph.end())
			continue;

		m_iCharToGlyph[(char) tolower(c)] = it->second;
	}
}

void Font::SetDefaultGlyph( FontPage *pPage )
{
	ASSERT( pPage );
	ASSERT( !pPage->m_aGlyphs.empty() );
	m_pDefault = pPage;
}


/* Given the INI for a font, find all of the texture pages for the font. */
void Font::GetFontPaths( const CString &sFontIniPath, CStringArray &asTexturePathsOut )
{
	CString sPrefix = SetExtension( sFontIniPath, "" );
	CStringArray asFiles;
	GetDirListing( sPrefix + "*", asFiles, false, true );

	for( unsigned i = 0; i < asFiles.size(); ++i )
	{
		if( asFiles[i].Right(4).CompareNoCase(".ini"))  
			asTexturePathsOut.push_back( asFiles[i] );
	}
}

CString Font::GetPageNameFromFileName( const CString &sFilename )
{
	size_t begin = sFilename.find_first_of( '[' );
	if( begin == string::npos )
		return "main";

	size_t end = sFilename.find_first_of( ']', begin );
	if( end == string::npos )
		return "main";

	begin++;
	end--;
	if( end == begin )
		return "main";
	return sFilename.substr( begin, end-begin+1 );
}

void Font::LoadFontPageSettings( FontPageSettings &cfg, IniFile &ini, const CString &sTexturePath, const CString &sPageName, CString sChars )
{
	cfg.m_sTexturePath = sTexturePath;

	/* If we have any characters to map, add them. */
	for( unsigned n=0; n<sChars.size(); n++ )
	{
		char c = sChars[n];
		cfg.CharToGlyphNo[c] = n;
	}
	int iNumFramesWide, iNumFramesHigh;
	RageTexture::GetFrameDimensionsFromFileName( sTexturePath, &iNumFramesWide, &iNumFramesHigh );
	int iNumFrames = iNumFramesWide * iNumFramesHigh;
	
	ini.RenameKey("Char Widths", "main");

//	LOG->Trace("Loading font page '%s' settings from page name '%s'",
//		TexturePath.c_str(), sPageName.c_str());
	
	ini.GetValue( sPageName, "DrawExtraPixelsLeft", cfg.m_iDrawExtraPixelsLeft );
	ini.GetValue( sPageName, "DrawExtraPixelsRight", cfg.m_iDrawExtraPixelsRight );
	ini.GetValue( sPageName, "AddToAllWidths", cfg.m_iAddToAllWidths );
	ini.GetValue( sPageName, "ScaleAllWidthsBy", cfg.m_fScaleAllWidthsBy );
	ini.GetValue( sPageName, "LineSpacing", cfg.m_iLineSpacing );
	ini.GetValue( sPageName, "Top", cfg.m_iTop );
	ini.GetValue( sPageName, "Baseline", cfg.m_iBaseline );
	ini.GetValue( sPageName, "DefaultWidth", cfg.m_iDefaultWidth );
	ini.GetValue( sPageName, "AdvanceExtraPixels", cfg.m_iAdvanceExtraPixels );
	ini.GetValue( sPageName, "TextureHints", cfg.m_sTextureHints );

	/* Iterate over all keys. */
	const XNode* pNode = ini.GetChild( sPageName );
	if( pNode )
	{
		FOREACH_CONST_Attr( pNode, pAttr )
		{
			CString sName = pAttr->m_sName;
			const CString &sValue = pAttr->m_sValue;

			sName.MakeUpper();

			/* If val is an integer, it's a width, eg. "10=27". */
			if( IsAnInt(sName) )
			{
				cfg.m_mapGlyphWidths[atoi(sName)] = atoi( sValue );
				continue;
			}

			/* "map codepoint=frame" maps a char to a frame. */
			if( sName.substr(0, 4) == "MAP " )
			{
				/*
				 * map CODEPOINT=frame. CODEPOINT can be
				 * 1. U+hexval
				 * 2. an alias ("oq")
				 * 3. a game type followed by a game alias, eg "pump menuleft"
				 * 4. a character in quotes ("X")
				 *
				 * map 1=2 is the same as
				 * range unicode #1-1=2
				 */
				CString sCodepoint = sName.substr(4); /* "CODEPOINT" */
			
				const Game* pGame = NULL;

				if( sCodepoint.find_first_of(' ') != sCodepoint.npos )
				{
					/* There's a space; the first word should be a game type. Split it. */
					unsigned pos = sCodepoint.find_first_of( ' ' );
					CString gamename = sCodepoint.substr( 0, pos );
					sCodepoint = sCodepoint.substr( pos+1 );

					pGame = GameManager::StringToGameType(gamename);

					if( pGame == NULL )
					{
						LOG->Warn( "Font definition '%s' uses unknown game type '%s'",
							ini.GetPath().c_str(), gamename.c_str() );
						continue;
					}
				}

				wchar_t c;
				if( sCodepoint.substr(0, 2) == "U+" && IsHexVal(sCodepoint.substr(2)) )
					sscanf( sCodepoint.substr(2).c_str(), "%x", &c );
				else if( sCodepoint.size() > 0 &&
						utf8_get_char_len(sCodepoint[0]) == int(sCodepoint.size()) )
				{
					c = utf8_get_char( sCodepoint.c_str() );
					if(c == wchar_t(-1))
						LOG->Warn("Font definition '%s' has an invalid value '%s'.",
							ini.GetPath().c_str(), sName.c_str() );
				}
				else if( !FontCharAliases::GetChar(sCodepoint, c) )
				{
					LOG->Warn("Font definition '%s' has an invalid value '%s'.",
						ini.GetPath().c_str(), sName.c_str() );
					continue;
				}

				cfg.CharToGlyphNo[c] = atoi( sValue );

				continue;
			}

			if( sName.substr(0, 6) == "RANGE " )
			{
				/*
				 * range CODESET=first_frame or
				 * range CODESET #start-end=first_frame
				 * eg
				 * range CP1252=0       (default for 256-frame fonts)
				 * range ASCII=0        (default for 128-frame fonts)
				 *
				 * (Start and end are in hex.)
				 *
				 * Map two high-bit portions of ISO-8859- to one font:
				 * range ISO-8859-2 #80-FF=0
				 * range ISO-8859-3 #80-FF=128
				 *
				 * Map hiragana to 0-84:
				 * range Unicode #3041-3094=0
				 */
				vector<CString> asMatches;
				static Regex parse("^RANGE ([A-Z\\-]+)( ?#([0-9A-F]+)-([0-9A-F]+))?$");
				bool bMatch = parse.Compare( sName, asMatches );
				
				ASSERT( asMatches.size() == 4 ); /* 4 parens */

				if( !bMatch || asMatches[0].empty() )
					RageException::Throw("Font definition '%s' has an invalid range '%s': parse error",
						ini.GetPath().c_str(), sName.c_str() );
				
				/* We must have either 1 match (just the codeset) or 4 (the whole thing). */

				int iCount = -1;
				int iFirst = 0;
				if( !asMatches[2].empty() )
				{
					sscanf( asMatches[2].c_str(), "%x", &iFirst );
					int iLast;
					sscanf( asMatches[3].c_str(), "%x", &iLast );
					if( iLast < iFirst )
						RageException::Throw("Font definition '%s' has an invalid range '%s': %i < %i.",
							ini.GetPath().c_str(), sName.c_str(), iLast < iFirst );

					iCount = iLast - iFirst + 1;
				}

				CString sRet = cfg.MapRange( asMatches[0], iFirst, atoi(sValue), iCount );
				if( !sRet.empty() )
					RageException::Throw( "Font definition '%s' has an invalid range '%s': %s.",
						ini.GetPath().c_str(), sName.c_str(), sRet.c_str() );

				continue;
			}

			if( sName.substr(0, 5) == "LINE " )
			{
				/* line ROW=CHAR1CHAR2CHAR3CHAR4
				 * eg.
				 * line 0=ABCDEFGH
				 *
				 * This lets us assign characters very compactly and readably. */

				CString sRowStr = sName.substr(5);
				ASSERT( IsAnInt(sRowStr) );
				const int iRow = atoi( sRowStr.c_str() );
				const int iFirstFrame = iRow * iNumFramesWide;

				if( iRow > iNumFramesHigh )
					RageException::Throw( "The font definition \"%s\" tries to assign line %i, but the font is only %i characters high",
						ini.GetPath().c_str(), iFirstFrame, iNumFramesHigh );

				/* Decode the string. */
				const wstring wdata( CStringToWstring(sValue) );

				if( int(wdata.size()) > iNumFramesWide )
					RageException::Throw( "The font definition \"%s\" assigns %i characters to row %i (\"%ls\"), but the font only has %i characters wide",
						ini.GetPath().c_str(), wdata.size(), iRow, wdata.c_str(), iNumFramesWide );

				for( unsigned i = 0; i < wdata.size(); ++i )
					cfg.CharToGlyphNo[wdata[i]] = iFirstFrame+i;
			}
		}
	}

	/* If it's 128 or 256 frames, default to ASCII or CP1252,
	 * respectively.  If it's anything else, we don't know what it
	 * is, so don't make any default mappings (the INI needs to do
	 * it itself). */
	if( sPageName != "common" && cfg.CharToGlyphNo.empty() )
	{
		if( iNumFrames == 128 )
			cfg.MapRange( "ascii", 0, 0, -1 );
		else if( iNumFrames == 256 )
			cfg.MapRange( "cp1252", 0, 0, -1 );
		else if( iNumFrames == 15 )
			cfg.MapRange( "numbers", 0, 0, -1 );
		else
			LOG->Trace( "Font page \"%s\" has no characters", sTexturePath.c_str() );
	}

	/* If ' ' is set and nbsp is not, set nbsp. */
	if( cfg.CharToGlyphNo.find(' ') != cfg.CharToGlyphNo.end() )
		cfg.CharToGlyphNo[0x00A0] = cfg.CharToGlyphNo[' '];
}

CString FontPageSettings::MapRange( CString sMapping, int iMapOffset, int iGlyphNo, int iCount )
{
	if( !sMapping.CompareNoCase("Unicode") )
	{
		/* Special case. */
		if( iCount == -1 )
			return "Can't map all of Unicode to one font page"; /* don't do that */

		/* What's a practical limit?  A 2048x2048 texture could contain 16x16 characters,
		 * which is 16384 glyphs.  (Use a grayscale map and that's only 4 megs.)  Let's use
		 * that as a cap.  (We don't want to go crazy if someone says "range Unicode
		 * #0-FFFFFFFF".) */
		if( iCount > 16384 )
			return ssprintf( "Can't map %i glyphs to one font page", iCount );

		while( iCount )
		{
			CharToGlyphNo[iMapOffset] = iGlyphNo;
			iMapOffset++;
			iGlyphNo++;
			iCount--;
		}

		return CString();
	}

	const wchar_t *pMapping = FontCharmaps::get_char_map( sMapping );
	if( pMapping == NULL )
		return "Unknown mapping";

	while( *pMapping != 0 && iMapOffset )
	{
		pMapping++;
		--iMapOffset;
	}
	if( iMapOffset )
		return "Map overflow"; /* there aren't enough characters in the map */

	/* If iCount is -1, set it to the number of characters in the map. */
	if( iCount == -1 )
		for( iCount = 0; pMapping[iCount] != 0; ++iCount ) ;

	while( *pMapping != 0 )
	{
		if( *pMapping != FontCharmaps::M_SKIP )
			CharToGlyphNo[*pMapping] = iGlyphNo;
		pMapping++;
		iGlyphNo++;
		iCount--;
	}

	if( iCount )
		return "Map overflow"; /* there aren't enough characters in the map */

	return CString();
}

static CStringArray LoadStack;

/*
 * A font set is a set of files, eg:
 *
 * Normal 16x16.png
 * Normal [other] 16x16.png
 * Normal [more] 8x8.png
 * Normal 16x16.ini           (the 16x16 here is optional)
 *
 * One INI and at least one texture is required.
 *
 * The entire font can be redirected; that's handled in ThemeManager.
 * Individual font files can not be redirected.
 *
 * If a file has no characters and sChars is not set, it will receive a default
 * mapping of ASCII or ISO-8859-1 if the font has exactly 128 or 256 frames.
 * However, if it doesn't, we don't know what it is and the font will receive
 * no default mapping.  A font isn't useful with no characters mapped.
 */
void Font::Load( const CString &sIniPath, CString sChars )
{
	ASSERT_M( !GetExtension(sIniPath).CompareNoCase("ini"), sIniPath );

	/* Check for recursion (recursive imports). */
	{
		for(unsigned i = 0; i < LoadStack.size(); ++i)
		{
			if( LoadStack[i] == sIniPath )
			{
				CString str = join("\n", LoadStack);
				str += "\n" + sIniPath;
				RageException::Throw("Font import recursion detected\n%s", str.c_str());
			}
		}
		LoadStack.push_back( sIniPath );
	}

	/* The font is not already loaded.  Figure out what we have. */
	CHECKPOINT_M( ssprintf("Font::Load(\"%s\",\"%s\").", sIniPath.c_str(), m_sChars.c_str()) );

	path = sIniPath;
	m_sChars = sChars;

	/* Get the filenames associated with this font. */
	CStringArray asTexturePaths;
	GetFontPaths( sIniPath, asTexturePaths );

	bool bCapitalsOnly = false;

	/* If we have an INI, load it. */
	IniFile ini;
	if( !sIniPath.empty() )
	{
		ini.ReadFile( sIniPath );
		ini.RenameKey("Char Widths", "main");
		ini.GetValue( "main", "CapitalsOnly", bCapitalsOnly );
	}

	{
		/* If this is a top-level font (not a subfont), load the default font first. */
		CStringArray ImportList;
		if( LoadStack.size() == 1 )
			ImportList.push_back("Common default");

		/* Check to see if we need to import any other fonts.  Do this
		 * before loading this font, so any characters in this font
		 * override imported characters. */
		CString imports;
		ini.GetValue( "main", "import", imports );
		split(imports, ",", ImportList, true);
		for(unsigned i = 0; i < ImportList.size(); ++i)
		{
			CString path = THEME->GetPathF( "", ImportList[i], true );
			if( path == "" )
			{
				LOG->Warn("Font \"%s\" imports a font \"%s\" that doesn't exist", sIniPath.c_str(), ImportList[i].c_str());
				continue;
			}

			Font subfont;
			subfont.Load(path, "");
			MergeFont(subfont);
		}
	}

	/* Load each font page. */
	for(unsigned i = 0; i < asTexturePaths.size(); ++i)
	{
		const CString &sTexturePath = asTexturePaths[i];

		FontPage *pPage = new FontPage;

		/* Grab the page name, eg "foo" from "Normal [foo].png". */
		CString sPagename = GetPageNameFromFileName( sTexturePath );

		/* Load settings for this page from the INI. */
		FontPageSettings cfg;
		LoadFontPageSettings( cfg, ini, sTexturePath, "common", sChars );
		LoadFontPageSettings( cfg, ini, sTexturePath, sPagename, sChars );

		/* Go. */
		pPage->Load( cfg );

		/* Expect at least as many frames as we have premapped characters. */
		/* Make sure that we don't map characters to frames we don't actually
		 * have.  This can happen if the font is too small for an sChars. */
		for( map<longchar,int>::const_iterator it = pPage->m_iCharToGlyphNo.begin();
			it != pPage->m_iCharToGlyphNo.end(); ++it )
		{
			if( it->second < pPage->m_pTexture->GetNumFrames() )
				continue; /* OK */
			CString sError = ssprintf( "The font '%s' maps %s to frame %i, but the font only has %i frames.",
				sTexturePath.c_str(), WcharDisplayText(wchar_t(it->first)).c_str(), it->second, pPage->m_pTexture->GetNumFrames() );
			RageException::Throw( sError );
		}

//		LOG->Trace( "Adding page %s (%s) to %s; %i glyphs",
//			TexturePaths[i].c_str(), sPagename.c_str(),
//			sFontOrTextureFilePath.c_str(), pPage->m_iCharToGlyphNo.size() );
		AddPage( pPage );

		/* If this is the first font loaded, or it's called "main", this page's
		 * properties become the font's properties. */
		if( i == 0 || sPagename == "main" )
			SetDefaultGlyph( pPage );
	}

	if( bCapitalsOnly )
		CapsOnly();

	if( m_iCharToGlyph.empty() )
		LOG->Warn( "Font %s has no characters", sIniPath.c_str() );

	LoadStack.pop_back();

	if( LoadStack.empty() )
	{
		/* Cache ASCII glyphs. */
		ZERO( m_iCharToGlyphCache );
		map<longchar,glyph*>::iterator it;
		for( it = m_iCharToGlyph.begin(); it != m_iCharToGlyph.end(); ++it )
			if( it->first < (int) ARRAYSIZE(m_iCharToGlyphCache) )
				m_iCharToGlyphCache[it->first] = it->second;
	}
}

/*
 * (c) 2001-2004 Glenn Maynard, Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
