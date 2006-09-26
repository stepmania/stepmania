/* MusicWheel - A wheel with song names used in the Select Music screen. */

#ifndef MUSIC_WHEEL_H
#define MUSIC_WHEEL_H

#include "AutoActor.h"
#include "ActorFrame.h"
#include "RageSound.h"
#include "GameConstantsAndTypes.h"
#include "ScreenMessage.h"
#include "ScoreDisplayNormal.h"
#include "ScrollBar.h"
#include "RageTimer.h"
#include "MusicWheelItem.h"
#include "ThemeMetric.h"
#include "WheelBase.h"

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

	bool ChangeSort( SortOrder new_so );		// return true if change successful
	bool NextSort();				// return true if change successful
	void StartRoulette();
	void StartRandom();
	bool IsRouletting() const;

	virtual bool Select();				// return true if this selection ends the screen
	WheelItemType	GetSelectedType()	{ return GetCurWheelItemData(m_iSelection)->m_Type; }
	Song		*GetSelectedSong();
	Course		*GetSelectedCourse()	{ return GetCurWheelItemData(m_iSelection)->m_pCourse; }
	RString		GetSelectedSection()	{ return GetCurWheelItemData(m_iSelection)->m_sText; }

	Song *GetPreferredSelectionForRandomOrPortal();

	bool SelectSong( const Song *p );
	bool SelectSection( const RString & SectionName );
	void SetOpenGroup( RString group );
	SortOrder GetSortOrder() const { return m_SortOrder; }
	virtual void ChangeMusic( int dist ); /* +1 or -1 */ //CHECK THIS
	void FinishChangingSorts();

protected:
	WheelItemBase *MakeItem();

	void GetSongList( vector<Song*> &arraySongs, SortOrder so, const RString &sPreferredGroup );
	void BuildWheelItemDatas( vector<WheelItemData *> &arrayWheelItems, SortOrder so );
	bool SelectSongOrCourse();
	bool SelectCourse( const Course *p );
	bool SelectModeMenuItem();

	virtual void UpdateSwitch();

	vector<WheelItemData *>		m_WheelItemDatas[NUM_SortOrder];
	const WheelItemData *GetCurWheelItemData( int i ) { return (const WheelItemData *) m_CurWheelItemData[i]; }
	
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
	ThemeMetric<RString>		MODE_MENU_CHOICE_NAMES;
	ThemeMetricMap<RString>		CHOICE;
	ThemeMetric1D<RageColor>	SECTION_COLORS;
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
