#ifndef OptionIcon_H
#define OptionIcon_H
/*
-----------------------------------------------------------------------------
 Class: OptionIcon

 Desc: Shows PlayerOptions and SongOptions in icon form.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "GameConstantsAndTypes.h"


class OptionIcon : public ActorFrame
{
public:
	OptionIcon();

	void Load( PlayerNumber pn, CString sText, bool bHeader = false );

protected:
	BitmapText	m_text;
	Sprite		m_spr;
};

#endif