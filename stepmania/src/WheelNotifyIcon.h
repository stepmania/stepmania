/* WheelNotifyIcon - A little graphic to the left of the song's text banner in the MusicWheel. */

#ifndef WHEEL_NOTIFY_ICON_H
#define WHEEL_NOTIFY_ICON_H

#include "Sprite.h"

class WheelNotifyIcon : public Sprite
{
public:
	WheelNotifyIcon();

	struct Flags
	{
		Flags() { bHasBeginnerOr1Meter = bEdits = false; iPlayersBestNumber = iStagesForSong = 0; }
		bool bHasBeginnerOr1Meter;
		int iPlayersBestNumber;
		bool bEdits;
		int iStagesForSong;
	};

	void SetFlags( Flags flags );

	virtual void Update( float fDeltaTime );
	virtual bool EarlyAbortDraw();

protected:
	enum Icons { training=0, best1, best2, best3, edits, long_ver, marathon, empty };

	vector<Icons> m_vIconsToShow;
};

#endif

/*
 * (c) 2001-2004 Chris Danford
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
