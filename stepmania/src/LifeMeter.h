//-----------------------------------------------------------------------------
// File: LifeMeter.h
//
// Desc: LifeMeter display at the bottom of the screen while dancing.
//
// Copyright (c) 2001 Chris Danford.  All rights reserved.
//-----------------------------------------------------------------------------

#ifndef _LIFEMETER_H_
#define _LIFEMETER_H_


#include "Sprite.h"

class LifeMeter
{
public:
	LifeMeter();
	~LifeMeter();

	void SetX( FLOAT iNewX );

	void Update( const FLOAT &fDeltaTime );
	void Draw( FLOAT fSongBeat );

	void SetLife( FLOAT fNewLife );

private:

	Sprite m_sprLifeMeterFrame;
	Sprite m_sprLifeMeterPills;

	FLOAT m_fLifePercentage;
};




#endif