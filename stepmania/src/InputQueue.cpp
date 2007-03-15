#include "global.h"
#include "InputQueue.h"
#include "RageTimer.h"
#include "RageLog.h"
#include "InputEventPlus.h"
#include "Foreach.h"
#include "InputMapper.h"


InputQueue*	INPUTQUEUE = NULL;	// global and accessable from anywhere in our program


InputQueue::InputQueue()
{
	FOREACH_GameController ( gc )
		m_aQueue[gc].resize( MAX_INPUT_QUEUE_LENGTH );
}

void InputQueue::RememberInput( const InputEventPlus &iep )
{
	int c = iep.GameI.controller;
	if( m_aQueue[c].size() >= MAX_INPUT_QUEUE_LENGTH )	// full
		m_aQueue[c].erase( m_aQueue[c].begin(), m_aQueue[c].begin() + (m_aQueue[c].size()-MAX_INPUT_QUEUE_LENGTH+1) );
	m_aQueue[c].push_back( iep );
}

bool InputQueue::WasPressedRecently( GameController c, const GameButton button, const RageTimer &OldestTimeAllowed, InputEventPlus *pIEP )
{
	for( int queue_index=m_aQueue[c].size()-1; queue_index>=0; queue_index-- )	// iterate newest to oldest
	{
		const InputEventPlus &iep = m_aQueue[c][queue_index];
		if( iep.DeviceI.ts < OldestTimeAllowed )	// buttons are too old.  Stop searching because we're not going to find a match
			return false;

		if( iep.GameI.button != button )
			continue;

		if( pIEP != NULL )
			*pIEP = iep;

		return true;
	}

	return false;	// didn't find the button
}

void InputQueue::ClearQueue( GameController c )
{
	m_aQueue[c].clear();
}

static const float g_fSimultaneousThreshold = 0.05f;

bool InputQueueCode::EnteredCode( GameController controller ) const
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

bool InputQueueCode::Load( RString sButtonsNames )
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

/*
 * (c) 2001-2007 Chris Danford, Glenn Maynard
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
