#include "global.h"
/*
-----------------------------------------------------------------------------
 File: ActiveAttackList.h

 Desc: A graphic displayed in the ActiveAttackList during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

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
	BitmapText::Update( fDelta ); 

	// refresh text only once every 1/2 second
	float fNowSeconds = RageTimer::GetTimeSinceStart()*2;
	float fLastSeconds = RageTimer::GetTimeSinceStart()*2 - fDelta;

	if( (int)fNowSeconds != (int)fLastSeconds )
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
			split( sMods, ", ", asMods );
			for( unsigned j=0; j<asMods.size(); j++ )
			{
				CString& sMod = asMods[j];

				// Strip out the approach speed token
				if( !sMod.empty() && sMod[0]=='*' )
				{
					int iPos = sMod.Find(' ');
					if( iPos != -1 )
						sMod.erase( sMod.begin(), sMod.begin()+iPos+1 );
				}

				// Strip out "100% "
#define PERCENT_100 "100% "
				if( !strncmp( sMod, PERCENT_100, sizeof(PERCENT_100)) )
				{
					sMod.erase( sMod.begin(), sMod.begin()+sizeof(PERCENT_100) );
				}

				// Capitalize all tokens
				CStringArray asTokens;
				split( sMod, " ", asTokens );
				sMod.erase( sMod.begin(), sMod.end() );
				for( unsigned i=0; i<asTokens.size(); i++ )
					sMod += Capitalize( asTokens[i] ) + " ";
				
				if( s.empty() )
					s = sMod;
				else
					s = sMod + "\n" + s;
			}
		}

		this->SetText( s );	// BitmapText will not rebuild vertices if these strings are the same.
	}
}
