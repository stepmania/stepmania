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

static CString GetAnimPath(Animation a) {return CString("Characters/") + anims[a];}


BeginnerHelper::BeginnerHelper()
{
	m_bShowBackground = true;
	m_bInitialized = false;
	m_iLastRowChecked = m_iLastRowFlashed = 0;
	for(int pn=0; pn<NUM_PLAYERS; pn++)
		m_bPlayerEnabled[pn] = false;
}

BeginnerHelper::~BeginnerHelper() {}

void BeginnerHelper::ShowStepCircle( int pn, int CSTEP )
{
	int	isc=0;	// Save OR issues within array boundries.. it's worth the extra few bytes of memory.
	switch(CSTEP) {
		case ST_LEFT:	isc=0;	break;
		case ST_RIGHT:	isc=1; 	break;
		case ST_UP:	isc=2;	break;
		case ST_DOWN:	isc=3;	break;
	}
	
	m_sStepCircle[pn][isc].SetEffectNone();
	m_sStepCircle[pn][isc].SetZoom(2);
	m_sStepCircle[pn][isc].StopTweening();
	m_sStepCircle[pn][isc].BeginTweening((GAMESTATE->m_fCurBPS/4), TWEEN_LINEAR);
	m_sStepCircle[pn][isc].SetZoom(0);
}

void BeginnerHelper::AddPlayer( int pn, NoteData *pSteps )
{
	ASSERT(!m_bInitialized);
	ASSERT(pSteps != NULL);
	ASSERT((pn >= 0) && (pn < NUM_PLAYERS));
	ASSERT(GAMESTATE->IsHumanPlayer(pn));

	if(!CanUse()) {return;}
	
	const Character *Character = GAMESTATE->m_pCurCharacters[pn];
	ASSERT( Character != NULL );
	if(!DoesFileExist(Character->GetModelPath())) {return;}

	m_NoteData[pn].CopyAll(pSteps);
	m_bPlayerEnabled[pn] = true;
}

bool BeginnerHelper::CanUse()
{
	for(int i=0; i<NUM_ANIMATIONS; ++i)
		if(!DoesFileExist(GetAnimPath((Animation)i))) {return false;}

	if(GAMESTATE->m_CurGame != GAME_DANCE) {return false;}

	switch( GAMESTATE->m_CurStyle ) {
		case STYLE_DANCE_SOLO:
		case STYLE_DANCE_DOUBLE: return false;
	}

	return true;
}

