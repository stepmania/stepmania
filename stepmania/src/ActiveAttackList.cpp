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

			CString sDisplayText = attack.sModifier;
			
			// Strip out the approach speed token
			if( !sDisplayText.empty() && sDisplayText[0]=='*' )
			{
				int iPos = sDisplayText.Find(' ');
				if( iPos != -1 )
					sDisplayText.erase( sDisplayText.begin(), sDisplayText.begin()+iPos+1 );
			}

			// Capitalize all tokens
			CStringArray asTokens;
			split( sDisplayText, " ", asTokens );
			sDisplayText.erase( sDisplayText.begin(), sDisplayText.end() );
			for( int i=0; i<asTokens.size(); i++ )
				sDisplayText += Capitalize( asTokens[i] );
			
			if( s.empty() )
				s = sDisplayText;
			else
				s = sDisplayText + "\n" + s;
		}

		this->SetText( s );	// BitmapText will not rebuild vertices if these strings are the same.
	}
}
