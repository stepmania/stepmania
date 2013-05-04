#ifndef MUSIC_WHEEL_ITEM_H
#define MUSIC_WHEEL_ITEM_H

#include "ActorFrame.h"
#include "BitmapText.h"
#include "WheelNotifyIcon.h"
#include "TextBanner.h"
#include "GameConstantsAndTypes.h"
#include "GameCommand.h"
#include "WheelItemBase.h"
#include "AutoActor.h"
#include "ThemeMetric.h"

class Course;
class Song;

struct MusicWheelItemData;

enum MusicWheelItemType
{
	MusicWheelItemType_Song,
	MusicWheelItemType_SectionExpanded,
	MusicWheelItemType_SectionCollapsed,
	MusicWheelItemType_Roulette,
	MusicWheelItemType_Course,
	MusicWheelItemType_Sort,
	MusicWheelItemType_Mode,
	MusicWheelItemType_Random,
	MusicWheelItemType_Portal,
	MusicWheelItemType_Custom,
	NUM_MusicWheelItemType,
	MusicWheelItemType_Invalid,
};
const RString& MusicWheelItemTypeToString( MusicWheelItemType i );
/** @brief An item on the MusicWheel. */
class MusicWheelItem : public WheelItemBase
{
public:
	MusicWheelItem(RString sType = "MusicWheelItem");
	MusicWheelItem( const MusicWheelItem &cpy );
	virtual ~MusicWheelItem();
	virtual MusicWheelItem *Copy() const { return new MusicWheelItem(*this); }

	virtual void LoadFromWheelItemData( const WheelItemBaseData* pWID, int iIndex, bool bHasFocus, int iDrawIndex );
	virtual void HandleMessage( const Message &msg );
	void RefreshGrades();

private:
	ThemeMetric<bool>	GRADES_SHOW_MACHINE;

	AutoActor		m_sprColorPart[NUM_MusicWheelItemType];
	AutoActor		m_sprNormalPart[NUM_MusicWheelItemType];
	AutoActor		m_sprOverPart[NUM_MusicWheelItemType];

	TextBanner		m_TextBanner;	// used by Type_Song instead of m_pText
	BitmapText		*m_pText[NUM_MusicWheelItemType];
	BitmapText		*m_pTextSectionCount;

	WheelNotifyIcon		m_WheelNotifyIcon;
	AutoActor		m_pGradeDisplay[NUM_PLAYERS];
};

struct MusicWheelItemData : public WheelItemBaseData
{
	MusicWheelItemData() : m_pCourse(nullptr), m_pSong(nullptr), m_Flags(),
		m_iSectionCount(0), m_sLabel(""), m_pAction() { }
	MusicWheelItemData( WheelItemDataType type, Song* pSong, 
			   RString sSectionName, Course* pCourse, 
			   RageColor color, int iSectionCount );

	Course*			m_pCourse;
	Song*			m_pSong;
	WheelNotifyIcon::Flags  m_Flags;

	// for TYPE_SECTION
	int			m_iSectionCount;

	// for TYPE_SORT
	RString			m_sLabel;
	HiddenPtr<GameCommand>	m_pAction;
};

#endif

/**
 * @file
 * @author Chris Danford, Chris Gomez, Glenn Maynard (c) 2001-2004
 * @section LICENSE
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
