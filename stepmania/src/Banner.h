#pragma once
/*
-----------------------------------------------------------------------------
 File: Banner.h

 Desc: The song's banner displayed in SelectSong.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
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
		m_bScrolling = false;
		m_fPercentScrolling = 0;

		m_fCropWidth = BANNER_WIDTH;
		m_fCropHeight = BANNER_HEIGHT;
	};

	virtual bool Load( CString sFilePath, bool bForceReload = false, int iMipMaps = 0, int iAlphaBits = 0, bool bDither = false, bool bStretch = false );

	virtual void Update( float fDeltaTime );

	bool LoadFromSong( Song* pSong );		// NULL means no song
	bool LoadFromGroup( CString sGroupName );
	bool LoadRoulette();

protected:
	bool m_bScrolling;
	float m_fPercentScrolling;
};
