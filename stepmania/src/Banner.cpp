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
		float fTexCoords[8] = 
		{
            0+m_fPercentScrolling, 1,	// bottom left
			0+m_fPercentScrolling, 0,	// top left
			1+m_fPercentScrolling, 1,	// bottom right
			1+m_fPercentScrolling, 0,	// top right
		};
		Sprite::SetCustomTextureCoords( fTexCoords );
	}
}

bool Banner::LoadFromSong( Song* pSong )		// NULL means no song
{
	TurnOffRoulette();

	Sprite::TurnShadowOff();

	if( pSong == NULL )					Banner::Load( THEME->GetPathTo(GRAPHIC_FALLBACK_BANNER) );
	else if( pSong->HasBanner() )		Banner::Load( pSong->GetBannerPath() );
	else if( pSong->HasBackground() )	Banner::Load( pSong->GetBackgroundPath() );
	else								Banner::Load( THEME->GetPathTo(GRAPHIC_FALLBACK_BANNER) );

	return true;
}

bool Banner::LoadFromGroup( CString sGroupName )
{
	TurnOffRoulette();

	CString sGroupBannerPath = SONGMAN->GetGroupBannerPath( sGroupName );

	if( sGroupBannerPath == "" )
		Banner::Load( sGroupBannerPath );
	else
		Banner::Load( THEME->GetPathTo(GRAPHIC_FALLBACK_BANNER) );

	return true;
}

bool Banner::LoadRoulette()
{
	Banner::Load( THEME->GetPathTo(GRAPHIC_SELECT_MUSIC_ROULETTE_BANNER), false, 0, 0, false, true );
	TurnOnRoulette();

	return true;
}

void Banner::TurnOnRoulette()
{
	m_bScrolling = true;
	m_fPercentScrolling = 0;

	SetEffectGlowing( 1, D3DXCOLOR(1,0,0,0), D3DXCOLOR(1,0,0,0.5f) );
}

void Banner::TurnOffRoulette()
{
	m_bScrolling = false;

	SetEffectNone();
}

