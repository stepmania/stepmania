#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: Course

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "CodeDetector.h"
#include "PlayerOptions.h"
#include "GameState.h"
#include "InputQueue.h"
#include "ThemeManager.h"
#include "RageLog.h"
#include "GameDef.h"
#include "StyleDef.h"
#include "RageUtil.h"

const CString g_sCodeNames[CodeDetector::NUM_CODES] = {
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
	"NextTransform",
	"NextScrollSpeed",
	"PreviousScrollSpeed",
	"NextAccel",
	"NextEffect",
	"NextAppearance",
	"NextTurn",
	"Reverse",
	"HoldNotes",
	"Dark",
	"Hidden",
	"RandomVanish",
	"CancelAll",
	"NextTheme",
	"NextAnnouncer"
};

const int MAX_CODE_LENGTH = 10;

struct CodeCacheItem {
	int iNumButtons;
	GameButton buttons[MAX_CODE_LENGTH];
	float fMaxSecondsBack;
};	
CodeCacheItem g_CodeCacheItems[CodeDetector::NUM_CODES];


bool CodeDetector::EnteredCode( GameController controller, Code code )
{
	if( g_CodeCacheItems[code].iNumButtons == 0 )
		return false;

	const CodeCacheItem& item = g_CodeCacheItems[code];
	return INPUTQUEUE->MatchesPattern(controller, item.buttons, item.iNumButtons, item.fMaxSecondsBack);
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
		LOG->Trace( "The code '%s' is less than 2 buttons, so it will be ignored.", sCodeName.c_str() );
		item.iNumButtons = 0;
		return;
	}

	for( unsigned i=0; i<asButtonNames.size() && i<MAX_CODE_LENGTH; i++ )	// for each button in this code
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
			LOG->Trace( "The code '%s' contains an unrecognized button '%s'.", sCodeName.c_str(), sButtonName.c_str() );
			item.iNumButtons = 0;
			return;
		}
	}

	item.fMaxSecondsBack = item.iNumButtons*0.4f;

	// if we make it here, we found all the buttons in the code
}

void CodeDetector::RefreshCacheItems()
{
	for( int i=0; i<NUM_CODES; i++ )
		RefreshCacheItem( i );
}

bool CodeDetector::EnteredEasierDifficulty( GameController controller )
{
	return EnteredCode(controller,CODE_EASIER1) || EnteredCode(controller,CODE_EASIER2);
}

bool CodeDetector::EnteredHarderDifficulty( GameController controller )
{
	return EnteredCode(controller,CODE_HARDER1) || EnteredCode(controller,CODE_HARDER2);
}

bool CodeDetector::EnteredNextSort( GameController controller )
{
	return EnteredCode(controller,CODE_NEXT_SORT1) || EnteredCode(controller,CODE_NEXT_SORT2);
}

#define  TOGGLE(v,a,b)	if(v!=a) v=a; else v=b;
#define  FLOAT_TOGGLE(v)	if(v!=1.f) v=1.f; else v=0.f;
#define  INCREMENT_SCROLL_SPEED(s)	(s==0.5f) ? s=0.75f : (s==0.75f) ? s=1.0f : (s==1.0f) ? s=1.5f : (s==1.5f) ? s=2.0f : (s==2.0f) ? s=3.0f : (s==3.0f) ? s=4.0f : (s==4.0f) ? s=5.0f : (s==5.0f) ? s=8.0f : s=0.5f;
#define  DECREMENT_SCROLL_SPEED(s)	(s==0.75f) ? s=0.5f : (s==1.0f) ? s=0.75f : (s==1.5f) ? s=1.0f : (s==2.0f) ? s=1.5f : (s==3.0f) ? s=2.0f : (s==4.0f) ? s=3.0f : (s==5.0f) ? s=4.0f : (s==8.0f) ? s=4.0f : s=8.0f;

