#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScreenSelectDifficulty.cpp

 Desc: Testing the Screen class.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


#include "ScreenSelectDifficulty.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "ScreenSelectGroup.h"
#include "ScreenTitleMenu.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "AnnouncerManager.h"


const float EXPLANATION_X	=	CENTER_X - 150;
const float EXPLANATION_Y	=	CENTER_Y - 170;

const float DIFFICULTY_CLASS_X[NUM_DIFFICULTY_CLASSES] = {
	CENTER_X-200,
	CENTER_X,
	CENTER_X+200
};
// these sprites are bottom aligned!
const float DIFFICULTY_CLASS_Y[NUM_DIFFICULTY_CLASSES] = {
	CENTER_Y-40,
	CENTER_Y-60,
	CENTER_Y-40
};

const float DIFFICULTY_ARROW_Y[NUM_DIFFICULTY_CLASSES] = {
	DIFFICULTY_CLASS_Y[0]+205,
	DIFFICULTY_CLASS_Y[1]+205,
	DIFFICULTY_CLASS_Y[2]+205
};
const float DIFFICULTY_ARROW_X[NUM_DIFFICULTY_CLASSES][NUM_PLAYERS] = {
	{ DIFFICULTY_CLASS_X[0]-40, DIFFICULTY_CLASS_X[0]+40 },
	{ DIFFICULTY_CLASS_X[1]-40, DIFFICULTY_CLASS_X[1]+40 },
	{ DIFFICULTY_CLASS_X[2]-40, DIFFICULTY_CLASS_X[2]+40 },
};

const float ARROW_SHADOW_OFFSET = 10;


const ScreenMessage SM_GoToPrevState		=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextState		=	ScreenMessage(SM_User + 2);
const ScreenMessage SM_StartTweeningOffScreen =	ScreenMessage(SM_User + 3);
const ScreenMessage SM_StartFadingOut		 =	ScreenMessage(SM_User + 4);


ScreenSelectDifficulty::ScreenSelectDifficulty()
{
	LOG->WriteLine( "ScreenSelectDifficulty::ScreenSelectDifficulty()" );

	
	m_Menu.Load(
		THEME->GetPathTo(GRAPHIC_SELECT_DIFFICULTY_BACKGROUND) , 
		THEME->GetPathTo(GRAPHIC_SELECT_DIFFICULTY_TOP_EDGE),
		ssprintf("Use %c %c to select, then press NEXT", char(1), char(2))
		);
	this->AddActor( &m_Menu );


	m_sprExplanation.Load( THEME->GetPathTo(GRAPHIC_SELECT_DIFFICULTY_EXPLANATION) );
	m_sprExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	this->AddActor( &m_sprExplanation );

	for( int d=0; d<NUM_DIFFICULTY_CLASSES; d++ )
	{
		ThemeElement te_header;
		ThemeElement te_picture;
		switch( d )
		{
		case 0:	te_header = GRAPHIC_SELECT_DIFFICULTY_EASY_HEADER;		te_picture = GRAPHIC_SELECT_DIFFICULTY_EASY_PICTURE;	break;
		case 1: te_header = GRAPHIC_SELECT_DIFFICULTY_MEDIUM_HEADER;	te_picture = GRAPHIC_SELECT_DIFFICULTY_MEDIUM_PICTURE;	break;
		case 2: te_header = GRAPHIC_SELECT_DIFFICULTY_HARD_HEADER;		te_picture = GRAPHIC_SELECT_DIFFICULTY_HARD_PICTURE;	break;
		}
		m_sprDifficultyPicture[d].Load( THEME->GetPathTo(te_picture) );
		m_sprDifficultyPicture[d].SetXY( DIFFICULTY_CLASS_X[d], DIFFICULTY_CLASS_Y[d] );
		m_sprDifficultyPicture[d].SetVertAlign( align_bottom );
		m_sprDifficultyPicture[d].TurnShadowOff();
		this->AddActor( &m_sprDifficultyPicture[d] );

		m_sprDifficultyHeader[d].Load( THEME->GetPathTo(te_header) );
		m_sprDifficultyHeader[d].SetXY( DIFFICULTY_CLASS_X[d], DIFFICULTY_CLASS_Y[d] );
		m_sprDifficultyHeader[d].SetVertAlign( align_top );
		m_sprDifficultyHeader[d].TurnShadowOff();
		this->AddActor( &m_sprDifficultyHeader[d] );

	}

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMEMAN->IsPlayerEnabled((PlayerNumber)p) )
			continue;

		ThemeElement te;
		switch( p )
		{
		case PLAYER_1:	te = GRAPHIC_SELECT_DIFFICULTY_ARROW_P1;	break;
		case PLAYER_2:	te = GRAPHIC_SELECT_DIFFICULTY_ARROW_P2;	break;
		default:	ASSERT( false );
		}
		m_sprArrowShadow[p].Load( THEME->GetPathTo(te) );
		m_sprArrowShadow[p].TurnShadowOff();
		m_sprArrowShadow[p].SetDiffuseColor( D3DXCOLOR(0,0,0,0.6f) );
		this->AddActor( &m_sprArrowShadow[p] );

		m_sprArrow[p].Load( THEME->GetPathTo(te) );
		m_sprArrow[p].TurnShadowOff();
		m_sprArrow[p].SetDiffuseColor( PlayerToColor((PlayerNumber)p) );
		m_sprArrow[p].SetEffectGlowing();
		this->AddActor( &m_sprArrow[p] );

		m_sprOK[p].Load( THEME->GetPathTo(GRAPHIC_SELECT_DIFFICULTY_OK) );
		this->AddActor( &m_sprOK[p] );

		m_iSelection[p] = 0;
		m_bChosen[p] = false;
	}
	

	m_Fade.SetZ( -2 );
	this->AddActor( &m_Fade );

	m_soundChange.Load( THEME->GetPathTo(SOUND_SELECT_DIFFICULTY_CHANGE) );
	m_soundSelect.Load( THEME->GetPathTo(SOUND_SELECT) );
	
	
	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_DIFFICULTY_INTRO) );


	if( !MUSIC->IsPlaying() )
	{
		MUSIC->Load( THEME->GetPathTo(SOUND_MENU_MUSIC) );
        MUSIC->Play( true );
	}

	m_Fade.OpenWipingRight();
	TweenOnScreen();
}


