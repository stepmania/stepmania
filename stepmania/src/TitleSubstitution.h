/* TitleSubst - automatic translation for song titles */

#ifndef TITLE_SUBSTITUTION_H
#define TITLE_SUBSTITUTION_H 1

struct TitleFields
{
	void SaveToStrings( 
		CString &sTitle, CString &sSubtitle, CString &sArtist,
		CString &sTitleTranslit, CString &sSubtitleTranslit, CString &sArtistTranslit ) const
	{
		sTitle = Title;
		sSubtitle = Subtitle;
		sArtist = Artist;
		sTitleTranslit = TitleTranslit;
		sSubtitleTranslit = SubtitleTranslit;
		sArtistTranslit = ArtistTranslit;
	}

	void LoadFromStrings( 
		CString sTitle, CString sSubtitle, CString sArtist,
		CString sTitleTranslit, CString sSubtitleTranslit, CString sArtistTranslit )
	{
		Title = sTitle;
		Subtitle = sSubtitle;
		Artist = sArtist;
		TitleTranslit = sTitleTranslit;
		SubtitleTranslit = sSubtitleTranslit;
		ArtistTranslit = sArtistTranslit;
	}
	CString Title, Subtitle, Artist;
	CString TitleTranslit, SubtitleTranslit, ArtistTranslit;
};
struct TitleTrans;

class TitleSubst
{
	vector<TitleTrans *> ttab;

	void AddTrans(const TitleTrans &tr);
public:
	TitleSubst(const CString &section);
	~TitleSubst();

	void Load(const CString &filename, const CString &section);

	void Subst( TitleFields &tf );
};

#endif

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