#define  TOGGLE_HIDDEN ZERO(GAMESTATE->m_PlayerOptions[pn].m_fAppearances); GAMESTATE->m_PlayerOptions[pn].m_fAppearances[PlayerOptions::APPEARANCE_HIDDEN] = 1; 
#define  TOGGLE_RANDOMVANISH ZERO(GAMESTATE->m_PlayerOptions[pn].m_fAppearances); GAMESTATE->m_PlayerOptions[pn].m_fAppearances[PlayerOptions::APPEARANCE_RANDOMVANISH] = 1;

bool CodeDetector::DetectAndAdjustMusicOptions( GameController controller )
{
	const StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();
	PlayerNumber pn = pStyleDef->ControllerToPlayerNumber( controller );

	for( int c=CODE_MIRROR; c<=CODE_CANCEL_ALL; c++ )
	{
		Code code = (Code)c;
		
		if( EnteredCode(controller,code) )
		{
			switch( code )
			{
			case CODE_MIRROR:			TOGGLE( GAMESTATE->m_PlayerOptions[pn].m_Turn, PlayerOptions::TURN_MIRROR,			PlayerOptions::TURN_NONE );	break;
			case CODE_LEFT:				TOGGLE( GAMESTATE->m_PlayerOptions[pn].m_Turn, PlayerOptions::TURN_LEFT,			PlayerOptions::TURN_NONE );	break;
			case CODE_RIGHT:			TOGGLE( GAMESTATE->m_PlayerOptions[pn].m_Turn, PlayerOptions::TURN_RIGHT,			PlayerOptions::TURN_NONE );	break;
			case CODE_SHUFFLE:			TOGGLE( GAMESTATE->m_PlayerOptions[pn].m_Turn, PlayerOptions::TURN_SHUFFLE,			PlayerOptions::TURN_NONE );	break;
			case CODE_SUPER_SHUFFLE:	TOGGLE( GAMESTATE->m_PlayerOptions[pn].m_Turn, PlayerOptions::TURN_SUPER_SHUFFLE,	PlayerOptions::TURN_NONE );	break;
			case CODE_NEXT_TRANSFORM:	GAMESTATE->m_PlayerOptions[pn].NextTransform();									break;
			case CODE_NEXT_SCROLL_SPEED:INCREMENT_SCROLL_SPEED( GAMESTATE->m_PlayerOptions[pn].m_fScrollSpeed );		break;
			case CODE_PREVIOUS_SCROLL_SPEED:DECREMENT_SCROLL_SPEED( GAMESTATE->m_PlayerOptions[pn].m_fScrollSpeed );	break;
			case CODE_NEXT_ACCEL:		GAMESTATE->m_PlayerOptions[pn].NextAccel();										break;
			case CODE_NEXT_EFFECT:		GAMESTATE->m_PlayerOptions[pn].NextEffect();									break;
			case CODE_NEXT_APPEARANCE:	GAMESTATE->m_PlayerOptions[pn].NextAppearance();								break;
			case CODE_NEXT_TURN:		GAMESTATE->m_PlayerOptions[pn].NextTurn();										break;
			case CODE_REVERSE:			FLOAT_TOGGLE( GAMESTATE->m_PlayerOptions[pn].m_fReverseScroll );				break;
			case CODE_HOLDS:			TOGGLE( GAMESTATE->m_PlayerOptions[pn].m_bHoldNotes, true, false );				break;
			case CODE_DARK:				FLOAT_TOGGLE( GAMESTATE->m_PlayerOptions[pn].m_fDark );							break;
			case CODE_CANCEL_ALL:		GAMESTATE->m_PlayerOptions[pn].Init();								break;
			case CODE_HIDDEN:			TOGGLE_HIDDEN; break;
			case CODE_RANDOMVANISH:		TOGGLE_RANDOMVANISH; break;
				
				// GAMESTATE->m_PlayerOptions[pn].SetOneAppearance(GAMESTATE->m_PlayerOptions[pn].GetFirstAppearance()); break;
			default: ;
			}
			return true;	// don't check any more
		}
	}

	return false;
}
