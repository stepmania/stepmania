#include "global.h"
#include "FadingBanner.h"
#include "RageTextureManager.h"
#include "ImageCache.h"
#include "Song.h"
#include "RageLog.h"
#include "Course.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "SongManager.h"
#include "ThemeMetric.h"
#include "ActorUtil.h"

REGISTER_ACTOR_CLASS( FadingBanner );

/* Allow fading from one banner to another. We can handle two fades at once;
 * this is used to fade from an old banner to a low-quality banner to a high-
 * quality banner smoothly.
 *
 * m_iIndexLatest is the latest banner loaded, and the one that we'll end up
 * displaying when the fades stop. */
FadingBanner::FadingBanner()
{
	m_bMovingFast = false;
	m_bSkipNextBannerUpdate = false;
	m_iIndexLatest = 0;
	for( int i=0; i<NUM_BANNERS; i++ )
	{
		m_Banner[i].SetName( "Banner" );
		ActorUtil::LoadAllCommandsAndOnCommand( m_Banner[i], "FadingBanner" );
		this->AddChild( &m_Banner[i] );
	}
}

void FadingBanner::ScaleToClipped( float fWidth, float fHeight )
{
	for( int i=0; i<NUM_BANNERS; i++ )
		m_Banner[i].ScaleToClipped( fWidth, fHeight );
}

void FadingBanner::UpdateInternal( float fDeltaTime )
{
	// update children manually
	// ActorFrame::UpdateInternal( fDeltaTime );
	Actor::UpdateInternal( fDeltaTime );

	if( !m_bSkipNextBannerUpdate )
	{
		for( int i = 0; i < NUM_BANNERS; ++i )
			m_Banner[i].Update( fDeltaTime );
	}

	m_bSkipNextBannerUpdate = false;
}

void FadingBanner::DrawPrimitives()
{
	// draw manually
//	ActorFrame::DrawPrimitives();

	// Render the latest banner first.
	for( int i = 0; i < NUM_BANNERS; ++i )
	{
		int index = m_iIndexLatest - i;
		wrap( index, NUM_BANNERS );
		m_Banner[index].Draw();
	}
}

void FadingBanner::Load( RageTextureID ID, bool bLowResToHighRes )
{
	BeforeChange( bLowResToHighRes );
	m_Banner[m_iIndexLatest].Load(ID);

	/* XXX: Hack to keep movies from updating multiple times.
	 * We need to either completely disallow movies in banners or support
	 * them. There are a number of files that use them currently in the
	 * wild. If we wanted to support them, then perhaps we should use an
	 * all-black texture for the low quality texture. */
	RageTexture *pTexture = m_Banner[m_iIndexLatest].GetTexture();
	if( !pTexture || !pTexture->IsAMovie() )
		return;
	m_Banner[m_iIndexLatest].SetSecondsIntoAnimation( 0.f );
	for( int i = 1; i < NUM_BANNERS; ++i )
	{
		int index = m_iIndexLatest - i;
		wrap( index, NUM_BANNERS );
		if( m_Banner[index].GetTexturePath() == ID.filename )
			m_Banner[index].UnloadTexture();
	}
}

/* If bLowResToHighRes is true, we're fading from a low-res banner to the
 * corresponding high-res banner. */
void FadingBanner::BeforeChange( bool bLowResToHighRes )
{
	RString sCommand;
	if( bLowResToHighRes )
		sCommand = "FadeFromCached";
	else
		sCommand = "FadeOff";

	m_Banner[m_iIndexLatest].PlayCommand( sCommand );
	++m_iIndexLatest;
	wrap( m_iIndexLatest, NUM_BANNERS );

	m_Banner[m_iIndexLatest].PlayCommand( "ResetFade" );

	/* We're about to load a banner. It'll probably cause a frame skip or two.
	 * Skip an update, so the fade-in doesn't skip. */
	m_bSkipNextBannerUpdate = true;
}

/* If this returns true, a low-resolution banner was loaded, and the full-res
 * banner should be loaded later. */
bool FadingBanner::LoadFromCachedBanner( const RString &path )
{
	// If we're already on the given banner, don't fade again.
	if( path != "" && m_Banner[m_iIndexLatest].GetTexturePath() == path )
		return false;

	if( path == "" )
	{
		LoadFallback();
		return false;
	}

	/* If we're currently fading to the given banner, go through this again,
	 * which will cause the fade-in to be further delayed. */

	RageTextureID ID;
	bool bLowRes = (PREFSMAN->m_ImageCache != IMGCACHE_FULL);
	if( !bLowRes )
	{
		ID = Sprite::SongBannerTexture( path );
	}
	else
	{
		// Try to load the low quality version.
		ID = IMAGECACHE->LoadCachedImage( "Banner", path );
	}

	if( !TEXTUREMAN->IsTextureRegistered(ID) )
	{
		/* Oops. We couldn't load a banner quickly. We can load the actual
		 * banner, but that's slow, so we don't want to do that when we're moving
		 * fast on the music wheel. In that case, we should just keep the banner
		 * that's there (or load a "moving fast" banner). Once we settle down,
		 * we'll get called again and load the real banner. */

		if( m_bMovingFast )
			return false;

		if( IsAFile(path) )
			Load( path );
		else
			LoadFallback();

		return false;
	}

	Load( ID );

	return bLowRes;
}