bool BeginnerHelper::Initialize( int iDancePadType )
{
	ASSERT( !m_bInitialized );
	if(!CanUse()) {return false;}

	// If no players were successfully added, bail.
	{
		bool bAnyLoaded = false;
		for(int pn=0; pn<NUM_PLAYERS; pn++)
			if(m_bPlayerEnabled[pn]) {bAnyLoaded = true;}
		if(!bAnyLoaded)	{return false;}
	}
	
	// Load the Background and flash.. Flash only shows if the BG does.
	if(m_bShowBackground) {
		m_sBackground.Load( THEME->GetPathToG("BeginnerHelper background") );
		this->AddChild(&m_sBackground);
		m_sBackground.SetXY(CENTER_X, CENTER_Y);
		m_sFlash.Load(THEME->GetPathToG("BeginnerHelper flash"));
		m_sFlash.SetXY(CENTER_X, CENTER_Y);
		m_sFlash.SetDiffuseAlpha(0);
	}
	
	// Load StepCircle graphics
	for(int lsc=0; lsc<NUM_PLAYERS; lsc++)
		for(int lsce=0; lsce<4; lsce++) {
			m_sStepCircle[lsc][lsce].Load(THEME->GetPathToG("BeginnerHelper stepcircle"));
			m_sStepCircle[lsc][lsce].SetZoom(0);	// Hide until needed.
			this->AddChild(&m_sStepCircle[lsc][lsce]);
			
			// Set coordinates of StepCircle
			switch(lsce) {
				case 0: m_sStepCircle[lsc][lsce].SetXY((HELPER_X+PLAYER_X(lsc)-80),HELPER_Y);	break;	// Left
				case 1: m_sStepCircle[lsc][lsce].SetXY((HELPER_X+PLAYER_X(lsc)+80),HELPER_Y);	break;	// Right
				case 2: m_sStepCircle[lsc][lsce].SetXY((HELPER_X+PLAYER_X(lsc)),(HELPER_Y-60));	break;	// Up
				case 3: m_sStepCircle[lsc][lsce].SetXY((HELPER_X+PLAYER_X(lsc)),(HELPER_Y+60));	break;	// Down
			}
		}

	// Load the DancePad
	switch(iDancePadType)
	{
		case 0: break; // No pad
		case 1: m_mDancePad.LoadMilkshapeAscii(GetAnimPath(ANIM_DANCE_PAD)); break;
		case 2: m_mDancePad.LoadMilkshapeAscii(GetAnimPath(ANIM_DANCE_PADS)); break;
	}
	
	m_mDancePad.SetHorizAlign( align_left );
	m_mDancePad.SetRotationX( DANCEPAD_ANGLE );
	m_mDancePad.SetX(HELPER_X);
	m_mDancePad.SetY(HELPER_Y);
	m_mDancePad.SetZoom(23);	// Pad should always be 3 units bigger in zoom than the dancer.

	for( int pl=0; pl<NUM_PLAYERS; pl++ )	// Load players
	{
		// Skip if not enabled
		if(!m_bPlayerEnabled[pl]) {continue;}

		// Load character data
		const Character *Character = GAMESTATE->m_pCurCharacters[pl];
		ASSERT( Character != NULL );

		// Load textures
		m_mDancer[pl].SetHorizAlign( align_left );
		m_mDancer[pl].LoadMilkshapeAscii( Character->GetModelPath() );

		// Load needed animations
		m_mDancer[pl].LoadMilkshapeAsciiBones("Step-LEFT", 	GetAnimPath(ANIM_LEFT));
		m_mDancer[pl].LoadMilkshapeAsciiBones("Step-DOWN", 	GetAnimPath(ANIM_DOWN));
		m_mDancer[pl].LoadMilkshapeAsciiBones("Step-UP", 	GetAnimPath(ANIM_UP));
		m_mDancer[pl].LoadMilkshapeAsciiBones("Step-RIGHT", 	GetAnimPath(ANIM_RIGHT));
		m_mDancer[pl].LoadMilkshapeAsciiBones("Step-JUMPLR", 	GetAnimPath(ANIM_JUMPLR));
		m_mDancer[pl].LoadMilkshapeAsciiBones("rest",		Character->GetRestAnimationPath());
		m_mDancer[pl].SetDefaultAnimation("rest");
		m_mDancer[pl].PlayAnimation("rest");
		m_mDancer[pl].SetRotationX(PLAYER_ANGLE);
		m_mDancer[pl].SetX(HELPER_X+PLAYER_X(pl));
		m_mDancer[pl].SetY(HELPER_Y+10);
		m_mDancer[pl].SetZoom(20);
		m_mDancer[pl].SetCullMode(CULL_NONE);	// many of the DDR PC character models have the vertex order flipped
		m_mDancer[pl].m_bRevertToDefaultAnimation = true;		// Stay bouncing after a step has finished animating.
	}

	m_bInitialized = true;
	return true;
}

void BeginnerHelper::DrawPrimitives()
{
	// If not initialized, don't bother with this
	if(!m_bInitialized) {return;}

	ActorFrame::DrawPrimitives();
	m_sFlash.Draw();

	bool DrawCelShaded = PREFSMAN->m_bCelShadeModels;
	if(DrawCelShaded) {m_mDancePad.DrawCelShaded();}
	else {
		DISPLAY->SetLighting( true );
		DISPLAY->SetLightDirectional( 
			0, 
			RageColor(0.5,0.5,0.5,1), 
			RageColor(1,1,1,1),
			RageColor(0,0,0,1),
			RageVector3(0, 0, 1) );

		m_mDancePad.Draw();
		DISPLAY->ClearZBuffer();	// So character doesn't step "into" the dance pad.
	}
	
	// Draw StepCircles
	DISPLAY->SetLighting(false);
	for(int scd=0; scd<NUM_PLAYERS; scd++)
		for(int scde=0; scde<4; scde++)
			m_sStepCircle[scd][scde].Draw();
	DISPLAY->SetLighting(true);
	
	if(DrawCelShaded) {
		FOREACH_PlayerNumber(pn)	// Draw each dancer
			if(GAMESTATE->IsHumanPlayer(pn)) {m_mDancer[pn].DrawCelShaded();}
	}
	else {
		FOREACH_PlayerNumber(pn)	// Draw each dancer
			if(GAMESTATE->IsHumanPlayer(pn)) {m_mDancer[pn].Draw();}

		DISPLAY->SetLightOff(0);
		DISPLAY->SetLighting(false);
	}
}

