#include "global.h"
#include "ActiveAttackList.h"
#include "RageUtil.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "Inventory.h"
#include "RageTimer.h"


ActiveAttackList::ActiveAttackList()
{
}

void ActiveAttackList::Init( PlayerNumber pn )
{
	m_PlayerNumber = pn;
}

void ActiveAttackList::Update( float fDelta ) 
{
	bool bTimeToRefresh = 
		IsFirstUpdate() || // check this before running Actor::Update()
		GAMESTATE->m_bAttackBeganThisUpdate[m_PlayerNumber] ||
		GAMESTATE->m_bAttackEndedThisUpdate[m_PlayerNumber];

	BitmapText::Update( fDelta ); 

	if( bTimeToRefresh )
		Refresh();
}

void ActiveAttackList::Refresh()
{
	CString s;

	const AttackArray& attacks = GAMESTATE->m_ActiveAttacks[m_PlayerNumber];	// NUM_INVENTORY_SLOTS
	
	// clear all lines, then add all active attacks
	for( unsigned i=0; i<attacks.size(); i++ )
	{
		const Attack& attack = attacks[i];

		if( !attack.bOn )
			continue; /* hasn't started yet */

		CString sMods = attack.sModifier;
		CStringArray asMods;
		split( sMods, ",", asMods );
		for( unsigned j=0; j<asMods.size(); j++ )
		{
			CString& sMod = asMods[j];
			TrimLeft( sMod );
			TrimRight( sMod );

			sMod = PlayerOptions::ThemeMod( sMod );

			if( s.empty() )
				s = sMod;
			else
				s = sMod + "\n" + s;
		}
	}

	this->SetText( s );	// BitmapText will not rebuild vertices if these strings are the same.
}

/*
 * (c) 2004 Chris Danford
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
