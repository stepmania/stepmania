#pragma once
/*
-----------------------------------------------------------------------------
 Class: BannerWithFrame

 Desc: bonus meters and max combo number.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "Sprite.h"
#include "Banner.h"


class BannerWithFrame : public ActorFrame
{
public:
	BannerWithFrame();

	void LoadFromSong( Song* pSong );

protected:

	Sprite		m_sprBannerFrame;
	Banner		m_Banner;
};
