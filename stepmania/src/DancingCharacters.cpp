#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: DancingCharacters

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "DancingCharacters.h"
#include "GameConstantsAndTypes.h"
#include "RageDisplay.h"
#include "RageUtil.h"
#include "RageMath.h"
#include "GameState.h"
#include "song.h"
#include "Character.h"


const float CAMERA_REST_DISTANCE = 32.f;
const float CAMERA_REST_LOOK_AT_HEIGHT = -9.f;

const float CAMERA_SWEEP_DISTANCE = 20.f;
const float CAMERA_SWEEP_DISTANCE_VARIANCE = 8.f;
const float CAMERA_SWEEP_HEIGHT_VARIANCE = 28.f;
const float CAMERA_SWEEP_PAN_Y_RANGE_DEGREES = 45.f;
const float CAMERA_SWEEP_PAN_Y_VARIANCE_DEGREES = 180.f;
const float CAMERA_SWEEP_LOOK_AT_HEIGHT = -9.f;

const float CAMERA_STILL_DISTANCE = 13.f;
const float CAMERA_STILL_DISTANCE_VARIANCE = 7.f;
const float CAMERA_STILL_PAN_Y_RANGE_DEGREES = 45.f;
const float CAMERA_STILL_HEIGHT_VARIANCE = 6.f;
const float CAMERA_STILL_LOOK_AT_HEIGHT = -14.f;

const float MODEL_X_ONE_PLAYER = 0;
const float MODEL_X_TWO_PLAYERS[NUM_PLAYERS] = { +8, -8 };


DancingCharacters::DancingCharacters()
{
	m_bDrawDangerLight = false;
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;

		Character* pChar = GAMESTATE->m_pCurCharacters[p];
		if( !pChar )
			continue;
		
		if( GAMESTATE->GetNumPlayersEnabled()==2 )
			m_Character[p].SetX( MODEL_X_TWO_PLAYERS[p] );
		else
			m_Character[p].SetX( MODEL_X_ONE_PLAYER );
		m_Character[p].LoadMilkshapeAscii( pChar->GetModelPath() );
		m_Character[p].LoadMilkshapeAsciiBones( "rest", pChar->GetRestAnimationPath() );
		m_Character[p].LoadMilkshapeAsciiBones( "warmup", pChar->GetWarmUpAnimationPath() );
		m_Character[p].LoadMilkshapeAsciiBones( "dance", pChar->GetDanceAnimationPath() );
		m_Character[p].LoadMilkshapeAsciiBones( "howtoplay", pChar->GetHowToPlayAnimationPath() );
		this->AddChild( &m_Character[p] );
	}
}

void DancingCharacters::LoadNextSong()
{
	// initial camera sweep is still
	m_CameraDistance = CAMERA_REST_DISTANCE;
	m_CameraPanYStart = 0;
	m_CameraPanYEnd = 0;
	m_fCameraHeightStart = CAMERA_REST_LOOK_AT_HEIGHT;
	m_fCameraHeightEnd = CAMERA_REST_LOOK_AT_HEIGHT;
	m_fLookAtHeight = CAMERA_REST_LOOK_AT_HEIGHT;
	m_fThisCameraStartBeat = 0;
	m_fThisCameraEndBeat = 0;
	/* XXX: This is being initialized before m_pCurSong is set by ScreenGameplay
	 * in course mode.  Init in first update? */
	if( GAMESTATE->m_pCurSong )
		m_fThisCameraEndBeat = GAMESTATE->m_pCurSong->m_fFirstBeat;
	
	for( int p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsPlayerEnabled(p) )
			m_Character[p].PlayAnimation( "rest" );
}


int Neg1OrPos1() { return rand()%2 ? -1 : +1; }

