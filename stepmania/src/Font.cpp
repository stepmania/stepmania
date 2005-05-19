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
	DrawExtraPixelsLeft = DrawExtraPixelsRight = 0;
}

void FontPage::Load( FontPageSettings cfg )
{
	m_sTexturePath = cfg.TexturePath;

	// load texture
	RageTextureID ID(m_sTexturePath);
	if( cfg.TextureHints != "default" )
		ID.AdditionalTextureHints = cfg.TextureHints;
	else
		ID.AdditionalTextureHints = "16bpp";

	m_pTexture = TEXTUREMAN->LoadTexture( ID );
	ASSERT( m_pTexture != NULL );

	// load character widths
	vector<int> FrameWidths;

	int default_width = m_pTexture->GetSourceFrameWidth();
	if(cfg.DefaultWidth != -1)
		default_width = cfg.DefaultWidth;

	// Assume each character is the width of the frame by default.
	for( int i=0; i<m_pTexture->GetNumFrames(); i++ )
	{
		map<int,int>::const_iterator it = cfg.GlyphWidths.find(i);
		if(it != cfg.GlyphWidths.end())
		{
			FrameWidths.push_back(it->second);
		} else {
			FrameWidths.push_back(default_width);
		}
	}

	if( cfg.AddToAllWidths )
	{
		for( int i=0; i<m_pTexture->GetNumFrames(); i++ )
			FrameWidths[i] += cfg.AddToAllWidths;
	}

	if( cfg.ScaleAllWidthsBy != 1 )
	{
		for( int i=0; i<m_pTexture->GetNumFrames(); i++ )
			FrameWidths[i] = int(roundf( FrameWidths[i] * cfg.ScaleAllWidthsBy ));
	}

	m_iCharToGlyphNo = cfg.CharToGlyphNo;

	LineSpacing = cfg.LineSpacing;
	if(LineSpacing == -1)
		LineSpacing = m_pTexture->GetSourceFrameHeight();

	int baseline=0;
	/* If we don't have a top and/or baseline, assume we're centered in the
	 * frame, and that LineSpacing is the total height. */
	if(cfg.Baseline == -1)
	{
		float center = m_pTexture->GetSourceFrameHeight()/2.0f;
		cfg.Baseline = int(center + LineSpacing/2);
	}
	if(cfg.Top == -1)
	{
		float center = m_pTexture->GetSourceFrameHeight()/2.0f;
		cfg.Top = int(center - LineSpacing/2);
	}
	baseline = cfg.Baseline;
	height = baseline-cfg.Top;
	DrawExtraPixelsLeft = cfg.DrawExtraPixelsLeft;
	DrawExtraPixelsRight = cfg.DrawExtraPixelsRight;

	/* Shift the character up so the top will be rendered at the baseline. */
	vshift = (float) -baseline;

	SetTextureCoords(FrameWidths, cfg.AdvanceExtraPixels);
	SetExtraPixels(cfg.DrawExtraPixelsLeft, cfg.DrawExtraPixelsRight);

//	LOG->Trace("Font %s: height %i, baseline %i ( == top %i)",
//		   m_sTexturePath.c_str(), height, baseline, baseline-height);
}

