#ifndef MemoryCardManager_H
#define MemoryCardManager_H
/*
-----------------------------------------------------------------------------
 Class: MemoryCardManager

 Desc: Holds user-chosen preferences that are saved between sessions.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
#include "PlayerNumber.h"

class MemoryCardManager
{
public:
	MemoryCardManager();
	~MemoryCardManager();

	void Update( float fDelta );
};

extern MemoryCardManager*	MEMCARDMAN;	// global and accessable from anywhere in our program

#endif
