#include "global.h"

#include "ActorUtil.h"
#include "BeginnerHelper.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "RageDisplay.h"
#include "ThemeManager.h"
#include "Steps.h"

// "PLAYER_X" offsets are relative to the pad.. ex: Setting this to 10, and the HELPER to 300, will put the dancer at 310
#define PLAYER_X( px )		THEME->GetMetricF("BeginnerHelper",ssprintf("Player%d_X",px+1))
#define PLAYER_ANGLE		THEME->GetMetricF("BeginnerHelper","PlayerAngle")
#define DANCEPAD_ANGLE		THEME->GetMetricF("BeginnerHelper","DancePadAngle")

// STEPCIRCLE commands are relative to the HELPER position
#define STEPCIRCLE_LEFT_COMMAND( pz )	THEME->GetMetric ("BeginnerHelper",ssprintf("StepCircleP%d_LEFT_COMMAND",pz+1))
#define STEPCIRCLE_DOWN_COMMAND( pz )	THEME->GetMetric ("BeginnerHelper",ssprintf("StepCircleP%d_DOWN_COMMAND",pz+1))
#define STEPCIRCLE_UP_COMMAND( pz )	THEME->GetMetric ("BeginnerHelper",ssprintf("StepCircleP%d_UP_COMMAND",pz+1))
#define STEPCIRCLE_RIGHT_COMMAND( pz )	THEME->GetMetric ("BeginnerHelper",ssprintf("StepCircleP%d_RIGHT_COMMAND",pz+1))

// "HELPER" offsets effect the pad/dancer as a whole.. Their relative Y cooridinates are hard-coded for eachother.
#define HELPER_X			THEME->GetMetricF("BeginnerHelper","HelperX")
#define HELPER_Y			THEME->GetMetricF("BeginnerHelper","HelperY")

#define ST_LEFT		0x01
#define ST_DOWN		0x02
#define ST_UP		0x04
#define ST_RIGHT	0x08
#define ST_JUMPLR	(ST_LEFT | ST_RIGHT)
#define ST_JUMPUD	(ST_UP | ST_DOWN)

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

static const char *anims[NUM_ANIMATIONS] =
{
	"DancePad-DDR.txt",
	"DancePads-DDR.txt",
	"BeginnerHelper_step-up.bones.txt",
	"BeginnerHelper_step-down.bones.txt",
	"BeginnerHelper_step-left.bones.txt",
	"BeginnerHelper_step-right.bones.txt",
	"BeginnerHelper_step-jumplr.bones.txt"
};


static CString GetAnimPath( Animation a )
{
	return ssprintf("Characters%s%s", SLASH, anims[a]);
}


BeginnerHelper::BeginnerHelper()
{
	m_bFlashEnabled = false;
	m_bShowBackground = true;
	m_bInitialized = false;
	m_iLastRowChecked = 0;
	this->AddChild(&m_sBackground);
	for( int pn=0; pn < NUM_PLAYERS; pn++ )
		m_bPlayerEnabled[pn] = false;
}

BeginnerHelper::~BeginnerHelper()
{
	LOG->Trace("BeginnerHelper::~BeginnerHelper()");
}

void BeginnerHelper::ShowStepCircle( int pn, int CSTEP )
{
	/*for( int ps=0; ps<NUM_PLAYERS; ps++ )
	{
		m_sStepCircle[ps].SetXY( (HELPER_X + PLAYER_X(ps)), (HELPER_Y + 10) );
		switch( CSTEP )
		{
			case ST_LEFT: m_sStepCircle[ps].SetRotationZ(270); break;
			case ST_DOWN: m_sStepCircle[ps].SetRotationZ(180); break;
			case ST_UP: break;
			case ST_RIGHT: m_sStepCircle[ps].SetRotationZ(90); break;
			default: break;
		}
		
		m_sStepCircle[ps].SetZoom(1);
		m_sStepCircle[ps].SetEffectNone();
		m_sStepCircle[ps].StopTweening();
		m_sStepCircle[ps].BeginTweening((GAMESTATE->m_fCurBPS/4), TWEEN_LINEAR);
		m_sStepCircle[ps].SetZoom(0);
	}*/
}

void BeginnerHelper::SetFlash( CString sFilename, float fX, float fY )
{
	m_sFlash.Load( sFilename );
	m_sFlash.SetX(fX);
	m_sFlash.SetY(fY);
}

void BeginnerHelper::AddPlayer( int pn, NoteData *pNotes )
{
	ASSERT( !m_bInitialized );
	ASSERT( pNotes != NULL );
	ASSERT( pn >= 0 && pn < NUM_PLAYERS );
	ASSERT( GAMESTATE->IsHumanPlayer(pn) );

	if( !CanUse() )
		return;
	const Character *Character = GAMESTATE->m_pCurCharacters[pn];
	ASSERT( Character != NULL );
	if( !DoesFileExist( Character->GetModelPath() ) )
		return;

	m_NoteData[pn].CopyAll( pNotes );
	m_bPlayerEnabled[pn] = true;
}

