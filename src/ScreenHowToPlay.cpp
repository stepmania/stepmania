#include "global.h"
#include "ScreenHowToPlay.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "Steps.h"
#include "GameManager.h"
#include "NotesLoaderSM.h"
#include "NotesLoaderSSC.h"
#include "GameSoundManager.h"
#include "Model.h"
#include "ThemeMetric.h"
#include "PlayerState.h"
#include "Style.h"
#include "PrefsManager.h"
#include "CharacterManager.h"
#include "StatsManager.h"
#include "RageDisplay.h"
#include "SongUtil.h"
#include "Character.h"
#include "LifeMeterBar.h"

#include <vector>


static const ThemeMetric<int>		NUM_W2S		("ScreenHowToPlay","NumW2s");
static const ThemeMetric<int>		NUM_MISSES	("ScreenHowToPlay","NumMisses");
static const ThemeMetric<bool>	USE_CHARACTER	("ScreenHowToPlay","UseCharacter");
static const ThemeMetric<bool>	USE_PAD		("ScreenHowToPlay","UsePad");
static const ThemeMetric<bool>	USE_PLAYER	("ScreenHowToPlay","UsePlayer");
static const ThemeMetric<RString>	CHARACTER_NAME("ScreenHowToPlay","CharacterName");

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

static const RString anims[NUM_ANIMATIONS] =
{
	"DancePad.txt",
	"DancePads.txt",
	"BeginnerHelper_step-up.bones.txt",
	"BeginnerHelper_step-down.bones.txt",
	"BeginnerHelper_step-left.bones.txt",
	"BeginnerHelper_step-right.bones.txt",
	"BeginnerHelper_step-jumplr.bones.txt"
};

static RString GetAnimPath( Animation a )
{
	return RString("Characters/") + anims[a];
}

static bool HaveAllCharAnimations()
{
	for( int i = ANIM_UP; i < NUM_ANIMATIONS; ++i )
		if( !DoesFileExist( GetAnimPath( (Animation) i ) ) )
			return false;
	return true;
}

REGISTER_SCREEN_CLASS( ScreenHowToPlay );
ScreenHowToPlay::ScreenHowToPlay()
{
	m_iW2s = 0;
	m_iNumW2s = NUM_W2S;

	// initialize these because they might not be used.
	m_pLifeMeterBar = nullptr;
	m_pmCharacter = nullptr;
	m_pmDancePad = nullptr;
}

