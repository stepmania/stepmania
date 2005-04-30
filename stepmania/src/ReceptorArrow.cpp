#include "global.h"
#include "ReceptorArrow.h"
#include "PrefsManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "NoteFieldPositioning.h"
#include "NoteSkinManager.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "Game.h"
#include "PlayerState.h"

// eventually, these will replace the pressblocks
const CString PRESS_COMMAND_NAME = "Press";
const CString LIFT_COMMAND_NAME = "Lift";

ReceptorArrow::ReceptorArrow()
{
	m_bIsPressed = false;
	m_bWasPressed = false;
	StopAnimating();
}

bool ReceptorArrow::Load( const PlayerState* pPlayerState, int iColNo )
{
	m_pPlayerState = pPlayerState;
	m_iColNo = iColNo;

	CString sButton = GAMESTATE->GetCurrentGame()->ColToButtonName( iColNo );

	m_pReceptorWaiting.Load( NOTESKIN->GetPath(sButton,"receptor waiting") );
	m_pReceptorGo.Load( NOTESKIN->GetPath(sButton,"receptor go") );
	FOREACH_TapNoteScore( i )
	{
		CString sJudge = TapNoteScoreToString( i );
		CString sCommand = Capitalize(sJudge)+"Command";
		m_sScoreCommand[i] = NOTESKIN->GetMetricA(m_sName,sCommand);
	}

	m_pPressBlock.Load( NOTESKIN->GetPath(sButton,"KeypressBlock") );

	m_pReceptorWaiting->SetEffectClock( Actor::CLOCK_BGM_BEAT );
	m_pReceptorGo->SetEffectClock( Actor::CLOCK_BGM_BEAT );
	m_pPressBlock->SetEffectClock( Actor::CLOCK_BGM_BEAT );

	// draw pressblock before receptors
	this->AddChild( m_pPressBlock );
	this->AddChild( m_pReceptorWaiting );
	this->AddChild( m_pReceptorGo );
	
	return true;
}

void ReceptorArrow::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	// update pressblock alignment based on scroll direction
	bool bReverse = m_pPlayerState->m_PlayerOptions.GetReversePercentForColumn(m_iColNo) > 0.5;
	m_pPressBlock->SetVertAlign( bReverse ? Actor::align_bottom : Actor::align_top );


	m_pReceptorGo->SetHidden( !GAMESTATE->m_bPastHereWeGo );
	m_pReceptorWaiting->SetHidden( GAMESTATE->m_bPastHereWeGo );


	m_pPressBlock->SetHidden( !m_bIsPressed );


	if( m_bWasPressed  &&  !m_bIsPressed )
	{
		m_pReceptorGo->PlayCommand( LIFT_COMMAND_NAME );
		m_pReceptorWaiting->PlayCommand( LIFT_COMMAND_NAME );	
	}

	
	m_bWasPressed = m_bIsPressed;
	m_bIsPressed = false;	// it may get turned back on next update
}

void ReceptorArrow::DrawPrimitives()
{

	ActorFrame::DrawPrimitives();
}

void ReceptorArrow::Step( TapNoteScore score )
{
	m_pReceptorGo->FinishTweening();
	m_pReceptorWaiting->FinishTweening();
	m_pReceptorGo->RunCommands( *m_sScoreCommand[score] );
	m_pReceptorWaiting->RunCommands( *m_sScoreCommand[score] );

	m_pReceptorGo->PlayCommand( PRESS_COMMAND_NAME );
	m_pReceptorWaiting->PlayCommand( PRESS_COMMAND_NAME );	
}

/*
 * (c) 2001-2004 Ben Nordstrom, Chris Danford
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
