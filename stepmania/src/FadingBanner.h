#pragma once
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


class FadingBanner : public ActorFrame
{
public:
	FadingBanner();

	void SetCroppedSize( float fWidth, float fHeight );

	void Update( float fDeltaTime );

	void SetFromSong( Song* pSong );
	void SetFromGroup( const CString &sGroupName );
	void SetRoulette();
	//void SetFromTexture( const CString &sTexturePath );

protected:
	void BeforeChange();

	Banner		m_Banner[2];	// front and back
};
