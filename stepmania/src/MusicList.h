#ifndef MUSICLIST_H
#define MUSICLIST_H

#include "ActorFrame.h"
#include "BitmapText.h"
#include "song.h"

const int MAX_MLIST_COLUMNS = 5;

class MusicList : public ActorFrame
{
	BitmapText		m_textTitles[MAX_MLIST_COLUMNS];
	
	struct group {
		CString ContentsText[MAX_MLIST_COLUMNS];
		int m_iNumSongsInGroup;
	};

	vector<group> m_ContentsText;

	int NumGroups, CurGroup;
	
public:
	MusicList();
	void Load();

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

/*
 * (c) 2001-2003 Chris Danford
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