void FontPage::SetTextureCoords(const vector<int> &widths, int AdvanceExtraPixels)
{
	for(int i = 0; i < m_pTexture->GetNumFrames(); ++i)
	{
		glyph g;

		g.fp = this;

		/* Make a copy of each texture rect, reducing each to the actual dimensions
		 * of the character (most characters don't take a full block). */
		g.rect = *m_pTexture->GetTextureCoordRect(i);;

		/* Set the width and height to the width and line spacing, respectively. */
		g.width = float(widths[i]);
		g.height = float(m_pTexture->GetSourceFrameHeight());

		g.hadvance = int(g.width) + AdvanceExtraPixels;

		/* Do the same thing with X.  Do this by changing the actual rendered
		 * rect, instead of shifting it, so we don't render more than we need to. */
		g.hshift = 0;
		{
			int iSourcePixelsToChopOff = m_pTexture->GetSourceFrameWidth() - widths[i];
			if((iSourcePixelsToChopOff % 2) == 1)
			{
				/* We don't want to chop off an odd number of pixels, since that'll
				 * put our texture coordinates between texels and make things blurrier. 
				 * Note that, since we set hadvance above, this merely expands what
				 * we render; it doesn't advance the cursor further.  So, glyphs
				 * that have an odd width should err to being a pixel offcenter left,
				 * not right. */
				iSourcePixelsToChopOff--;
				g.width++;
			}

			const float fTexCoordsToChopOff = iSourcePixelsToChopOff * m_pTexture->GetSourceToTexCoordsRatio();

			g.rect.left  += fTexCoordsToChopOff/2;
			g.rect.right -= fTexCoordsToChopOff/2;
		}

		g.Texture = m_pTexture;

		glyphs.push_back(g);
	}
}

void FontPage::SetExtraPixels(int DrawExtraPixelsLeft, int DrawExtraPixelsRight)
{
	/* Hack: do one more than we were asked to; I think a lot of fonts are one
	 * too low. */
	DrawExtraPixelsRight++;
	DrawExtraPixelsLeft++;

	if((DrawExtraPixelsLeft % 2) == 1)
		DrawExtraPixelsLeft++;

	/* Adjust for DrawExtraPixelsLeft and DrawExtraPixelsRight. */
	for(unsigned i = 0; i < glyphs.size(); ++i)
	{
		int iFrameWidth = m_pTexture->GetSourceFrameWidth();
		float iCharWidth = glyphs[i].width;

		/* Extra pixels to draw to the left and right.  We don't have to
		 * worry about alignment here; CharWidth is always even (by
		 * SetTextureCoords) and iFrameWidth are almost always even. */
		float ExtraLeft = min( float(DrawExtraPixelsLeft), (iFrameWidth-iCharWidth)/2.0f );
		float ExtraRight = min( float(DrawExtraPixelsRight), (iFrameWidth-iCharWidth)/2.0f );

		/* Move left and expand right. */
		glyphs[i].rect.left -= ExtraLeft * m_pTexture->GetSourceToTexCoordsRatio();
		glyphs[i].rect.right += ExtraRight * m_pTexture->GetSourceToTexCoordsRatio();
		glyphs[i].hshift -= ExtraLeft;
		glyphs[i].width += ExtraLeft + ExtraRight;
	}
}

FontPage::~FontPage()
{
	if( m_pTexture != NULL )
		TEXTUREMAN->UnloadTexture( m_pTexture );
}

int Font::GetLineWidthInSourcePixels( const wstring &szLine ) const
{
	int LineWidth = 0;
	
	for( unsigned i=0; i<szLine.size(); i++ )
		LineWidth += GetGlyph(szLine[i]).hadvance;

	if( szLine.size() > 0 )
	{
		/* Add overdraw. */
		LineWidth += GetGlyph(szLine[0]).fp->DrawExtraPixelsLeft;
		LineWidth += GetGlyph(szLine[szLine.size()-1]).fp->DrawExtraPixelsRight;
	}

	return LineWidth;
}

int Font::GetLineHeightInSourcePixels( const wstring &szLine ) const
{
	int iLineHeight = 0;

	/* The height of a line is the height of its tallest used font page. */
	for( unsigned i=0; i<szLine.size(); i++ )
		iLineHeight = max(iLineHeight, GetGlyph(szLine[i]).fp->height);

	return iLineHeight;
}


Font::Font()
{
	//LOG->Trace( "Font::LoadFromFontName(%s)", sASCIITexturePath.c_str() );

	m_iRefCount = 1;
	def = NULL;
}

Font::~Font()
{
	Unload();
}

void Font::Unload()
{
	for(unsigned i = 0; i < pages.size(); ++i)
		delete pages[i];
	pages.clear();
	
	m_iCharToGlyph.clear();
	def = NULL;

	/* Don't clear the refcount.  We've unloaded, but that doesn't mean things
	 * aren't still pointing to us. */
}

