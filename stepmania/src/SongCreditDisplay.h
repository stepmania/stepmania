#ifndef _SongCreditDisplay_H_
#define _SongCreditDisplay_H_
/*
-----------------------------------------------------------------------------
 File: SongCreditDisplay.h

 Desc: A graphic displayed in the SongCreditDisplay during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "BitmapText.h"

class SongCreditDisplay : public ActorFrame
{
public:
	SongCreditDisplay();

protected:
	BitmapText m_text;
};

#endif
