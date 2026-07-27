#ifndef BackgroundUtil_H
#define BackgroundUtil_H

#include <vector>


class Song;
class XNode;

extern const RString RANDOM_BACKGROUND_FILE;
extern const RString NO_SONG_BG_FILE;
extern const RString SONG_BACKGROUND_FILE;

extern const RString SBE_UpperLeft;
extern const RString SBE_Centered;
extern const RString SBE_StretchNormal;
extern const RString SBE_StretchNoLoop;
extern const RString SBE_StretchRewind;
extern const RString SBT_CrossFade;

struct BackgroundDef
{
	bool operator<( const BackgroundDef &other ) const;
	bool operator==( const BackgroundDef &other ) const;
	bool IsEmpty() const { return m_sFile1.empty() && m_sFile2.empty(); }
	RString	m_sEffect;	// "" == automatically choose
	RString m_sFile1;	// must not be ""
	RString m_sFile2;	// may be ""
	RString m_sColor1;	// "" == use default
	RString m_sColor2;	// "" == use default

	XNode *CreateNode() const;

	/** @brief Set up the BackgroundDef with default values. */
	BackgroundDef(): m_sEffect(""), m_sFile1(""), m_sFile2(""),
		m_sColor1(""), m_sColor2("") {}

	/**
	 * @brief Set up the BackgroundDef with some defined values.
	 * @param effect the intended effect.
	 * @param f1 the primary filename for the definition.
	 * @param f2 the secondary filename (optional). */
	BackgroundDef(RString effect, RString f1, RString f2):
		m_sEffect(effect), m_sFile1(f1), m_sFile2(f2),
		m_sColor1(""), m_sColor2("") {}
};

struct BackgroundChange
{
	BackgroundChange(): m_def(), m_fStartBeat(-1), m_fRate(1),
		m_sTransition("") {}

	BackgroundChange(
		float s,
		RString f1,
		RString f2=RString(),
		float r=1.f,
		RString e=SBE_Centered,
		RString t=RString()
			 ):
		m_def(e, f1, f2), m_fStartBeat(s),
		m_fRate(r), m_sTransition(t) {}

	BackgroundDef m_def;
	float m_fStartBeat;
	float m_fRate;
	RString m_sTransition;

	RString GetTextDescription() const;

	/**
	 * @brief Get the string representation of the change.
	 * @return the string representation. */
	RString ToString() const;
};
/** @brief Shared background-related routines. */
namespace BackgroundUtil
{
	void AddBackgroundChange( std::vector<BackgroundChange> &vBackgroundChanges, BackgroundChange seg );
	void SortBackgroundChangesArray( std::vector<BackgroundChange> &vBackgroundChanges );

	void GetBackgroundEffects(	const RString &sName, std::vector<RString> &vsPathsOut, std::vector<RString> &vsNamesOut );
	void GetBackgroundTransitions(	const RString &sName, std::vector<RString> &vsPathsOut, std::vector<RString> &vsNamesOut );

	void GetSongBGAnimations(	const Song *pSong, const RString &sMatch, std::vector<RString> &vsPathsOut, std::vector<RString> &vsNamesOut );
	void GetSongMovies(		const Song *pSong, const RString &sMatch, std::vector<RString> &vsPathsOut, std::vector<RString> &vsNamesOut );
	void GetSongBitmaps(		const Song *pSong, const RString &sMatch, std::vector<RString> &vsPathsOut, std::vector<RString> &vsNamesOut );
	void GetGlobalBGAnimations(	const Song *pSong, const RString &sMatch, std::vector<RString> &vsPathsOut, std::vector<RString> &vsNamesOut );
	void GetGlobalRandomMovies(	const Song *pSong, const RString &sMatch, std::vector<RString> &vsPathsOut, std::vector<RString> &vsNamesOut, bool bTryInsideOfSongGroupAndGenreFirst = true, bool bTryInsideOfSongGroupFirst = true );

	void BakeAllBackgroundChanges( Song *pSong );
};


#endif

/*
 * (c) 2001-2004 Chris Danford, Ben Nordstrom
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
