#include "global.h"
#include "stdlib.h"
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
#include "RageDisplay.h"
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

	StepsType nt = GAMESTATE->GetCurrentStyleDef()->m_StepsType;
	int iNumOfTracks = GAMEMAN->NotesTypeToNumTracks( nt );

	ASSERT(iNumOfTracks > 0); // crazy to have less than 1 track....

	m_LifeMeterBar.Load( PLAYER_1 );
	m_LifeMeterBar.SetXY( 480, 40 );
	// we need to be a little lower than half if we want to hit
	// zero with the actual miss steps. cheat.
	m_LifeMeterBar.ChangeLife(TNS_MISS);

	// Display random character+pad
	if( GAMESTATE->m_pCharacters.size() )
	{
		Character* rndchar = GAMESTATE->GetRandomCharacter();

		m_mCharacter.LoadMilkshapeAscii( rndchar->GetModelPath() );
		m_mDancePad.LoadMilkshapeAscii("Characters" SLASH "DancePad-DDR.txt");
		m_mCharacter.LoadMilkshapeAsciiBones("howtoplay", rndchar->GetHowToPlayAnimationPath() );
		
		m_mCharacter.SetRotationX( 40 );
		m_mDancePad.SetRotationX( 35 );
		m_mCharacter.Command("X,120;Y,300;Zoom,15;RotationY,180;sleep,4.7;linear,1.0;RotationY,360;Zoom,20;X,120;Y,400");
		m_mDancePad.Command("X,40;Y,310;Zoom,15;RotationY,180;sleep,4.7;linear,1.0;RotationY,360;Zoom,20;X,230;Y,390");
		this->AddChild(&m_mCharacter);		
		this->AddChild(&m_mDancePad);
	}
	//

	NoteData* pND = new NoteData;
	pND->SetNumTracks( iNumOfTracks );
	/* Old notes.. Until our HowToPlay animation changes, Steps changed.
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
	*/

	pND->SetTapNote( 1, ROWS_PER_BEAT*16, TAP_TAP );
	pND->SetTapNote( 2, ROWS_PER_BEAT*18, TAP_TAP );
	pND->SetTapNote( 0, ROWS_PER_BEAT*20, TAP_TAP );
	pND->SetTapNote( 0, ROWS_PER_BEAT*22, TAP_TAP );
	pND->SetTapNote( 3, ROWS_PER_BEAT*22, TAP_TAP );
	//Misses
	pND->SetTapNote( 0, (int)(ROWS_PER_BEAT*24.0f), TAP_TAP );
	pND->SetTapNote( 1, (int)(ROWS_PER_BEAT*24.1f), TAP_TAP );
	pND->SetTapNote( 3, (int)(ROWS_PER_BEAT*24.2f), TAP_TAP );
	pND->SetTapNote( 1, (int)(ROWS_PER_BEAT*24.3f), TAP_TAP );
	pND->SetTapNote( 2, (int)(ROWS_PER_BEAT*24.4f), TAP_TAP );
	pND->SetTapNote( 0, (int)(ROWS_PER_BEAT*24.5f), TAP_TAP );

	m_pSong = new Song;
	float fSongBPM = SONG_BPM;	// need this on a separate line, otherwise VC6 release build screws up and returns 0
	m_pSong->AddBPMSegment( BPMSegment(0.0,fSongBPM) );
	/* Same as above
	m_pSong->AddStopSegment( StopSegment(12,2) );
	m_pSong->AddStopSegment( StopSegment(16,2) );
	m_pSong->AddStopSegment( StopSegment(20,2) );
	m_pSong->AddStopSegment( StopSegment(24,2) );
	m_pSong->AddStopSegment( StopSegment(28,2) );
	*/
	m_pSong->AddStopSegment( StopSegment(16,2) );
	m_pSong->AddStopSegment( StopSegment(18,2) );
	m_pSong->AddStopSegment( StopSegment(20,2) );
	m_pSong->AddStopSegment( StopSegment(22,2) );

	GAMESTATE->m_pCurSong = m_pSong;
	GAMESTATE->m_bPastHereWeGo = true;
	GAMESTATE->m_PlayerController[PLAYER_1] = PC_AUTOPLAY;
	m_Player.Load( PLAYER_1, pND, &m_LifeMeterBar, NULL, NULL, NULL, NULL, NULL );
	
	m_Player.SetX( 480 );
	// Don't show judgement
	GAMESTATE->m_PlayerOptions[PLAYER_1].m_fBlind = 1;
	GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
	GAMESTATE->m_bDemonstrationOrJukebox = true;
	this->AddChild( &m_Player );
	this->AddChild( &m_LifeMeterBar );
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
	if(GAMESTATE->m_pCurSong != NULL)
	{
		GAMESTATE->UpdateSongPosition( m_fFakeSecondsIntoSong );
		m_fFakeSecondsIntoSong += fDelta;

		if( GAMESTATE->m_bFreeze )
		{
			switch(int(GAMESTATE->m_fSongBeat))
			{
			case 16: m_mCharacter.SetFrame(30); break;
			case 18: m_mCharacter.SetFrame(85); break;
			case 20: m_mCharacter.SetFrame(150); break;
			case 22: m_mCharacter.SetFrame(270); break;
			};
		}
		else
		{
			/* HowToPlay animations.. This will be retimed soon to be a reasonable speed.
			   This was just a quick'n'dirty way of showing this screen correctly. */
			// Until we get a better HowToPlay animation done, it has to be this way..
			if( (GAMESTATE->m_fSongBeat >= 15.0f && GAMESTATE->m_fSongBeat <= 15.2f) )
			{
				m_mCharacter.SetFrame(1);
			}
			if( (GAMESTATE->m_fSongBeat >= 17.0f && GAMESTATE->m_fSongBeat <= 17.2f) )
			{ 
				m_mCharacter.SetFrame(55);
			}
			if( (GAMESTATE->m_fSongBeat >= 19.0f && GAMESTATE->m_fSongBeat <= 19.2f) )
			{ 
				m_mCharacter.SetFrame(120);
			}
			if( (GAMESTATE->m_fSongBeat >= 21.0f && GAMESTATE->m_fSongBeat <= 21.2f) )
			{ 
				m_mCharacter.SetFrame(240);
			}
			// ----------------------------------------------------------------------- */

			// we want misses from here on out, so change to a HUMAN controller.
			// since we aren't taking input from the user, the steps are always missed.
			if(GAMESTATE->m_fSongBeat > 22.8f)
				GAMESTATE->m_PlayerController[PLAYER_1] = PC_HUMAN;

			if( (GAMESTATE->m_fSongBeat >= 0.0f && GAMESTATE->m_fSongBeat <= 15.0f) ||
				(GAMESTATE->m_fSongBeat >= 16.8f && GAMESTATE->m_fSongBeat <= 17.0f) ||
				(GAMESTATE->m_fSongBeat >= 18.8f && GAMESTATE->m_fSongBeat <= 19.0f) ||
				(GAMESTATE->m_fSongBeat >= 20.8f && GAMESTATE->m_fSongBeat <= 21.0f) ||
				(GAMESTATE->m_fSongBeat >= 22.8f) )
			{
				if( (m_mCharacter.GetCurFrame() >=243) || (m_mCharacter.GetCurFrame() <=184) )
					m_mCharacter.SetFrame(184);
				//Loop for HowToPlay static movement is 184~243
			};

			m_mCharacter.Update( (0+(fDelta*1.08f)) );
		};	

	}

	ScreenAttract::Update( fDelta );
}

void ScreenHowToPlay::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToNextScreen:
		GAMESTATE->Reset();
		break;
	}
	ScreenAttract::HandleScreenMessage( SM );
}

void ScreenHowToPlay::DrawPrimitives()
{
	Screen::DrawPrimitives();
	DISPLAY->SetLighting( true );
	DISPLAY->SetLightDirectional( 
		0, 
		RageColor(0.5,0.5,0.5,1), 
		RageColor(1,1,1,1),
		RageColor(0,0,0,1),
		RageVector3(0, 0, 1) );

	m_mCharacter.Draw();
	m_mDancePad.Draw();

	DISPLAY->SetLightOff( 0 );
	DISPLAY->SetLighting( false );
}
