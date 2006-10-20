/* MusicWheelItem - An item on the music wheel. */

#ifndef MUSIC_WHEEL_ITEM_H
#define MUSIC_WHEEL_ITEM_H

#include "ActorFrame.h"
#include "GradeDisplay.h"
#include "BitmapText.h"
#include "WheelNotifyIcon.h"
#include "TextBanner.h"
#include "GameConstantsAndTypes.h"
#include "GameCommand.h"
#include "WheelItemBase.h"

class Course;
class Song;

struct WheelItemData;

class MusicWheelItem : public WheelItemBase
{
public:
	MusicWheelItem(RString sType = "MusicWheelItem");
	MusicWheelItem( const MusicWheelItem &cpy );
	virtual ~MusicWheelItem();
	virtual MusicWheelItem *Copy() const { return new MusicWheelItem(*this); }

	virtual void LoadFromWheelItemData( const WheelItemBaseData* pWID );
	virtual void HandleMessage( const RString& sMessage );
	void RefreshGrades();

	const WheelItemData *data;

private:
	Sprite			m_sprSongBar;
	Sprite			m_sprSectionBar;
	Sprite			m_sprExpandedBar;
	Sprite			m_sprModeBar;
	Sprite			m_sprSortBar;
	WheelNotifyIcon		m_WheelNotifyIcon;
	TextBanner		m_TextBanner;
	BitmapText		m_textSection;
	BitmapText		m_textRoulette;
	BitmapText		m_textCourse;
	BitmapText		m_textSort;
	GradeDisplay		*m_pGradeDisplay[NUM_PLAYERS];
};

struct WheelItemData : public WheelItemBaseData
{
	WheelItemData() {}
	WheelItemData( WheelItemType wit, Song* pSong, RString sSectionName, Course* pCourse, RageColor color );

	Course*			m_pCourse;
	Song*			m_pSong;

	// for TYPE_SORT
	RString			m_sLabel;
	HiddenPtr<GameCommand>	m_pAction;
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
