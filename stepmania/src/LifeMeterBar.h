#pragma once
/*
-----------------------------------------------------------------------------
 Class: LifeMeterBar

 Desc: A graphic displayed in the LifeMeterBar during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "LifeMeter.h"
#include "Sprite.h"
#include "Quad.h"


class LifeMeterBar : public LifeMeter
{
public:
	LifeMeterBar();
	
	virtual void Load( PlayerNumber pn );

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	virtual void ChangeLife( TapNoteScore score );
	virtual void OnDancePointsChange() {};	// this life meter doesn't care
	virtual bool IsInDanger();
	virtual bool IsHot();
	virtual bool IsFailing();
	virtual bool FailedEarlier();

private:
	void ResetBarVelocity();

	ActorFrame	m_frame;	// hold everything and mirror this for PLAYER_2 instead of mirroring all the individual Actors 

	Quad		m_quadBlackBackground;
	Sprite		m_sprStreamNormal;
	Sprite		m_sprStreamHot;
	Sprite		m_sprFrame;

	float		m_fLifePercentage;
	float		m_fTrailingLifePercentage;	// this approaches m_fLifePercentage
	float		m_fLifeVelocity;	// how m_fTrailingLifePercentage approaches m_fLifePercentage
	float		m_fHotAlpha;
	bool		m_bFailedEarlier;		// set this to true when life dips below 0
	int			m_iMeterWidth;
	int			m_iMeterHeight;
	float		m_fDangerThreshold;
};