void ScreenHowToPlay::Init()
{
	ScreenAttract::Init();

	if( (bool)USE_PAD && DoesFileExist( GetAnimPath(ANIM_DANCE_PAD) ) )
	{
		m_pmDancePad = new Model;
		m_pmDancePad->SetName( "Pad" );
		m_pmDancePad->LoadMilkshapeAscii( GetAnimPath(ANIM_DANCE_PAD) );
		// xxx: hardcoded rotation. can be undone, but still. -freem
		m_pmDancePad->SetRotationX( 35 );
		ActorUtil::LoadAllCommandsAndSetXY( m_pmDancePad, m_sName );
	}

	// Display a character
	std::vector<Character*> vpCharacters;
	CHARMAN->GetCharacters( vpCharacters );
	if( (bool)USE_CHARACTER && vpCharacters.size() && HaveAllCharAnimations() )
	{
		Character* displayChar;
		if( !CHARACTER_NAME.GetValue().empty() && CHARMAN->GetCharacterFromID(CHARACTER_NAME.GetValue()) )
			displayChar = CHARMAN->GetCharacterFromID(CHARACTER_NAME.GetValue());
		else
			displayChar = CHARMAN->GetRandomCharacter();

		RString sModelPath = displayChar->GetModelPath();
		if( sModelPath != "" )
		{
			m_pmCharacter = new Model;
			m_pmCharacter->SetName( "Character" );
			m_pmCharacter->LoadMilkshapeAscii( displayChar->GetModelPath() );
			m_pmCharacter->LoadMilkshapeAsciiBones( "Step-LEFT", GetAnimPath( ANIM_LEFT ) );
			m_pmCharacter->LoadMilkshapeAsciiBones( "Step-DOWN", GetAnimPath( ANIM_DOWN ) );
			m_pmCharacter->LoadMilkshapeAsciiBones( "Step-UP", GetAnimPath( ANIM_UP ) );
			m_pmCharacter->LoadMilkshapeAsciiBones( "Step-RIGHT", GetAnimPath( ANIM_RIGHT ) );
			m_pmCharacter->LoadMilkshapeAsciiBones( "Step-JUMPLR", GetAnimPath( ANIM_JUMPLR ) );
			RString sRestFile = displayChar->GetRestAnimationPath();
			ASSERT( !sRestFile.empty() );
			m_pmCharacter->LoadMilkshapeAsciiBones( "rest",displayChar->GetRestAnimationPath() );
			m_pmCharacter->SetDefaultAnimation( "rest" );
			m_pmCharacter->PlayAnimation( "rest" ); // Stay bouncing after a step has finished animating.

			// xxx: hardcoded rotation. can be undone, but still. -freem
			m_pmCharacter->SetRotationX( 40 );
			m_pmCharacter->SetCullMode( CULL_NONE ); // many of the models floating around have the vertex order flipped
			m_pmCharacter->RunCommands( displayChar->m_cmdInit ); // run InitCommand from file
			ActorUtil::LoadAllCommandsAndSetXY( m_pmCharacter, m_sName );
		}
	}

	GAMESTATE->SetCurrentStyle( GAMEMAN->GetHowToPlayStyleForGame(GAMESTATE->m_pCurGame), PLAYER_INVALID );

	if( USE_PLAYER )
	{
		GAMESTATE->SetMasterPlayerNumber(PLAYER_1);

		m_pLifeMeterBar = new LifeMeterBar;
		m_pLifeMeterBar->Load( GAMESTATE->m_pPlayerState[PLAYER_1], &STATSMAN->m_CurStageStats.m_player[PLAYER_1] );
		m_pLifeMeterBar->SetName("LifeMeterBar");
		ActorUtil::LoadAllCommandsAndSetXY( m_pLifeMeterBar, m_sName );
		m_pLifeMeterBar->FillForHowToPlay( NUM_W2S, NUM_MISSES );

		// Allow themers to use either a .ssc or .sm file for this. -aj
		RString sStepsPath = THEME->GetPathO(m_sName, "steps");
		SSCLoader loaderSSC;
		SMLoader loaderSM;
		if( sStepsPath.Right(4) == ".ssc" )
			loaderSSC.LoadFromSimfile( sStepsPath, m_Song, false );
		else
			loaderSM.LoadFromSimfile( sStepsPath, m_Song, false );
		m_Song.AddAutoGenNotes();

		const Style* pStyle = GAMESTATE->GetCurrentStyle(PLAYER_INVALID);

		Steps *pSteps = SongUtil::GetClosestNotes( &m_Song, pStyle->m_StepsType, Difficulty_Beginner );
		if(pSteps == nullptr)
		{
			LuaHelpers::ReportScriptErrorFmt("No playable steps of StepsType '%s' for ScreenHowToPlay in file %s", StringConversion::ToString(pStyle->m_StepsType).c_str(), sStepsPath.c_str());
		}
		else
		{
			m_Song.m_SongTiming.TidyUpData( false );
			pSteps->m_Timing.TidyUpData( true );
			NoteData tempNoteData;
			pSteps->GetNoteData( tempNoteData );
			pStyle->GetTransformedNoteDataForStyle( PLAYER_1, tempNoteData, m_NoteData );

			GAMESTATE->m_pCurSong.Set( &m_Song );
			GAMESTATE->m_pCurSteps[PLAYER_1].Set(pSteps);
			GAMESTATE->m_bGameplayLeadIn.Set( false );
			GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerController = PC_AUTOPLAY;

			m_Player->Init("Player", GAMESTATE->m_pPlayerState[PLAYER_1],
				nullptr, m_pLifeMeterBar, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
			m_Player.Load( m_NoteData );
			m_Player->SetName( "Player" );
			this->AddChild( m_Player );
			ActorUtil::LoadAllCommandsAndSetXY( m_Player, m_sName );

			// Don't show judgment
			PO_GROUP_ASSIGN( GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions, ModsLevel_Stage, m_fBlind, 1.0f );
			GAMESTATE->m_bDemonstrationOrJukebox = true;
		}
	}

	// deferred until after the player, so the notes go under it
	if( m_pLifeMeterBar )
		this->AddChild( m_pLifeMeterBar );
	if( m_pmDancePad )
		this->AddChild( m_pmDancePad );
	if( m_pmCharacter )
		this->AddChild( m_pmCharacter );

	m_fFakeSecondsIntoSong = 0;

	this->MoveToTail( &m_In );
	this->MoveToTail( &m_Out );
}

ScreenHowToPlay::~ScreenHowToPlay()
{
	delete m_pLifeMeterBar;
	delete m_pmCharacter;
	delete m_pmDancePad;
}

void ScreenHowToPlay::Step()
{
// xxx: assumes dance. -freem
#define ST_LEFT		0x01
#define ST_DOWN		0x02
#define ST_UP		0x04
#define ST_RIGHT	0x08
#define ST_JUMPLR	(ST_LEFT | ST_RIGHT)
#define ST_JUMPUD	(ST_UP | ST_DOWN)

	int iStep = 0;
	const int iNoteRow = BeatToNoteRowNotRounded( GAMESTATE->m_Position.m_fSongBeat + 0.6f );
	// if we want to miss from here on out, don't process steps.
	if( m_iW2s < m_iNumW2s && m_NoteData.IsThereATapAtRow( iNoteRow ) )
	{
		const int iNumTracks = m_NoteData.GetNumTracks();
		for( int k=0; k<iNumTracks; k++ )
			if( m_NoteData.GetTapNote(k, iNoteRow).type == TapNoteType_Tap )
				iStep |= 1 << k;

		switch( iStep )
		{
		case ST_LEFT:	m_pmCharacter->PlayAnimation( "Step-LEFT", 1.8f ); break;
		case ST_RIGHT:	m_pmCharacter->PlayAnimation( "Step-RIGHT", 1.8f ); break;
		case ST_UP:	m_pmCharacter->PlayAnimation( "Step-UP", 1.8f ); break;
		case ST_DOWN:	m_pmCharacter->PlayAnimation( "Step-DOWN", 1.8f ); break;
		case ST_JUMPLR: m_pmCharacter->PlayAnimation( "Step-JUMPLR", 1.8f ); break;
		case ST_JUMPUD:
			// Until I can get an UP+DOWN jump animation, this will have to do.
			m_pmCharacter->PlayAnimation( "Step-JUMPLR", 1.8f );

			m_pmCharacter->StopTweening();
			m_pmCharacter->BeginTweening( GAMESTATE->m_Position.m_fCurBPS /8, TWEEN_LINEAR );
			m_pmCharacter->SetRotationY( 90 );
			m_pmCharacter->BeginTweening( (1/(GAMESTATE->m_Position.m_fCurBPS * 2) ) ); //sleep between jump-frames
			m_pmCharacter->BeginTweening( GAMESTATE->m_Position.m_fCurBPS /6, TWEEN_LINEAR );
			m_pmCharacter->SetRotationY( 0 );
			break;
		}
	}
}

void ScreenHowToPlay::Update( float fDelta )
{
	if( GAMESTATE->m_pCurSong != nullptr )
	{
		RageTimer tm;
		GAMESTATE->UpdateSongPosition( m_fFakeSecondsIntoSong, GAMESTATE->m_pCurSong->m_SongTiming, tm );
		m_fFakeSecondsIntoSong += fDelta;

		static int iLastNoteRowCounted = 0;
		int iCurNoteRow = BeatToNoteRowNotRounded( GAMESTATE->m_Position.m_fSongBeat );

		if( iCurNoteRow != iLastNoteRowCounted &&m_NoteData.IsThereATapAtRow(iCurNoteRow) )
		{
			if( m_pLifeMeterBar && !m_Player )
			{
				if ( m_iW2s < m_iNumW2s )
					m_pLifeMeterBar->ChangeLife( TNS_W2 );
				else
					m_pLifeMeterBar->ChangeLife( TNS_Miss );
			}
			m_iW2s++;
			iLastNoteRowCounted = iCurNoteRow;
		}

		// Once we hit the number of perfects we want, we want to fail. Switch
		// the controller to HUMAN. Since we aren't taking input, the steps will
		// always be misses.
		if( m_iW2s > m_iNumW2s )
			GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerController = PC_HUMAN;

		// Per the above code, we don't always want the character stepping.
		// If they try to make all of the steps in the miss part, they look
		// silly. Have then stand still instead. - freem
		if ( m_pmCharacter )
		{
			if( m_iW2s <= m_iNumW2s )
				Step();
		}
	}

	ScreenAttract::Update( fDelta );
}

void ScreenHowToPlay::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_GainFocus )
	{
		// We do this ourself.
		SOUND->HandleSongTimer( false );
	}
	else if( SM == SM_LoseFocus )
	{
		SOUND->HandleSongTimer( true );
	}
	else if( SM == SM_GoToNextScreen )
	{
		GAMESTATE->Reset();
	}
	ScreenAttract::HandleScreenMessage( SM );
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the ScreenHowToPlay. */
class LunaScreenHowToPlay: public Luna<ScreenHowToPlay>
{
public:
	static int GetLifeMeter( T* p, lua_State *L )
	{
		//PlayerNumber pn = Enum::Check<PlayerNumber>( L, 1 );

		p->m_pLifeMeterBar->PushSelf( L );
		return 1;
	}

	LunaScreenHowToPlay()
	{
  		ADD_METHOD( GetLifeMeter );
	}
};

LUA_REGISTER_DERIVED_CLASS( ScreenHowToPlay, ScreenAttract )
// lua end

/*
 * (c) 2001-2004 Chris Danford, Tracy Ward
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
