#ifndef MUSICSTATUSDISPLAY_H
#define MUSICSTATUSDISPLAY_H
/*
-----------------------------------------------------------------------------
 Class: MusicStatusDisplay

 Desc: A little graphic to the left of the song's text banner in the MusicWheel.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Sprite.h"



class MusicStatusDisplay : public Sprite
{
public:
	MusicStatusDisplay();

	struct Flags
	{
		Flags() { bHasBeginnerOr1Meter = bEdits = false; iPlayersBestNumber = iStagesForSong = 0; }
		bool bHasBeginnerOr1Meter;
		int iPlayersBestNumber;
		bool bEdits;
		int iStagesForSong;
	};

	void SetFlags( Flags flags );

	virtual void DrawPrimitives();
	
protected:
	enum Icons { training=0, best1, best2, best3, edits, long_ver, marathon, empty };

	vector<Icons> m_vIconsToShow;
};

#endif
