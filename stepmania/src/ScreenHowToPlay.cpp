#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenHowToPlay

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenHowToPlay.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "GameDef.h"
#include "RageLog.h"
#include "SongManager.h"
#include "NoteFieldPositioning.h"


ScreenHowToPlay::ScreenHowToPlay() : ScreenAttract("ScreenHowToPlay")
{
	GAMESTATE->m_CurStyle = STYLE_DANCE_SINGLE;

	m_LifeMeter.Load( PLAYER_1 );
	m_LifeMeter.SetXY( 480, 40 );
	this->AddChild( &m_LifeMeter );

	NoteData* pND = new NoteData;
	pND->SetNumTracks( 4 );
	pND->SetTapNote( 0, ROWS_PER_BEAT*12, TAP_TAP );
	pND->SetTapNote( 1, ROWS_PER_BEAT*16, TAP_TAP );
	pND->SetTapNote( 2, ROWS_PER_BEAT*20, TAP_TAP );
	pND->SetTapNote( 0, ROWS_PER_BEAT*24, TAP_TAP );
	pND->SetTapNote( 3, ROWS_PER_BEAT*24, TAP_TAP );
	pND->SetTapNote( 0, ROWS_PER_BEAT*28, TAP_TAP );

	m_pSong = new Song;
	m_pSong->AddBPMSegment( BPMSegment(0,120) );
	m_pSong->AddStopSegment( StopSegment(12,2) );
	m_pSong->AddStopSegment( StopSegment(16,2) );
	m_pSong->AddStopSegment( StopSegment(20,2) );
	m_pSong->AddStopSegment( StopSegment(24,2) );
	m_pSong->AddStopSegment( StopSegment(28,2) );
	GAMESTATE->m_pCurSong = m_pSong;
	GAMESTATE->m_bPastHereWeGo = true;

	m_Player.Load( PLAYER_1, pND, NULL, NULL, NULL, NULL );
	m_Player.SetX( 480 );
	this->AddChild( &m_Player );

	delete pND;

	m_fFakeSecondsIntoSong = 0;

	GAMESTATE->m_Position[PLAYER_1]->LoadFromStyleDef(GAMESTATE->GetCurrentStyleDef(), PLAYER_1);
}

ScreenHowToPlay::~ScreenHowToPlay()
{
	delete m_pSong;
}

void ScreenHowToPlay::Update( float fDelta )
{
	m_fFakeSecondsIntoSong += fDelta;
	GAMESTATE->UpdateSongPosition( m_fFakeSecondsIntoSong );
	
	ScreenAttract::Update( fDelta );
}