bool BeginnerHelper::CanUse()
{
	for( int i = 0; i < NUM_ANIMATIONS; ++i )
		if( !DoesFileExist( GetAnimPath( (Animation) i ) ) )
			return false;

	if( GAMESTATE->m_CurGame != GAME_DANCE )
		return false;

	switch( GAMESTATE->m_CurStyle )
	{
	case STYLE_DANCE_SOLO:
	case STYLE_DANCE_DOUBLE: return false;
	}
	return true;
}

bool BeginnerHelper::Initialize( int iDancePadType )
{
	ASSERT( !m_bInitialized );

	/* If no players were successfully added, bail. */
	{
		bool bAnyLoaded = false;
		for( int pn=0; pn < NUM_PLAYERS; pn++ )
			if( m_bPlayerEnabled[pn] )
				bAnyLoaded = true;
		if( !bAnyLoaded )
			return false;
	}

	if( !CanUse() )		// if we can't be used, bail now.
		return false;

	// Load the StepCircle, Background, and flash animation
	if( m_bShowBackground )
	{
		m_sBackground.Load( THEME->GetPathToG("BeginnerHelper background") );
		m_sBackground.SetXY( CENTER_X, CENTER_Y);
	}
	
	m_sFlash.Load( THEME->GetPathToG("BeginnerHelper flash") );
	m_sFlash.SetXY( CENTER_X, CENTER_Y );
	m_sFlash.SetDiffuseAlpha(0);

	/*for( int sc=0; sc<NUM_PLAYERS*2; sc++ )
	{
		m_sStepCircle[sc].Load( THEME->GetPathToG("BeginnerHelper stepcircle") );
		m_sStepCircle[sc].SetZoom(0);	// Hide until needed.
		this->AddChild(&m_sStepCircle[sc]);
	}*/

	// Load the DancePad
	switch(iDancePadType)
	{
		case 0: break; // No pad
		case 1: m_mDancePad.LoadMilkshapeAscii( GetAnimPath(ANIM_DANCE_PAD) ); break;
		case 2: m_mDancePad.LoadMilkshapeAscii( GetAnimPath(ANIM_DANCE_PADS) ); break;
	}
	m_mDancePad.SetHorizAlign( align_left );
	m_mDancePad.SetRotationX( DANCEPAD_ANGLE );
	m_mDancePad.SetX(HELPER_X);
	m_mDancePad.SetY(HELPER_Y);
	m_mDancePad.SetZoom( 23 );	// Pad should always be 3 units bigger in zoom than the dancer.

	for( int pl=0; pl<NUM_PLAYERS; pl++ )	// Load players
	{
		if( !m_bPlayerEnabled[pl] )
			continue;	// skip

		const Character *Character = GAMESTATE->m_pCurCharacters[pl];
		ASSERT( Character != NULL );

		// Load textures
		m_mDancer[pl].SetHorizAlign( align_left );
		m_mDancer[pl].LoadMilkshapeAscii( Character->GetModelPath() );

		// Load needed animations
		m_mDancer[pl].LoadMilkshapeAsciiBones( "Step-LEFT", GetAnimPath( ANIM_LEFT ) );
		m_mDancer[pl].LoadMilkshapeAsciiBones( "Step-DOWN", GetAnimPath( ANIM_DOWN ) );
		m_mDancer[pl].LoadMilkshapeAsciiBones( "Step-UP", GetAnimPath( ANIM_UP ) );
		m_mDancer[pl].LoadMilkshapeAsciiBones( "Step-RIGHT", GetAnimPath( ANIM_RIGHT ) );
		m_mDancer[pl].LoadMilkshapeAsciiBones( "Step-JUMPLR", GetAnimPath( ANIM_JUMPLR ) );
		/*m_mDancer[pl].LoadMilkshapeAsciiBones( "Step-JUMPUD","Characters\\BeginnerHelper_step-jumpud.bones.txt" );*/
		m_mDancer[pl].LoadMilkshapeAsciiBones( "rest",Character->GetRestAnimationPath() );
		m_mDancer[pl].SetDefaultAnimation( "rest" );
		m_mDancer[pl].PlayAnimation( "rest" );
		m_mDancer[pl].SetRotationX( PLAYER_ANGLE );
		m_mDancer[pl].SetX( HELPER_X + PLAYER_X(pl) );
		m_mDancer[pl].SetY( HELPER_Y + 10 );
		m_mDancer[pl].SetZoom( 20 );
		m_mDancer[pl].m_bRevertToDefaultAnimation = true;		// Stay bouncing after a step has finished animating.
	}

	m_bInitialized = true;
	return true;
}

