#include "global.h"
/*
-----------------------------------------------------------------------------
 File: Banner.h

 Desc: The song's banner displayed in SelectSong.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "Banner.h"
#include "PrefsManager.h"
#include "SongManager.h"
#include "RageBitmapTexture.h"
#include "ThemeManager.h"
#include "RageUtil.h"
#include "song.h"
#include "RageTextureManager.h"
#include "Course.h"
#include "Character.h"


Banner::Banner()
{
	m_bScrolling = false;
	m_fPercentScrolling = 0;

	TEXTUREMAN->CacheTexture( SongBannerTexture(THEME->GetPathToG("Banner all music")) );
	TEXTUREMAN->CacheTexture( SongBannerTexture(THEME->GetPathToG("Common fallback banner")) );
	TEXTUREMAN->CacheTexture( SongBannerTexture(THEME->GetPathToG("Banner roulette")) );
	TEXTUREMAN->CacheTexture( SongBannerTexture(THEME->GetPathToG("Banner random")) );
}

bool Banner::Load( RageTextureID ID )
{
	ID = SongBannerTexture(ID);

	m_fPercentScrolling = 0;
	m_bScrolling = false;

	TEXTUREMAN->DisableOddDimensionWarning();
	TEXTUREMAN->VolatileTexture( ID );
	bool ret = Sprite::Load( ID );
	TEXTUREMAN->EnableOddDimensionWarning();

	return ret;
};

void Banner::Update( float fDeltaTime )
{
	Sprite::Update( fDeltaTime );

	if( m_bScrolling )
	{
        m_fPercentScrolling += fDeltaTime/2;
		m_fPercentScrolling -= (int)m_fPercentScrolling;

		const RectF *pTextureRect = m_pTexture->GetTextureCoordRect(0);
 
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

	/* Set up the texture coord rects for the current state. */
	Update(0);
}

void Banner::LoadFromSong( Song* pSong )		// NULL means no song
{
	if( pSong == NULL )					LoadFallback();
	else if( pSong->HasBanner() )		Load( pSong->GetBannerPath() );
	else								LoadFallback();

	m_bScrolling = false;
}

void Banner::LoadAllMusic()
{
	Load( THEME->GetPathToG("Banner all") );
	m_bScrolling = false;
}

void Banner::LoadFromGroup( CString sGroupName )
{
	CString sGroupBannerPath = SONGMAN->GetGroupBannerPath( sGroupName );
	if( sGroupBannerPath != "" )	Load( sGroupBannerPath );
	else							LoadFallback();
	m_bScrolling = false;
}

void Banner::LoadFromCourse( Course* pCourse )		// NULL means no course
{
	if( pCourse == NULL )						LoadFallback();
	else if( pCourse->m_sBannerPath != "" )		Load( pCourse->m_sBannerPath );
	else										LoadFallback();

	m_bScrolling = false;
}

void Banner::LoadCardFromCharacter( Character* pCharacter )	
{
	ASSERT( pCharacter );

	if( pCharacter->GetCardPath() != "" )		Load( pCharacter->GetCardPath() );
	else										LoadFallback();

	m_bScrolling = false;
}

void Banner::LoadIconFromCharacter( Character* pCharacter )	
{
	ASSERT( pCharacter );

	if( pCharacter->GetIconPath() != "" )		Load( pCharacter->GetIconPath() );
	else if( pCharacter->GetCardPath() != "" )	Load( pCharacter->GetCardPath() );
	else										LoadFallback();

	m_bScrolling = false;
}

void Banner::LoadTABreakFromCharacter( Character* pCharacter )
{
	if( pCharacter == NULL )					Load( THEME->GetPathToG("Common fallback takingabreak") );
	else 
	{
		Load( pCharacter->GetTakingABreakPath() );
		m_bScrolling = false;
	}
}

void Banner::LoadFallback()
{
	Load( THEME->GetPathToG("Common fallback banner") );
}

void Banner::LoadRoulette()
{
	Load( THEME->GetPathToG("Banner roulette") );
	m_bScrolling = true;
}

void Banner::LoadRandom()
{
	Load( THEME->GetPathToG("Banner random") );
	m_bScrolling = true;
}
