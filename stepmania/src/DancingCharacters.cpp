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
#include "PrefsManager.h"
#include "GameState.h"
#include "song.h"



const float CAMERA_REST_DISTANCE = 32.f;
const float CAMERA_SWEEP_DISTANCE = 26.f;
const float CAMERA_SWEEP_HEIGHT_VARIANCE = 28.f;
const float CAMERA_SWEEP_PAN_Y_RANGE_DEGREES = 80.f;
const float CAMERA_SWEEP_PAN_Y_VARIANCE_DEGREES = 20.f;
const float CAMERA_STILL_DISTANCE = 20.f;
const float CAMERA_STILL_PAN_Y_RANGE_DEGREES = 45.f;
const float CAMERA_STILL_HEIGHT_VARIANCE = 6.f;
const float LOOK_AT_HEIGHT = -9.f;

const float MODEL_X[NUM_PLAYERS] = { -8, 8 };


DancingCharacters::DancingCharacters()
{
	if( PREFSMAN->m_bShowDancingCharacters )
	{
		m_Character[PLAYER_1].SetX( MODEL_X[PLAYER_1] );
		m_Character[PLAYER_1].LoadMilkshapeAscii( "C:\\My Documents\\Dev\\ddrpc char hacking\\DDRPCRip\\models\\char0000\\model.txt" );
		m_Character[PLAYER_1].LoadMilkshapeAsciiBones( "rest", "C:\\My Documents\\Dev\\ddrpc char hacking\\DDRPCRip\\models\\rest.bones.txt" );
		m_Character[PLAYER_1].LoadMilkshapeAsciiBones( "warmup", "C:\\My Documents\\Dev\\ddrpc char hacking\\DDRPCRip\\models\\warmup.bones.txt" );
		m_Character[PLAYER_1].LoadMilkshapeAsciiBones( "dance", "C:\\My Documents\\Dev\\ddrpc char hacking\\DDRPCRip\\models\\dance0001.bones.txt" );
		m_Character[PLAYER_1].PlayAnimation( "rest" );
		this->AddChild( &m_Character[PLAYER_1] );

		m_Character[PLAYER_2].SetX( MODEL_X[PLAYER_2] );
		m_Character[PLAYER_2].LoadMilkshapeAscii( "C:\\My Documents\\Dev\\ddrpc char hacking\\DDRPCRip\\models\\char0011\\model.txt" );
		m_Character[PLAYER_2].LoadMilkshapeAsciiBones( "rest", "C:\\My Documents\\Dev\\ddrpc char hacking\\DDRPCRip\\models\\rest.bones.txt" );
		m_Character[PLAYER_2].LoadMilkshapeAsciiBones( "warmup", "C:\\My Documents\\Dev\\ddrpc char hacking\\DDRPCRip\\models\\warmup.bones.txt" );
		m_Character[PLAYER_2].LoadMilkshapeAsciiBones( "dance", "C:\\My Documents\\Dev\\ddrpc char hacking\\DDRPCRip\\models\\dance0001.bones.txt" );
		m_Character[PLAYER_2].PlayAnimation( "rest" );
		this->AddChild( &m_Character[PLAYER_2] );
	}

	// initial camera sweep is still
	m_CameraDistance = CAMERA_REST_DISTANCE;
	m_CameraPanYStart = 0;
	m_CameraPanYEnd = 0;
	m_fCameraHeightStart = LOOK_AT_HEIGHT;
	m_fCameraHeightEnd = LOOK_AT_HEIGHT;
	m_fThisCameraStartBeat = 0;
	m_fThisCameraEndBeat = GAMESTATE->m_pCurSong->m_fFirstBeat;

}

int Neg1OrPos1() { return rand()%2 ? -1 : +1; }

