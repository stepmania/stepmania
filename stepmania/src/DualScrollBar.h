#ifndef DUAL_SCROLLBAR_H
#define DUAL_SCROLLBAR_H

#include "ActorFrame.h"
#include "Sprite.h"
#include "PlayerNumber.h"

class DualScrollBar: public ActorFrame
{
public:
	DualScrollBar();

	void Load();
	void SetBarHeight( float fHeight ) { m_fBarHeight = fHeight; }
	void SetBarTime( float fTime ) { m_fBarTime = fTime; }
	void SetPercentage( PlayerNumber pn, float fPercent );
	void EnablePlayer( PlayerNumber pn, bool on );

private:
	float	m_fBarHeight;
	float	m_fBarTime;

	Sprite	m_sprScrollThumbOverHalf[NUM_PLAYERS];
	Sprite	m_sprScrollThumbUnderHalf[NUM_PLAYERS];
};

#endif
