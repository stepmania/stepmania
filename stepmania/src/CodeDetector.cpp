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
#include "InputMapper.h"
#include "ThemeManager.h"
#include "RageLog.h"
#include "GameDef.h"
#include "StyleDef.h"
#include "RageUtil.h"
#include "PrefsManager.h"

const CString g_sCodeNames[CodeDetector::NUM_CODES] = {
	"Easier1",
	"Easier2",
	"Harder1",
	"Harder2",
	"NextSort1",
	"NextSort2",
	"NextSort3",
	"NextSort4",
	"SortMenu1",
	"SortMenu2",
	"ModeMenu1",
	"ModeMenu2",
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
	"Mines",
	"Dark",
	"Hidden",
	"RandomVanish",
	"CancelAll",
	"NextTheme",
	"NextTheme2",
	"NextAnnouncer",
	"NextAnnouncer2",
	"NextGame",
	"NextGame2",
	"NextBannerGroup",
	"NextBannerGroup2"
};

const unsigned MAX_CODE_LENGTH = 10;

struct CodeCacheItem {
	int iNumButtons;
	GameButton buttons[MAX_CODE_LENGTH];
	enum Type { 
		sequence,		// press the buttons in sequence
		hold_and_press,	// hold the first iNumButtons-1 buttons, then press the last
		tap				// press all buttons simultaneously
	};
	Type m_Type;
	float fMaxSecondsBack;
};	
CodeCacheItem g_CodeCacheItems[CodeDetector::NUM_CODES];


bool CodeDetector::EnteredCode( GameController controller, Code code )
{
	if( g_CodeCacheItems[code].iNumButtons == 0 )
		return false;

	const CodeCacheItem& item = g_CodeCacheItems[code];
	switch( item.m_Type )
	{
	case CodeCacheItem::sequence:
		return INPUTQUEUE->MatchesSequence(controller, item.buttons, item.iNumButtons, item.fMaxSecondsBack);
	case CodeCacheItem::hold_and_press:
		{
			// check that all but the last are being held
			for( int i=0; i<item.iNumButtons-1; i++ )
			{
				GameInput gi(controller, item.buttons[i]);
				if( !INPUTMAPPER->IsButtonDown(gi) )
					return false;
			}
			// just pressed the last button
			return INPUTQUEUE->MatchesSequence(controller, &item.buttons[item.iNumButtons-1], 1, 0.05f);
		}
		break;
	case CodeCacheItem::tap:
		return INPUTQUEUE->AllWerePressedRecently(controller, item.buttons, item.iNumButtons, item.fMaxSecondsBack);
	default:
		ASSERT(0);
		return false;
	}
}

void RefreshCacheItem( int iIndex )
{
	CodeCacheItem& item = g_CodeCacheItems[iIndex];
	const GameDef* pGameDef = GAMESTATE->GetCurrentGameDef();
	CString sCodeName = g_sCodeNames[iIndex];
	CString sButtonsNames = THEME->GetMetric("CodeDetector",sCodeName);
	CStringArray asButtonNames;

	bool bHasAPlus = sButtonsNames.Find( '+' ) != -1;
	bool bHasADash = sButtonsNames.Find( '-' ) != -1;

	if( bHasAPlus )
	{
		item.m_Type = CodeCacheItem::tap;
		split( sButtonsNames, "+", asButtonNames, false );
	}
	else if( bHasADash )
	{
		item.m_Type = CodeCacheItem::hold_and_press;
		split( sButtonsNames, "-", asButtonNames, false );
	}
	else
	{
		item.m_Type = CodeCacheItem::sequence;
		split( sButtonsNames, ",", asButtonNames, false );
	}

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

	switch( item.m_Type )
	{
	case CodeCacheItem::sequence:
		item.fMaxSecondsBack = item.iNumButtons*0.4f;
		break;
	case CodeCacheItem::hold_and_press:
		item.fMaxSecondsBack = -1.f;	// not applicable
		break;
	case CodeCacheItem::tap:
		item.fMaxSecondsBack = 0.05f;	// simultaneous
		break;
	default:
		ASSERT(0);
	}

	// if we make it here, we found all the buttons in the code
}

void CodeDetector::RefreshCacheItems()
{
	for( int i=0; i<NUM_CODES; i++ )
		RefreshCacheItem( i );
}

bool CodeDetector::EnteredNextBannerGroup( GameController controller )
{
	return EnteredCode(controller,CODE_BW_NEXT_GROUP) || EnteredCode(controller,CODE_BW_NEXT_GROUP2);
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
	return EnteredCode(controller,CODE_NEXT_SORT1) ||
		   EnteredCode(controller,CODE_NEXT_SORT2) ||
		   EnteredCode(controller,CODE_NEXT_SORT3) ||
		   EnteredCode(controller,CODE_NEXT_SORT4);
}

bool CodeDetector::EnteredSortMenu( GameController controller )
{
	return EnteredCode(controller,CODE_SORT_MENU1) || EnteredCode(controller,CODE_SORT_MENU2);
}

bool CodeDetector::EnteredModeMenu( GameController controller )
{
	return EnteredCode(controller,CODE_MODE_MENU1) || EnteredCode(controller,CODE_MODE_MENU2);
}

#define  TOGGLE(v,a,b)	if(v!=a) v=a; else v=b;
#define  FLOAT_TOGGLE(v)	if(v!=1.f) v=1.f; else v=0.f;
// XXX: Read the metrics file instead!
// Using this can give us unlisted scroll speeds on the Options screen.
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
			case CODE_REVERSE:			GAMESTATE->m_PlayerOptions[pn].NextScroll();									break;
			case CODE_HOLDS:			TOGGLE( GAMESTATE->m_PlayerOptions[pn].m_bTransforms[PlayerOptions::TRANSFORM_NOHOLDS], true, false );				break;
			case CODE_MINES:			TOGGLE( GAMESTATE->m_PlayerOptions[pn].m_bTransforms[PlayerOptions::TRANSFORM_NOMINES], true, false );					break;
			case CODE_DARK:				FLOAT_TOGGLE( GAMESTATE->m_PlayerOptions[pn].m_fDark );							break;
			case CODE_CANCEL_ALL:		GAMESTATE->m_PlayerOptions[pn].Init();
										GAMESTATE->m_PlayerOptions[pn].FromString( PREFSMAN->m_sDefaultModifiers );		break;
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
