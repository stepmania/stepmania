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

	Model	m_Character[NUM_PLAYERS];

	float	m_CameraDistance;
	float	m_CameraPanYStart;
	float	m_CameraPanYEnd;
	float	m_fCameraHeightStart;
	float	m_fCameraHeightEnd;
	float	m_fThisCameraStartBeat;
	float	m_fThisCameraEndBeat;
};

#endif