void BeginnerHelper::FlashOnce()
{
	m_sFlash.SetDiffuseAlpha(1);
	m_sFlash.SetEffectNone();
	m_sFlash.StopTweening();
	m_sFlash.BeginTweening( 1/GAMESTATE->m_fCurBPS * 0.5f );
	m_sFlash.SetDiffuseAlpha(0);
}

void BeginnerHelper::DrawPrimitives()
{
	if(!m_bInitialized)
		return;

	ActorFrame::DrawPrimitives();

	m_sFlash.Draw();

	DISPLAY->SetLighting( true );
	DISPLAY->SetLightDirectional( 
		0, 
		RageColor(0.5,0.5,0.5,1), 
		RageColor(1,1,1,1),
		RageColor(0,0,0,1),
		RageVector3(0, 0, 1) );

	m_mDancePad.Draw();
	for( int scd=0; scd<NUM_PLAYERS*2; scd++ )
		m_sStepCircle[scd].Draw();		// Should be drawn before the dancer, but after the pad, so it is drawn over the pad and under the dancer.
	
	for( int pn=0; pn<NUM_PLAYERS; pn++ )	// Draw each dancer
		if( GAMESTATE->IsHumanPlayer(pn) )
			m_mDancer[pn].Draw();

	DISPLAY->SetLightOff( 0 );
	DISPLAY->SetLighting( false );
}

void BeginnerHelper::Step( int pn, int CSTEP )
{
	LOG->Trace( "BeginnerHelper::Step()" );

	ShowStepCircle( pn, CSTEP);
	m_mDancer[pn].StopTweening();
	m_mDancer[pn].SetRotationY(0);	// Make sure we're not still inside of a JUMPUD tween.
	switch( CSTEP )
	{
	case ST_LEFT:	m_mDancer[pn].PlayAnimation( "Step-LEFT", 1.5f ); break;
	case ST_RIGHT:	m_mDancer[pn].PlayAnimation( "Step-RIGHT", 1.5f ); break;
	case ST_UP:		m_mDancer[pn].PlayAnimation( "Step-UP", 1.5f ); break;
	case ST_DOWN:	m_mDancer[pn].PlayAnimation( "Step-DOWN", 1.5f ); break;
	case ST_JUMPLR: m_mDancer[pn].PlayAnimation( "Step-JUMPLR", 1.5f ); break;
	case ST_JUMPUD:
		// Until I can get an UP+DOWN jump animation, this will have to do.
		m_mDancer[pn].PlayAnimation( "Step-JUMPLR", 1.5f );
		
		m_mDancer[pn].StopTweening();
		m_mDancer[pn].BeginTweening( GAMESTATE->m_fCurBPS /8, TWEEN_LINEAR );
		m_mDancer[pn].SetRotationY( 90 );
		m_mDancer[pn].BeginTweening( (1/(GAMESTATE->m_fCurBPS * 2) ) ); //sleep between jump-frames
		m_mDancer[pn].BeginTweening( GAMESTATE->m_fCurBPS /6, TWEEN_LINEAR );
		m_mDancer[pn].SetRotationY( 0 );
		break;
	}
}

void BeginnerHelper::Update( float fDeltaTime )
{
	if(!m_bInitialized)
		return;

	// the row we want to check on this update
	int iCurRow = BeatToNoteRowNotRounded(GAMESTATE->m_fSongBeat + 0.4f);

	for(int pn = 0; pn < NUM_PLAYERS; pn++ )
	{
		if( !m_bPlayerEnabled[pn] )
			continue;	// skip

		if( (m_NoteData[pn].IsThereATapAtRow( BeatToNoteRowNotRounded((GAMESTATE->m_fSongBeat+0.01f)) ) ) )
			FlashOnce();
		for(int iRow=m_iLastRowChecked; iRow<iCurRow; iRow++)
		{
			if( !m_NoteData[pn].IsThereATapAtRow( iRow ) )
				continue;

			int iStep = 0;
			const int iNumTracks = m_NoteData[pn].GetNumTracks(); 
			for( int k=0; k<iNumTracks; k++ )
				if( m_NoteData[pn].GetTapNote(k, iRow ) == TAP_TAP )
					iStep |= 1 << k;
			Step( pn, iStep );
		}
	}
	m_iLastRowChecked = iCurRow;

	ActorFrame::Update( fDeltaTime );
	m_mDancePad.Update( fDeltaTime );
	m_sFlash.Update( fDeltaTime );

	float beat = fDeltaTime * GAMESTATE->m_fCurBPS;

	for( int pu=0; pu<NUM_PLAYERS; pu++ )
	{
		if(!GAMESTATE->IsHumanPlayer(pu))
			continue;

		m_mDancer[pu].Update( beat );	//Update dancers
		for( int scu=0; scu<2; scu++ )
			m_sStepCircle[pu+scu].Update( beat );
	}
}