void DancingCharacters::Update( float fDelta )
{
	if( GAMESTATE->m_bFreeze )
	{
		// spin the camera Matrix style
		m_CameraPanYStart += fDelta*40;
		m_CameraPanYEnd += fDelta*40;
	}
	else
	{
		// make the characters move
		float fBPM = GAMESTATE->m_fCurBPS*60;
		float fUpdateScale = SCALE( fBPM, 60.f, 300.f, 0.75f, 1.5f );
		CLAMP( fUpdateScale, 0.75f, 1.5f );

		/* It's OK for the animation to go slower than natural when we're
		 * at a very low music rate. */
		fUpdateScale *= GAMESTATE->m_SongOptions.m_fMusicRate;

		ActorFrame::Update( fDelta*fUpdateScale );
	}

	static bool bWasHereWeGo = false;
	bool bIsHereWeGo = GAMESTATE->m_bPastHereWeGo;
	if( !bWasHereWeGo && bIsHereWeGo )
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
			m_Character[p].PlayAnimation( "warmup" );
	}
	bWasHereWeGo = bIsHereWeGo;

	static float fLastBeat = GAMESTATE->m_fSongBeat;
	float fThisBeat = GAMESTATE->m_fSongBeat;
	if( fLastBeat < GAMESTATE->m_pCurSong->m_fFirstBeat &&
		fThisBeat >= GAMESTATE->m_pCurSong->m_fFirstBeat )
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
			m_Character[p].PlayAnimation( "dance" );
	}
	fLastBeat = fThisBeat;


	// time for a new sweep?
	if( GAMESTATE->m_fSongBeat > m_fThisCameraEndBeat )
	{
		if( (rand()%5) >= 2 )
		{
			// sweeping camera
			m_CameraDistance = CAMERA_SWEEP_DISTANCE + RandomInt(-1,1) * CAMERA_SWEEP_DISTANCE_VARIANCE;
			m_CameraPanYStart = m_CameraPanYEnd = RandomInt(-1,1) * CAMERA_SWEEP_PAN_Y_RANGE_DEGREES;
			m_fCameraHeightStart = m_fCameraHeightEnd = CAMERA_STILL_LOOK_AT_HEIGHT;
			
			m_CameraPanYEnd += RandomInt(-1,1) * CAMERA_SWEEP_PAN_Y_VARIANCE_DEGREES;
			m_fCameraHeightStart = m_fCameraHeightEnd = m_fCameraHeightStart + RandomInt(-1,1) * CAMERA_SWEEP_HEIGHT_VARIANCE;

			float fCameraHeightVariance = RandomInt(-1,1) * CAMERA_SWEEP_HEIGHT_VARIANCE;
			m_fCameraHeightStart -= fCameraHeightVariance;
			m_fCameraHeightEnd += fCameraHeightVariance;

			m_fLookAtHeight = CAMERA_SWEEP_LOOK_AT_HEIGHT;
		}
		else
		{
			// still camera
			m_CameraDistance = CAMERA_STILL_DISTANCE + RandomInt(-1,1) * CAMERA_STILL_DISTANCE_VARIANCE;
			m_CameraPanYStart = m_CameraPanYEnd = Neg1OrPos1() * CAMERA_STILL_PAN_Y_RANGE_DEGREES;
			m_fCameraHeightStart = m_fCameraHeightEnd = CAMERA_SWEEP_LOOK_AT_HEIGHT + Neg1OrPos1() * CAMERA_STILL_HEIGHT_VARIANCE;

			m_fLookAtHeight = CAMERA_STILL_LOOK_AT_HEIGHT;
		}

		int iCurBeat = (int)GAMESTATE->m_fSongBeat;
		iCurBeat -= iCurBeat%8;

		m_fThisCameraStartBeat = (float) iCurBeat;
		m_fThisCameraEndBeat = float(iCurBeat + 8);
	}
}

void DancingCharacters::DrawPrimitives()
{
	DISPLAY->EnterPerspective( 45, false );

	float fPercentIntoSweep;
	if(m_fThisCameraStartBeat == m_fThisCameraEndBeat)
		fPercentIntoSweep = 0;
	else 
		fPercentIntoSweep = SCALE(GAMESTATE->m_fSongBeat, m_fThisCameraStartBeat, m_fThisCameraEndBeat, 0.f, 1.f );
	float fCameraPanY = SCALE( fPercentIntoSweep, 0.f, 1.f, m_CameraPanYStart, m_CameraPanYEnd );
	float fCameraHeight = SCALE( fPercentIntoSweep, 0.f, 1.f, m_fCameraHeightStart, m_fCameraHeightEnd );

	RageVector3 m_CameraPoint( 0, fCameraHeight, -m_CameraDistance );
	const RageMatrix CameraRot = RageMatrixRotationY(fCameraPanY);
	RageVec3TransformCoord( &m_CameraPoint, &m_CameraPoint, &CameraRot );

	RageVector3 m_LookAt( 0, m_fLookAtHeight, 0 );

	DISPLAY->LookAt( 
		m_CameraPoint,
		m_LookAt,
		RageVector3(0,1,0) );

	RageColor	DL;
	DL = RageColor(0.8f,0.8f,0.8f,0.8f);
	if( m_bDrawDangerLight )
		DL = RageColor(1,0,0,1);

	DISPLAY->SetLighting( true );
	DISPLAY->SetLightDirectional( 
		0, 
		RageColor(0.4f,0.4f,0.4f,1), 
		DL,
		RageColor(0.8f,0.8f,0.8f,0.8f),
		RageVector3(+1, 0, +1) );

	ActorFrame::DrawPrimitives();

	DISPLAY->SetLightOff( 0 );
	DISPLAY->SetLighting( false );

	DISPLAY->ExitPerspective();
}
