/*
-----------------------------------------------------------------------------
 File: Banner.h

 Desc: The song's banner displayed in SelectSong.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _Banner_H_
#define _Banner_H_


#include "Sprite.h"
#include "Song.h"


const float BANNER_WIDTH	= 192;		// from the source art of DDR
const float BANNER_HEIGHT	= 55;


class Banner : public Sprite
{
public:

	bool LoadFromSong( Song &song);

};




#endif