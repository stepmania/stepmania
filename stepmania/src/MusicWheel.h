#ifndef MUSICWHEEL_H
#define MUSICWHEEL_H
/*
-----------------------------------------------------------------------------
 Class: MusicWheel

 Desc: A wheel with song names used in the Select Music screen.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"
#include "song.h"
#include "ActorFrame.h"
#include "BitmapText.h"
#include "Quad.h"
#include "TextBanner.h"
#include "RandomSample.h"
#include "SmallGradeDisplay.h"
#include "GameConstantsAndTypes.h"
#include "MusicSortDisplay.h"
#include "WheelNotifyIcon.h"
#include "Screen.h"		// for ScreenMessage
#include "ScoreDisplayNormal.h"
#include "ScrollBar.h"
#include "Course.h"
#include "RageTimer.h"
#include "MusicWheelItem.h"


const int NUM_WHEEL_ITEMS_TO_DRAW	=	13;


const ScreenMessage	SM_SongChanged		=	ScreenMessage(SM_User+47);	// this should be unique!
const ScreenMessage SM_SortOrderChanged	=	ScreenMessage(SM_User+48);	




struct CompareSongPointerArrayBySectionName;

class MusicWheel : public ActorFrame
{
	friend struct CompareSongPointerArrayBySectionName;

public:
	MusicWheel();
	~MusicWheel();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	virtual void TweenOnScreen(bool changing_sort);
	virtual void TweenOffScreen(bool changing_sort);
	virtual void TweenOnScreen() { TweenOnScreen(false); }
	virtual void TweenOffScreen() { TweenOffScreen(false); }

	void Move(int n);
	bool PrevSort();
	bool NextSort();
	void StartRoulette();
	void StartRandom();
	bool IsRouletting() const;
	/* Return true if we're moving fast automatically. */
	int IsMoving() const;

	void NotesChanged( PlayerNumber pn );	// update grade graphics and top score

	float GetBannerX( float fPosOffsetsFromMiddle );
	float GetBannerY( float fPosOffsetsFromMiddle );

	bool Select();	// return true if the selected item is a music, otherwise false
	WheelItemType	GetSelectedType()	{ return m_CurWheelItemData[m_iSelection]->m_WheelItemType; };
	Song*			GetSelectedSong()	{ return m_CurWheelItemData[m_iSelection]->m_pSong; };
	Course*			GetSelectedCourse()	{ return m_CurWheelItemData[m_iSelection]->m_pCourse; };
	CString			GetSelectedSection(){ return m_CurWheelItemData[m_iSelection]->m_sSectionName; };

	bool WheelIsLocked() { return (m_WheelState == STATE_LOCKED ? true : false); }
	void RebuildWheelItemDisplays();

protected:
	void GetSongList(vector<Song*> &arraySongs, bool bRoulette );
	void BuildWheelItemDatas( vector<WheelItemData> &arrayWheelItems, SongSortOrder so );
	void SetOpenGroup(CString group, SongSortOrder so = NUM_SORT_ORDERS);
	bool SelectSong(const Song *p);
	bool SelectCourse(const Course *p);
	void ChangeMusic(int dist); /* +1 or -1 */

	ScrollBar			m_ScrollBar;
	Sprite				m_sprSelectionOverlay;

	vector<WheelItemData> m_WheelItemDatas[NUM_SORT_ORDERS];
	vector<WheelItemData *> m_CurWheelItemData;
	
	WheelItemDisplay	m_WheelItemDisplays[NUM_WHEEL_ITEMS_TO_DRAW];
	
	int					m_iSelection;		// index into m_CurWheelItemData
	CString				m_sExpandedSectionName;

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

//	bool				m_bUseRandomExtra;

	RageSound m_soundChangeMusic;
	RageSound m_soundChangeSort;
	RageSound m_soundExpand;
	RageSound m_soundStart;
	RageSound m_soundLocked;

	static CString GetSectionNameFromSongAndSort( const Song* pSong, SongSortOrder so );
	bool WheelItemIsVisible(int n);
	void UpdateScrollbar();
};

#endif
