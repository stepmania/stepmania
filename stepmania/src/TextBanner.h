/*
-----------------------------------------------------------------------------
 File: TextBanner.h

 Desc: The song's TextBanner displayed in SelectSong.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _TextBanner_H_
#define _TextBanner_H_


#include "ActorFrame.h"
#include "Song.h"
#include "BitmapText.h"


const float TEXT_BANNER_WIDTH	= 192;		// from the source art of DDR
const float TEXT_BANNER_HEIGHT	= 55;


class TextBanner : public ActorFrame
{
public:
	TextBanner();
	bool LoadFromSong( Song &song);

private:
	BitmapText	m_textTitle, m_textSubTitle, m_textArtist;


};




#endif