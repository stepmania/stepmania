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
#include "SongManager.h"
#include "BitmapText.h"
#include "SongManager.h"
#include "PlayerOptions.h"
#include "SongOptions.h"


class MusicBannerWheel : public ActorFrame
{
public:
	MusicBannerWheel();
	~MusicBannerWheel();
	void BannersLeft();
	void BannersRight();
	Song* GetSelectedSong();
	int CheckSongsExist() { return SongsExist; }
	void PlayMusicSample();
	void StartBouncing();
	void StopBouncing();
private:
	void SetNewPos(int NewPos);
	void LoadSongData();
	void ChangeNotes();
	void InsertNewBanner(int direction);

	#ifdef DEBUG
		BitmapText	m_debugtext;
	#endif

	ScrollingList m_ScrollingList;
	int currentPos;
	int scrlistPos;
	
	int SongsExist;
	int SingleLoad;

	vector<Song*> arraySongs;
};

#endif
