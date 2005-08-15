#include "global.h"
#include "FadingBanner.h"
#include "RageTextureManager.h"
#include "BannerCache.h"
#include "song.h"
#include "RageLog.h"
#include "Course.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "SongManager.h"
#include "ThemeMetric.h"
#include "ActorUtil.h"

REGISTER_ACTOR_CLASS( FadingBanner )

/*
 * Allow fading from one banner to another.  We can handle two fades at once;
 * this is used to fade from an old banner to a low-quality banner to a high-
 * quality banner smoothly.
 *
 * m_iIndexLatest is the latest banner loaded, and the one that we'll end up
 * displaying when the fades stop.
 */
FadingBanner::FadingBanner()
{
	m_bMovingFast = false;
	m_bSkipNextBannerUpdate = false;
	m_iIndexLatest = 0;
	for( int i=0; i<NUM_BANNERS; i++ )
	{
		m_Banner[i].SetName( "Banner" );
		ActorUtil::OnCommand( m_Banner[i], "FadingBanner" );
		this->AddChild( &m_Banner[i] );
	}
}

void FadingBanner::ScaleToClipped( float fWidth, float fHeight )
{
	for( int i=0; i<NUM_BANNERS; i++ )
		m_Banner[i].ScaleToClipped( fWidth, fHeight );
}

void FadingBanner::Update( float fDeltaTime )
{
	// update children manually
	// ActorFrame::Update( fDeltaTime );
	Actor::Update( fDeltaTime );

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

	/* Render the latest banner first. */
	for( int i = 0; i < NUM_BANNERS; ++i )
	{
		int index = m_iIndexLatest - i;
		wrap( index, NUM_BANNERS );
		m_Banner[index].Draw();
	}
}

bool FadingBanner::Load( RageTextureID ID, bool bLowResToHighRes )
{
	BeforeChange( bLowResToHighRes );
	bool bRet = m_Banner[m_iIndexLatest].Load(ID);
	return bRet;
}

/* If bLowResToHighRes is true, we're fading from a low-res banner to the
 * corresponding high-res banner. */
void FadingBanner::BeforeChange( bool bLowResToHighRes )
{
	CString sCommand;
	if( bLowResToHighRes )
		sCommand = "FadeFromCached";
	else
		sCommand = "FadeOff";

	m_Banner[m_iIndexLatest].PlayCommand( sCommand );
	++m_iIndexLatest;
	wrap( m_iIndexLatest, NUM_BANNERS );

	m_Banner[m_iIndexLatest].PlayCommand( "ResetFade" );

	/* We're about to load a banner.  It'll probably cause a frame skip or
	 * two.  Skip an update, so the fade-in doesn't skip. */
	m_bSkipNextBannerUpdate = true;
}

/* If this returns true, a low-resolution banner was loaded, and the full-res
 * banner should be loaded later. */
bool FadingBanner::LoadFromCachedBanner( const CString &path )
{
	/* If we're already on the given banner, don't fade again. */
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
	bool bLowRes = (PREFSMAN->m_BannerCache != PrefsManager::BNCACHE_FULL);
	if( !bLowRes )
	{
		ID = Sprite::SongBannerTexture( path );
	}
	else
	{
		/* Try to load the low quality version. */
		ID = BANNERCACHE->LoadCachedBanner( path );
	}

	if( !TEXTUREMAN->IsTextureRegistered(ID) )
	{
		/* Oops.  We couldn't load a banner quickly.  We can load the actual
		 * banner, but that's slow, so we don't want to do that when we're moving
		 * fast on the music wheel.  In that case, we should just keep the banner
		 * that's there (or load a "moving fast" banner).  Once we settle down,
		 * we'll get called again and load the real banner. */

		if( m_bMovingFast )
			return false;

		if( IsAFile(path) )
			Load( path );
		else
			LoadFallback();

		return false;
	}

	BeforeChange();
	m_Banner[m_iIndexLatest].Load( ID );

	return bLowRes;
}

void FadingBanner::LoadFromSong( const Song* pSong )
{
	if( pSong == NULL )
	{
		LoadFallback();
		return;
	}

	/* Don't call HasBanner.  That'll do disk access and cause the music wheel
	 * to skip. */
	CString sPath = pSong->GetBannerPath();
	if( sPath.empty() )
		LoadFallback();
	else
		LoadFromCachedBanner( sPath );
}

void FadingBanner::LoadAllMusic()
{
	BeforeChange();
	m_Banner[m_iIndexLatest].LoadAllMusic();
}

void FadingBanner::LoadMode()
{
	BeforeChange();
	m_Banner[m_iIndexLatest].LoadMode();
}

void FadingBanner::LoadFromSongGroup( CString sSongGroup )
{
	const CString sGroupBannerPath = SONGMAN->GetSongGroupBannerPath( sSongGroup );
	LoadFromCachedBanner( sGroupBannerPath );
}

void FadingBanner::LoadFromCourse( const Course* pCourse )
{
	if( pCourse == NULL )
	{
		LoadFallback();
		return;
	}

	/* Don't call HasBanner.  That'll do disk access and cause the music wheel
	 * to skip. */
	CString sPath = pCourse->m_sBannerPath;
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

void FadingBanner::LoadRoulette()
{
	BeforeChange();
	m_Banner[m_iIndexLatest].LoadRoulette();
}

void FadingBanner::LoadRandom()
{
	BeforeChange();
	m_Banner[m_iIndexLatest].LoadRandom();
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


// lua start
#include "LuaBinding.h"

class LunaFadingBanner: public Luna<FadingBanner>
{
public:
	LunaFadingBanner() { LUA->Register( Register ); }

	static int ScaleToClipped( T* p, lua_State *L )			{ p->ScaleToClipped(FArg(1),FArg(2)); return 0; }
	static int LoadFromSong( T* p, lua_State *L )
	{ 
		if( lua_isnil(L,1) ) { p->LoadFromSong( NULL ); }
		else { Song *pS = Luna<Song>::check(L,1); p->LoadFromSong( pS ); }
		return 0;
	}
	static int LoadFromCourse( T* p, lua_State *L )
	{ 
		if( lua_isnil(L,1) ) { p->LoadFromCourse( NULL ); }
		else { Course *pC = Luna<Course>::check(L,1); p->LoadFromCourse( pC ); }
		return 0;
	}
	static int LoadIconFromCharacter( T* p, lua_State *L )
	{ 
		if( lua_isnil(L,1) ) { p->LoadIconFromCharacter( NULL ); }
		else { Character *pC = Luna<Character>::check(L,1); p->LoadIconFromCharacter( pC ); }
		return 0;
	}
	static int LoadCardFromCharacter( T* p, lua_State *L )
	{ 
		if( lua_isnil(L,1) ) { p->LoadIconFromCharacter( NULL ); }
		else { Character *pC = Luna<Character>::check(L,1); p->LoadIconFromCharacter( pC ); }
		return 0;
	}

	static void Register(lua_State *L) 
	{
		ADD_METHOD( ScaleToClipped )
		ADD_METHOD( LoadFromSong )
		ADD_METHOD( LoadFromCourse )
		ADD_METHOD( LoadIconFromCharacter )
		ADD_METHOD( LoadCardFromCharacter )
		Luna<T>::Register( L );
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
