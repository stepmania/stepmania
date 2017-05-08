#ifndef STYLEUTIL_H
#define STYLEUTIL_H

class Style;
class Song;
class XNode;

class StyleID
{
	std::string sGame;
	std::string sStyle;

public:
	StyleID(): sGame(""), sStyle("") { }
	void Unset() { FromStyle(nullptr); }
	void FromStyle( const Style *p );
	const Style *ToStyle() const;
	bool operator<( const StyleID &rhs ) const;
	bool operator==(const StyleID& rhs) const
	{ return sGame == rhs.sGame && sStyle == rhs.sStyle; }

	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );
	bool IsValid() const;
	static void FlushCache( Song* pStaleSong );

	std::string get_game() const { return sGame; }
	std::string get_style() const { return sStyle; }
};

namespace std
{
	template<>
    struct hash<StyleID>
	{
		std::size_t operator()(StyleID const& s) const
		{
			std::size_t const h1(std::hash<std::string>()(s.get_game()));
			std::size_t const h2(std::hash<std::string>()(s.get_style()));
			return h1 ^ (h2 << 1);
		}
	};
}

#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
