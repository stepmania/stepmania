#ifndef MUSICLIST_H
#define MUSICLIST_H

#include "ActorFrame.h"
#include "BitmapText.h"
#include "Song.h"

const int MAX_MLIST_COLUMNS = 5;

class MusicList : public ActorFrame {
	BitmapText		m_textTitles[MAX_MLIST_COLUMNS];
	
	struct group {
		CString ContentsText[MAX_MLIST_COLUMNS];
		int m_iNumSongsInGroup;
	};

	vector<group> m_ContentsText;

	int NumGroups, CurGroup;
	
public:
	MusicList();

	/* Add a new group. */
	void AddGroup();

	/* Add songs to the group that was just added. */
	void AddSongsToGroup(const vector<Song*> &songs);

	/* Set the displayed group number. */
	void SetGroupNo(int group);

	void TweenOnScreen();
	void TweenOffScreen();
	int GetNumSongs() const { return m_ContentsText[CurGroup].m_iNumSongsInGroup; }
};

#endif
