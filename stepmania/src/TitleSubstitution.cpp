#include "global.h"
#include "TitleSubstitution.h"
#include "arch/arch.h"

#include "RageUtil.h"
#include "RageLog.h"
#include "FontCharAliases.h"

#include "RageFile.h"

#define TRANSLATION_PATH "Data/Translation.dat"

struct TitleTrans
{
	Regex TitleFrom, SubFrom, ArtistFrom;
	TitleFields Dest;

	/* If this is true, no translit fields will be generated automatically. */
	bool translit;
	TitleTrans() { translit = true; }

	TitleTrans( const TitleFields &tf, bool translit_):
		Dest(tf), translit(translit_) { }

	bool Matches( const TitleFields &tf );
};

vector<TitleTrans> ttab;

bool TitleTrans::Matches( const TitleFields &tf )
{
	if( !TitleFrom.Compare(tf.Title) )
		return false; /* no match */
	if( !SubFrom.Compare(tf.Subtitle) )
		return false; /* no match */
	if( !ArtistFrom.Compare(tf.Artist) )
		return false; /* no match */

	return true;
}

void TitleSubst::AddTrans(const TitleTrans &tr)
{
	ttab.push_back(new TitleTrans(tr));
}

void TitleSubst::Subst( TitleFields &tf )
{
	for(unsigned i = 0; i < ttab.size(); ++i)
	{
		if(!ttab[i]->Matches(tf))
			continue;

		/* The song matches.  Replace whichever strings aren't empty. */
		if( !ttab[i]->Dest.Title.empty() && tf.Title != ttab[i]->Dest.Title )
		{
			if( ttab[i]->translit )
				tf.TitleTranslit = tf.Title;
			tf.Title = (ttab[i]->Dest.Title != "-erase-")? ttab[i]->Dest.Title: "";
			FontCharAliases::ReplaceMarkers( tf.Title );
		}
		if( !ttab[i]->Dest.Subtitle.empty() && tf.Subtitle != ttab[i]->Dest.Subtitle )
		{
			if( ttab[i]->translit )
				tf.SubtitleTranslit = tf.Subtitle;
			tf.Subtitle = (ttab[i]->Dest.Subtitle != "-erase-")? ttab[i]->Dest.Subtitle: "";
			FontCharAliases::ReplaceMarkers( tf.Subtitle );
		}
		if( !ttab[i]->Dest.Artist.empty() && tf.Artist != ttab[i]->Dest.Artist )
		{
			if( ttab[i]->translit )
				tf.ArtistTranslit = tf.Artist;
			tf.Artist = (ttab[i]->Dest.Artist != "-erase-")? ttab[i]->Dest.Artist: "";
			FontCharAliases::ReplaceMarkers( tf.Artist );
		}

		/* These are used when applying kanji to a field that doesn't have the
		 * correct data.  Should be used sparingly. */
		if( !ttab[i]->Dest.TitleTranslit.empty() )
		{
			tf.TitleTranslit = (ttab[i]->Dest.TitleTranslit != "-erase-")? ttab[i]->Dest.TitleTranslit: "";
			FontCharAliases::ReplaceMarkers( tf.TitleTranslit );
		}
		if( !ttab[i]->Dest.SubtitleTranslit.empty() )
		{
			tf.SubtitleTranslit = (ttab[i]->Dest.SubtitleTranslit != "-erase-")? ttab[i]->Dest.SubtitleTranslit: "";
			FontCharAliases::ReplaceMarkers( tf.SubtitleTranslit );
		}
		if( !ttab[i]->Dest.ArtistTranslit.empty() )
		{
			tf.ArtistTranslit = (ttab[i]->Dest.ArtistTranslit != "-erase-")? ttab[i]->Dest.ArtistTranslit: "";
			FontCharAliases::ReplaceMarkers( tf.ArtistTranslit );
		}
	}
}


TitleSubst::TitleSubst(const CString &section)
{
	Load( TRANSLATION_PATH, section);
}

void TitleSubst::Load(const CString &filename, const CString &section)
{
	RageFile f;
	if( !f.Open(filename) )
	{
		LOG->Trace("Error opening %s: %s", filename.c_str(), f.GetError().c_str() );
		return;
	}
	
	CString CurrentSection;
	TitleTrans tr;
	
	while (!f.AtEOF())
	{
		CString line;
		int ret = f.GetLine( line );
		if( ret == 0 )
			break;
		if( ret == -1 )
		{
			LOG->Trace("Error reading %s: %s", filename.c_str(), f.GetError().c_str() );
			break;
		}

		if(line.size() > 0 && utf8_get_char(line.c_str()) == 0xFEFF)
		{
			/* Annoying header that Windows puts on UTF-8 plaintext
			 * files; remove it. */
			line.erase(0, utf8_get_char_len(line[0]));
		}

		TrimLeft(line);
		TrimRight(line);

		if(line.size() == 0) continue; /* blank */
		if(line[0] == '#') continue; /* comment */

		if(!line.CompareNoCase("DontTransliterate"))
		{
			tr.translit = false;
			continue;
		}

		size_t pos = line.find_first_of(':');
		if(pos != string::npos)
		{
			/* x: y */
			CString id = line.substr(0, pos);
			CString txt = line.substr(pos+1);
			TrimLeft(txt);

			/* Surround each regex with ^(...)$, to force all comparisons to default
			 * to being a full-line match.  (Add ".*" manually if this isn't wanted.) */
			if(!id.CompareNoCase("TitleFrom")) tr.TitleFrom = "^(" + txt + ")$";
			else if(!id.CompareNoCase("ArtistFrom")) tr.ArtistFrom = "^(" + txt + ")$";
			else if(!id.CompareNoCase("SubtitleFrom")) tr.SubFrom = "^(" + txt + ")$";
			else if(!id.CompareNoCase("TitleTo")) tr.Dest.Title = txt;
			else if(!id.CompareNoCase("ArtistTo")) tr.Dest.Artist = txt;
			else if(!id.CompareNoCase("SubtitleTo")) tr.Dest.Subtitle = txt;
			else if(!id.CompareNoCase("TitleTransTo")) tr.Dest.TitleTranslit = txt;
			else if(!id.CompareNoCase("ArtistTransTo")) tr.Dest.ArtistTranslit = txt;
			else if(!id.CompareNoCase("SubtitleTransTo")) tr.Dest.SubtitleTranslit = txt;
			else
				LOG->Warn( "Unknown TitleSubst tag: \"%s\"", id.c_str() );
		}

		/* Add the translation if this is a terminator (*) or section
		 * marker ([foo]). */
		if(line[0] == '*' || line[0] == '[')
		{
			if(!CurrentSection.CompareNoCase(section))
				AddTrans(tr);
			
			/* Reset. */
			tr = TitleTrans();
		}

		if(line[0] == '[' && line[line.size()-1] == ']')
		{
			CurrentSection = line.substr(1, line.size()-2);
		}
	}
}

TitleSubst::~TitleSubst()
{
	for(unsigned i = 0; i < ttab.size(); ++i)
		delete ttab[i];
}

/*
 * (c) 2003-2004 Glenn Maynard
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
