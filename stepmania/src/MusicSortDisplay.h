#pragma once
/*
-----------------------------------------------------------------------------
 Class: MusicSortDisplay

 Desc: A graphic displayed in the MusicSortDisplay during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

class MusicSortDisplay;


#include "Sprite.h"
#include "GameConstantsAndTypes.h"



class MusicSortDisplay : public Sprite
{
public:
	MusicSortDisplay();
	void Set( SongSortOrder so );

protected:

};