void Font::Reload()
{
	Unload();
	ASSERT(!path.empty());
	Load(path, Chars);
}


void Font::AddPage(FontPage *fp)
{
	pages.push_back(fp);

	for(map<longchar,int>::const_iterator it = fp->m_iCharToGlyphNo.begin();
		it != fp->m_iCharToGlyphNo.end(); ++it)
	{
		m_iCharToGlyph[it->first] = &fp->glyphs[it->second];
	}
}

void Font::MergeFont(Font &f)
{
	/* If we don't have a font page yet, and f does, grab the default font
	 * page.  It'll usually be overridden later on by one of our own font
	 * pages; this will be used only if we don't have any font pages at
	 * all. */
	if(def == NULL)
		def = f.def;

	for(map<longchar,glyph*>::iterator it = f.m_iCharToGlyph.begin();
		it != f.m_iCharToGlyph.end(); ++it)
	{
		m_iCharToGlyph[it->first] = it->second;
	}

	pages.insert(pages.end(), f.pages.begin(), f.pages.end());

	f.pages.clear();
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
	map<longchar,glyph*>::const_iterator def = m_iCharToGlyph.find(DEFAULT_GLYPH);
	if(def == m_iCharToGlyph.end()) 
		RageException::Throw( "The default glyph is missing from the font '%s'", path.c_str() );

	for(unsigned i = 0; i < str.size(); ++i)
	{
		/* If the glyph for this character is the default glyph, we're incomplete. */
		const glyph &g = GetGlyph(str[i]);
		if(&g == def->second)
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

void Font::SetDefaultGlyph(FontPage *fp)
{
	ASSERT(fp);
	ASSERT(!fp->glyphs.empty());
	def = fp;
}


CString Font::GetFontName(CString FileName)
{
	CString orig = FileName;

	CString sDir, sFName, sExt;
	splitpath( FileName, sDir, sFName, sExt );
	FileName = sFName;

	/* If it ends in an extension, remove it. */
	static Regex drop_ext("\\....$");
	if(drop_ext.Compare(FileName))
		FileName.erase(FileName.size()-4);

	/* If it ends in a resolution spec, remove it. */
	CStringArray mat;
	static Regex ResSpec("( \\(res [0-9]+x[0-9]+\\))$");
	if(ResSpec.Compare(FileName, mat))
		FileName.erase(FileName.size()-mat[0].size());

	/* If it ends in a dimension spec, remove it. */
	static Regex DimSpec("( [0-9]+x[0-9]+)$");
	if(DimSpec.Compare(FileName, mat))
		FileName.erase(FileName.size()-mat[0].size());

	/* If it ends in texture hints, remove them. */
	static Regex Hints("( \\([^\\)]+\\))$");
	if(Hints.Compare(FileName, mat))
		FileName.erase(FileName.size()-mat[0].size());

	/* If it ends in a page name, remove it. */
	static Regex PageName("( \\[.+\\])$");
	if(PageName.Compare(FileName, mat))
		FileName.erase(FileName.size()-mat[0].size());

	TrimRight(FileName);

	if(FileName.empty())
		RageException::Throw("Can't parse font filename \"%s\"", orig.c_str());

	FileName.MakeLower();
	return FileName;
}


void Font::WeedFontNames(vector<CString> &v, const CString &FileName)
{
	CString FontName = Font::GetFontName(FileName);
	/* Weed out false matches.  (For example, this gets rid of "normal2" when
	 * we're really looking for "normal".) */
	for(unsigned i = 0; i < v.size(); ) {
		if(FontName.CompareNoCase(Font::GetFontName(v[i])))
			v.erase(v.begin()+i);
		else i++;
	}
}

/* Given a file in a font, find all of the files for the font.
 * 
 * Possibilities:
 *
 * Normal 16x16.png
 * Normal [other] 16x16.png
 * Normal [more] 8x8.png
 * Normal 16x16.ini
 * Normal.ini
 *
 * Any of the above should find all of the above.  Allow the
 * extension to be omitted. */
void Font::GetFontPaths(const CString &sFontOrTextureFilePath,
							   CStringArray &TexturePaths, CString &IniPath)
{
	CString sDir, sFName, sExt;
	splitpath( sFontOrTextureFilePath, sDir, sFName, sExt );

	/* Don't give us a redir; resolve those before sending them here. */
	ASSERT(sExt.CompareNoCase("redir"));

	/* sFName can't be empty, or we don't know what to search for. */
	ASSERT(!sFName.empty());

	CString FontName = GetFontName(sFName);

	CStringArray Files;
	GetDirListing( sDir+FontName + "*", Files, false, false );

	for(unsigned i = 0; i < Files.size(); ++i)
	{
		/* We now have a list of possibilities, but it may include false positives,
		 * such as "Normal2" when the font name is "Normal".  Weed them. */
		if(GetFontName(Files[i]).CompareNoCase(FontName))
			continue;

		/* If it's an INI, and we don't already have an INI, use it. */
		if(!Files[i].Right(4).CompareNoCase(".ini")) 
		{
			if(!IniPath.empty())
				RageException::Throw("More than one INI found\n%s\n%s", IniPath.c_str(), Files[i].c_str());
			
			IniPath = sDir+Files[i];
			continue;
		}

		TexturePaths.push_back(sDir+Files[i]);
	}
}

CString Font::GetPageNameFromFileName(const CString &fn)
{
	size_t begin = fn.find_first_of('[');
	if(begin == fn.npos) return "main";
	size_t end = fn.find_first_of(']', begin);
	if(end == fn.npos) return "main";
	begin++; end--;
	if(end == begin) return "main";
	return fn.substr(begin, end-begin+1);
}

void Font::LoadFontPageSettings(FontPageSettings &cfg, IniFile &ini, const CString &TexturePath, const CString &PageName, CString sChars)
{
	cfg.TexturePath = TexturePath;

	/* If we have any characters to map, add them. */
	for( unsigned n=0; n<sChars.size(); n++ )
	{
		char c = sChars[n];
		cfg.CharToGlyphNo[c] = n;
	}
	int NumFramesWide, NumFramesHigh;
	RageTexture::GetFrameDimensionsFromFileName(TexturePath, &NumFramesWide, &NumFramesHigh);
	int NumFrames = NumFramesWide * NumFramesHigh;
	
	ini.RenameKey("Char Widths", "main");

//	LOG->Trace("Loading font page '%s' settings from page name '%s'",
//		TexturePath.c_str(), PageName.c_str());
	
	ini.GetValue( PageName, "DrawExtraPixelsLeft", cfg.DrawExtraPixelsLeft );
	ini.GetValue( PageName, "DrawExtraPixelsRight", cfg.DrawExtraPixelsRight );
	ini.GetValue( PageName, "AddToAllWidths", cfg.AddToAllWidths );
	ini.GetValue( PageName, "ScaleAllWidthsBy", cfg.ScaleAllWidthsBy );
	ini.GetValue( PageName, "LineSpacing", cfg.LineSpacing );
	ini.GetValue( PageName, "Top", cfg.Top );
	ini.GetValue( PageName, "Baseline", cfg.Baseline );
	ini.GetValue( PageName, "DefaultWidth", cfg.DefaultWidth );
	ini.GetValue( PageName, "AdvanceExtraPixels", cfg.AdvanceExtraPixels );
	ini.GetValue( PageName, "TextureHints", cfg.TextureHints );

	/* Iterate over all keys. */
	const XNode* pNode = ini.GetChild( PageName );
	if( pNode )
	{
		FOREACH_CONST_Attr( pNode, pAttr )
		{
			CString name = pAttr->m_sName;
			const CString &value = pAttr->m_sValue;

			name.MakeUpper();

			/* If val is an integer, it's a width, eg. "10=27". */
			if(IsAnInt(name))
			{
				cfg.GlyphWidths[atoi(name)] = atoi(value);
				continue;
			}

			/* "map codepoint=frame" maps a char to a frame. */
			if(name.substr(0, 4) == "MAP ")
			{
				/* map CODEPOINT=frame. CODEPOINT can be
				 * 1. U+hexval
				 * 2. an alias ("oq")
				 * 3. a game type followed by a game alias, eg "pump menuleft"
				 * 4. a character in quotes ("X")
				 *
				 * map 1=2 is the same as
				 * range unicode #1-1=2
				 */
				CString codepoint = name.substr(4); /* "CODEPOINT" */
			
				const Game* pGame = NULL;

				if(codepoint.find_first_of(' ') != codepoint.npos)
				{
					/* There's a space; the first word should be a game type. Split it. */
					unsigned pos = codepoint.find_first_of(' ');
					CString gamename = codepoint.substr(0, pos);
					codepoint = codepoint.substr(pos+1);

					pGame = GameManager::StringToGameType(gamename);

					if(pGame == NULL)
					{
						LOG->Warn( "Font definition '%s' uses unknown game type '%s'",
							ini.GetPath().c_str(), gamename.c_str() );
						continue;
					}
				}

				wchar_t c;
				if(codepoint.substr(0, 2) == "U+" && IsHexVal(codepoint.substr(2)))
					sscanf(codepoint.substr(2).c_str(), "%x", &c);
				else if(codepoint.size() > 0 &&
						utf8_get_char_len(codepoint[0]) == int(codepoint.size()))
				{
					c = utf8_get_char(codepoint.c_str());
					if(c == wchar_t(-1))
						LOG->Warn("Font definition '%s' has an invalid value '%s'.",
							ini.GetPath().c_str(), name.c_str() );
				}
				else if(!FontCharAliases::GetChar(codepoint, c))
				{
					LOG->Warn("Font definition '%s' has an invalid value '%s'.",
						ini.GetPath().c_str(), name.c_str() );
					continue;
				}

				cfg.CharToGlyphNo[c] = atoi(value);

				continue;
			}

			if(name.substr(0, 6) == "RANGE ")
			{
				/* range CODESET=first_frame or
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
				vector<CString> matches;
				static Regex parse("^RANGE ([A-Z\\-]+)( ?#([0-9A-F]+)-([0-9A-F]+))?$");
				bool match = parse.Compare(name, matches);
				
				ASSERT(matches.size() == 4); /* 4 parens */

				if(!match || matches[0].empty())
					RageException::Throw("Font definition '%s' has an invalid range '%s': parse error",
						ini.GetPath().c_str(), name.c_str() );
				
				/* We must have either 1 match (just the codeset) or 4 (the whole thing). */

				int cnt = -1;
				int first = 0;
				if(!matches[2].empty())
				{
					sscanf(matches[2].c_str(), "%x", &first);
					int last;
					sscanf(matches[3].c_str(), "%x", &last);
					if(last < first)
						RageException::Throw("Font definition '%s' has an invalid range '%s': %i < %i.",
							ini.GetPath().c_str(), name.c_str(), last < first );

					cnt = last-first+1;
				}

				CString ret = cfg.MapRange(matches[0], first, atoi(value), cnt);
				if(!ret.empty())
					RageException::Throw("Font definition '%s' has an invalid range '%s': %s.",
						ini.GetPath().c_str(), name.c_str(), ret.c_str() );

				continue;
			}

			if(name.substr(0, 5) == "LINE ")
			{
				/* line ROW=CHAR1CHAR2CHAR3CHAR4
				 * eg.
				 * line 0=ABCDEFGH
				 *
				 * This lets us assign characters very compactly and readably. */

				CString row_str = name.substr(5);
				ASSERT(IsAnInt(row_str));
				const int row = atoi(row_str.c_str());
				const int first_frame = row * NumFramesWide;

				if(row > NumFramesHigh)
					RageException::Throw("The font definition \"%s\" tries to assign line %i, but the font is only %i characters high",
						ini.GetPath().c_str(), first_frame, NumFramesHigh);

				/* Decode the string. */
				const wstring wdata(CStringToWstring(value));

				if(int(wdata.size()) > NumFramesWide)
					RageException::Throw("The font definition \"%s\" assigns %i characters to row %i (\"%ls\"), but the font only has %i characters wide",
						ini.GetPath().c_str(), wdata.size(), row, wdata.c_str(), NumFramesWide);

				for(unsigned i = 0; i < wdata.size(); ++i)
					cfg.CharToGlyphNo[wdata[i]] = first_frame+i;
			}
		}
	}

	/* If it's 128 or 256 frames, default to ASCII or CP1252,
	 * respectively.  If it's anything else, we don't know what it
	 * is, so don't make any default mappings (the INI needs to do
	 * it itself). */
	if( PageName != "common" && cfg.CharToGlyphNo.empty() )
	{
		if( NumFrames == 128 )
			cfg.MapRange( "ascii", 0, 0, -1 );
		else if( NumFrames == 256 )
			cfg.MapRange( "cp1252", 0, 0, -1 );
		else if( NumFrames == 15 )
			cfg.MapRange( "numbers", 0, 0, -1 );
		else
			LOG->Trace( "Font page \"%s\" has no characters", TexturePath.c_str() );
	}

	/* If ' ' is set and nbsp is not, set nbsp. */
	if( cfg.CharToGlyphNo.find(' ') != cfg.CharToGlyphNo.end() )
		cfg.CharToGlyphNo[0x00A0] = cfg.CharToGlyphNo[' '];
}

CString FontPageSettings::MapRange(CString Mapping, int map_offset, int glyphno, int cnt)
{
	if(!Mapping.CompareNoCase("Unicode"))
	{
		/* Special case. */
		if(cnt == -1)
			return "Can't map all of Unicode to one font page"; /* don't do that */

		/* What's a practical limit?  A 2048x2048 texture could contain 16x16 characters,
		 * which is 16384 glyphs.  (Use a grayscale map and that's only 4 megs.)  Let's use
		 * that as a cap.  (We don't want to go crazy if someone says "range Unicode
		 * #0-FFFFFFFF".) */
		if(cnt > 16384)
			return ssprintf("Can't map %i glyphs to one font page", cnt);

		while(cnt)
		{
			CharToGlyphNo[map_offset] = glyphno;
			map_offset++;
			glyphno++;
			cnt--;
		}

		return "";
	}

	const wchar_t *mapping = FontCharmaps::get_char_map(Mapping);
	if(mapping == NULL)
		return "Unknown mapping";

	while(*mapping != 0 && map_offset) { mapping++; map_offset--; }
	if(map_offset)
		return "Map overflow"; /* there aren't enough characters in the map */

	/* If cnt is -1, set it to the number of characters in the map. */
	if(cnt == -1)
		for(cnt = 0; mapping[cnt] != 0; ++cnt) ;

	while(*mapping != 0)
	{
		if(*mapping != FontCharmaps::M_SKIP)
			CharToGlyphNo[*mapping] = glyphno;
		mapping++;
		glyphno++;
		cnt--;
	}

	if(cnt)
		return "Map overflow"; /* there aren't enough characters in the map */

	return "";
}

static CStringArray LoadStack;

/* A font set is a set of files, eg:
 *
 * Normal 16x16.png
 * Normal [other] 16x16.png
 * Normal [more] 8x8.png
 * Normal 16x16.ini           (the 16x16 here is optional)
 *
 * Only one texture is required; the INI is optional.  [1] This is
 * designed to be backwards-compatible.
 *
 * sFontOrTextureFilePath can be a partial path, eg.
 * "Themes/default/Fonts/Normal"
 * or a complete path to a texture file (in which case no other
 * files will be searched for).
 *
 * The entire font can be redirected; that's handled in ThemeManager.
 * Individual font files can not be redirected.
 *
 * TODO: 
 * [main]
 * import=FontName,FontName2 (load other fonts)
 *
 * [1] If a file has no INI and sChars is not set, it will receive a default
 * mapping of ASCII or ISO-8859-1 if the font has exactly 128 or 256 frames.
 * However, if it doesn't, we don't know what it is and the font will receive
 * no default mapping.  A font isn't useful with no characters mapped.
 */
void Font::Load(const CString &sFontOrTextureFilePath, CString sChars)
{
	/* Check for recursion (recursive imports). */
	{
		for(unsigned i = 0; i < LoadStack.size(); ++i)
		{
			if(LoadStack[i] == sFontOrTextureFilePath)
			{
				CString str = join("\n", LoadStack);
				str += "\n" + sFontOrTextureFilePath;
				RageException::Throw("Font import recursion detected\n%s", str.c_str());
			}
		}
		LoadStack.push_back(sFontOrTextureFilePath);
	}

	/* The font is not already loaded.  Figure out what we have. */
	CHECKPOINT_M( ssprintf("Font::Load(\"%s\",\"%s\").", sFontOrTextureFilePath.c_str(), Chars.c_str()) );

	path = sFontOrTextureFilePath;
	Chars = sChars;

	/* Get the filenames associated with this font. */
	CStringArray TexturePaths;
	CString IniPath;
	GetFontPaths(sFontOrTextureFilePath, TexturePaths, IniPath);

	/* If we don't have at least one INI or at least one texture path,
	 * we have nothing at all. */
	ASSERT(!IniPath.empty() || TexturePaths.size());

	bool CapitalsOnly = false;

	/* If we have an INI, load it. */
	IniFile ini;
	if( !IniPath.empty() )
	{
		ini.ReadFile( IniPath );
		ini.RenameKey("Char Widths", "main");
		ini.GetValue( "main", "CapitalsOnly", CapitalsOnly );
	}

	{
		/* If this is a top-level font (not a subfont), load the default font first. */
		CStringArray ImportList;
		if(LoadStack.size() == 1)
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
				LOG->Warn("Font \"%s\" imports a font \"%s\" that doesn't exist", sFontOrTextureFilePath.c_str(), ImportList[i].c_str());
				continue;
			}

			Font subfont;
			subfont.Load(path, "");
			MergeFont(subfont);
		}
	}

	/* Load each font page. */
	for(unsigned i = 0; i < TexturePaths.size(); ++i)
	{
		const CString sTexturePath = TexturePaths[i];

		FontPage *fp = new FontPage;

		/* Grab the page name, eg "foo" from "Normal [foo].png". */
		CString pagename = GetPageNameFromFileName(sTexturePath);

		/* Load settings for this page from the INI. */
		FontPageSettings cfg;
		LoadFontPageSettings(cfg, ini, TexturePaths[i], "common", sChars);
		LoadFontPageSettings(cfg, ini, TexturePaths[i], pagename, sChars);

		/* Go. */
		fp->Load(cfg);

		/* Expect at least as many frames as we have premapped characters. */
		/* Make sure that we don't map characters to frames we don't actually
		 * have.  This can happen if the font is too small for an sChars. */
		for(map<longchar,int>::const_iterator it = fp->m_iCharToGlyphNo.begin();
			it != fp->m_iCharToGlyphNo.end(); ++it)
		{
			if(it->second < fp->m_pTexture->GetNumFrames()) continue; /* OK */
			CString sError = ssprintf( "The font '%s' maps %s to frame %i, but the font only has %i frames.",
				TexturePaths[i].c_str(), WcharDisplayText(wchar_t(it->first)).c_str(), it->second, fp->m_pTexture->GetNumFrames() );
			RageException::Throw( sError );
		}

//		LOG->Trace("Adding page %s (%s) to %s; %i glyphs",
//			TexturePaths[i].c_str(), pagename.c_str(),
//			sFontOrTextureFilePath.c_str(), fp->m_iCharToGlyphNo.size());
		AddPage(fp);

		/* If this is the first font loaded, or it's called "main", this page's
		 * properties become the font's properties. */
		if(i == 0 || pagename == "main")
			SetDefaultGlyph(fp);
	}

	if(CapitalsOnly)
		CapsOnly();

	if(m_iCharToGlyph.empty())
		LOG->Warn("Font %s has no characters", sFontOrTextureFilePath.c_str());

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
