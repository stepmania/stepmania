/* MusicBannerWheel - A wheel with song banners used in the Select Music screen. */

#ifndef MUSICBANNERWHEEL_H
#define MUSICBANNERWHEEL_H

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
	void SetScanMode(bool Scanmode);
	void ScanToNextGroup();

private:
	void SetNewPos(int NewPos);
	void LoadSongData();
	void ChangeNotes();
	void InsertNewBanner(int direction);

	bool bScanning;

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

/*
 * (c) 2003 "Frieza"
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
