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
#include "NotesLoaderSM.h"

#define SECONDS_TO_SHOW						THEME->GetMetricF("ScreenHowToPlay","SecondsToShow")
//
#define USELIFEBAR							THEME->GetMetricB("ScreenHowToPlay","UseLifeMeterBar")
#define LIFEBARONCOMMAND					THEME->GetMetric ("ScreenHowToPlay","LifeMeterBarOnCommand")
//
#define USECHARACTER						THEME->GetMetricB("ScreenHowToPlay","UseCharacter")
#define CHARACTERONCOMMAND					THEME->GetMetric ("ScreenHowToPlay","CharacterOnCommand")
//
#define USEPAD								THEME->GetMetricB("ScreenHowToPlay","UsePad")
#define PADONCOMMAND						THEME->GetMetric ("ScreenHowToPlay","PadOnCommand")
//
#define USEPLAYER							THEME->GetMetricB("ScreenHowToPlay","UseNotefield")
#define PLAYERX								THEME->GetMetricF("ScreenHowToPlay","PlayerX")
#define NUM_PERFECTS						THEME->GetMetricI("ScreenHowToPlay","NumPerfects")
#define NUM_MISSES							THEME->GetMetricI("ScreenHowToPlay","NumMisses")
//#define SONG_BPM							THEME->GetMetricF("ScreenHowToPlay","SongBPM")
#define STEPFILE							THEME->GetMetric ("ScreenHowToPlay","Stepfile")

ScreenHowToPlay::ScreenHowToPlay() : ScreenAttract("ScreenHowToPlay")
{
	m_bUsingLifeBar = USELIFEBAR;
	m_bUsingPlayer = USEPLAYER;
	m_bUsingPad = (USEPAD && DoesFileExist("Characters" SLASH "DancePad-DDR.txt"));
	m_bUsingCharacter = (USECHARACTER && GAMESTATE->m_pCharacters.size());

	m_iPerfects = 0;
	m_iNumPerfects = NUM_PERFECTS;

	m_In.Load( THEME->GetPathToB("ScreenHowToPlay in") );
	m_In.StartTransitioning();

	m_Out.Load( THEME->GetPathToB("ScreenHowToPlay out") );

	m_Overlay.LoadFromAniDir( THEME->GetPathToB("ScreenHowToPlay overlay") );
	this->AddChild( &m_Overlay );

	if( m_bUsingPad )
	{
		m_mDancePad.LoadMilkshapeAscii("Characters" SLASH "DancePad-DDR.txt");
		m_mDancePad.SetRotationX( 35 );
		m_mDancePad.Command( PADONCOMMAND );
	}
	
	// Display random character
	if( m_bUsingCharacter )
	{
		Character* rndchar = GAMESTATE->GetRandomCharacter();

		m_mCharacter.LoadMilkshapeAscii( rndchar->GetModelPath() );
//		m_mCharacter.LoadMilkshapeAsciiBones("howtoplay", rndchar->GetHowToPlayAnimationPath() );
		m_mCharacter.LoadMilkshapeAsciiBones( "Step-LEFT","Characters" SLASH "BeginnerHelper_step-left.bones.txt" );
		m_mCharacter.LoadMilkshapeAsciiBones( "Step-DOWN","Characters" SLASH "BeginnerHelper_step-down.bones.txt" );
		m_mCharacter.LoadMilkshapeAsciiBones( "Step-UP","Characters" SLASH "BeginnerHelper_step-up.bones.txt" );
		m_mCharacter.LoadMilkshapeAsciiBones( "Step-RIGHT","Characters" SLASH "BeginnerHelper_step-right.bones.txt" );
		m_mCharacter.LoadMilkshapeAsciiBones( "Step-JUMPLR","Characters" SLASH "BeginnerHelper_step-jumplr.bones.txt" );
		m_mCharacter.LoadMilkshapeAsciiBones( "rest",rndchar->GetRestAnimationPath() );
		m_mCharacter.SetDefaultAnimation( "rest" );
		m_mCharacter.PlayAnimation( "rest" );
		m_mCharacter.m_bRevertToDefaultAnimation = true;		// Stay bouncing after a step has finished animating.
		
		m_mCharacter.SetRotationX( 40 );
		m_mCharacter.Command( CHARACTERONCOMMAND );
	}
	
	LifeMeterBar *pBarUsed = NULL;
	// silly to use the lifebar without a player, since the player updates the lifebar
	if( m_bUsingLifeBar )
	{
		m_LifeMeterBar.Load( PLAYER_1 );
		m_LifeMeterBar.Command( LIFEBARONCOMMAND );
		m_LifeMeterBar.FillForHowToPlay( NUM_PERFECTS, NUM_MISSES );
		pBarUsed = &m_LifeMeterBar;
	}

	switch(GAMESTATE->m_CurGame) // which style should we use to demonstrate?
	{
		case GAME_DANCE:	GAMESTATE->m_CurStyle = STYLE_DANCE_SINGLE;		break;
		case GAME_PUMP:		GAMESTATE->m_CurStyle = STYLE_PUMP_SINGLE;		break;
		case GAME_EZ2:		GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE;		break;
		case GAME_PARA:		GAMESTATE->m_CurStyle = STYLE_PARA_SINGLE;		break;
		case GAME_DS3DDX:	GAMESTATE->m_CurStyle = STYLE_DS3DDX_SINGLE;	break;
		case GAME_BM:		GAMESTATE->m_CurStyle = STYLE_BM_SINGLE;		break;
		case GAME_MANIAX:	GAMESTATE->m_CurStyle = STYLE_MANIAX_SINGLE;	break;
		default: ASSERT(0); // we should cover all gametypes....
	}

	m_pSong = new Song;
	SMLoader smfile;
	smfile.LoadFromSMFile( THEME->GetCurThemeDir() + STEPFILE, *m_pSong, false );
	ASSERT( m_pSong->m_apNotes.size() == 1 );

	GAMESTATE->m_pCurSong = m_pSong;
	GAMESTATE->m_bPastHereWeGo = true;
	GAMESTATE->m_PlayerController[PLAYER_1] = PC_AUTOPLAY;

	NoteData *pND = new NoteData;
	m_pSong->m_apNotes[0]->GetNoteData(pND);
	m_Player.Load( PLAYER_1, pND, pBarUsed, NULL, NULL, NULL, NULL, NULL );
	delete pND;
	m_Player.SetX( PLAYERX );

	// Don't show judgement
	GAMESTATE->m_PlayerOptions[PLAYER_1].m_fBlind = 1;
	GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
	GAMESTATE->m_bDemonstrationOrJukebox = true;
	this->AddChild( &m_Player );
	if( m_bUsingLifeBar )
		this->AddChild( &m_LifeMeterBar );

	m_fFakeSecondsIntoSong = 0;
	this->ClearMessageQueue();
	this->PostScreenMessage( SM_BeginFadingOut, SECONDS_TO_SHOW );

	this->MoveToTail( &m_Overlay );
	this->MoveToTail( &m_In );
	this->MoveToTail( &m_Out );
}

