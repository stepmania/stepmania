#ifndef _BPMDisplay_H_
#define _BPMDisplay_H_
/*
-----------------------------------------------------------------------------
 File: BPMDisplay.h

 Desc: A graphic displayed in the BPMDisplay during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"
#include "song.h"
#include "ActorFrame.h"
#include "BitmapText.h"
#include "Quad.h"
class Song;
class Course;

class BPMDisplay : public ActorFrame
{
public:
	BPMDisplay();
	virtual void Update( float fDeltaTime ); 
	virtual void DrawPrimitives(); 
	void SetBPM( const Song* pSong );
	void SetBPM( const Course* pCourse );
	void CycleRandomly();
	void NoBPM();

protected:
	float GetActiveBPM() const;
	void SetBPMRange( const vector<float> &m_BPMS );

	BitmapText m_textBPM;
	Sprite m_sprLabel;

	float m_fBPMFrom, m_fBPMTo;
	int m_iCurrentBPM;
	vector<float> m_BPMS;
	float m_fPercentInState;
	float m_fCycleTime;
};

#endif
