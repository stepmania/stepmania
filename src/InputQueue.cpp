#include "global.h"
#include "InputQueue.h"
#include "RageThreads.h"
#include "RageTimer.h"
#include "RageLog.h"
#include "InputEventPlus.h"
#include "InputMapper.h"

using std::vector;

InputQueue*	INPUTQUEUE = nullptr;	// global and accessible from anywhere in our program

const unsigned MAX_INPUT_QUEUE_LENGTH = 32;

std::map<std::string, int> g_PPS;
RageMutex g_ppsMut("PPSCounter");

// Used by the InputHandlers.
void InputQueue::SetPPS(std::string driver, int pps)
{
	LockMut(g_ppsMut);
	g_PPS[driver] = pps;
}

std::map<std::string, int> InputQueue::GetPPS() const
{
	return g_PPS;
}

InputQueue::InputQueue()
{
	FOREACH_ENUM( GameController, gc )
	{
		m_aQueue[gc].resize( MAX_INPUT_QUEUE_LENGTH );
	}
}

void InputQueue::RememberInput( const InputEventPlus &iep )
{
	if( !iep.GameI.IsValid() )
		return;

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

		if( pIEP != nullptr )
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
	using std::min;
	if( controller == GameController_Invalid )
		return false;
	if( m_aPresses.size() == 0 )
		return false;

	RageTimer OldestTimeAllowed;
	if( m_fMaxSecondsBack == -1 )
		OldestTimeAllowed.SetZero();
	else
		OldestTimeAllowed += -m_fMaxSecondsBack;

	// iterate newest to oldest
	int iSequenceIndex = m_aPresses.size()-1;	// count down
	const vector<InputEventPlus> &aQueue = INPUTQUEUE->GetQueue( controller );
	int iQueueIndex = aQueue.size()-1;
	while( iQueueIndex >= 0 )
	{
		/* If the buttons are too old, stop searching because we're not going to find a match. */
		if( !OldestTimeAllowed.IsZero() && aQueue[iQueueIndex].DeviceI.ts < OldestTimeAllowed )
			return false;

		/* If the last press is an input type we're not interested in, skip it
		 * and look again. */
		const ButtonPress &Press = m_aPresses[iSequenceIndex];
		if( !Press.m_InputTypes[aQueue[iQueueIndex].type] )
		{
			--iQueueIndex;
			continue;
		}

		/* Search backwards for all of Press.m_aButtonsToPress pressed within g_fTapThreshold seconds
		 * with m_aButtonsToHold pressed.  Start looking at iQueueIndex. */
		RageTimer OldestTimeAllowedForTap( aQueue[iQueueIndex].DeviceI.ts );
		OldestTimeAllowedForTap += -g_fSimultaneousThreshold;

		bool bMatched = false;
		int iMinSearchIndexUsed = iQueueIndex;
		for( int b=0; b<(int) Press.m_aButtonsToPress.size(); ++b )
		{
			const InputEventPlus *pIEP = nullptr;
			int iQueueSearchIndex = iQueueIndex;
			for( ; iQueueSearchIndex>=0; --iQueueSearchIndex )	// iterate newest to oldest
			{
				const InputEventPlus &iep = aQueue[iQueueSearchIndex];
				if( iep.DeviceI.ts < OldestTimeAllowedForTap )	// buttons are too old.  Stop searching because we're not going to find a match
					break;

				if( !Press.m_InputTypes[iep.type] )
					continue;

				if( iep.GameI.button == Press.m_aButtonsToPress[b] )
				{
					pIEP = &iep;
					break;
				}
			}
			if( pIEP == nullptr )
				break;	// didn't find the button

			// Check that m_aButtonsToHold were being held when the buttons were pressed.
			bool bAllHeldButtonsOK = true;
			for (auto &button: Press.m_aButtonsToHold)
			{
				GameInput gi( controller, button );
				if( !INPUTMAPPER->IsBeingPressed(gi, MultiPlayer_Invalid, &pIEP->InputList) )
					bAllHeldButtonsOK = false;
			}
			for (auto &button: Press.m_aButtonsToNotHold)
			{
				GameInput gi( controller, button );
				if( INPUTMAPPER->IsBeingPressed(gi, MultiPlayer_Invalid, &pIEP->InputList) )
					bAllHeldButtonsOK = false;
			}
			if( !bAllHeldButtonsOK )
				continue;
			iMinSearchIndexUsed = min( iMinSearchIndexUsed, iQueueSearchIndex );
			if( b == (int) Press.m_aButtonsToPress.size()-1 )
				bMatched = true;
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

bool InputQueueCode::Load( std::string sButtonsNames )
{
	m_aPresses.clear();

	auto asPresses = Rage::split( sButtonsNames, ",", Rage::EmptyEntries::include );
	for (auto &sPress: asPresses)
	{
		auto asButtonNames = Rage::split(sPress, "-", Rage::EmptyEntries::include);
		if( asButtonNames.size() < 1 )
		{
			if( sButtonsNames != "" )
				LOG->Trace( "Ignoring empty code \"%s\".", sButtonsNames.c_str() );
			return false;
		}

		m_aPresses.push_back( ButtonPress() );
		// TODO: Verify if a reference can be used.
		for (auto sButtonName: asButtonNames)
		{
			bool bHold = false;
			bool bNotHold = false;
			for(;;)
			{
				if (sButtonName[0] == '+')
				{
					m_aPresses.back().m_InputTypes[IET_REPEAT] = true;
					sButtonName.erase(0, 1);
				}
				else if (sButtonName[0] == '~')
				{
					m_aPresses.back().m_InputTypes[IET_FIRST_PRESS] = false;
					m_aPresses.back().m_InputTypes[IET_RELEASE] = true;
					sButtonName.erase(0, 1);
				}
				else if (sButtonName[0] == '@')
				{
					sButtonName.erase(0, 1);
					bHold = true;
				}
				else if (sButtonName[0] == '!')
				{
					sButtonName.erase(0, 1);
					bNotHold = true;
				}
				else
				{
					break;
				}
			}

			// Search for the corresponding GameButton
			const GameButton gb = INPUTMAPPER->GetInputScheme()->ButtonNameToIndex( sButtonName );
			if( gb == GameButton_Invalid )
			{
				LOG->Trace( "The code \"%s\" contains an unrecognized button \"%s\".", sButtonsNames.c_str(), sButtonName.c_str() );
				m_aPresses.clear();
				return false;
			}

			if( bHold )
				m_aPresses.back().m_aButtonsToHold.push_back( gb );
			else if( bNotHold )
				m_aPresses.back().m_aButtonsToNotHold.push_back( gb );
			else
				m_aPresses.back().m_aButtonsToPress.push_back( gb );
		}
	}

	if( m_aPresses.size() == 1 )
		m_fMaxSecondsBack = 0.55f;
	else
		m_fMaxSecondsBack = (m_aPresses.size()-1)*0.6f;

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
