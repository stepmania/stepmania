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
	virtual ~Banner() { }

	virtual bool Load( RageTextureID ID );

	virtual void Update( float fDeltaTime );

	void LoadFromSong( Song* pSong );		// NULL means no song
	void LoadFromGroup( CString sGroupName );
	void LoadFromCourse( Course* pCourse );
	void LoadRoulette();
	void LoadFallback();

	void SetScrolling( bool bScroll, float Percent = 0);
	bool IsScrolling() const { return m_bScrolling; }
	float ScrollingPercent() const { return m_fPercentScrolling; }

protected:
	bool m_bScrolling;
	float m_fPercentScrolling;
};

#endif
