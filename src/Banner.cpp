#include "global.h"
#include "Banner.h"
#include "ImageCache.h"
#include "SongManager.h"
#include "RageUtil.h"
#include "Song.h"
#include "RageTextureManager.h"
#include "Course.h"
#include "Character.h"
#include "ThemeMetric.h"
#include "CharacterManager.h"
#include "ActorUtil.h"
#include "UnlockManager.h"
#include "PrefsManager.h"

REGISTER_ACTOR_CLASS( Banner );

ThemeMetric<bool> SCROLL_RANDOM	("Banner","ScrollRandom");
ThemeMetric<bool> SCROLL_ROULETTE	("Banner","ScrollRoulette");
ThemeMetric<bool> SCROLL_MODE	("Banner","ScrollMode");
ThemeMetric<bool> SCROLL_SORT_ORDER	("Banner","ScrollSortOrder");
ThemeMetric<float> SCROLL_SPEED_DIVISOR	("Banner","ScrollSpeedDivisor");

Banner::Banner()
{
	m_bScrolling = false;
	m_fPercentScrolling = 0;
}

// Ugly: if sIsBanner is false, we're actually loading something other than a banner.
void Banner::Load( RageTextureID ID, bool bIsBanner )
{
	if( ID.filename == "" )
	{
		LoadFallback();
		return;
	}

	if( bIsBanner )
		ID = SongBannerTexture(ID);

	m_fPercentScrolling = 0;
	m_bScrolling = false;

	TEXTUREMAN->DisableOddDimensionWarning();
	TEXTUREMAN->VolatileTexture( ID );
	Sprite::Load( ID );
	TEXTUREMAN->EnableOddDimensionWarning();
};

void Banner::LoadFromCachedBanner( const RString &sPath )
{
	if( sPath.empty() )
	{
		LoadFallback();
		return;
	}

	RageTextureID ID;
	bool bLowRes = (PREFSMAN->m_ImageCache != IMGCACHE_FULL);
	if( !bLowRes )
	{
		ID = Sprite::SongBannerTexture( sPath );
	}
	else
	{
		// Try to load the low quality version.
		ID = IMAGECACHE->LoadCachedImage( "Banner", sPath );
	}

	if( TEXTUREMAN->IsTextureRegistered(ID) )
		Load( ID );
	else if( IsAFile(sPath) )
		Load( sPath );
	else
		LoadFallback();
}

void Banner::Update( float fDeltaTime )
{
	Sprite::Update( fDeltaTime );

	if( m_bScrolling )
	{
		m_fPercentScrolling += fDeltaTime/(float)SCROLL_SPEED_DIVISOR;
		m_fPercentScrolling -= (int)m_fPercentScrolling;

		const RectF *pTextureRect = GetCurrentTextureCoordRect();
 
		float fTexCoords[8] = 
		{
			0+m_fPercentScrolling, pTextureRect->top,		// top left
			0+m_fPercentScrolling, pTextureRect->bottom,	// bottom left
			1+m_fPercentScrolling, pTextureRect->bottom,	// bottom right
			1+m_fPercentScrolling, pTextureRect->top,		// top right
		};
		Sprite::SetCustomTextureCoords( fTexCoords );
	}
}

void Banner::SetScrolling( bool bScroll, float Percent)
{
	m_bScrolling = bScroll;
	m_fPercentScrolling = Percent;

	// Set up the texture coord rects for the current state.
	Update(0);
}

void Banner::LoadFromSong( Song* pSong ) // nullptr means no song
{
	if( pSong == nullptr )	LoadFallback();
	else if( pSong->HasBanner() ) Load( pSong->GetBannerPath() );
	else					LoadFallback();

	m_bScrolling = false;
}

void Banner::LoadMode()
{
	Load( THEME->GetPathG("Banner","Mode") );
	m_bScrolling = (bool)SCROLL_MODE;
}

