#ifndef DIFFICULTY_DISPLAY_H
#define DIFFICULTY_DISPLAY_H

/*
-----------------------------------------------------------------------
File: DifficultyDisplay.h

Desc: A graphic displayed on the screen during dance

Copyright (c) 2003 by the person(s) listed below. All rights reserved.
  Steven Towle
-----------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"
#include "Sprite.h"
#include "ActorFrame.h"

class Song;

class DifficultyDisplay : public ActorFrame
{
public:
	DifficultyDisplay();
	void SetDifficulties( const Song* pSong, StepsType curType );
	void UnsetDifficulties();

protected:
	Sprite m_difficulty[NUM_DIFFICULTIES];
};

#endif
