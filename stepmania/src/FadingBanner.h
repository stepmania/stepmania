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

class FadingBanner : public Banner
{
public:
	virtual ~FadingBanner() { }
	virtual bool Load( RageTextureID ID );
	void SetCroppedSize( float fWidth, float fHeight );

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

protected:
	void BeforeChange();

	Banner		m_FrontBanner;
};

#endif
