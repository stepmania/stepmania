#ifndef FADINGBANNER_H
#define FADINGBANNER_H
/*
-----------------------------------------------------------------------------
 Class: FadingBanner

 Desc: Fades between two banners.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Banner.h"
#include "ActorFrame.h"
#include "RageTimer.h"

class FadingBanner : public ActorFrame
{
public:
	FadingBanner();

	virtual bool Load( RageTextureID ID );
	void SetCroppedSize( float fWidth, float fHeight );

	void LoadFromSong( Song* pSong );		// NULL means no song
	void LoadAllMusic();
	void LoadFromGroup( CString sGroupName );
	void LoadFromCourse( Course* pCourse );
	void LoadRoulette();
	void LoadRandom();
	void LoadFallback();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

protected:
	void BeforeChange();

	Banner		m_Banner[2];
	int			m_iIndexFront;
	int			GetBackIndex() { return m_iIndexFront==0 ? 1 : 0; }

	CString		m_sPendingBanner;
	RageTimer	m_PendingTimer;
};

#endif