void Banner::LoadFromSongGroup( RString sSongGroup )
{
	RString sGroupBannerPath = SONGMAN->GetSongGroupBannerPath( sSongGroup );
	if( sGroupBannerPath != "" )			Load( sGroupBannerPath );
	else						LoadGroupFallback();
	m_bScrolling = false;
}

void Banner::LoadFromCourse( const Course *pCourse )		// nullptr means no course
{
	if( pCourse == nullptr )				LoadFallback();
	else if( pCourse->GetBannerPath() != "" )	Load( pCourse->GetBannerPath() );
	else						LoadCourseFallback();

	m_bScrolling = false;
}

void Banner::LoadCardFromCharacter( const Character *pCharacter )
{
	if( pCharacter == nullptr )			LoadFallback();
	else if( pCharacter->GetCardPath() != "" )	Load( pCharacter->GetCardPath() );
	else						LoadFallback();

	m_bScrolling = false;
}

void Banner::LoadIconFromCharacter( const Character *pCharacter )
{
	if( pCharacter == nullptr )			LoadFallbackCharacterIcon();
	else if( pCharacter->GetIconPath() != "" )	Load( pCharacter->GetIconPath(), false );
	else						LoadFallbackCharacterIcon();

	m_bScrolling = false;
}

void Banner::LoadBannerFromUnlockEntry( const UnlockEntry* pUE )
{
	if( pUE == nullptr )
		LoadFallback();
	else 
	{
		RString sFile = pUE->GetBannerFile();
		Load( sFile );
		m_bScrolling = false;
	}
}

void Banner::LoadBackgroundFromUnlockEntry( const UnlockEntry* pUE )
{
	if( pUE == nullptr )
		LoadFallback();
	else 
	{
		RString sFile = pUE->GetBackgroundFile();
		Load( sFile );
		m_bScrolling = false;
	}
}

void Banner::LoadFallback()
{
	Load( THEME->GetPathG("Common","fallback banner") );
}

void Banner::LoadFallbackBG()
{
	Load( THEME->GetPathG("Common","fallback background") );
}

void Banner::LoadGroupFallback()
{
	Load( THEME->GetPathG("Banner","group fallback") );
}

void Banner::LoadCourseFallback()
{
	Load( THEME->GetPathG("Banner","course fallback") );
}

void Banner::LoadFallbackCharacterIcon()
{
	Character *pCharacter = CHARMAN->GetDefaultCharacter();
	if( pCharacter  &&  !pCharacter->GetIconPath().empty() )
		Load( pCharacter->GetIconPath(), false );
	else
		LoadFallback();
}

void Banner::LoadRoulette()
{
	Load( THEME->GetPathG("Banner","roulette") );
	m_bScrolling = (bool)SCROLL_ROULETTE;
}

void Banner::LoadRandom()
{
	Load( THEME->GetPathG("Banner","random") );
	m_bScrolling = (bool)SCROLL_RANDOM;
}

