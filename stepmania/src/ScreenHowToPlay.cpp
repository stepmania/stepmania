#include "global.h"
#include <cstdlib>
#include "ScreenHowToPlay.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "Game.h"
#include "RageLog.h"
#include "RageDisplay.h"
#include "SongManager.h"
#include "Steps.h"
#include "NoteFieldPositioning.h"
#include "GameManager.h"
#include "NotesLoaderSM.h"
#include "GameSoundManager.h"
#include "Model.h"
#include "ThemeMetric.h"
#include "PlayerState.h"
#include "Style.h"
#include "ActorUtil.h"
#include "PrefsManager.h"
#include "CharacterManager.h"
#include "StatsManager.h"

static const ThemeMetric<CString>	STEPFILE		("ScreenHowToPlay","Stepfile");
static const ThemeMetric<int>		NUM_W2S			("ScreenHowToPlay","NumW2s");
static const ThemeMetric<int>		NUM_MISSES		("ScreenHowToPlay","NumMisses");
static const ThemeMetric<bool>		USELIFEBAR		("ScreenHowToPlay","UseLifeMeterBar");
static const ThemeMetric<bool>		USECHARACTER	("ScreenHowToPlay","UseCharacter");
static const ThemeMetric<bool>		USEPAD			("ScreenHowToPlay","UsePad");
static const ThemeMetric<bool>		USEPLAYER		("ScreenHowToPlay","UseNotefield");

enum Animation
{
	ANIM_DANCE_PAD,
	ANIM_DANCE_PADS,
	ANIM_UP,
	ANIM_DOWN,
	ANIM_LEFT,
	ANIM_RIGHT,
	ANIM_JUMPLR,
	NUM_ANIMATIONS
};

static const CString anims[NUM_ANIMATIONS] =
{
	"DancePad.txt",
	"DancePads.txt",
	"BeginnerHelper_step-up.bones.txt",
	"BeginnerHelper_step-down.bones.txt",
	"BeginnerHelper_step-left.bones.txt",
	"BeginnerHelper_step-right.bones.txt",
	"BeginnerHelper_step-jumplr.bones.txt"
};


static CString GetAnimPath( Animation a )
{
	return CString("Characters/") + anims[a];
}

static bool HaveAllCharAnimations()
{
	for( int i = ANIM_UP; i < NUM_ANIMATIONS; ++i )
		if( !DoesFileExist( GetAnimPath( (Animation) i ) ) )
			return false;
	return true;
}

REGISTER_SCREEN_CLASS( ScreenHowToPlay );
ScreenHowToPlay::ScreenHowToPlay( CString sName ) : ScreenAttract( sName )
{
	m_iW2s = 0;
	m_iNumW2s = NUM_W2S;

	// initialize these because they might not be used.
	m_pPlayer = NULL;
	m_pLifeMeterBar = NULL;
	m_pmCharacter = NULL;
	m_pmDancePad = NULL;
}

