#pragma once
/*
-----------------------------------------------------------------------------
 File: Banner.h

 Desc: The song's banner displayed in SelectSong.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "CroppedSprite.h"
#include "Song.h"


const float BANNER_WIDTH	= 286;
const float BANNER_HEIGHT	= 92;


class Banner : public CroppedSprite
{
public:
	Banner()
	{
		m_fCropWidth = BANNER_WIDTH;
		m_fCropHeight = BANNER_HEIGHT;
	};

	bool LoadFromSong( Song* pSong );		// NULL means no song

};
