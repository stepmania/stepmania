#include "global.h"
#include "TitleSubstitution.h"

#include "RageUtil.h"
#include "FontCharAliases.h"

#include <fstream>

struct TitleTrans
{
	Regex TitleFrom, SubFrom, ArtistFrom;
	CString TitleTo, SubTo, ArtistTo;			/* plain text */
	bool translit;

	TitleTrans(CString tf, CString sf, CString af, CString tt, CString st, CString at,
			   bool translit_):
		TitleFrom(tf), SubFrom(sf), ArtistFrom(af),
			TitleTo(tt), SubTo(st), ArtistTo(at),
			translit(translit_) { }

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

void TitleSubst::AddTrans(const CString &tf, const CString &sf, const CString &af,
		      const CString &tt, const CString &st, const CString &at, bool translit)
{
	ttab.push_back(new TitleTrans(tf, sf, af, tt, st, at, translit));
}

void TitleSubst::Subst(CString &title, CString &subtitle, CString &artist,
					   CString &ttitle, CString &tsubtitle, CString &tartist)
{
	for(unsigned i = 0; i < ttab.size(); ++i)
	{
		if(!ttab[i]->Matches(title, subtitle, artist))
			continue;

		/* The song matches.  Replace whichever strings aren't empty.
		 * Don't replace any that already have a transliteration set. */
		if(!ttab[i]->TitleTo.empty() && ttitle.empty())
		{
			if(ttab[i]->translit) ttitle = title;
			title = ttab[i]->TitleTo;
			FontCharAliases::ReplaceMarkers( title );
		}
		if(!ttab[i]->SubTo.empty() && tsubtitle.empty())
		{
			if(ttab[i]->translit) tsubtitle = subtitle;
			subtitle = ttab[i]->SubTo;
			FontCharAliases::ReplaceMarkers( subtitle );
		}
		if(!ttab[i]->ArtistTo.empty() && tartist.empty())
		{
			if(ttab[i]->translit) tartist = artist;
			artist = ttab[i]->ArtistTo;
			FontCharAliases::ReplaceMarkers( artist );
		}
	}
}


TitleSubst::TitleSubst()
{
	Load("Translation.dat");
}

void TitleSubst::Load(const CString &filename)
{
	ifstream f;
	f.open(filename);
	if(!f.good()) return;
	while(!f.eof())
	{
		CString TitleFrom, ArtistFrom, SubtitleFrom, TitleTo, ArtistTo, SubtitleTo;
		bool translit = true;

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

			if(!line.CompareNoCase("DontTransliterate"))
			{
				translit = false;
				continue;
			}

			if(line.size() == 0) continue; /* blank */
			if(line[0] == '#') continue; /* comment */

			if(line[0] == '*') break;

			/* x: y */

			unsigned pos = line.find_first_of(':');
			if(pos == string::npos) continue;
			CString id = line.substr(0, pos);
			CString txt = line.substr(pos+1);
			TrimLeft(txt);

			if(!id.CompareNoCase("TitleFrom")) TitleFrom = txt;
			else if(!id.CompareNoCase("ArtistFrom")) ArtistFrom = txt;
			else if(!id.CompareNoCase("SubtitleFrom")) SubtitleFrom = txt;
			else if(!id.CompareNoCase("TitleTo")) TitleTo = txt;
			else if(!id.CompareNoCase("ArtistTo")) ArtistTo = txt;
			else if(!id.CompareNoCase("SubtitleTo")) SubtitleTo = txt;
		}

		/* Surround each regex with ^(...)$, to force all comparisons to default
		 * to being a full-line match.  (Add ".*" manually if htis isn't wanted.) */
		if(TitleFrom.size()) TitleFrom = "^(" + TitleFrom + ")$";
		if(ArtistFrom.size()) ArtistFrom = "^(" + ArtistFrom + ")$";
		if(SubtitleFrom.size()) SubtitleFrom = "^(" + SubtitleFrom + ")$";

		AddTrans(TitleFrom, SubtitleFrom, ArtistFrom, TitleTo, SubtitleTo, ArtistTo, translit);
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
