/* Banner - The song's banner displayed in SelectSong. */

#ifndef BANNER_H
#define BANNER_H

#include "Sprite.h"
#include "RageTextureID.h"
class Song;
class Course;
class Character;
class UnlockEntry;

class Banner : public Sprite
{
public:
	Banner();
	virtual ~Banner() { }
	virtual Actor *Copy() const;

	void Load( RageTextureID ID, bool bIsBanner );
	virtual void Load( RageTextureID ID ) { Load( ID, true ); }

	virtual void Update( float fDeltaTime );

	void LoadFromSong( Song* pSong );		// NULL means no song
	void LoadAllMusic();
	void LoadMode();
	void LoadFromSongGroup( RString sSongGroup );
	void LoadFromCourse( const Course *pCourse );
	void LoadCardFromCharacter( const Character *pCharacter );
	void LoadIconFromCharacter( const Character *pCharacter );
	void LoadBannerFromUnlockEntry( const UnlockEntry* pUE );
	void LoadBackgroundFromUnlockEntry( const UnlockEntry* pUE );
	void LoadRoulette();
	void LoadRandom();
	void LoadFallback();
	void LoadCourseFallback();
	void LoadFallbackCharacterIcon();

	void SetScrolling( bool bScroll, float Percent = 0 );
	float ScrollingPercent() const { return m_fPercentScrolling; }

	// Lua
	void PushSelf( lua_State *L );

protected:
	bool m_bScrolling;
	float m_fPercentScrolling;
};

#endif

/*
 * (c) 2001-2004 Chris Danford
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
