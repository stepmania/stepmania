#include "global.h"
#include "CodeDetector.h"
#include "PlayerOptions.h"
#include "GameState.h"
#include "InputQueue.h"
#include "InputMapper.h"
#include "ThemeManager.h"
#include "RageLog.h"
#include "Game.h"
#include "RageUtil.h"
#include "PlayerState.h"
#include "InputEventPlus.h"
#include "Foreach.h"

const char *CodeNames[] = {
	"Easier1",
	"Easier2",
	"Harder1",
	"Harder2",
	"NextSort1",
	"NextSort2",
	"NextSort3",
	"NextSort4",
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
	"NextBannerGroup",
	"NextBannerGroup2",
	"SaveScreenshot1",
	"SaveScreenshot2",
	"CancelAllPlayerOptions",
	"BackInEventMode",
};
XToString( Code );

static CodeItem g_CodeItems[NUM_Code];

static const float g_fSimultaneousThreshold = 0.05f;

bool CodeItem::EnteredCode( GameController controller ) const
{
	if( controller == GameController_Invalid )
		return false;
	if( m_aPresses.size() == 0 )
		return false;

	RageTimer OldestTimeAllowed;
	OldestTimeAllowed += -fMaxSecondsBack;

	// iterate newest to oldest
	int iSequenceIndex = m_aPresses.size()-1;	// count down
	const vector<InputEventPlus> &aQueue = INPUTQUEUE->GetQueue( controller );
	int iQueueIndex = aQueue.size()-1;
	while( iQueueIndex >= 0 )
	{
		{
			const InputEventPlus &iep = aQueue[iQueueIndex];
			if( iep.DeviceI.ts < OldestTimeAllowed )
				return false;
		}

		/* Search backwards for all of Press.m_aButtonsToPress pressed within g_fTapThreshold seconds
		 * with m_aButtonsToHold pressed. */
		const ButtonPress &Press = m_aPresses[iSequenceIndex];
		bool bMatched = false;
		int iMinSearchIndexUsed = iQueueIndex;
		for( int b=0; b<(int) Press.m_aButtonsToPress.size(); ++b )
		{
			/* Search backwards for the buttons in this tap, within the tap threshold since iQueueIndex. */
			RageTimer OldestTimeAllowedForTap( aQueue[iQueueIndex].DeviceI.ts );
			OldestTimeAllowedForTap += -g_fSimultaneousThreshold;

			const InputEventPlus *pIEP = NULL;
			int iQueueSearchIndex = iQueueIndex;
			for( ; iQueueSearchIndex>=0; --iQueueSearchIndex )	// iterate newest to oldest
			{
				const InputEventPlus &iep = aQueue[iQueueSearchIndex];
				if( iep.DeviceI.ts < OldestTimeAllowedForTap )	// buttons are too old.  Stop searching because we're not going to find a match
					break;

				if( iep.GameI.button == Press.m_aButtonsToPress[b] )
				{
					pIEP = &iep;
					break;
				}
			}
			if( pIEP == NULL )
				break;	// didn't find the button

			// Check that m_aButtonsToHold were being held when the buttons were pressed.
			bool bAllButtonsPressed = true;
			for( unsigned i=0; i<Press.m_aButtonsToHold.size(); i++ )
			{
				GameInput gi( controller, Press.m_aButtonsToHold[i] );
				if( !INPUTMAPPER->IsBeingPressed(gi, MultiPlayer_Invalid, &pIEP->InputList) )
					bAllButtonsPressed = false;
			}
			if( !bAllButtonsPressed )
				continue;
			if( b == (int) Press.m_aButtonsToPress.size()-1 )
			{
				bMatched = true;
				iMinSearchIndexUsed = min( iMinSearchIndexUsed, iQueueSearchIndex );
			}
		}

		if( !bMatched )
		{
			/* The press wasn't matched.  If m_bAllowIntermediatePresses is true,
			 * skip the last press, and look again. */
			if( !Press.m_bAllowIntermediatePresses )
				return false;
			--iQueueIndex;
			continue;
		}

		if( iSequenceIndex == 0 )
		{
			// we matched the whole pattern.  Empty the queue so we don't match on it again.
			INPUTQUEUE->ClearQueue( controller );
			return true;
		}

		/* The press was matched. */
		iQueueIndex = iMinSearchIndexUsed - 1;
		--iSequenceIndex;
	}

	return false;
}

