/*
-----------------------------------------------------------------------------
 File: Background.h

 Desc: A graphic displayed in the background during Dancing.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _Background_H_
#define _Background_H_


#include "Sprite.h"
#include "Song.h"


class Background : public Sprite
{
public:
	void LoadFromSong( Song& song );

	void Update( const FLOAT& fDeltaTime);
	void Draw();

	Sprite m_sprVis;

};




#endif