ScreenHowToPlay::~ScreenHowToPlay()
{
	delete m_pSong;
}

void ScreenHowToPlay::Step( float fDelta )
{
#define ST_LEFT		0x08
#define ST_DOWN		0x04
#define ST_UP		0x02
#define ST_RIGHT	0x01
#define ST_JUMPLR	(ST_LEFT | ST_RIGHT)
#define ST_JUMPUD	(ST_UP | ST_DOWN)

	float rate = 1; //GAMESTATE->m_fCurBPS;

	int iStep = 0;
	int iNoteRow = BeatToNoteRowNotRounded( GAMESTATE->m_fSongBeat + 0.6f );
	int iNumTracks = m_Player.GetNumTracks();
	// if we want to miss from here on out, don't process steps.
	if((m_iPerfects < m_iNumPerfects) && (m_Player.IsThereATapAtRow( iNoteRow )))
	{
		for( int k=0; k<iNumTracks; k++ )
			if( m_Player.GetTapNote(k, iNoteRow ) == TAP_TAP )
				iStep += 1 << (iNumTracks - (k + 1));

		switch( iStep )
		{
		case ST_LEFT:	m_mCharacter.PlayAnimation( "Step-LEFT", 1.8f ); break;
		case ST_RIGHT:	m_mCharacter.PlayAnimation( "Step-RIGHT", 1.8f ); break;
		case ST_UP:		m_mCharacter.PlayAnimation( "Step-UP", 1.8f ); break;
		case ST_DOWN:	m_mCharacter.PlayAnimation( "Step-DOWN", 1.8f ); break;
		case ST_JUMPLR: m_mCharacter.PlayAnimation( "Step-JUMPLR", 1.8f ); break;
		case ST_JUMPUD:
			// Until I can get an UP+DOWN jump animation, this will have to do.
			m_mCharacter.PlayAnimation( "Step-JUMPLR", 1.8f );
			
			m_mCharacter.StopTweening();
			m_mCharacter.BeginTweening( GAMESTATE->m_fCurBPS /8, TWEEN_LINEAR );
			m_mCharacter.SetRotationY( 90 );
			m_mCharacter.BeginTweening( (1/(GAMESTATE->m_fCurBPS * 2) ) ); //sleep between jump-frames
			m_mCharacter.BeginTweening( GAMESTATE->m_fCurBPS /6, TWEEN_LINEAR );
			m_mCharacter.SetRotationY( 0 );
			break;
		}
	}

	// if we want to freeze, freeze.
	if( GAMESTATE->m_bFreeze )
		rate = 0;

	m_mCharacter.Update( fDelta * rate );
}

void ScreenHowToPlay::Update( float fDelta )
{
	if(GAMESTATE->m_pCurSong != NULL)
	{
		GAMESTATE->UpdateSongPosition( m_fFakeSecondsIntoSong );
		m_fFakeSecondsIntoSong += fDelta;

		static int iLastNoteRowCounted = 0;
		int iCurNoteRow = BeatToNoteRowNotRounded( GAMESTATE->m_fSongBeat );

		if(( iCurNoteRow != iLastNoteRowCounted ) &&(m_Player.IsThereATapAtRow( iCurNoteRow )))
		{
			m_iPerfects++;
			iLastNoteRowCounted = iCurNoteRow;
		}

		// we want misses from beat 22.8 on out, so change to a HUMAN controller.
		// since we aren't taking input from the user, the steps are always missed.
		if(m_iPerfects > m_iNumPerfects)
			GAMESTATE->m_PlayerController[PLAYER_1] = PC_HUMAN;

		if ( m_bUsingCharacter )
		{
			Step( fDelta );
		}
	}

	if( m_bUsingPad )
		m_mDancePad.Update( fDelta );

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

	if( m_bUsingPad || m_bUsingCharacter )
	{
		DISPLAY->SetLighting( true );
		DISPLAY->SetLightDirectional( 
			0, 
			RageColor(0.5,0.5,0.5,1), 
			RageColor(1,1,1,1),
			RageColor(0,0,0,1),
			RageVector3(0, 0, 1) );

		if( m_bUsingCharacter )
			m_mCharacter.Draw();
		if( m_bUsingPad )
			m_mDancePad.Draw();

		DISPLAY->SetLightOff( 0 );
		DISPLAY->SetLighting( false );
	}

	m_Overlay.DrawPrimitives();
	m_In.DrawPrimitives();
	m_Out.DrawPrimitives();
}
