#ifndef LIFEMETERBAR_H
#define LIFEMETERBAR_H
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
class LifeMeterStream;


class LifeMeterBar : public LifeMeter
{
public:
	LifeMeterBar();
	~LifeMeterBar();
	
	virtual void Load( PlayerNumber pn );

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	virtual void ChangeLife( TapNoteScore score );
	virtual void ChangeLife( HoldNoteScore score, TapNoteScore tscore  );
	virtual void AfterLifeChanged();
	virtual void OnDancePointsChange() {};	// this life meter doesn't care
	virtual bool IsInDanger();
	virtual bool IsHot();
	virtual bool IsFailing();
	virtual bool FailedEarlier();

private:
	void ResetBarVelocity();

	Quad		m_quadBlackBackground;
	LifeMeterStream*	m_pStream;

	float		m_fLifePercentage;
	float		m_fTrailingLifePercentage;	// this approaches m_fLifePercentage
	float		m_fLifeVelocity;

	float		m_fHotAlpha;
	bool		m_bFailedEarlier;		// set this to true when life dips below 0
};

#endif
