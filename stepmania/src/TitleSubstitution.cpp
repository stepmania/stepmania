#include "global.h"
#include "TitleSubstitution.h"

#include "RageUtil.h"
#include "FontCharAliases.h"

#include <fstream>

struct TitleTrans
{
	Regex TitleFrom, SubFrom, ArtistFrom;
	CString TitleTo, SubTo, ArtistTo;					/* plain text */
	CString TitleTransTo, SubTransTo, ArtistTransTo;	/* plain text */

	/* If this is true, no translit fields will be generated automatically. */
	bool translit;
	TitleTrans() { translit = true; }

	TitleTrans(CString tf, CString sf, CString af, CString tt, CString st, CString at,
			   bool translit_):
		TitleFrom(tf), SubFrom(sf), ArtistFrom(af),
			TitleTo(tt), SubTo(st), ArtistTo(at), translit(translit_) { }

	bool Matches(CString title, CString sub, CString artist);
};

vector<TitleTrans> ttab;

bool TitleTrans::Matches(CString title, CString sub, CString artist)
{
	if(!TitleFrom.Compare(title)) return false; /* no match */
	if(!SubFrom.Compare(sub)) return false; /* no match */
	if(!ArtistFrom.Compare(artist)) return false; /* no match */

	return true;
}

void TitleSubst::AddTrans(const TitleTrans &tr)
{
	ttab.push_back(new TitleTrans(tr));
}

void TitleSubst::Subst(CString &title, CString &subtitle, CString &artist,
					   CString &ttitle, CString &tsubtitle, CString &tartist)
{
	for(unsigned i = 0; i < ttab.size(); ++i)
	{
		if(!ttab[i]->Matches(title, subtitle, artist))
			continue;

		/* The song matches.  Replace whichever strings aren't empty. */
		if(!ttab[i]->TitleTo.empty())
		{
			if(ttab[i]->translit && title != ttab[i]->TitleTo) ttitle = title;
			title = ttab[i]->TitleTo;
			FontCharAliases::ReplaceMarkers( title );
		}
		if(!ttab[i]->SubTo.empty())
		{
			if(ttab[i]->translit && subtitle != ttab[i]->SubTo) tsubtitle = subtitle;
			subtitle = ttab[i]->SubTo;
			FontCharAliases::ReplaceMarkers( subtitle );
		}

		if(!ttab[i]->ArtistTo.empty() && artist != ttab[i]->ArtistTo)
		{
			if(ttab[i]->translit) tartist = artist;
			artist = ttab[i]->ArtistTo;
			FontCharAliases::ReplaceMarkers( artist );
		}

		/* These are used when applying kanji to a field that doesn't have the
		 * correct data.  Should be used sparingly. */
		if(!ttab[i]->TitleTransTo.empty())
		{
			ttitle = ttab[i]->TitleTransTo;
			FontCharAliases::ReplaceMarkers( ttitle );
		}
		if(!ttab[i]->SubTransTo.empty())
		{
			tsubtitle = ttab[i]->SubTransTo;
			FontCharAliases::ReplaceMarkers( tsubtitle );
		}
		if(!ttab[i]->ArtistTransTo.empty())
		{
			tartist = ttab[i]->ArtistTransTo;
			FontCharAliases::ReplaceMarkers( tartist );
		}
	}
}


TitleSubst::TitleSubst(const CString &section)
{
	Load("data/Translation.dat", section);
}

void TitleSubst::Load(const CString &filename, const CString &section)
{
	ifstream f;
	f.open(filename);
	if(!f.good()) return;

	CString CurrentSection;
	TitleTrans tr;

	while(!f.eof())
	{
		CString line;
		if(!getline(f, line)) continue;

		if(line.size() > 0 && utf8_get_char(line.c_str()) == 0xFEFF)
		{
			/* Annoying header that Windows puts on UTF-8 plaintext
				* files; remove it. */
			line.erase(0, utf8_get_char_len(line.c_str()));
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

		unsigned pos = line.find_first_of(':');
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
			else if(!id.CompareNoCase("TitleTo")) tr.TitleTo = txt;
			else if(!id.CompareNoCase("ArtistTo")) tr.ArtistTo = txt;
			else if(!id.CompareNoCase("SubtitleTo")) tr.SubTo = txt;
			else if(!id.CompareNoCase("TitleTransTo")) tr.TitleTransTo = txt;
			else if(!id.CompareNoCase("ArtistTransTo")) tr.ArtistTransTo = txt;
			else if(!id.CompareNoCase("SubtitleTransTo")) tr.SubTransTo = txt;
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
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
