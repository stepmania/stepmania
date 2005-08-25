/* Transition - Transition that draws a BGAnimation. */

#ifndef TRANSITION_H
#define TRANSITION_H

#include "ActorFrame.h"
#include "AutoActor.h"
#include "ScreenMessage.h"
#include "RandomSample.h"


class Transition : public ActorFrame
{
public:
	Transition();

	void Load( CString sBGAniDir );

	virtual void UpdateInternal( float fDeltaTime );

	virtual void StartTransitioning( ScreenMessage send_when_done = SM_None );
	virtual bool EarlyAbortDraw() const;
	virtual float GetTweenTimeLeft() const;
	void Reset(); // explicitly allow transitioning again

	bool IsTransitioning() const	{ return m_State == transitioning; };
	bool IsFinished() const	{ return m_State == finished; };
	float GetLengthSeconds() const;

protected:

	enum State { 
		waiting,	
		transitioning, 
		finished 
	} m_State;

	AutoActor m_sprTransition;
	float	m_fLengthSeconds;
	RandomSample	m_sound;

	ScreenMessage	m_MessageToSendWhenDone;
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
