/* MusicWheel - A wheel with song names used in the Select Music screen. */

#ifndef MUSIC_WHEEL_H
#define MUSIC_WHEEL_H

#include "RageSound.h"
#include "GameConstantsAndTypes.h"
#include "MusicWheelItem.h"
#include "ThemeMetric.h"
#include "WheelBase.h"

#include <vector>


class Course;
class Song;

struct CompareSongPointerArrayBySectionName;

class MusicWheel : public WheelBase
{
	friend struct CompareSongPointerArrayBySectionName;

public:
	virtual ~MusicWheel();
	virtual void Load( RString sType );
	void BeginScreen();

	bool ChangeSort( SortOrder new_so, bool allowSameSort = false );	// return true if change successful
	bool NextSort();						// return true if change successful
	void StartRoulette();
	void StartRandom();
	bool IsRouletting() const;

	virtual bool Select();				// return true if this selection ends the screen
	WheelItemDataType	GetSelectedType()	{ return GetCurWheelItemData(m_iSelection)->m_Type; }
	Song			*GetSelectedSong();
	Course			*GetSelectedCourse()	{ return GetCurWheelItemData(m_iSelection)->m_pCourse; }
	RString			GetSelectedSection()	{ return GetCurWheelItemData(m_iSelection)->m_sText; }

	Song *GetPreferredSelectionForRandomOrPortal();

	bool SelectSong( const Song *p );
	bool SelectCourse( const Course *p );
	bool SelectSection( const RString & SectionName );
	void SetOpenSection( RString group );
	SortOrder GetSortOrder() const { return m_SortOrder; }
	virtual void ChangeMusic( int dist ); /* +1 or -1 */ //CHECK THIS
	void FinishChangingSorts();
	void PlayerJoined();
	// sm-ssc additions
	RString JumpToNextGroup();
	RString JumpToPrevGroup();
	const MusicWheelItemData *GetCurWheelItemData( int i ) { return (const MusicWheelItemData *) m_CurWheelItemData[i]; }

	virtual void ReloadSongList();

	void GetCurrentSections(std::vector<RString> &sections);
	// Lua
	void PushSelf( lua_State *L );

protected:
	MusicWheelItem *MakeItem();

	void GetSongList( std::vector<Song*> &arraySongs, SortOrder so );
	bool SelectSongOrCourse();
	bool SelectModeMenuItem();

	virtual void UpdateSwitch();

	std::vector<MusicWheelItemData *> & getWheelItemsData(SortOrder so);
	void readyWheelItemsData(SortOrder so);

	RString				m_sLastModeMenuItem;
	SortOrder			m_SortOrder;
	RageSound			m_soundChangeSort;

	bool WheelItemIsVisible(int n);

	ThemeMetric<float>		ROULETTE_SWITCH_SECONDS;
	ThemeMetric<int>		ROULETTE_SLOW_DOWN_SWITCHES;
	ThemeMetric<int>		NUM_SECTION_COLORS;
	ThemeMetric<RageColor>		SONG_REAL_EXTRA_COLOR;
	ThemeMetric<RageColor>		SORT_MENU_COLOR;
	ThemeMetric<bool>		SHOW_ROULETTE;
	ThemeMetric<bool>		SHOW_RANDOM;
	ThemeMetric<bool>		SHOW_PORTAL;
	ThemeMetric<bool>		RANDOM_PICKS_LOCKED_SONGS;
	ThemeMetric<int>		MOST_PLAYED_SONGS_TO_SHOW;
	ThemeMetric<int>		RECENT_SONGS_TO_SHOW;
	ThemeMetric<RString>		MODE_MENU_CHOICE_NAMES;
	ThemeMetricMap<RString>		CHOICE;
	ThemeMetric1D<RageColor>	SECTION_COLORS;
	ThemeMetric<LuaReference>	SORT_ORDERS;
	ThemeMetric<bool>		SHOW_EASY_FLAG;
	// sm-ssc additions:
	ThemeMetric<bool>		USE_SECTIONS_WITH_PREFERRED_GROUP;
	ThemeMetric<bool>		HIDE_INACTIVE_SECTIONS;
	ThemeMetric<bool>		HIDE_ACTIVE_SECTION_TITLE;
	ThemeMetric<bool>		REMIND_WHEEL_POSITIONS;
	ThemeMetric<RageColor>	ROULETTE_COLOR;
	ThemeMetric<RageColor>	RANDOM_COLOR;
	ThemeMetric<RageColor>	PORTAL_COLOR;
	ThemeMetric<RageColor>	EMPTY_COLOR;
	std::vector <int> m_viWheelPositions;
	ThemeMetric<RString>	CUSTOM_WHEEL_ITEM_NAMES;
	ThemeMetricMap<RString>	CUSTOM_CHOICES;
	ThemeMetricMap<RageColor>	CUSTOM_CHOICE_COLORS;

private:
	//use getWheelItemsData instead of touching this one
	enum {INVALID,NEEDREFILTER,VALID} m_WheelItemDatasStatus[NUM_SortOrder];
	std::vector<MusicWheelItemData *> m__WheelItemDatas[NUM_SortOrder];
	std::vector<MusicWheelItemData *> m__UnFilteredWheelItemDatas[NUM_SortOrder];

	void BuildWheelItemDatas( std::vector<MusicWheelItemData *> &arrayWheelItems, SortOrder so );
	void FilterWheelItemDatas(std::vector<MusicWheelItemData *> &aUnFilteredDatas, std::vector<MusicWheelItemData *> &aFilteredData, SortOrder so );
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
