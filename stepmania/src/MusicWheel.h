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
#include "Song.h"
#include "ActorFrame.h"
#include "BitmapText.h"
#include "Quad.h"
#include "TextBanner.h"
#include "RandomSample.h"
#include "SmallGradeDisplay.h"
#include "GameConstantsAndTypes.h"
#include "MusicSortDisplay.h"
#include "MusicStatusDisplay.h"
#include "Screen.h"		// for ScreenMessage
#include "ScoreDisplayNormal.h"
#include "ScrollBar.h"
#include "Course.h"
#include "RageTimer.h"


const int NUM_WHEEL_ITEMS_TO_DRAW	=	13;


const ScreenMessage	SM_SongChanged		=	ScreenMessage(SM_User+47);	// this should be unique!
const ScreenMessage SM_SortOrderChanged	=	ScreenMessage(SM_User+48);	


enum WheelItemType { TYPE_SECTION, TYPE_SONG, TYPE_ROULETTE, TYPE_RANDOM, TYPE_COURSE };


struct WheelItemData
{
public:
	WheelItemData() {}
	WheelItemData( WheelItemType wit, Song* pSong, const CString &sSectionName, Course* pCourse, const RageColor color );

	WheelItemType	m_WheelItemType;
	CString			m_sSectionName;
	Course*			m_pCourse;
	Song*			m_pSong;
	RageColor		m_color;	// either text color or section background color
	MusicStatusDisplay::IconType  m_IconType;
};


class WheelItemDisplay: public ActorFrame
{
public:
	WheelItemDisplay();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	
	void LoadFromWheelItemData( WheelItemData* pWID );
	void RefreshGrades();

	WheelItemData *data;
	float				m_fPercentGray;

	// for TYPE_SECTION and TYPE_ROULETTE
	Sprite				m_sprSectionBar;
	// for TYPE_SECTION
	BitmapText			m_textSectionName;
	// for TYPE_ROULETTE
	BitmapText			m_textRoulette;

	// for a TYPE_MUSIC
	Sprite				m_sprSongBar;
	MusicStatusDisplay	m_MusicStatusDisplay;
	TextBanner			m_TextBanner;
	SmallGradeDisplay	m_GradeDisplay[NUM_PLAYERS];

	// for TYPE_COURSE
	BitmapText			m_textCourse;
};


enum { SORT_ROULETTE = NUM_SORT_ORDERS+1 };

class MusicWheel : public ActorFrame
{
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

protected:
	void GetSongList(vector<Song*> &arraySongs, bool bRoulette );
	void BuildWheelItemDatas( vector<WheelItemData> &arrayWheelItems, SongSortOrder so );
	void RebuildWheelItemDisplays();
	void SetOpenGroup(CString group, SongSortOrder so = NUM_SORT_ORDERS);
	bool SelectSong(const Song *p);
	bool SelectCourse(const Course *p);
	void NextMusic();
	void PrevMusic();

	ScrollBar			m_ScrollBar;
	Sprite				m_sprSelectionOverlay;

	vector<WheelItemData> m_WheelItemDatas[NUM_SORT_ORDERS+2];
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

	CString GetSectionNameFromSongAndSort( Song* pSong, SongSortOrder so );
	bool WheelItemIsVisible(int n);
	void UpdateScrollbar();
};

#endif
