#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: Banner.h

 Desc: The song's banner displayed in SelectSong.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "RageUtil.h"

#include "Banner.h"
#include "ThemeManager.h"
#include "RageBitmapTexture.h"


bool Banner::LoadFromSong( Song* pSong )		// NULL means no song
{
	Sprite::TurnShadowOff();

	if( pSong == NULL )					Banner::Load( THEME->GetPathTo(GRAPHIC_FALLBACK_BANNER), TEXTURE_HINT_NOMIPMAPS );
	else if( pSong->HasBanner() )		Banner::Load( pSong->GetBannerPath(), TEXTURE_HINT_NOMIPMAPS );
	else if( pSong->HasBackground() )	Banner::Load( pSong->GetBackgroundPath(), TEXTURE_HINT_NOMIPMAPS );
	else								Banner::Load( THEME->GetPathTo(GRAPHIC_FALLBACK_BANNER), TEXTURE_HINT_NOMIPMAPS );

	return true;
}


