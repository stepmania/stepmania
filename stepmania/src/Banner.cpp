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

	Banner::Load( THEME->GetPathTo("Graphics","fallback banner") );
}

bool Banner::Load( CString sFilePath, RageTexturePrefs prefs )
{
	// note that the defaults are changes for faster loading
	return CroppedSprite::Load( sFilePath, prefs );
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

bool Banner::LoadFromSong( Song* pSong )		// NULL means no song
{
	m_bScrolling = false;

	Sprite::TurnShadowOff();

	if( pSong == NULL )					Banner::Load( THEME->GetPathTo("Graphics","fallback banner") );
	else if( pSong->HasBanner() )		Banner::Load( pSong->GetBannerPath() );
	else if( PREFSMAN->m_bUseBGIfNoBanner  &&  pSong->HasBackground() )	Banner::Load( pSong->GetBackgroundPath() );
	else								Banner::Load( THEME->GetPathTo("Graphics","fallback banner") );

	return true;
}

bool Banner::LoadFromGroup( CString sGroupName )
{
	m_bScrolling = false;

	CString sGroupBannerPath = SONGMAN->GetGroupBannerPath( sGroupName );

	if( sGroupName == "ALL MUSIC" )
		Banner::Load( THEME->GetPathTo("Graphics","all music banner") );
	else if( sGroupBannerPath != "" )
		Banner::Load( sGroupBannerPath );
	else
		Banner::Load( THEME->GetPathTo("Graphics","fallback banner") );

	return true;
}

bool Banner::LoadFromCourse( Course* pCourse )		// NULL means no course
{
	m_bScrolling = false;

	Sprite::TurnShadowOff();

	if( pCourse == NULL )						Banner::Load( THEME->GetPathTo("Graphics","fallback banner") );
	else if( pCourse->m_sBannerPath != "" )		Banner::Load( pCourse->m_sBannerPath );
	else										Banner::Load( THEME->GetPathTo("Graphics","fallback banner") );

	return true;
}

bool Banner::LoadRoulette()
{
	RageTexturePrefs prefs;
	prefs.iMipMaps = 0;
	Banner::Load( THEME->GetPathTo("Graphics","select music roulette banner"), prefs );
	m_bScrolling = true;

	return true;
}
