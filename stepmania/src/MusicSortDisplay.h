/*
-----------------------------------------------------------------------------
 File: MusicSortDisplay.h

 Desc: A graphic displayed in the MusicSortDisplay during Dancing.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

class MusicSortDisplay;


#ifndef _MusicSortDisplay_H_
#define _MusicSortDisplay_H_


#include "Sprite.h"


enum MusicSortOrder { SORT_GROUP, SORT_TITLE, SORT_BPM, SORT_ARTIST, SORT_MOST_PLAYED, NUM_SORT_ORDERS };


class MusicSortDisplay : public Sprite
{
public:
	MusicSortDisplay();
	void Set( MusicSortOrder so );

protected:

};


#endif