bool CodeItem::Load( RString sButtonsNames )
{
	m_aPresses.clear();

	vector<RString> asButtonNames;

	bool bHasAPlus = sButtonsNames.find( '+' ) != string::npos;
	bool bHasADash = sButtonsNames.find( '-' ) != string::npos;

	if( bHasAPlus )
	{
		// press all buttons simultaneously
		split( sButtonsNames, "+", asButtonNames, false );
	}
	else if( bHasADash )
	{
		// hold the first iNumButtons-1 buttons, then press the last
		split( sButtonsNames, "-", asButtonNames, false );
	}
	else
	{
		// press the buttons in sequence
		split( sButtonsNames, ",", asButtonNames, false );
	}

	if( asButtonNames.size() < 1 )
	{
		if( sButtonsNames != "" )
			LOG->Trace( "Ignoring empty code \"%s\".", sButtonsNames.c_str() );
		return false;
	}

	vector<GameButton> buttons;
	for( unsigned i=0; i<asButtonNames.size(); i++ )	// for each button in this code
	{
		const RString sButtonName = asButtonNames[i];

		// Search for the corresponding GameButton
		const GameButton gb = INPUTMAPPER->GetInputScheme()->ButtonNameToIndex( sButtonName );
		if( gb == GameButton_Invalid )
		{
			LOG->Trace( "The code \"%s\" contains an unrecognized button \"%s\".", sButtonsNames.c_str(), sButtonName.c_str() );
			m_aPresses.clear();
			return false;
		}

		buttons.push_back( gb );
	}

	if( bHasAPlus )
	{
		m_aPresses.push_back( ButtonPress() );
		FOREACH( GameButton, buttons, gb )
		{
			m_aPresses.back().m_aButtonsToPress.push_back( *gb );
		}
	}
	else if( bHasADash )
	{
		m_aPresses.push_back( ButtonPress() );
		m_aPresses.back().m_aButtonsToHold.insert( m_aPresses.back().m_aButtonsToHold.begin(), buttons.begin(), buttons.end()-1 );
		m_aPresses.back().m_aButtonsToPress.push_back( buttons.back() );
	}
	else
	{
		FOREACH( GameButton, buttons, gb )
		{
			m_aPresses.push_back( ButtonPress() );
			m_aPresses.back().m_aButtonsToPress.push_back( *gb );
			m_aPresses.back().m_bAllowIntermediatePresses = false;
		}
	}

	if( m_aPresses.size() == 1 )
		fMaxSecondsBack = 0.55f;
	else
		fMaxSecondsBack = (m_aPresses.size()-1)*0.6f;

	// if we make it here, we found all the buttons in the code
	return true;
}

bool CodeDetector::EnteredCode( GameController controller, Code code )
{
	return g_CodeItems[code].EnteredCode( controller );
}


void CodeDetector::RefreshCacheItems( RString sClass )
{
	if( sClass == "" )
		sClass = "CodeDetector";
	FOREACH_Code( c )
	{
		CodeItem& item = g_CodeItems[c];
		const RString sCodeName = CodeToString(c);
		const RString sButtonsNames = THEME->GetMetric(sClass,sCodeName);

		item.Load( sButtonsNames );
	}
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

bool CodeDetector::DetectAndAdjustMusicOptions( GameController controller )
{
	PlayerNumber pn = INPUTMAPPER->ControllerToPlayerNumber( controller );

	for( int c=CODE_MIRROR; c<=CODE_CANCEL_ALL; c++ )
	{
		Code code = (Code)c;
		
		PlayerOptions po = GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.GetPreferred();

		if( EnteredCode(controller,code) )
		{
			switch( code )
			{
			case CODE_MIRROR:			po.ToggleOneTurn( PlayerOptions::TURN_MIRROR );		break;
			case CODE_LEFT:				po.ToggleOneTurn( PlayerOptions::TURN_LEFT );		break;
			case CODE_RIGHT:			po.ToggleOneTurn( PlayerOptions::TURN_RIGHT );		break;
			case CODE_SHUFFLE:			po.ToggleOneTurn( PlayerOptions::TURN_SHUFFLE );	break;
			case CODE_SUPER_SHUFFLE:		po.ToggleOneTurn( PlayerOptions::TURN_SUPER_SHUFFLE );	break;
			case CODE_NEXT_TRANSFORM:		po.NextTransform();					break;
			case CODE_NEXT_SCROLL_SPEED:		INCREMENT_SCROLL_SPEED( po.m_fScrollSpeed );		break;
			case CODE_PREVIOUS_SCROLL_SPEED:	DECREMENT_SCROLL_SPEED( po.m_fScrollSpeed );		break;
			case CODE_NEXT_ACCEL:			po.NextAccel();						break;
			case CODE_NEXT_EFFECT:			po.NextEffect();					break;
			case CODE_NEXT_APPEARANCE:		po.NextAppearance();					break;
			case CODE_NEXT_TURN:			po.NextTurn();						break;
			case CODE_REVERSE:			po.NextScroll();					break;
			case CODE_HOLDS:			TOGGLE( po.m_bTransforms[PlayerOptions::TRANSFORM_NOHOLDS], true, false );	break;
			case CODE_MINES:			TOGGLE( po.m_bTransforms[PlayerOptions::TRANSFORM_NOMINES], true, false );	break;
			case CODE_DARK:				FLOAT_TOGGLE( po.m_fDark );				break;
			case CODE_CANCEL_ALL:			GAMESTATE->GetDefaultPlayerOptions( po );		break;
			case CODE_HIDDEN:
				ZERO(po.m_fAppearances);
				po.m_fAppearances[PlayerOptions::APPEARANCE_HIDDEN] = 1;
				break;
			case CODE_RANDOMVANISH:
				ZERO(po.m_fAppearances);
				po.m_fAppearances[PlayerOptions::APPEARANCE_RANDOMVANISH] = 1;
				break;
			default:	break;
			}

			GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.Assign( ModsLevel_Preferred, po );


			return true;	// don't check any more
		}
	}

	return false;
}

/*
 * (c) 2001-2004 Chris Danford
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
