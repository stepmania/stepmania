#ifndef MODESWITCHER_H
#define MODESWITCHER_H
/*
-----------------------------------------------------------------------------
 Class: ModeSwitcher

 Desc: A ModeSwitcher for ScreenSelectMode

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Andrew Livy
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "Banner.h"
#include "Sprite.h"
#include "ScrollingList.h"
#include "SongManager.h"
#include "BitmapText.h"
#include "SongManager.h"
#include "PlayerOptions.h"
#include "SongOptions.h"
#include "BitmapText.h"

class ModeSwitcher : public ActorFrame
{
public:
	ModeSwitcher();
	~ModeSwitcher();
	void NextMode(int pn);
	void PrevMode(int pn);

private:
	BitmapText	m_Stylename;
	BitmapText m_Nextmode;
	BitmapText m_Prevmode;
	Sprite			m_NextIcon;
	Sprite			m_PrevIcon;
	CString GetStyleName();
	CString GetNextStyleName();
	CString GetPrevStyleName();
};

#endif
