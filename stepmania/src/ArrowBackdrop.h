#ifndef ARROW_BACKDROP_H
#define ARROW_BACKDROP_H

#include "BGAnimation.h"
#include "PlayerNumber.h"

class ArrowBackdrop: public BGAnimation
{
	PlayerNumber m_PlayerNumber;

public:
	virtual void BeginDraw();	// pushes transform onto world matrix stack
	virtual void EndDraw();		// pops transform from world matrix stack
	void SetPlayer( PlayerNumber pn ) { m_PlayerNumber = pn; }
};

#endif