ScreenSelectDifficulty::~ScreenSelectDifficulty()
{
	LOG->WriteLine( "ScreenSelectDifficulty::~ScreenSelectDifficulty()" );

}


void ScreenSelectDifficulty::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->WriteLine( "ScreenSelectDifficulty::Input()" );

	if( m_Fade.IsClosing() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

void ScreenSelectDifficulty::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenSelectDifficulty::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_MenuTimer:
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
				MenuStart( (PlayerNumber)p );
		}
		break;
	case SM_GoToPrevState:
		SCREENMAN->SetNewScreen( new ScreenTitleMenu );
		break;
	case SM_GoToNextState:
		SCREENMAN->SetNewScreen( new ScreenSelectGroup );
		break;
	case SM_StartTweeningOffScreen:
		TweenOffScreen();
		this->SendScreenMessage( SM_StartFadingOut, 0.8f );
		break;
	case SM_StartFadingOut:
		m_Fade.CloseWipingRight( SM_GoToNextState );
		break;
	}
}

void ScreenSelectDifficulty::MenuLeft( PlayerNumber p )
{
	if( m_iSelection[p] == 0 )	// can't go left any more
		return;
	if( m_bChosen[p] )
		return;

	m_iSelection[p]--;

	m_sprArrow[p].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
	m_sprArrow[p].SetTweenX( DIFFICULTY_ARROW_X[m_iSelection[p]][p] - ARROW_SHADOW_OFFSET );
	m_sprArrow[p].SetTweenY( DIFFICULTY_ARROW_Y[m_iSelection[p]] - ARROW_SHADOW_OFFSET );

	m_sprArrowShadow[p].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
	m_sprArrowShadow[p].SetTweenX( DIFFICULTY_ARROW_X[m_iSelection[p]][p] );
	m_sprArrowShadow[p].SetTweenY( DIFFICULTY_ARROW_Y[m_iSelection[p]] );

	m_soundChange.PlayRandom();
}


