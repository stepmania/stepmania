#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ReceptorArrow

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ReceptorArrow.h"
#include "PrefsManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "NoteFieldPositioning.h"
#include "NoteSkinManager.h"
#include "RageLog.h"
#include "RageUtil.h"


ReceptorArrow::ReceptorArrow()
{
	m_bIsPressed = false;
	StopAnimating();
}

bool ReceptorArrow::Load( CString NoteSkin, PlayerNumber pn, int iColNo )
{
	m_PlayerNumber = pn;
	m_iColNo = iColNo;

	CString sButton = g_NoteFieldMode[pn].GrayButtonNames[iColNo];
	if( sButton == "" )
		sButton = NoteSkinManager::ColToButtonName( iColNo );

	CString sPath;
	m_pReceptorWaiting.Load( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin,sButton,"receptor waiting") );
	m_pReceptorGo.Load( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin,sButton,"receptor go") );
	m_sStepCommand = NOTESKIN->GetMetric(GAMESTATE->m_PlayerOptions[pn].m_sNoteSkin,"ReceptorArrow","StepCommand");

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

void ReceptorArrow::Step()
{
	m_pReceptorGo->FinishTweening();
	m_pReceptorWaiting->FinishTweening();
	m_pReceptorGo->Command( m_sStepCommand );
	m_pReceptorWaiting->Command( m_sStepCommand );
}
