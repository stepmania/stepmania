#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: Course

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "CodeDetector.h"
#include "GameState.h"
#include "InputQueue.h"
#include "ThemeManager.h"
#include "RageLog.h"
#include "GameDef.h"
#include "StyleDef.h"

enum Code {
	CODE_EASIER1,
	CODE_EASIER2,
	CODE_HARDER1,
	CODE_HARDER2,
	CODE_NEXT_SORT1,
	CODE_NEXT_SORT2,
	CODE_MIRROR,
	CODE_LEFT,
	CODE_RIGHT,
	CODE_SHUFFLE,
	CODE_SUPER_SHUFFLE,
	CODE_LITTLE,
	CODE_NEXT_SCROLL_SPEED,
	CODE_PREVIOUS_SCROLL_SPEED,
	CODE_NEXT_ACCEL,
	CODE_NEXT_EFFECT,
	CODE_NEXT_APPEARANCE,
	CODE_NEXT_TURN,
	CODE_REVERSE,
	CODE_NEXT_COLOR,
	CODE_HOLDS,
	CODE_DARK,
	CODE_CANCEL_ALL,
	NUM_CODES	// leave this at the end
};

const CString g_sCodeNames[NUM_CODES] = {
	"Easier1",
	"Easier2",
	"Harder1",
	"Harder2",
	"NextSort1",
	"NextSort2",
	"Mirror",
	"Left",
	"Right",
	"Shuffle",
	"SuperShuffle",
	"Little",
	"NextScrollSpeed",
	"PreviousScrollSpeed",
	"NextAccel",
	"NextEffect",
	"NextAppearance",
	"NextTurn",
	"Reverse",
	"NextColor",
	"HoldNotes",
	"Dark",
	"CancelAll"
};

struct CodeCacheItem {
	int iNumButtons;
	GameButton buttons[10];
};	
CodeCacheItem g_CodeCacheItems[NUM_CODES];


bool MatchesCacheItem( GameController controller, Code code )
{
	if( g_CodeCacheItems[code].iNumButtons > 0 )
		if( INPUTQUEUE->MatchesPattern(controller, g_CodeCacheItems[code].buttons, g_CodeCacheItems[code].iNumButtons) )
			return true;

	return false;
}

void RefreshCacheItem( int iIndex )
{
	CodeCacheItem& item = g_CodeCacheItems[iIndex];
	const GameDef* pGameDef = GAMESTATE->GetCurrentGameDef();
	CString sCodeName = g_sCodeNames[iIndex];
	CString sButtonsNames = THEME->GetMetric("CodeDetector",sCodeName);
	CStringArray asButtonNames;
	split( sButtonsNames, ",", asButtonNames, false );

	if( asButtonNames.size() < 2 )
	{
		LOG->Trace( "The code '%s' is less than 2 buttons, so it will be ignored.", sCodeName.GetString() );
		item.iNumButtons = 0;
		return;
	}

	for( unsigned i=0; i<asButtonNames.size(); i++ )	// for each button in this code
	{
		CString sButtonName = asButtonNames[i];

		// Search for the corresponding GameButton
		GameButton& gb = item.buttons[i];
		gb = -1;
		for( int j=0; j<pGameDef->m_iButtonsPerController; j++ )
		{
			if( 0==stricmp(sButtonName,pGameDef->m_szButtonNames[j]) )
			{
				gb = j;
				item.iNumButtons = i+1;
				break;	// found it.  Don't keep searching
			}
		}
		if( gb == -1 )	// didn't find it
		{
			LOG->Trace( "The code '%s' contains an unrecognized button '%s'.", sCodeName.GetString(), sButtonName.GetString() );
			item.iNumButtons = 0;
			return;
		}
	}

	// if we make it here, we found all the buttons in the code
}

void CodeDetector::RefreshCacheItems()
{
	for( int i=0; i<NUM_CODES; i++ )
		RefreshCacheItem( i );
}

bool CodeDetector::EnteredEasierDifficulty( GameController controller )
{
	return MatchesCacheItem(controller,CODE_EASIER1) || MatchesCacheItem(controller,CODE_EASIER2);
}

bool CodeDetector::EnteredHarderDifficulty( GameController controller )
{
	return MatchesCacheItem(controller,CODE_HARDER1) || MatchesCacheItem(controller,CODE_HARDER2);
}

