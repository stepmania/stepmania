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
	
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	virtual void ChangeLife( TapNoteScore score );
	virtual bool IsDoingGreat();
	virtual bool IsAboutToFail();
	virtual bool HasFailed();

private:
	D3DXCOLOR GetColor( float fPercentIntoSection );
	void ResetBarVelocity();

	float		m_fLifePercentage;
	float		m_fTrailingLifePercentage;	// this approaches m_fLifePercentage
	float		m_fLifeVelocity;	// how m_fTrailingLifePercentage approaches m_fLifePercentage
	bool		m_bHasFailed;		// set this to true when life dips below 0
};
