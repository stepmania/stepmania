/*
-----------------------------------------------------------------------------
 File: TextBanner.h

 Desc: The song's TextBanner displayed in SelectSong.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _TextBanner_H_
#define _TextBanner_H_


#include "ActorFrame.h"
#include "Song.h"
#include "BitmapText.h"
#include "RectangleActor.h"


const float TEXT_BANNER_WIDTH	= 180;
const float TEXT_BANNER_HEIGHT	= 40;


class TextBanner : public ActorFrame
{
public:
	TextBanner();
	bool LoadFromSong( Song* pSong );

private:
	BitmapText	m_textTitle, m_textSubTitle, m_textArtist;
};




#endif