#ifndef PRO_TIMING_DISPLAY_H
#define PRO_TIMING_DISPLAY_H

#include "ActorFrame.h"
#include "BitmapText.h"
#include "GameConstantsAndTypes.h"

class ProTimingDisplay: public ActorFrame
{
	BitmapText m_Judgment;

public:
	ProTimingDisplay();
	void SetJudgment( int ms, TapNoteScore score );
	void Reset();
};

#endif

