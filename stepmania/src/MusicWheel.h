/* MusicWheel - A wheel with song names used in the Select Music screen. */

#ifndef MUSICWHEEL_H
#define MUSICWHEEL_H

#include "AutoActor.h"
#include "ActorFrame.h"
#include "RageSound.h"
#include "RandomSample.h"
#include "GameConstantsAndTypes.h"
#include "MusicSortDisplay.h"
#include "ScreenMessage.h"
#include "ScoreDisplayNormal.h"
#include "ScrollBar.h"
#include "RageTimer.h"
#include "MusicWheelItem.h"
#include "ThemeMetric.h"
class Course;
class Song;

struct CompareSongPointerArrayBySectionName;

class MusicWheel : public ActorFrame
{
	friend struct CompareSongPointerArrayBySectionName;

public:
	MusicWheel();
	~MusicWheel();
	void Load( CString sType );

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	void DrawItem( int index );

	virtual void TweenOnScreen(bool changing_sort);
	virtual void TweenOffScreen(bool changing_sort);
	virtual void TweenOnScreen() { TweenOnScreen(false); }
	virtual void TweenOffScreen() { TweenOffScreen(false); }

	void Move(int n);
	bool ChangeSort( SortOrder new_so );	// return true if change successful
	bool NextSort();		// return true if change successful
	void StartRoulette();
	void StartRandom();
	bool IsRouletting() const;
	/* Return true if we're moving fast automatically. */
	int IsMoving() const;
	bool IsSettled() const;

	void NotesOrTrailChanged( PlayerNumber pn );	// update grade graphics and top score

	void GetItemPosition( float fPosOffsetsFromMiddle, float& fX_out, float& fY_out, float& fZ_out, float& fRotationX_out );
	void SetItemPosition( Actor &item, float fPosOffsetsFromMiddle );

	bool Select();	// return true if this selection ends the screen
	WheelItemType	GetSelectedType()	{ return m_CurWheelItemData[m_iSelection]->m_Type; }
	Song*			GetSelectedSong();
	Course*			GetSelectedCourse()	{ return m_CurWheelItemData[m_iSelection]->m_pCourse; }
	CString			GetSelectedSection(){ return m_CurWheelItemData[m_iSelection]->m_sSectionName; }

	bool WheelIsLocked() { return (m_WheelState == STATE_LOCKED ? true : false); }
	void RebuildAllMusicWheelItems();
	void RebuildMusicWheelItems( int dist );

	Song *GetPreferredSelectionForRandomOrPortal();

	bool SelectSong( Song *p );
	bool SelectSection( const CString & SectionName );
	void SetOpenGroup(CString group, SortOrder so = SORT_INVALID);

protected:
	void GetSongList(vector<Song*> &arraySongs, SortOrder so, CString sPreferredGroup );
	void BuildWheelItemDatas( vector<WheelItemData> &arrayWheelItems, SortOrder so );
	bool SelectSongOrCourse();
	bool SelectCourse( Course *p );
	bool SelectModeMenuItem();
	void ChangeMusic(int dist); /* +1 or -1 */

	ScrollBar			m_ScrollBar;
	AutoActor			m_sprHighlight;

	vector<WheelItemData> m_WheelItemDatas[NUM_SORT_ORDERS];
	vector<WheelItemData *> m_CurWheelItemData;
	vector<MusicWheelItem *> m_MusicWheelItems;
	
	int					m_iSelection;		// index into m_CurWheelItemData
	CString				m_sExpandedSectionName;
	CString				m_sLastModeMenuItem;

	int					m_iSwitchesLeftInSpinDown;		
	float				m_fLockedWheelVelocity;
	/* 0 = none; -1 or 1 = up/down */
	int					m_Moving;
	RageTimer			m_MovingSoundTimer;
	float				m_TimeBeforeMovingBegins;
	float				m_SpinSpeed;
	enum WheelState { 
		STATE_SELECTING_MUSIC,
		STATE_FLYING_OFF_BEFORE_NEXT_SORT, 
		STATE_FLYING_ON_AFTER_NEXT_SORT, 
		STATE_TWEENING_ON_SCREEN, 
		STATE_TWEENING_OFF_SCREEN, 
		STATE_WAITING_OFF_SCREEN,
		STATE_ROULETTE_SPINNING,
		STATE_ROULETTE_SLOWING_DOWN,
		STATE_RANDOM_SPINNING,
		STATE_LOCKED,
	};
	WheelState			m_WheelState;
	float				m_fTimeLeftInState;
	float				m_fPositionOffsetFromSelection;

	RageSound m_soundChangeMusic;
	RageSound m_soundChangeSort;
	RageSound m_soundExpand;
	RageSound m_soundLocked;

	bool WheelItemIsVisible(int n);
	void UpdateScrollbar();


	ThemeMetric<float>	SWITCH_SECONDS;
	ThemeMetric<float> ROULETTE_SWITCH_SECONDS;
	ThemeMetric<int> ROULETTE_SLOW_DOWN_SWITCHES;
	ThemeMetric<float> LOCKED_INITIAL_VELOCITY;
	ThemeMetric<float> SCROLL_BAR_X;
	ThemeMetric<int> SCROLL_BAR_HEIGHT;
	ThemeMetric<float>	ITEM_CURVE_X;
	ThemeMetric<bool> USE_LINEAR_WHEEL;
	ThemeMetric<float>	ITEM_SPACING_Y;
	ThemeMetric<float>	WHEEL_3D_RADIUS;
	ThemeMetric<float>	CIRCLE_PERCENT;
	ThemeMetric<int> NUM_SECTION_COLORS;
	ThemeMetric<RageColor> SONG_REAL_EXTRA_COLOR;
	ThemeMetric<RageColor> SORT_MENU_COLOR;
	ThemeMetric<bool> SHOW_ROULETTE;
	ThemeMetric<bool> SHOW_RANDOM;
	ThemeMetric<bool> SHOW_PORTAL;
	ThemeMetric<bool>	USE_3D;
	ThemeMetric<float>	NUM_WHEEL_ITEMS_TO_DRAW;
	ThemeMetric<int> MOST_PLAYED_SONGS_TO_SHOW;
	ThemeMetric<CString> MODE_MENU_CHOICE_NAMES;
	ThemeMetricMap<CString> CHOICE;
	ThemeMetric<float> WHEEL_ITEM_ON_DELAY_CENTER;
	ThemeMetric<float> WHEEL_ITEM_ON_DELAY_OFFSET;
	ThemeMetric<float> WHEEL_ITEM_OFF_DELAY_CENTER;
	ThemeMetric<float> WHEEL_ITEM_OFF_DELAY_OFFSET;
	ThemeMetric1D<RageColor> SECTION_COLORS;
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
