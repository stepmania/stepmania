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


const float CAMERA_SWEEP_SECONDS = 2.f;
const float CAMERA_SWEEP_SECONDS_VARIANCE = 1.f;
const float CAMERA_DISTANCE = 16.f;
const float CAMERA_DISTANCE_VARIANCE = 3.f;
const float CAMERA_HEIGHT = -9.f;
const float CAMERA_HEIGHT_VARIANCE = 3.f;

const float MODEL_X[NUM_PLAYERS] = { -8, 8 };


DancingCharacters::DancingCharacters()
{
	if( PREFSMAN->m_bShowDancingCharacters )
	{
		m_Character[PLAYER_1].SetX( MODEL_X[PLAYER_1] );
		m_Character[PLAYER_1].LoadMilkshapeAscii( "D:\\Dev\\ddrpc char hacking\\DDRPCRip\\models\\char0000\\model.txt" );
		m_Character[PLAYER_1].LoadMilkshapeAsciiBones( "D:\\Dev\\ddrpc char hacking\\DDRPCRip\\models\\dance0001.bones.txt" );
		this->AddChild( &m_Character[PLAYER_1] );

		m_Character[PLAYER_2].SetX( MODEL_X[PLAYER_2] );
		m_Character[PLAYER_2].LoadMilkshapeAscii( "D:\\Dev\\ddrpc char hacking\\DDRPCRip\\models\\char0011\\model.txt" );
		m_Character[PLAYER_2].LoadMilkshapeAsciiBones( "D:\\Dev\\ddrpc char hacking\\DDRPCRip\\models\\dance0002.bones.txt" );
		this->AddChild( &m_Character[PLAYER_2] );

		StartCameraSweep();
	}
}

void DancingCharacters::Update( float fDelta )
{
	ActorFrame::Update( fDelta );

	// update camera sweep
	m_fSecsIntoSweep += fDelta;
	if( m_fSecsIntoSweep > m_fSweepSecs )
		StartCameraSweep();

	float fPercentIntoSweep = m_fSecsIntoSweep / m_fSweepSecs;
	m_CameraEyeCurrent = m_CameraEyeStart * (1-fPercentIntoSweep) + m_CameraEyeEnd * fPercentIntoSweep;
}

void DancingCharacters::DrawPrimitives()
{
	DISPLAY->EnterPerspective( 90, false );
	DISPLAY->LookAt( 
		m_CameraEyeCurrent,
		m_CameraAt,
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

void DancingCharacters::StartCameraSweep()
{
	m_CameraEyeStart = RageVector3( randomf(), randomf(), randomf() );
	RageVec3Normalize( &m_CameraEyeStart, &m_CameraEyeStart );
	m_CameraEyeStart *= CAMERA_DISTANCE + randomf()*CAMERA_DISTANCE_VARIANCE;
	m_CameraEyeStart.y += CAMERA_HEIGHT + randomf()*CAMERA_HEIGHT_VARIANCE;

	m_CameraEyeEnd = RageVector3( randomf(), randomf(), randomf() );
	m_CameraEyeEnd += m_CameraEyeStart/2;	// bias the end toward the start to that we don't sweep through the chars' bodies
	RageVec3Normalize( &m_CameraEyeEnd, &m_CameraEyeEnd );
	m_CameraEyeEnd *= CAMERA_DISTANCE + randomf()*CAMERA_DISTANCE_VARIANCE;
	m_CameraEyeEnd.y += CAMERA_HEIGHT + randomf()*CAMERA_HEIGHT_VARIANCE;

	m_CameraAt = RageVector3(0,CAMERA_HEIGHT + randomf()*CAMERA_HEIGHT_VARIANCE,0);

	m_fSweepSecs = CAMERA_SWEEP_SECONDS + randomf()*CAMERA_SWEEP_SECONDS_VARIANCE;
	
	m_fSecsIntoSweep = 0;
}