//-----------------------------------------------------------------------------
// File: Banner.h
//
// Desc: The song's banner displayed in Song Select.
//
// Copyright (c) 2001 Chris Danford.  All rights reserved.
//-----------------------------------------------------------------------------

#ifndef _Banner_H_
#define _Banner_H_


#include "Sprite.h"
#include "Song.h"


#define COMMON_BANNER_TEXTURE_WIDTH		384
#define COMMON_BANNER_TEXTURE_HEIGHT	110

#define BANNER_WIDTH	(COMMON_BANNER_TEXTURE_WIDTH/2)
#define BANNER_HEIGHT	(COMMON_BANNER_TEXTURE_HEIGHT / 2)


class Banner : public Sprite
{
public:

	BOOL LoadFromSong( Song &song);

};




#endif