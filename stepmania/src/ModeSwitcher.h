/* ModeSwitcher - A ModeSwitcher for ScreenSelectMode. */

#ifndef MODESWITCHER_H
#define MODESWITCHER_H

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
	void ChangeMode(PlayerNumber pn, int dir);

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

/*
 * (c) 2003 "Frieza"
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