void ScreenHowToPlay::Init()
{
	ScreenAttract::Init();

	if( (bool)USEPAD && DoesFileExist( GetAnimPath(ANIM_DANCE_PAD) ) )
	{
		m_pmDancePad = new Model;
		m_pmDancePad->SetName( "Pad" );
		m_pmDancePad->LoadMilkshapeAscii( GetAnimPath(ANIM_DANCE_PAD) );
		m_pmDancePad->SetRotationX( 35 );
		SET_XY_AND_ON_COMMAND( m_pmDancePad );
	}
	
	// Display random character
	vector<Character*> vpCharacters;
	CHARMAN->GetCharacters( vpCharacters );
	if( (bool)USECHARACTER && vpCharacters.size() && HaveAllCharAnimations() )
	{
		Character* rndchar = CHARMAN->GetRandomCharacter();

		CString sModelPath = rndchar->GetModelPath();
		if( sModelPath != "" )
		{
			m_pmCharacter = new Model;
			m_pmCharacter->SetName( "Character" );
			m_pmCharacter->LoadMilkshapeAscii( rndchar->GetModelPath() );
			m_pmCharacter->LoadMilkshapeAsciiBones( "Step-LEFT", GetAnimPath( ANIM_LEFT ) );
			m_pmCharacter->LoadMilkshapeAsciiBones( "Step-DOWN", GetAnimPath( ANIM_DOWN ) );
			m_pmCharacter->LoadMilkshapeAsciiBones( "Step-UP", GetAnimPath( ANIM_UP ) );
			m_pmCharacter->LoadMilkshapeAsciiBones( "Step-RIGHT", GetAnimPath( ANIM_RIGHT ) );
			m_pmCharacter->LoadMilkshapeAsciiBones( "Step-JUMPLR", GetAnimPath( ANIM_JUMPLR ) );
			m_pmCharacter->LoadMilkshapeAsciiBones( "rest",rndchar->GetRestAnimationPath() );
			m_pmCharacter->SetDefaultAnimation( "rest" );
			m_pmCharacter->PlayAnimation( "rest" );
			m_pmCharacter->m_bRevertToDefaultAnimation = true;		// Stay bouncing after a step has finished animating.
			
			m_pmCharacter->SetRotationX( 40 );
			m_pmCharacter->SetCullMode( CULL_NONE );	// many of the models floating around have the vertex order flipped
			SET_XY_AND_ON_COMMAND( m_pmCharacter );
		}
	}
	
	// silly to use the lifebar without a player, since the player updates the lifebar
	if( USELIFEBAR )
	{
		m_pLifeMeterBar = new LifeMeterBar;
		m_pLifeMeterBar->SetName("LifeMeterBar");
		m_pLifeMeterBar->Load( GAMESTATE->m_pPlayerState[PLAYER_1], &STATSMAN->m_CurStageStats.m_player[PLAYER_1] );
		SET_XY_AND_ON_COMMAND( m_pLifeMeterBar );
		m_pLifeMeterBar->FillForHowToPlay( NUM_W2S, NUM_MISSES );
	}

	GAMESTATE->m_pCurStyle.Set( GAMEMAN->GetHowToPlayStyleForGame(GAMESTATE->m_pCurGame) );

	if( USEPLAYER )
	{
		SMLoader smfile;		
		smfile.LoadFromSMFile( THEME->GetPathO("", STEPFILE), m_Song, false );
		m_Song.AddAutoGenNotes();

		const Style* pStyle = GAMESTATE->GetCurrentStyle();
		
		Steps *pSteps = m_Song.GetStepsByDescription( pStyle->m_StepsType, "" );
		ASSERT_M( pSteps != NULL, ssprintf("%i", pStyle->m_StepsType) );

		NoteData tempNoteData;
		pSteps->GetNoteData( tempNoteData );
		pStyle->GetTransformedNoteDataForStyle( PLAYER_1, tempNoteData, m_NoteData );

		GAMESTATE->m_pCurSong.Set( &m_Song );
		GAMESTATE->m_bGameplayLeadIn.Set( false );
		GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerController = PC_AUTOPLAY;

		m_pPlayer = new Player;
		m_pPlayer->Init( 
			"Player",
			GAMESTATE->m_pPlayerState[PLAYER_1], 
			NULL,
			m_pLifeMeterBar, 
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			NULL );
		m_pPlayer->Load( m_NoteData );
		m_pPlayer->SetName( "Player" );
		this->AddChild( m_pPlayer );
		SET_XY_AND_ON_COMMAND( m_pPlayer );

		// Don't show judgement
		GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions.m_fBlind = 1;
		GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
		GAMESTATE->m_bDemonstrationOrJukebox = true;
	}

	// deferred until after the player, so the notes go under it
	if( m_pLifeMeterBar )
		this->AddChild( m_pLifeMeterBar );

	m_fFakeSecondsIntoSong = 0;

	this->MoveToTail( &m_In );
	this->MoveToTail( &m_Out );
}

ScreenHowToPlay::~ScreenHowToPlay()
{
	delete m_pLifeMeterBar;
	delete m_pmCharacter;
	delete m_pmDancePad;
	delete m_pPlayer;
}

