/* MusicWheelItem - An item on the music wheel. */

#ifndef MUSICWHEELITEM_H
#define MUSICWHEELITEM_H

#include "ActorFrame.h"
#include "GradeDisplay.h"
#include "BitmapText.h"
#include "WheelNotifyIcon.h"
#include "TextBanner.h"
#include "GameConstantsAndTypes.h"
#include "GameCommand.h"
class Course;
class Song;

struct WheelItemData;

class MusicWheelItem : public ActorFrame
{
public:
	MusicWheelItem();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void LoadFromWheelItemData( WheelItemData* pWID, bool bExpanded );
	void RefreshGrades();

	WheelItemData *data;
	float				m_fPercentGray;

	Sprite				m_sprSongBar;
	Sprite				m_sprSectionBar;
	Sprite				m_sprExpandedBar;
	WheelNotifyIcon		m_WheelNotifyIcon;
	TextBanner			m_TextBanner;
	BitmapText			m_textSectionName;
	BitmapText			m_textRoulette;
	BitmapText			m_textCourse;
	BitmapText			m_textSort;
	GradeDisplay		m_GradeDisplay[NUM_PLAYERS];
};

enum WheelItemType 
{
	TYPE_SECTION, 
	TYPE_SONG, 
	TYPE_ROULETTE, 
	TYPE_RANDOM, 
	TYPE_PORTAL, 
	TYPE_COURSE, 
	TYPE_SORT 
};

struct WheelItemData
{
	WheelItemData() {}
	WheelItemData( WheelItemType wit, Song* pSong, CString sSectionName, Course* pCourse, RageColor color );

	WheelItemType	m_Type;
	CString			m_sSectionName;
	Course*			m_pCourse;
	Song*			m_pSong;
	RageColor		m_color;	// either text color or section background color
	WheelNotifyIcon::Flags  m_Flags;

	// for TYPE_SORT
	CString			m_sLabel;
	GameCommand		m_Action;
};

#endif

/*
 * (c) 2001-2004 Chris Danford, Chris Gomez, Glenn Maynard
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
