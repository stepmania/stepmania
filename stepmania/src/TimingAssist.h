#ifndef TIMING_ASSIST_H
#define TIMING_ASSIST_H

#include "Actor.h"
#include "RandomSample.h"
#include "GameConstantsAndTypes.h"
#include "PlayerNumber.h"
#include "NoteDataWithScoring.h"

class TimingAssist: public Actor
{
public:
	TimingAssist();
	void Load(PlayerNumber pn, NoteDataWithScoring *nd);
	void DrawPrimitives() { }
	void Update(float fDeltaTime);
	void Reset();

private:
	void DoTimingAssist(PlayerNumber pn);
	void Announce(TapNoteScore tns, bool early);

	RandomSample Timing[NUM_TAP_NOTE_SCORES][2];
	RandomSample Miss, Exact;

	NoteDataWithScoring *data[NUM_PLAYERS];
	int LastRow;
};

#endif
/*
-----------------------------------------------------------------------------
 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
-----------------------------------------------------------------------------
*/
