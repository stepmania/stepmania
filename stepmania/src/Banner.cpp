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
#include "ThemeManager.h"
#include "SongManager.h"
#include "RageBitmapTexture.h"

bool Banner::Load( CString sFilePath, bool bForceReload, int iMipMaps, int iAlphaBits, bool bDither, bool bStretch )
{
	// note that the defaults are changes for faster loading
	return CroppedSprite::Load( sFilePath, bForceReload, iMipMaps, iAlphaBits, bDither, bStretch );
};

void Banner::Update( float fDeltaTime )
{
	CroppedSprite::Update( fDeltaTime );

	if( m_bScrolling )
	{
        m_fPercentScrolling += fDeltaTime/2;
		m_fPercentScrolling -= (int)m_fPercentScrolling;

		FRECT* pTextureRect = m_pTexture->GetTextureCoordRect(0);

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

bool Banner::LoadFromSong( Song* pSong )		// NULL means no song
{
	m_bScrolling = false;

	Sprite::TurnShadowOff();

	if( pSong == NULL )					Banner::Load( THEME->GetPathTo(GRAPHIC_FALLBACK_BANNER) );
	else if( pSong->HasBanner() )		Banner::Load( pSong->GetBannerPath() );
//	else if( pSong->HasBackground() )	Banner::Load( pSong->GetBackgroundPath() );
	else								Banner::Load( THEME->GetPathTo(GRAPHIC_FALLBACK_BANNER) );

	return true;
}

bool Banner::LoadFromGroup( CString sGroupName )
{
	m_bScrolling = false;

	CString sGroupBannerPath = SONGMAN->GetGroupBannerPath( sGroupName );

	if( sGroupBannerPath != "" )
		Banner::Load( sGroupBannerPath );
	else
		Banner::Load( THEME->GetPathTo(GRAPHIC_SELECT_MUSIC_SECTION_BANNER) );

	return true;
}

bool Banner::LoadFromCourse( Course* pCourse )		// NULL means no course
{
	m_bScrolling = false;

	Sprite::TurnShadowOff();

	if( pCourse == NULL )						Banner::Load( THEME->GetPathTo(GRAPHIC_FALLBACK_BANNER) );
	else if( pCourse->m_sBannerPath != "" )		Banner::Load( pCourse->m_sBannerPath );
	else										Banner::Load( THEME->GetPathTo(GRAPHIC_FALLBACK_BANNER) );

	return true;
}

bool Banner::LoadRoulette()
{
	Banner::Load( THEME->GetPathTo(GRAPHIC_SELECT_MUSIC_ROULETTE_BANNER), false, 0, 0, false, false );
	m_bScrolling = true;

	return true;
}
