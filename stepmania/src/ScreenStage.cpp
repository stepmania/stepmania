#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenStage

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenStage.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "BitmapText.h"
#include "TransitionFadeWipe.h"
#include "TransitionFade.h"
#include "ScreenSelectMusic.h"
#include "ScreenGameplay.h"
#include "SongManager.h"
#include "Sprite.h"
#include "AnnouncerManager.h"
#include "GameState.h"


const ScreenMessage SM_StartFadingOut	=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_DoneFadingIn		=	ScreenMessage(SM_User + 2);
const ScreenMessage SM_GoToNextState	=	ScreenMessage(SM_User + 3);

enum StageType
{
	TYPE_NORMAL,
	TYPE_FINAL,
	TYPE_EXTRA1,
	TYPE_EXTRA2,
	TYPE_ONI,
	TYPE_ENDLESS
};

ScreenStage::ScreenStage()
{
	MUSIC->Stop();

	m_pNextScreen = NULL;


	for( int i=0; i<4; i++ )
	{
		m_sprNumbers[i].Load( THEME->GetPathTo("Graphics","stage numbers") );
		m_sprNumbers[i].StopAnimating();
	}
	m_sprStage.Load( THEME->GetPathTo("Graphics","stage stage") );	// we may load a different graphic into here later
	const float fStageHeight = m_sprStage.GetUnzoomedHeight();

	const float fStageOffScreenY = CENTER_Y+fStageHeight;

	m_quadMask.SetDiffuseColor( D3DXCOLOR(0,0,0,0) );
	m_quadMask.StretchTo( CRect(SCREEN_LEFT, roundf(fStageOffScreenY-fStageHeight/2), SCREEN_RIGHT, roundf(fStageOffScreenY+fStageHeight/2)) );
	m_quadMask.SetZ( -1 );		// important: fill Z buffer with values that will cause subsequent draws to fail the Z test

	m_frameStage.SetXY( CENTER_X, fStageOffScreenY );
	m_frameStage.BeginTweening(0.8f, Actor::TWEEN_BIAS_BEGIN );
	m_frameStage.SetTweenY( CENTER_Y );

	
	StageType		stage_type;
	if( GAMESTATE->m_PlayMode == PLAY_MODE_ONI )			stage_type = TYPE_ONI;
	else if( GAMESTATE->m_PlayMode == PLAY_MODE_ENDLESS )	stage_type = TYPE_ENDLESS;
	else if( GAMESTATE->IsExtraStage() )					stage_type = TYPE_EXTRA1;
	else if( GAMESTATE->IsExtraStage2() )					stage_type = TYPE_EXTRA2;
	else if( GAMESTATE->IsFinalStage() )					stage_type = TYPE_FINAL;
	else													stage_type = TYPE_NORMAL;


	switch( stage_type )
	{
	case TYPE_NORMAL:
		{
			const int iStageNo = GAMESTATE->GetStageIndex()+1;
			switch( iStageNo )
			{
			case 1:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_1) );	break;
			case 2:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_2) );	break;
			case 3:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_3) );	break;
			case 4:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_4) );	break;
			case 5:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_5) );	break;
			default:	;	break;	// play nothing
			}
		}
		break;
	case TYPE_FINAL:
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_FINAL) );
		break;
	case TYPE_EXTRA1:
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_EXTRA1) );
		break;
	case TYPE_EXTRA2:
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_EXTRA2) );
		break;
	case TYPE_ONI:
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_ONI) );
		break;
	case TYPE_ENDLESS:
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_ENDLESS) );
		break;
	default:
		ASSERT(0);
	}

	switch( stage_type )
	{
	case TYPE_NORMAL:
		{
			const int iStageNo = GAMESTATE->GetStageIndex()+1;
			CString sStageNo = ssprintf("%d", iStageNo);

			// Set frame of numbers
			int i;
			for( i=0; i<sStageNo.GetLength(); i++ )
				m_sprNumbers[i].SetState( atoi(CString(sStageNo[i])) );

			// Set frame of suffix
			int iIndexOfSuffix = sStageNo.GetLength();
			if( ( (iStageNo/10) % 10 ) == 1 )	// in the teens (e.g. 19, 213)
				m_sprNumbers[iIndexOfSuffix].SetState( 13 );	// th
			else	// not in the teens
			{
				const int iLastDigit = iStageNo%10;
				switch( iLastDigit )
				{
				case 1:		m_sprNumbers[iIndexOfSuffix].SetState( 10 );	break;	// st
				case 2:		m_sprNumbers[iIndexOfSuffix].SetState( 11 );	break;	// nd
				case 3:		m_sprNumbers[iIndexOfSuffix].SetState( 12 );	break;	// rd
				default:	m_sprNumbers[iIndexOfSuffix].SetState( 13 );	break;	// th
				}
			}
			
			// Set X positions
			const float fFrameWidth = m_sprNumbers[0].GetUnzoomedWidth();
			const int iNumChars = iIndexOfSuffix+1;
			const float fTotalCharsWidth = m_sprNumbers[0].GetUnzoomedWidth();
			const float fSpaceBetweenNumsAndStage = fFrameWidth;
			const float fStageWidth = m_sprStage.GetUnzoomedWidth();
			const float fTotalWidth = fTotalCharsWidth + fSpaceBetweenNumsAndStage + fStageWidth;
			const float fCharsCenterX = -fTotalWidth/2.0f + fTotalCharsWidth/2.0f;
			const float fStageCenterX = +fTotalWidth/2.0f - fStageWidth/2.0f;

			for( i=0; i<iNumChars; i++ )
			{
				float fOffsetX = SCALE(i, 0, iNumChars-1, -(iNumChars-1)/2.0f*fFrameWidth, (iNumChars-1)/2.0f*fFrameWidth);
				m_sprNumbers[i].SetX( fCharsCenterX + fOffsetX );
			}
			m_sprStage.SetX( fStageCenterX );

			for( i=0; i<iNumChars; i++ )
				m_frameStage.AddSubActor( &m_sprNumbers[i] );
			m_frameStage.AddSubActor( &m_sprStage );
		}
		break;
	case TYPE_FINAL:
		m_sprStage.Load( THEME->GetPathTo("Graphics","stage final") );
		m_frameStage.AddSubActor( &m_sprStage );
		break;
	case TYPE_EXTRA1:
		m_sprStage.Load( THEME->GetPathTo("Graphics","stage extra1") );
		m_frameStage.AddSubActor( &m_sprStage );
		break;
	case TYPE_EXTRA2:
		m_sprStage.Load( THEME->GetPathTo("Graphics","stage extra2") );
		m_frameStage.AddSubActor( &m_sprStage );
		break;
	case TYPE_ONI:
		m_sprStage.Load( THEME->GetPathTo("Graphics","stage oni") );
		m_frameStage.AddSubActor( &m_sprStage );
		break;
	case TYPE_ENDLESS:
		m_sprStage.Load( THEME->GetPathTo("Graphics","stage endless") );
		m_frameStage.AddSubActor( &m_sprStage );
		break;
	default:
		ASSERT(0);
	}

	m_Fade.SetOpened();

	this->SendScreenMessage( SM_DoneFadingIn, 1.0f );
	this->SendScreenMessage( SM_StartFadingOut, 1.2f );
}

void ScreenStage::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );

	m_quadMask.Update( fDeltaTime );
	m_frameStage.Update( fDeltaTime );
	m_Fade.Update( fDeltaTime );
}

void ScreenStage::DrawPrimitives()
{
	DISPLAY->EnableZBuffer();
	m_quadMask.Draw();
	m_frameStage.Draw();
	DISPLAY->DisableZBuffer();
	m_Fade.Draw();
}

void ScreenStage::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_StartFadingOut:
		m_Fade.CloseWipingRight( SM_GoToNextState );
		break;
	case SM_DoneFadingIn:
		m_pNextScreen = new ScreenGameplay;
		break;
	case SM_GoToNextState:
		SCREENMAN->SetNewScreen( m_pNextScreen );
		break;
	}
}
