#ifndef OptionIconRow_H
#define OptionIconRow_H
/*
-----------------------------------------------------------------------------
 Class: OptionIconRow

 Desc: Shows PlayerOptions and SongOptions in icon form.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ActorFrame.h"
#include "OptionIcon.h"
struct PlayerOptions;


const int NUM_OPTION_COLS = 8;


class OptionIconRow : public ActorFrame
{
public:
	OptionIconRow();

	void Refresh( PlayerNumber pn );
	virtual void DrawPrimitives();

protected:
	PlayerNumber	m_PlayerNumber;
	OptionIcon		m_OptionIcon[NUM_OPTION_COLS];
};

#endif