void ScreenSelectDifficulty::MenuRight( PlayerNumber p )
{
	if( m_iSelection[p] == 2 )	// can't go right any more
		return;
	if( m_bChosen[p] )
		return;

	m_iSelection[p]++;
	
	m_sprArrow[p].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
	m_sprArrow[p].SetTweenX( DIFFICULTY_ARROW_X[m_iSelection[p]][p] - ARROW_SHADOW_OFFSET );
	m_sprArrow[p].SetTweenY( DIFFICULTY_ARROW_Y[m_iSelection[p]] - ARROW_SHADOW_OFFSET );

	m_sprArrowShadow[p].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
	m_sprArrowShadow[p].SetTweenX( DIFFICULTY_ARROW_X[m_iSelection[p]][p] );
	m_sprArrowShadow[p].SetTweenY( DIFFICULTY_ARROW_Y[m_iSelection[p]] );

	m_soundChange.PlayRandom();
}

void ScreenSelectDifficulty::MenuStart( PlayerNumber pn )
{
	if( m_bChosen[pn] == true )
		return;
	m_bChosen[pn] = true;

	m_soundSelect.PlayRandom();
	int iSelection = m_iSelection[pn];

	switch( iSelection )
	{
	case 0:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_DIFFICULTY_COMMENT_EASY) );	break;
	case 1:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_DIFFICULTY_COMMENT_MEDIUM) );	break;
	case 2:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_DIFFICULTY_COMMENT_HARD) );	break;
	}

	m_sprArrow[pn].BeginTweeningQueued( 0.2f );
	m_sprArrow[pn].BeginTweeningQueued( 0.2f );
	m_sprArrow[pn].SetTweenX( DIFFICULTY_ARROW_X[iSelection][pn] );
	m_sprArrow[pn].SetTweenY( DIFFICULTY_ARROW_Y[iSelection] );

	m_sprOK[pn].SetX( DIFFICULTY_ARROW_X[iSelection][pn] );
	m_sprOK[pn].SetY( DIFFICULTY_ARROW_Y[iSelection] );
	m_sprOK[pn].SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	m_sprOK[pn].SetZoom( 2 );

	m_sprOK[pn].BeginTweening( 0.2f );
	m_sprOK[pn].SetTweenZoom( 1 );
	m_sprOK[pn].SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );

	m_sprArrowShadow[pn].BeginTweeningQueued( 0.2f );
	m_sprArrowShadow[pn].BeginTweeningQueued( 0.2f );
	m_sprArrowShadow[pn].SetDiffuseColor( D3DXCOLOR(0,0,0,0) );


	// check to see if everyone has chosen
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( m_bChosen[p] == false )
			return;
	}
	this->SendScreenMessage( SM_StartTweeningOffScreen, 0.7f );
}

void ScreenSelectDifficulty::MenuBack( PlayerNumber p )
{
	m_Fade.CloseWipingLeft( SM_GoToPrevState );

	TweenOffScreen();
}

void ScreenSelectDifficulty::TweenOffScreen()
{
	m_Menu.TweenTopEdgeOffScreen();

	m_sprExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	m_sprExplanation.BeginTweening( 0.5, Actor::TWEEN_BOUNCE_BEGIN );
	m_sprExplanation.SetTweenXY( EXPLANATION_X-400, EXPLANATION_Y );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMEMAN->IsPlayerEnabled((PlayerNumber)p) )
			continue;

		m_sprArrow[p].BeginTweening( 0.3f );
		m_sprArrow[p].SetTweenZoom( 0 );

		m_sprOK[p].BeginTweening( 0.3f );
		m_sprOK[p].SetTweenZoom( 0 );

		m_sprArrowShadow[p].BeginTweeningQueued( 0.3f );
		m_sprArrowShadow[p].SetTweenDiffuseColor( D3DXCOLOR(0,0,0,0) );
	}

	for( int d=0; d<NUM_DIFFICULTY_CLASSES; d++ )
	{
		const float fPauseTime = d*0.2f;

		// pause
		m_sprDifficultyHeader[d].BeginTweeningQueued( fPauseTime );

		m_sprDifficultyPicture[d].BeginTweeningQueued( fPauseTime );

		// roll up
		m_sprDifficultyHeader[d].BeginTweeningQueued( 0.3f, TWEEN_BOUNCE_BEGIN );

		m_sprDifficultyPicture[d].BeginTweeningQueued( 0.3f, TWEEN_BOUNCE_BEGIN );
		m_sprDifficultyPicture[d].SetTweenZoomY( 0 );

		// fly off
		m_sprDifficultyHeader[d].BeginTweeningQueued( 0.4f, TWEEN_BIAS_END );
		m_sprDifficultyHeader[d].SetTweenXY( DIFFICULTY_CLASS_X[d]-700, DIFFICULTY_CLASS_Y[d] );

		m_sprDifficultyPicture[d].BeginTweeningQueued( 0.4f, TWEEN_BIAS_END );
		m_sprDifficultyPicture[d].SetTweenXY( DIFFICULTY_CLASS_X[d]-700, DIFFICULTY_CLASS_Y[d] );
	}
}

