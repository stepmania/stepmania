#ifndef BANNER_H
#define BANNER_H
/*
-----------------------------------------------------------------------------
 File: Banner.h

 Desc: The song's banner displayed in SelectSong.  Must call SetCroppedSize.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "CroppedSprite.h"
#include "Song.h"
class Course;


class Banner : public CroppedSprite
{
public:
	Banner();

	virtual bool Load( CString sFilePath, RageTexturePrefs prefs );
	virtual bool Load( CString sFilePath ) { return Load( sFilePath, RageTexturePrefs() ); }

	virtual void Update( float fDeltaTime );

	bool LoadFromSong( Song* pSong );		// NULL means no song
	bool LoadFromGroup( CString sGroupName );
	bool LoadFromCourse( Course* pCourse );
	bool LoadRoulette();

	void SetScrolling( bool bScroll, float Percent = 0);
	bool IsScrolling() { return m_bScrolling; }
	float ScrollingPercent() { return m_fPercentScrolling; }

protected:
	bool m_bScrolling;
	float m_fPercentScrolling;
};

#endif