void BeginnerHelper::Step( int pn, int CSTEP )
{
	m_mDancer[pn].StopTweening();
	m_mDancer[pn].SetRotationY(0);	// Make sure we're not still inside of a JUMPUD tween.

	switch(CSTEP) {
		case ST_LEFT:
			ShowStepCircle(pn, ST_LEFT);
			m_mDancer[pn].PlayAnimation("Step-LEFT", 1.5f);
			break;
		case ST_RIGHT:
			ShowStepCircle(pn, ST_RIGHT);
			m_mDancer[pn].PlayAnimation("Step-RIGHT", 1.5f);
			break;
		case ST_UP:
			ShowStepCircle(pn, ST_UP);
			m_mDancer[pn].PlayAnimation("Step-UP", 1.5f);
			break;
		case ST_DOWN:
			ShowStepCircle(pn, ST_DOWN);
			m_mDancer[pn].PlayAnimation("Step-DOWN", 1.5f);
			break;
		case ST_JUMPLR:
			ShowStepCircle(pn, ST_LEFT);
			ShowStepCircle(pn, ST_RIGHT);
			m_mDancer[pn].PlayAnimation("Step-JUMPLR", 1.5f);
			break;
		case ST_JUMPUD:
			ShowStepCircle(pn, ST_UP);
			ShowStepCircle(pn, ST_DOWN);
			m_mDancer[pn].StopTweening();
			m_mDancer[pn].PlayAnimation("Step-JUMPLR", 1.5f);
			m_mDancer[pn].BeginTweening((GAMESTATE->m_fCurBPS /8), TWEEN_LINEAR);
			m_mDancer[pn].SetRotationY(90);
			m_mDancer[pn].BeginTweening((1/(GAMESTATE->m_fCurBPS * 2))); //sleep between jump-frames
			m_mDancer[pn].BeginTweening(GAMESTATE->m_fCurBPS /6, TWEEN_LINEAR);
			m_mDancer[pn].SetRotationY(0);
			break;
	}
	
	m_sFlash.SetEffectNone();
	m_sFlash.StopTweening();
	m_sFlash.Sleep(GAMESTATE->m_fCurBPS /16);
	m_sFlash.SetDiffuseAlpha(1);
	m_sFlash.BeginTweening(1/GAMESTATE->m_fCurBPS * 0.5f);
	m_sFlash.SetDiffuseAlpha(0);
}

void BeginnerHelper::Update( float fDeltaTime )
{
	if(!m_bInitialized) {return;}

	// the row we want to check on this update
	int iCurRow = BeatToNoteRowNotRounded(GAMESTATE->m_fSongBeat + 0.4f);
	int iActualRow = BeatToNoteRowNotRounded(GAMESTATE->m_fSongBeat);
	for(int pn=0; pn<NUM_PLAYERS; pn++)
	{
		// Skip if not enabled
		if(!m_bPlayerEnabled[pn]) {continue;}
		
		for(int iRow=m_iLastRowChecked; iRow<iCurRow; iRow++)
		{
			// Check if there are any notes at all on this row.. If not, save scanning.
			if(!m_NoteData[pn].IsThereATapAtRow(iRow)) {continue;}
			
			// Find all steps on this row, in order to show the correct animations
			int iStep = 0;
			const int iNumTracks = m_NoteData[pn].GetNumTracks(); 
			for(int k=0; k<iNumTracks; k++)
				if(m_NoteData[pn].GetTapNote(k, iRow) == TAP_TAP) {iStep |= 1 << k;}
			
			// Assign new data
			this->Step(pn, iStep);
		}
	}
	
	// Make sure we don't accidentally scan a row 2x
	m_iLastRowChecked = iCurRow;
	
	// Update animations
	ActorFrame::Update(fDeltaTime);
	m_mDancePad.Update(fDeltaTime);
	m_sFlash.Update(fDeltaTime);

	float beat = (fDeltaTime*GAMESTATE->m_fCurBPS);
	for(int pu=0; pu<NUM_PLAYERS; pu++)
	{
		// If this is not a human player, the dancer is not shown
		if(!GAMESTATE->IsHumanPlayer(pu)) {continue;}
		
		// Update dancer's animation and StepCircles
		m_mDancer[pu].Update( beat );
		for(int scu=0; scu<NUM_PLAYERS; scu++)
			for(int scue=0; scue<4; scue++)
				m_sStepCircle[scu][scue].Update(beat);
	}
}

/*
 * (c) 2003 Kevin Slaughter
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