void Banner::LoadFromSortOrder( SortOrder so )
{
	// TODO: See if the check for nullptr/PREFERRED(?) is needed.
	if( so == SortOrder_Invalid )
	{
		LoadFallback();
	}
	else
	{
		if( so != SORT_GROUP && so != SORT_RECENT )
			Load( THEME->GetPathG("Banner",ssprintf("%s",SortOrderToString(so).c_str())) );
	}
	m_bScrolling = (bool)SCROLL_SORT_ORDER;
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the Banner. */ 
class LunaBanner: public Luna<Banner>
{
public:
	static int scaletoclipped( T* p, lua_State *L )			{ p->ScaleToClipped(FArg(1),FArg(2)); COMMON_RETURN_SELF; }
	static int ScaleToClipped( T* p, lua_State *L )			{ p->ScaleToClipped(FArg(1),FArg(2)); COMMON_RETURN_SELF; }
	static int LoadFromSong( T* p, lua_State *L )
	{ 
		if( lua_isnil(L,1) ) { p->LoadFromSong(nullptr); }
		else { Song *pS = Luna<Song>::check(L,1); p->LoadFromSong( pS ); }
		COMMON_RETURN_SELF;
	}
	static int LoadFromCourse( T* p, lua_State *L )
	{ 
		if( lua_isnil(L,1) ) { p->LoadFromCourse(nullptr); }
		else { Course *pC = Luna<Course>::check(L,1); p->LoadFromCourse( pC ); }
		COMMON_RETURN_SELF;
	}
	static int LoadFromCachedBanner( T* p, lua_State *L )
	{ 
		p->LoadFromCachedBanner( SArg(1) );
		COMMON_RETURN_SELF;
	}
	static int LoadIconFromCharacter( T* p, lua_State *L )
	{ 
		if( lua_isnil(L,1) ) { p->LoadIconFromCharacter(nullptr); }
		else { Character *pC = Luna<Character>::check(L,1); p->LoadIconFromCharacter( pC ); }
		COMMON_RETURN_SELF;
	}
	static int LoadCardFromCharacter( T* p, lua_State *L )
	{ 
		if( lua_isnil(L,1) ) { p->LoadIconFromCharacter(nullptr); }
		else { Character *pC = Luna<Character>::check(L,1); p->LoadIconFromCharacter( pC ); }
		COMMON_RETURN_SELF;
	}
	static int LoadBannerFromUnlockEntry( T* p, lua_State *L )
	{ 
		if( lua_isnil(L,1) ) { p->LoadBannerFromUnlockEntry(nullptr); }
		else { UnlockEntry *pUE = Luna<UnlockEntry>::check(L,1); p->LoadBannerFromUnlockEntry( pUE ); }
		COMMON_RETURN_SELF;
	}
	static int LoadBackgroundFromUnlockEntry( T* p, lua_State *L )
	{ 
		if( lua_isnil(L,1) ) { p->LoadBackgroundFromUnlockEntry(nullptr); }
		else { UnlockEntry *pUE = Luna<UnlockEntry>::check(L,1); p->LoadBackgroundFromUnlockEntry( pUE ); }
		COMMON_RETURN_SELF;
	}
	static int LoadFromSongGroup( T* p, lua_State *L )
	{ 
		p->LoadFromSongGroup( SArg(1) );
		COMMON_RETURN_SELF;
	}
	static int LoadFromSortOrder( T* p, lua_State *L )
	{
		if( lua_isnil(L,1) ) { p->LoadFromSortOrder( SortOrder_Invalid ); }
		else
		{
			SortOrder so = Enum::Check<SortOrder>(L, 1);
			p->LoadFromSortOrder( so );
		}
		COMMON_RETURN_SELF;
	}
	static int GetScrolling( T* p, lua_State *L ){ lua_pushboolean( L, p->GetScrolling() ); return 1; }
	static int SetScrolling( T* p, lua_State *L ){ p->SetScrolling( BArg(1), FArg(2) ); COMMON_RETURN_SELF; }
	static int GetPercentScrolling( T* p, lua_State *L ){ lua_pushnumber( L, p->ScrollingPercent() ); return 1; }

	LunaBanner()
	{
		ADD_METHOD( scaletoclipped );
		ADD_METHOD( ScaleToClipped );
		ADD_METHOD( LoadFromSong );
		ADD_METHOD( LoadFromCourse );
		ADD_METHOD( LoadFromCachedBanner );
		ADD_METHOD( LoadIconFromCharacter );
		ADD_METHOD( LoadCardFromCharacter );
		ADD_METHOD( LoadBannerFromUnlockEntry );
		ADD_METHOD( LoadBackgroundFromUnlockEntry );
		ADD_METHOD( LoadFromSongGroup );
		ADD_METHOD( LoadFromSortOrder );
		ADD_METHOD( GetScrolling );
		ADD_METHOD( SetScrolling );
		ADD_METHOD( GetPercentScrolling );
	}
};

LUA_REGISTER_DERIVED_CLASS( Banner, Sprite )
// lua end


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