void FadingBanner::LoadFromSong( const Song* pSong )
{
	if( pSong == nullptr )
	{
		LoadFallback();
		return;
	}

	/* Don't call HasBanner. That'll do disk access and cause the music wheel
	 * to skip. */
	RString sPath = pSong->GetBannerPath();
	if( sPath.empty() )
		LoadFallback();
	else
		LoadFromCachedBanner( sPath );
}

void FadingBanner::LoadMode()
{
	BeforeChange();
	m_Banner[m_iIndexLatest].LoadMode();
}

void FadingBanner::LoadFromSongGroup( RString sSongGroup )
{
	const RString sGroupBannerPath = SONGMAN->GetSongGroupBannerPath( sSongGroup );
	LoadFromCachedBanner( sGroupBannerPath );
}

void FadingBanner::LoadFromCourse( const Course* pCourse )
{
	if( pCourse == nullptr )
	{
		LoadFallback();
		return;
	}

	/* Don't call HasBanner. That'll do disk access and cause the music wheel
	 * to skip. */
	RString sPath = pCourse->GetBannerPath();
	if( sPath.empty() )
		LoadCourseFallback();
	else
		LoadFromCachedBanner( sPath );
}

void FadingBanner::LoadIconFromCharacter( Character* pCharacter )
{
	BeforeChange();
	m_Banner[m_iIndexLatest].LoadIconFromCharacter( pCharacter );
}

void FadingBanner::LoadBannerFromUnlockEntry( const UnlockEntry* pUE )
{
	BeforeChange();
	m_Banner[m_iIndexLatest].LoadBannerFromUnlockEntry( pUE );
}

void FadingBanner::LoadRoulette()
{
	BeforeChange();
	m_Banner[m_iIndexLatest].LoadRoulette();
	m_Banner[m_iIndexLatest].PlayCommand( "Roulette" );
}

void FadingBanner::LoadRandom()
{
	BeforeChange();
	m_Banner[m_iIndexLatest].LoadRandom();
	m_Banner[m_iIndexLatest].PlayCommand( "Random" );
}

void FadingBanner::LoadFromSortOrder( SortOrder so )
{
	BeforeChange();
	m_Banner[m_iIndexLatest].LoadFromSortOrder(so);
}

void FadingBanner::LoadFallback()
{
	BeforeChange();
	m_Banner[m_iIndexLatest].LoadFallback();
}

void FadingBanner::LoadCourseFallback()
{
	BeforeChange();
	m_Banner[m_iIndexLatest].LoadCourseFallback();
}

void FadingBanner::LoadCustom( RString sBanner )
{
	BeforeChange();
	m_Banner[m_iIndexLatest].Load( THEME->GetPathG( "Banner", sBanner ) );
	m_Banner[m_iIndexLatest].PlayCommand( sBanner );
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the FadingBanner. */ 
class LunaFadingBanner: public Luna<FadingBanner>
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
	static int LoadFromSongGroup( T* p, lua_State *L )	{ p->LoadFromSongGroup( SArg(1) ); COMMON_RETURN_SELF; }
	static int LoadRandom( T* p, lua_State *L ) { p->LoadRandom(); COMMON_RETURN_SELF; }
	static int LoadRoulette( T* p, lua_State *L ) { p->LoadRoulette(); COMMON_RETURN_SELF; }
	static int LoadCourseFallback( T* p, lua_State *L ) { p->LoadCourseFallback(); COMMON_RETURN_SELF; }
	static int LoadFallback( T* p, lua_State *L ) { p->LoadFallback(); COMMON_RETURN_SELF; }
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
	static int GetLatestIndex( T* p, lua_State *L ){ lua_pushnumber( L, p->GetLatestIndex() ); return 1; }

	LunaFadingBanner()
	{
		ADD_METHOD( scaletoclipped );
		ADD_METHOD( ScaleToClipped );
		ADD_METHOD( LoadFromSong );
		ADD_METHOD( LoadFromSongGroup );
		ADD_METHOD( LoadFromCourse );
		ADD_METHOD( LoadIconFromCharacter );
		ADD_METHOD( LoadCardFromCharacter );
		ADD_METHOD( LoadRandom );
		ADD_METHOD( LoadRoulette );
		ADD_METHOD( LoadCourseFallback );
		ADD_METHOD( LoadFallback );
		ADD_METHOD( LoadFromSortOrder );
		ADD_METHOD( GetLatestIndex );
		//ADD_METHOD( GetBanner );
	}
};

LUA_REGISTER_DERIVED_CLASS( FadingBanner, ActorFrame )
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
