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


ReceptorArrow::ReceptorArrow()
{
	m_bIsPressed = false;
	StopAnimating();
}

bool ReceptorArrow::Load( CString NoteSkin, PlayerNumber pn, int iColNo )
{
	m_PlayerNumber = pn;
	m_iColNo = iColNo;

    NoteFieldMode &mode = g_NoteFieldMode[pn];
	CString sButton = mode.GrayButtonNames[iColNo];
	if( sButton == "" )
		sButton = GAMESTATE->GetCurrentGame()->ColToButtonName( iColNo );

	CString sPath;
	m_pReceptorWaiting.Load( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin,sButton,"receptor waiting") );
	m_pReceptorGo.Load( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin,sButton,"receptor go") );
	FOREACH_TapNoteScore( i )
	{
		CString sJudge = TapNoteScoreToString( i );
		CString sCommand = Capitalize(sJudge)+"Command";
		m_sScoreCommand[i] = NOTESKIN->GetMetric( NoteSkin, m_sName, sCommand );
	}

	m_pPressBlock.Load( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin,sButton,"KeypressBlock") );

	// draw pressblock before receptors
	this->AddChild( m_pPressBlock );
	this->AddChild( m_pReceptorWaiting );
	this->AddChild( m_pReceptorGo );
	
	return true;
}

void ReceptorArrow::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	float fPercentIntoBeat = GAMESTATE->m_fSongBeat;
	wrap(fPercentIntoBeat, 1);	/* Beats can start in very negative territory */

	// TODO: Generalize this to allow any actor to animate based on the beat.
	m_pReceptorWaiting->SetSecondsIntoAnimation( fPercentIntoBeat );
	m_pReceptorGo->SetSecondsIntoAnimation( fPercentIntoBeat );
	m_pPressBlock->SetSecondsIntoAnimation( fPercentIntoBeat );

	// update pressblock alignment based on scroll direction
	bool bReverse = GAMESTATE->m_PlayerOptions[m_PlayerNumber].GetReversePercentForColumn(m_iColNo) > 0.5;
	m_pPressBlock->SetVertAlign( bReverse ? Actor::align_bottom : Actor::align_top );
}

void ReceptorArrow::DrawPrimitives()
{
	m_pReceptorGo->SetHidden( !GAMESTATE->m_bPastHereWeGo );
	m_pReceptorWaiting->SetHidden( GAMESTATE->m_bPastHereWeGo );
	m_pPressBlock->SetHidden( !m_bIsPressed );
	m_bIsPressed = false;	// it may get turned back on next update

	ActorFrame::DrawPrimitives();
}

void ReceptorArrow::Step( TapNoteScore score )
{
	m_pReceptorGo->FinishTweening();
	m_pReceptorWaiting->FinishTweening();
	m_pReceptorGo->Command( m_sScoreCommand[score] );
	m_pReceptorWaiting->Command( m_sScoreCommand[score] );
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