void DancingCharacters::Update( float fDelta )
{
	if( !GAMESTATE->m_bFreeze )
		ActorFrame::Update( fDelta );


	static bool bWasHereWeGo = false;
	bool bIsHereWeGo = GAMESTATE->m_bPastHereWeGo;
	if( !bWasHereWeGo && bIsHereWeGo )
	{
		m_Character[PLAYER_1].PlayAnimation( "warmup" );
		m_Character[PLAYER_2].PlayAnimation( "warmup" );
	}
	bWasHereWeGo = bIsHereWeGo;

	static float fLastBeat = GAMESTATE->m_fSongBeat;
	float fThisBeat = GAMESTATE->m_fSongBeat;
	if( fLastBeat < GAMESTATE->m_pCurSong->m_fFirstBeat &&
		fThisBeat >= GAMESTATE->m_pCurSong->m_fFirstBeat )
	{
		m_Character[PLAYER_1].PlayAnimation( "dance" );
		m_Character[PLAYER_2].PlayAnimation( "dance" );
	}
	fLastBeat = fThisBeat;


	// time for a new sweep?
	if( GAMESTATE->m_fSongBeat > m_fThisCameraEndBeat )
	{
		if( rand()%2 )
		{
			m_CameraDistance = CAMERA_SWEEP_DISTANCE;
			m_CameraPanYStart = m_CameraPanYEnd = RandomInt(-1,1) * CAMERA_SWEEP_PAN_Y_RANGE_DEGREES;
			m_fCameraHeightStart = m_fCameraHeightEnd = LOOK_AT_HEIGHT;
			
			m_CameraPanYEnd += RandomInt(-1,1) * CAMERA_SWEEP_PAN_Y_VARIANCE_DEGREES;
			m_fCameraHeightStart = m_fCameraHeightEnd = m_fCameraHeightStart + RandomInt(-1,1) * CAMERA_SWEEP_HEIGHT_VARIANCE;

			float fCameraHeightVariance = RandomInt(-1,1) * CAMERA_SWEEP_HEIGHT_VARIANCE;
			m_fCameraHeightStart -= fCameraHeightVariance;
			m_fCameraHeightEnd += fCameraHeightVariance;
		}
		else
		{
			m_CameraDistance = CAMERA_STILL_DISTANCE;
			m_CameraPanYStart = m_CameraPanYEnd = Neg1OrPos1() * CAMERA_STILL_PAN_Y_RANGE_DEGREES;
			m_fCameraHeightStart = m_fCameraHeightEnd = LOOK_AT_HEIGHT + Neg1OrPos1() * CAMERA_STILL_HEIGHT_VARIANCE;
		}

		int iCurBeat = (int)GAMESTATE->m_fSongBeat;
		iCurBeat -= iCurBeat%8;

		m_fThisCameraStartBeat = iCurBeat;
		m_fThisCameraEndBeat = iCurBeat + 8;
	}
}

void DancingCharacters::DrawPrimitives()
{
	DISPLAY->EnterPerspective( 45, false );

	float fPercentIntoSweep = SCALE(GAMESTATE->m_fSongBeat, m_fThisCameraStartBeat, m_fThisCameraEndBeat, 0.f, 1.f );
	float fCameraPanY = SCALE( fPercentIntoSweep, 0.f, 1.f, m_CameraPanYStart, m_CameraPanYEnd );
	float fCameraHeight = SCALE( fPercentIntoSweep, 0.f, 1.f, m_fCameraHeightStart, m_fCameraHeightEnd );

	RageVector3 m_CameraPoint( 0, fCameraHeight, -m_CameraDistance );
	RageVec3TransformCoord( &m_CameraPoint, &m_CameraPoint, &RageMatrixRotationY(fCameraPanY) );

	RageVector3 m_LookAt( 0, LOOK_AT_HEIGHT, 0 );

	DISPLAY->LookAt( 
		m_CameraPoint,
		m_LookAt,
		RageVector3(0,1,0) );

	DISPLAY->SetLighting( true );
	DISPLAY->SetLightDirectional( 
		0, 
		RageColor(0.4f,0.4f,0.4f,1), 
		RageColor(0.8f,0.8f,0.8f,0.8f),
		RageColor(0.8f,0.8f,0.8f,0.8f),
		RageVector3(+1, 0, +1) );

	ActorFrame::DrawPrimitives();

	DISPLAY->SetLightOff( 0 );
	DISPLAY->SetLighting( false );

	DISPLAY->ExitPerspective();
}
