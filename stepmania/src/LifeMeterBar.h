#pragma once
/*
-----------------------------------------------------------------------------
 Class: LifeMeterBar

 Desc: A graphic displayed in the LifeMeterBar during Dancing.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "Sprite.h"
#include "ScoreDisplayRolling.h"


class LifeMeterBar : public ActorFrame
{
public:
	LifeMeterBar();
	void SetPlayerOptions(const PlayerOptions& po);
	
	virtual void Update( float fDeltaTime );
	virtual void RenderPrimitives();

	void SetBeat( float fSongBeat ) { m_fSongBeat = fSongBeat; };

	void ChangeLife( TapNoteScore score );
	float GetLifePercentage();

private:
	D3DXCOLOR GetColor( float fPercentIntoSection );

	float m_fSongBeat;

	float		m_fLifePercentage;
	float		m_fTrailingLifePercentage;	// this approaches m_fLifePercentage
	float		m_fLifeVelocity;	// how m_fTrailingLifePercentage approaches m_fLifePercentage

	PlayerOptions	m_po;
};
