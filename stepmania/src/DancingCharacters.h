#ifndef DancingCharacters_H
#define DancingCharacters_H
/*
-----------------------------------------------------------------------------
 Class: DancingCharacters

 Desc: A graphic displayed in the DancingCharacters during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Model.h"
#include "ActorFrame.h"
#include "PlayerNumber.h"


class DancingCharacters : public ActorFrame
{
public:
	DancingCharacters();

	virtual void Update( float fDelta );
	virtual void DrawPrimitives();
	
protected:
	void StartCameraSweep();

	Model	m_Character[NUM_PLAYERS];

	RageVector3		m_CameraEyeStart;
	RageVector3		m_CameraEyeEnd;
	RageVector3		m_CameraEyeCurrent;
	RageVector3		m_CameraAt;
	float			m_fSweepSecs;
	float			m_fSecsIntoSweep;
};

#endif