bool CodeDetector::EnteredNextSort( GameController controller )
{
	return MatchesCacheItem(controller,CODE_NEXT_SORT1) || MatchesCacheItem(controller,CODE_NEXT_SORT2);
}

#define  TOGGLE(v,a,b)	if(v!=a) v=a; else v=b;
#define  INCREMENT_SCROLL_SPEED(s)	(s==0.5f) ? s=0.75f : (s==0.75f) ? s=1.0f : (s==1.0f) ? s=1.5f : (s==1.5f) ? s=2.0f : (s==2.0f) ? s=3.0f : (s==3.0f) ? s=4.0f : (s==4.0f) ? s=5.0f : (s==5.0f) ? s=8.0f : s=0.5f;
#define  DECREMENT_SCROLL_SPEED(s)	(s==0.75f) ? s=0.5f : (s==1.0f) ? s=0.75f : (s==1.5f) ? s=1.0f : (s==2.0f) ? s=1.5f : (s==3.0f) ? s=2.0f : (s==4.0f) ? s=3.0f : (s==5.0f) ? s=4.0f : (s==8.0f) ? s=4.0f : s=8.0f;

bool CodeDetector::DetectAndAdjustOptions( GameController controller )
{
	const StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();
	PlayerNumber pn = pStyleDef->ControllerToPlayerNumber( controller );

	for( int c=CODE_MIRROR; c<NUM_CODES; c++ )
	{
		Code code = (Code)c;
		
		if( MatchesCacheItem(controller,code) )
		{
			switch( code )
			{
			case CODE_MIRROR:			TOGGLE( GAMESTATE->m_PlayerOptions[pn].m_TurnType, PlayerOptions::TURN_MIRROR,			PlayerOptions::TURN_NONE );	break;
			case CODE_LEFT:				TOGGLE( GAMESTATE->m_PlayerOptions[pn].m_TurnType, PlayerOptions::TURN_LEFT,			PlayerOptions::TURN_NONE );	break;
			case CODE_RIGHT:			TOGGLE( GAMESTATE->m_PlayerOptions[pn].m_TurnType, PlayerOptions::TURN_RIGHT,			PlayerOptions::TURN_NONE );	break;
			case CODE_SHUFFLE:			TOGGLE( GAMESTATE->m_PlayerOptions[pn].m_TurnType, PlayerOptions::TURN_SHUFFLE,			PlayerOptions::TURN_NONE );	break;
			case CODE_SUPER_SHUFFLE:	TOGGLE( GAMESTATE->m_PlayerOptions[pn].m_TurnType, PlayerOptions::TURN_SUPER_SHUFFLE,	PlayerOptions::TURN_NONE );	break;
			case CODE_LITTLE:			TOGGLE( GAMESTATE->m_PlayerOptions[pn].m_bLittle, true, false );				break;
			case CODE_NEXT_SCROLL_SPEED:INCREMENT_SCROLL_SPEED( GAMESTATE->m_PlayerOptions[pn].m_fScrollSpeed );	break;
			case CODE_PREVIOUS_SCROLL_SPEED:DECREMENT_SCROLL_SPEED( GAMESTATE->m_PlayerOptions[pn].m_fScrollSpeed );	break;
			case CODE_NEXT_ACCEL:		GAMESTATE->m_PlayerOptions[pn].NextAccel();									break;
			case CODE_NEXT_EFFECT:		GAMESTATE->m_PlayerOptions[pn].NextEffect();									break;
			case CODE_NEXT_APPEARANCE:	GAMESTATE->m_PlayerOptions[pn].NextAppearance();;				break;
			case CODE_NEXT_TURN:		GAMESTATE->m_PlayerOptions[pn].NextTurn();						break;
			case CODE_REVERSE:			TOGGLE( GAMESTATE->m_PlayerOptions[pn].m_bReverseScroll, true, false );			break;
			case CODE_NEXT_COLOR:		GAMESTATE->m_PlayerOptions[pn].NextColor();		break;
			case CODE_HOLDS:			TOGGLE( GAMESTATE->m_PlayerOptions[pn].m_bHoldNotes, true, false );				break;
			case CODE_DARK:				TOGGLE( GAMESTATE->m_PlayerOptions[pn].m_bDark, true, false );					break;
			case CODE_CANCEL_ALL:		GAMESTATE->m_PlayerOptions[pn] = PlayerOptions();								break;
			default:
				ASSERT(0);	// unhandled code
			}
			return true;	// don't check any more
		}
	}

	return false;
}
