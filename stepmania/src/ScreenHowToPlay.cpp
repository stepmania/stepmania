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
#include "GameManager.h"

#define SONG_BPM				THEME->GetMetricF("ScreenHowToPlay","SongBPM")
#define SECONDS_TO_SHOW			THEME->GetMetricF("ScreenHowToPlay","SecondsToShow")

ScreenHowToPlay::ScreenHowToPlay() : ScreenAttract("ScreenHowToPlay")
{
	switch(GAMESTATE->m_CurGame) // which style should we use to demonstrate?
	{
		case GAME_DANCE:	GAMESTATE->m_CurStyle = STYLE_DANCE_SINGLE; break;
		case GAME_PUMP:		GAMESTATE->m_CurStyle = STYLE_PUMP_SINGLE;	break;
		case GAME_EZ2:		GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE;	break;
		case GAME_PARA:		GAMESTATE->m_CurStyle = STYLE_PARA_SINGLE;	break;
		case GAME_DS3DDX:	GAMESTATE->m_CurStyle = STYLE_DS3DDX_SINGLE; break;
		case GAME_BM:		GAMESTATE->m_CurStyle = STYLE_BM_SINGLE;	break;
		default: ASSERT(0); // we should cover all gametypes....
	}

	NotesType nt = GAMESTATE->GetCurrentStyleDef()->m_NotesType;
	int iNumOfTracks = GAMEMAN->NotesTypeToNumTracks( nt );

	ASSERT(iNumOfTracks > 0); // crazy to have less than 1 track....

	m_LifeMeter.Load( PLAYER_1 );
	m_LifeMeter.SetXY( 480, 40 );
	this->AddChild( &m_LifeMeter );

	NoteData* pND = new NoteData;
	pND->SetNumTracks( iNumOfTracks );
	pND->SetTapNote( 0, ROWS_PER_BEAT*12, TAP_TAP );
	pND->SetTapNote( 1, ROWS_PER_BEAT*16, TAP_TAP );
	pND->SetTapNote( 2, ROWS_PER_BEAT*20, TAP_TAP );
	pND->SetTapNote( 0, ROWS_PER_BEAT*24, TAP_TAP );
	pND->SetTapNote( 3, ROWS_PER_BEAT*24, TAP_TAP );
	pND->SetTapNote( 0, ROWS_PER_BEAT*28, TAP_TAP );
	pND->AddHoldNote( HoldNote(2, 32, 35) );
	pND->AddHoldNote( HoldNote(0, 36, 39) );
	pND->AddHoldNote( HoldNote(3, 36, 39) );
	pND->AddHoldNote( HoldNote(2, 40, 43) );
	pND->SetTapNote( 0, ROWS_PER_BEAT*44,    TAP_TAP );
	pND->SetTapNote( 1, (int)(ROWS_PER_BEAT*44.5f), TAP_TAP );
	pND->SetTapNote( 2, ROWS_PER_BEAT*45,    TAP_TAP );
	pND->SetTapNote( 3, (int)(ROWS_PER_BEAT*45.5f), TAP_TAP );
	pND->SetTapNote( 0, ROWS_PER_BEAT*46,    TAP_TAP );
	pND->SetTapNote( 2, ROWS_PER_BEAT*46,    TAP_TAP );

	m_pSong = new Song;
	float fSongBPM = SONG_BPM;	// need this on a separate line, otherwise VC6 release build screws up and returns 0
	m_pSong->AddBPMSegment( BPMSegment(0.0,fSongBPM) );
	m_pSong->AddStopSegment( StopSegment(15,2) );
	m_pSong->AddStopSegment( StopSegment(16,2) );
	m_pSong->AddStopSegment( StopSegment(20,2) );
	m_pSong->AddStopSegment( StopSegment(24,2) );
	m_pSong->AddStopSegment( StopSegment(28,2) );

	GAMESTATE->m_pCurSong = m_pSong;
	GAMESTATE->m_bPastHereWeGo = true;
	GAMESTATE->m_PlayerController[PLAYER_1] = PC_AUTOPLAY;
	m_Player.Load( PLAYER_1, pND, &m_LifeMeter, NULL, NULL, NULL );

	m_Player.SetX( 480 );
	m_Player.DontShowJudgement();
	this->AddChild( &m_Player );

	delete pND;

	m_fFakeSecondsIntoSong = 0;
	this->ClearMessageQueue();
	this->PostScreenMessage( SM_BeginFadingOut, SECONDS_TO_SHOW );
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

void ScreenHowToPlay::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_BeginFadingOut:
		GAMESTATE->m_pCurSong = NULL;
		break;
	}
	ScreenAttract::HandleScreenMessage( SM );
}
