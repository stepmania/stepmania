#ifndef BANNERWITHFRAME_H
#define BANNERWITHFRAME_H
/*
-----------------------------------------------------------------------------
 Class: BannerWithFrame

 Desc: show the banner inside a frame.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "Sprite.h"
#include "Banner.h"
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
};

#endif
