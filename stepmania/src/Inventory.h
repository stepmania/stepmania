/* Inventory - Inventory management for PLAY_MODE_BATTLE. */

#ifndef Inventory_H
#define Inventory_H

#include "Actor.h"
#include "PlayerNumber.h"
#include "RageSound.h"

class PlayerState;

class Inventory : public Actor
{
public:
	Inventory();
	~Inventory();
	void Load( PlayerState* pPlayerState );

	virtual void Update( float fDelta );
	virtual void DrawPrimitives() {};

	void UseItem( int iSlot );

protected:
	void AwardItem( int iItemIndex );

	PlayerState* m_pPlayerState;
	int m_iLastSeenCombo;

	RageSound m_soundAcquireItem;
	vector<RageSound*> m_vpSoundUseItem;
	RageSound m_soundItemEnding;
};

#endif

/*
 * (c) 2003 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
