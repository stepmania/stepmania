#pragma once
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

	inline void SetScrolling( bool bScroll ) { m_bScrolling = bScroll; };
	inline bool IsScrolling() { return m_bScrolling; };

protected:
	bool m_bScrolling;
	float m_fPercentScrolling;
};
