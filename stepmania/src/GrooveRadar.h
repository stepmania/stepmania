/*
-----------------------------------------------------------------------------
 Class: GrooveRadar

 Desc: The song's GrooveRadar displayed in SelectSong.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _GrooveRadar_H_
#define _GrooveRadar_H_


#include "ActorFrame.h"
#include "Sprite.h"
#include "Song.h"


class GrooveRadar : public ActorFrame
{
public:
	GrooveRadar::GrooveRadar();

	void LoadFromSong( Song* pSong );	// NULL means no Song

protected:
	Sprite m_sprRadar;
};




#endif