void ScreenHowToPlay::Step()
{
#define ST_LEFT		0x01
#define ST_DOWN		0x02
#define ST_UP		0x04
#define ST_RIGHT	0x08
#define ST_JUMPLR	(ST_LEFT | ST_RIGHT)
#define ST_JUMPUD	(ST_UP | ST_DOWN)

	int iStep = 0;
	const int iNoteRow = BeatToNoteRowNotRounded( GAMESTATE->m_fSongBeat + 0.6f );
	// if we want to miss from here on out, don't process steps.
	if( m_iW2s < m_iNumW2s && m_NoteData.IsThereATapAtRow( iNoteRow ) )
	{
		const int iNumTracks = m_NoteData.GetNumTracks();
		for( int k=0; k<iNumTracks; k++ )
			if( m_NoteData.GetTapNote(k, iNoteRow).type == TapNote::tap )
				iStep |= 1 << k;

		switch( iStep )
		{
		case ST_LEFT:	m_pmCharacter->PlayAnimation( "Step-LEFT", 1.8f ); break;
		case ST_RIGHT:	m_pmCharacter->PlayAnimation( "Step-RIGHT", 1.8f ); break;
		case ST_UP:		m_pmCharacter->PlayAnimation( "Step-UP", 1.8f ); break;
		case ST_DOWN:	m_pmCharacter->PlayAnimation( "Step-DOWN", 1.8f ); break;
		case ST_JUMPLR: m_pmCharacter->PlayAnimation( "Step-JUMPLR", 1.8f ); break;
		case ST_JUMPUD:
			// Until I can get an UP+DOWN jump animation, this will have to do.
			m_pmCharacter->PlayAnimation( "Step-JUMPLR", 1.8f );
			
			m_pmCharacter->StopTweening();
			m_pmCharacter->BeginTweening( GAMESTATE->m_fCurBPS /8, TWEEN_LINEAR );
			m_pmCharacter->SetRotationY( 90 );
			m_pmCharacter->BeginTweening( (1/(GAMESTATE->m_fCurBPS * 2) ) ); //sleep between jump-frames
			m_pmCharacter->BeginTweening( GAMESTATE->m_fCurBPS /6, TWEEN_LINEAR );
			m_pmCharacter->SetRotationY( 0 );
			break;
		}
	}
}

void ScreenHowToPlay::Update( float fDelta )
{
	if(GAMESTATE->m_pCurSong != NULL)
	{
		GAMESTATE->UpdateSongPosition( m_fFakeSecondsIntoSong, GAMESTATE->m_pCurSong->m_Timing );
		m_fFakeSecondsIntoSong += fDelta;

		static int iLastNoteRowCounted = 0;
		int iCurNoteRow = BeatToNoteRowNotRounded( GAMESTATE->m_fSongBeat );

		if(( iCurNoteRow != iLastNoteRowCounted ) &&(m_NoteData.IsThereATapAtRow( iCurNoteRow )))
		{
			if( m_pLifeMeterBar && !m_pPlayer )
			{
				if ( m_iW2s < m_iNumW2s )
					m_pLifeMeterBar->ChangeLife(TNS_W2);
				else
					m_pLifeMeterBar->ChangeLife(TNS_Miss);
			}
			m_iW2s++;
			iLastNoteRowCounted = iCurNoteRow;
		}

		// once we hit the number of perfects we want, we want to fail.
		// switch the controller to HUMAN. since we aren't taking input,
		// the steps will always be misses.
		if(m_iW2s > m_iNumW2s)
			GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerController = PC_HUMAN;

		if ( m_pmCharacter )
		{
			Step();
			if( !GAMESTATE->m_bFreeze )
				m_pmCharacter->Update( fDelta );
		}
	}

	if( m_pmDancePad )
		m_pmDancePad->Update( fDelta );

	ScreenAttract::Update( fDelta );
}

void ScreenHowToPlay::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GainFocus:
		/* We do this ourself. */
		SOUND->HandleSongTimer( false );
		break;
	case SM_LoseFocus:
		SOUND->HandleSongTimer( true );
		break;
	case SM_GoToNextScreen:
		GAMESTATE->Reset();
		break;
	}
	ScreenAttract::HandleScreenMessage( SM );
}

void ScreenHowToPlay::DrawPrimitives()
{
	Screen::DrawPrimitives();

	if( m_pmDancePad || m_pmCharacter )
	{
		if(PREFSMAN->m_bCelShadeModels)
		{
			if( m_pmDancePad ) m_pmDancePad->DrawCelShaded();
			if( m_pmCharacter ) m_pmCharacter->DrawCelShaded();
		}
		else
		{
			DISPLAY->SetLighting( true );
			DISPLAY->SetLightDirectional( 
				0, 
				RageColor(0.5,0.5,0.5,1), 
				RageColor(1,1,1,1),
				RageColor(0,0,0,1),
				RageVector3(0, 0, 1) );
			
			if( m_pmCharacter ) m_pmCharacter->Draw();
			if( m_pmDancePad ) m_pmDancePad->Draw();

			DISPLAY->SetLightOff( 0 );
			DISPLAY->SetLighting( false );
		}
		
		m_In.DrawPrimitives();
		m_Out.DrawPrimitives();
	}
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
