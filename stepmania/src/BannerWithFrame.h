#pragma once
/*
-----------------------------------------------------------------------------
 Class: BannerWithFrame

 Desc: bonus meters and max combo number.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "Sprite.h"
#include "Banner.h"
#include "DifficultyIcon.h"
class Song;
class Course;

class BannerWithFrame : public ActorFrame
{
public:
	BannerWithFrame();

	void LoadFromSongAndNotes( Song* pSong, Notes* pNotes[NUM_PLAYERS] );
	void LoadFromSong( Song* pSong );
	void LoadFromGroup( CString sGroupName );
	void LoadFromCourse( Course* pCourse );

protected:

	Sprite		m_sprBannerFrame;
	Banner		m_Banner;
	DifficultyIcon	m_Icon[NUM_PLAYERS];
};
