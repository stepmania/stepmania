#ifndef _ActiveAttackList_H_
#define _ActiveAttackList_H_
/*
-----------------------------------------------------------------------------
 File: ActiveAttackList.h

 Desc: A graphic displayed in the ActiveAttackList during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "BitmapText.h"
#include "PlayerNumber.h"


class ActiveAttackList : public BitmapText
{
public:
	ActiveAttackList();

	void Init( PlayerNumber pn );

	virtual void Update( float fDelta );

protected:

	PlayerNumber m_PlayerNumber;
};

#endif
