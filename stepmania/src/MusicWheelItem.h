#ifndef MUSICWHEELITEM_H
#define MUSICWHEELITEM_H
/*
-----------------------------------------------------------------------------
 Class: MusicWheelItem

 Desc: An item on the music wheel

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GradeDisplay.h"


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
	WheelNotifyIcon::Flags  m_Flags;
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
	WheelNotifyIcon		m_WheelNotifyIcon;
	TextBanner			m_TextBanner;
	GradeDisplay		m_GradeDisplay[NUM_PLAYERS];

	// for TYPE_COURSE
	BitmapText			m_textCourse;
};

#endif
