#ifndef MUSICBANNERWHEEL_H
#define MUSICBANNERWHEEL_H
/*
-----------------------------------------------------------------------------
 Class: MusicBannerWheel

 Desc: A wheel with song banners used in the Select Music screen.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Andrew Livy
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "Banner.h"
#include "Sprite.h"
#include "ScrollingList.h"

class MusicBannerWheel : public ActorFrame
{
public:
	MusicBannerWheel();
	~MusicBannerWheel();
	void BannersLeft();
	void BannersRight();
	Song* GetSelectedSong();
private:
	void SetNewPos(int NewPos);
	void PlayMusicSample();

	ScrollingList m_ScrollingList;
	int currentPos;
};

#endif
