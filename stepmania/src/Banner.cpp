#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: Banner.h

 Desc: The song's banner displayed in SelectSong.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "RageUtil.h"

#include "Banner.h"
#include "PrefsManager.h"
#include "SongManager.h"
#include "RageBitmapTexture.h"
#include "ThemeManager.h"

Banner::Banner()
{
	m_bScrolling = false;
	m_fPercentScrolling = 0;

	LoadFallback();
}

bool Banner::Load( RageTextureID ID )
{
	// note that the defaults are changes for faster loading
	return CroppedSprite::Load( ID );
};

void Banner::Update( float fDeltaTime )
{
	CroppedSprite::Update( fDeltaTime );

	if( m_bScrolling )
	{
        m_fPercentScrolling += fDeltaTime/2;
		m_fPercentScrolling -= (int)m_fPercentScrolling;

		const RectF *pTextureRect = m_pTexture->GetTextureCoordRect(0);

		float fTexCoords[8] = 
		{
			0+m_fPercentScrolling, pTextureRect->bottom,	// bottom left
			0+m_fPercentScrolling, pTextureRect->top,		// top left
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

	/* Set up the texture coord rects for the current state. */
	Update(0);
}
#include "RageLog.h"

void Banner::LoadFromSong( Song* pSong )		// NULL means no song
{
	m_bScrolling = false;

	Sprite::TurnShadowOff();

	LOG->Trace("xxxxxxx");
	if( pSong == NULL )					LoadFallback();
	else if( pSong->HasBanner() )
	{
		/* Song banners often have HOT PINK color keys. */
	LOG->Trace("xxxxxxx 2 %s", pSong->GetBannerPath().GetString());
		RageTextureID ID(pSong->GetBannerPath());
		ID.bHotPinkColorKey = true;
		Load( ID );
	}
	else if( PREFSMAN->m_bUseBGIfNoBanner  &&  pSong->HasBackground() )	Load( pSong->GetBackgroundPath() );
	else								LoadFallback();
}

void Banner::LoadFromGroup( CString sGroupName )
{
	m_bScrolling = false;

	CString sGroupBannerPath = SONGMAN->GetGroupBannerPath( sGroupName );

	if( sGroupName == "ALL MUSIC" )
		Load( THEME->GetPathTo("Graphics","all music banner") );
	else if( sGroupBannerPath != "" )
		Load( sGroupBannerPath );
	else
		LoadFallback();
}

void Banner::LoadFromCourse( Course* pCourse )		// NULL means no course
{
	m_bScrolling = false;

	Sprite::TurnShadowOff();

	if( pCourse == NULL )						LoadFallback();
	else if( pCourse->m_sBannerPath != "" )		Load( pCourse->m_sBannerPath );
	else										LoadFallback();
}

void Banner::LoadFallback()
{
	Load( THEME->GetPathTo("Graphics","fallback banner") );
}

void Banner::LoadRoulette()
{
	RageTextureID ID(THEME->GetPathTo("Graphics","select music roulette banner"));
	ID.iMipMaps = 0;
	Load( ID );
	m_bScrolling = true;
}