void ScreenSelectDifficulty::TweenOnScreen() 
{
	m_Menu.TweenTopEdgeOnScreen();

	m_sprExplanation.SetXY( EXPLANATION_X-400, EXPLANATION_Y );
	m_sprExplanation.BeginTweening( 0.3f, Actor::TWEEN_BOUNCE_END );
	m_sprExplanation.SetTweenXY( EXPLANATION_X, EXPLANATION_Y );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMEMAN->IsPlayerEnabled((PlayerNumber)p) )
			continue;

		int iSelection = m_iSelection[p];

		m_sprArrow[p].SetX( DIFFICULTY_ARROW_X[iSelection][p] );
		m_sprArrow[p].SetY( DIFFICULTY_ARROW_Y[iSelection] );
		m_sprArrow[p].SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
		m_sprArrow[p].SetRotation( D3DX_PI );
		m_sprArrow[p].SetZoom( 2 );
		m_sprArrow[p].BeginTweening( 0.3f );
		m_sprArrow[p].SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );
		m_sprArrow[p].SetTweenRotationZ( 0 );
		m_sprArrow[p].SetTweenZoom( 1 );

		m_sprArrowShadow[p].SetX( DIFFICULTY_ARROW_X[iSelection][p] );
		m_sprArrowShadow[p].SetY( DIFFICULTY_ARROW_Y[iSelection] );
		D3DXCOLOR colorOriginal = m_sprArrowShadow[p].GetDiffuseColor();
		m_sprArrowShadow[p].SetDiffuseColor( D3DXCOLOR(0,0,0,0) );
		m_sprArrowShadow[p].BeginTweening( 0.3f );
		m_sprArrowShadow[p].SetTweenDiffuseColor( colorOriginal );
	}

	for( int d=0; d<NUM_DIFFICULTY_CLASSES; d++ )
	{
		const float fPauseTime = d*0.2f;

		// set off screen
		m_sprDifficultyHeader[d].SetXY( DIFFICULTY_CLASS_X[d]-700, DIFFICULTY_CLASS_Y[d] );
	
		m_sprDifficultyPicture[d].SetXY( DIFFICULTY_CLASS_X[d]-700, DIFFICULTY_CLASS_Y[d] );
		m_sprDifficultyPicture[d].SetZoomY( 0 );

		// pause
		m_sprDifficultyHeader[d].BeginTweeningQueued( fPauseTime );

		m_sprDifficultyPicture[d].BeginTweeningQueued( fPauseTime );

		// fly on
		m_sprDifficultyHeader[d].BeginTweeningQueued( 0.5f, TWEEN_BIAS_BEGIN );
		m_sprDifficultyHeader[d].SetTweenXY( DIFFICULTY_CLASS_X[d], DIFFICULTY_CLASS_Y[d] );

		m_sprDifficultyPicture[d].BeginTweeningQueued( 0.5f, TWEEN_BIAS_BEGIN );
		m_sprDifficultyPicture[d].SetTweenXY( DIFFICULTY_CLASS_X[d], DIFFICULTY_CLASS_Y[d] );

		// roll down
		m_sprDifficultyHeader[d].BeginTweeningQueued( 0.3f, TWEEN_BOUNCE_END );

		m_sprDifficultyPicture[d].BeginTweeningQueued( 0.3f, TWEEN_BOUNCE_END );
		m_sprDifficultyPicture[d].SetTweenZoomY( 1 );

	}
}
